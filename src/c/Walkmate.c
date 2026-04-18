#include "gcolor_definitions.h"
#include <inttypes.h>
#include <pebble.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static Window *    s_window;
static TextLayer * s_date_layer;
static TextLayer * s_time_layer;
static Layer *     s_progress_layer;
static GFont       s_date_font;
static GFont       s_time_font;
static GFont       s_steps_font;

enum {
	APP_KEY_STEP_GOAL      = 10000,
	PERSIST_KEY_STEP_GOAL  = 1,
	DEFAULT_STEP_GOAL      = 10000,
	MIN_STEP_GOAL          = 1000,
	MAX_STEP_GOAL          = 99999,
	MAX_STEP_DISPLAY       = 99999,
	ARROWHEAD_ANGLE_OFFSET = 12,
};

static uint32_t      s_step_goal                          = DEFAULT_STEP_GOAL;
static const int16_t s_progress_ring_outer_padding        = 1;
static const uint8_t s_progress_ring_width                = 16;
static const int16_t s_progress_arrow_base_extra          = 1;
static const uint8_t s_progress_arrow_overflow_line_width = 2;

static const char month[12][4] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

static void prv_mark_progress_dirty(void);

static inline char prv_num_to_digit(uint32_t n)
{
	return '0' + n;
}

static bool prv_is_valid_step_goal(const uint32_t step_goal)
{
	return step_goal >= MIN_STEP_GOAL && step_goal <= MAX_STEP_GOAL;
}

static void prv_set_step_goal(const uint32_t step_goal)
{
	if (!prv_is_valid_step_goal(step_goal)) {
		return;
	}

	s_step_goal = step_goal;
	persist_write_int(PERSIST_KEY_STEP_GOAL, s_step_goal);
	prv_mark_progress_dirty();
}

static void prv_load_step_goal(void)
{
	if (persist_exists(PERSIST_KEY_STEP_GOAL)) {
		const uint32_t persisted_step_goal = persist_read_int(PERSIST_KEY_STEP_GOAL);
		if (prv_is_valid_step_goal(persisted_step_goal)) {
			s_step_goal = persisted_step_goal;
			return;
		}
	}

	s_step_goal = DEFAULT_STEP_GOAL;
}

static uint32_t prv_get_today_steps(void)
{
	const time_t start = time_start_of_today();
	const time_t end   = time(NULL);

	const HealthServiceAccessibilityMask mask = health_service_metric_accessible(HealthMetricStepCount, start, end);
	if ((mask & HealthServiceAccessibilityMaskAvailable) == 0) {
		return 0;
	}

	return (uint32_t) health_service_sum_today(HealthMetricStepCount);
}

static void prv_mark_progress_dirty(void)
{
	if (s_progress_layer != NULL) {
		layer_mark_dirty(s_progress_layer);
	}
}

static void prv_inbox_received_handler(DictionaryIterator * const iter, void * const context)
{
	(void) context;

	const Tuple * const step_goal_tuple = dict_find(iter, APP_KEY_STEP_GOAL);
	if (step_goal_tuple == NULL || step_goal_tuple->type != TUPLE_INT) {
		return;
	}

	prv_set_step_goal(step_goal_tuple->value->int32);
}

static void prv_fill_ring_segment(GContext * const ctx, const GRect rect, const GColor color, const int32_t start_angle, const int32_t end_angle)
{
	graphics_context_set_fill_color(ctx, color);
	graphics_fill_radial(ctx, rect, GOvalScaleModeFitCircle, s_progress_ring_width, start_angle, end_angle);
}

static GPoint prv_point_on_circle(const GRect rect, const int16_t radius, const int32_t angle)
{
	const GPoint center = grect_center_point(&rect);

	return GPoint(center.x + (int16_t) ((sin_lookup(angle) * radius) / TRIG_MAX_RATIO),
	              center.y + (int16_t) ((-cos_lookup(angle) * radius) / TRIG_MAX_RATIO));
}

static void prv_get_ring_arrowhead_points(const GRect rect, const int32_t angle, GPoint * const points)
{
	const int16_t base_radius  = rect.size.w / 2;
	const int16_t outer_radius = base_radius + s_progress_arrow_base_extra;
	const int16_t inner_radius = base_radius - s_progress_ring_width - s_progress_arrow_base_extra;
	const int16_t mid_radius   = base_radius - (s_progress_ring_width / 2);

	points[0] = prv_point_on_circle(rect, inner_radius, angle);
	points[1] = prv_point_on_circle(rect, mid_radius, angle + DEG_TO_TRIGANGLE(ARROWHEAD_ANGLE_OFFSET));
	points[2] = prv_point_on_circle(rect, outer_radius, angle);
}

static void prv_fill_ring_arrowhead(GContext * const ctx, const GRect rect, const GColor color, const int32_t angle)
{
	GPoint points[3];

	prv_get_ring_arrowhead_points(rect, angle, points);

	const GPathInfo arrowhead = {
		.num_points = 3,
		.points     = points,
	};
	GPath * const path = gpath_create(&arrowhead);

	if (path == NULL) {
		return;
	}

	graphics_context_set_fill_color(ctx, color);
	gpath_draw_filled(ctx, path);
	gpath_destroy(path);
}

static void prv_draw_ring_arrowhead(GContext * const ctx, const GRect rect, const int32_t angle)
{
	GPoint points[3];

	prv_get_ring_arrowhead_points(rect, angle, points);

	const GPathInfo arrowhead = {
		.num_points = 3,
		.points     = points,
	};
	GPath * const path = gpath_create(&arrowhead);

	if (path == NULL) {
		return;
	}

	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_stroke_width(ctx, s_progress_arrow_overflow_line_width);
	gpath_draw_outline_open(ctx, path);
	gpath_destroy(path);
}

static void prv_progress_update_proc(Layer * const layer, GContext * const ctx)
{
	static char steps_text[] = "99999";

	const GRect bounds = layer_get_bounds(layer);
	uint32_t    steps  = prv_get_today_steps();

	const int32_t raw_angle = (int32_t)(((uint64_t)TRIG_MAX_ANGLE * steps) / s_step_goal);
	int32_t       angle     = raw_angle;
	int32_t       overflow_angle;

	if (angle > TRIG_MAX_ANGLE) {
		angle = TRIG_MAX_ANGLE;
	}

	overflow_angle = raw_angle % TRIG_MAX_ANGLE;

	const int16_t diameter   = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
	const int16_t ring_inset = s_progress_ring_outer_padding;
	const GRect   ring_rect  = GRect((bounds.size.w - diameter) / 2 + ring_inset,
	                                 (bounds.size.h - diameter) / 2 + ring_inset,
	                                 diameter - ring_inset * 2,
	                                 diameter - ring_inset * 2);

	if (angle > 0) {
		prv_fill_ring_segment(ctx, ring_rect, GColorWhite, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(0) + angle);
		prv_fill_ring_arrowhead(ctx, ring_rect, GColorWhite, DEG_TO_TRIGANGLE(-2) + angle);
		prv_draw_ring_arrowhead(ctx, ring_rect, DEG_TO_TRIGANGLE(1) + overflow_angle);

		const uint32_t display_steps = (steps < MAX_STEP_DISPLAY) ? steps : MAX_STEP_DISPLAY;
		snprintf(steps_text, sizeof(steps_text), "%" PRIu32, display_steps);
		graphics_context_set_text_color(ctx, GColorWhite);
		graphics_draw_text(ctx,
		                   steps_text,
		                   s_steps_font,
		                   GRect(0, bounds.size.h / 2 - 12, bounds.size.w, 28),
		                   GTextOverflowModeTrailingEllipsis,
		                   GTextAlignmentCenter,
		                   NULL);
	}
}

static void prv_health_handler(HealthEventType event, void * context)
{
	(void) context;

	if (event == HealthEventMovementUpdate || event == HealthEventSignificantUpdate) {
		prv_mark_progress_dirty();
	}
}

static void prv_tick_handler(struct tm * tick_time, TimeUnits units_changed)
{
	static char date_text[] = "MMM/99";
	static char time_text[] = "99:99am";

	(void) units_changed;

	// Date
	strcpy(date_text, month[tick_time->tm_mon]);
	date_text[3] = '/';
	if (tick_time->tm_mday >= 10) {
		date_text[4] = prv_num_to_digit(tick_time->tm_mday / 10);
		date_text[5] = prv_num_to_digit(tick_time->tm_mday % 10);
		date_text[6] = '\0';
	} else {
		date_text[4] = prv_num_to_digit(tick_time->tm_mday);
		date_text[5] = '\0';
	}
	text_layer_set_text(s_date_layer, date_text);

	// Time
	if (clock_is_24h_style()) {
		time_text[0] = prv_num_to_digit(tick_time->tm_hour / 10);
		time_text[1] = prv_num_to_digit(tick_time->tm_hour % 10);
		time_text[2] = ':';
		time_text[3] = prv_num_to_digit(tick_time->tm_min / 10);
		time_text[4] = prv_num_to_digit(tick_time->tm_min % 10);
		time_text[5] = '\0';
	} else {
		const uint16_t hour_12 = (tick_time->tm_hour % 12 == 0) ? 12 : (tick_time->tm_hour % 12);
		const char     am_pm   = (tick_time->tm_hour >= 12) ? 'p' : 'a';

		if (hour_12 >= 10) {
			time_text[0] = '1';
			time_text[1] = prv_num_to_digit(hour_12 % 10);
			time_text[2] = ':';
			time_text[3] = prv_num_to_digit(tick_time->tm_min / 10);
			time_text[4] = prv_num_to_digit(tick_time->tm_min % 10);
			time_text[5] = am_pm;
			time_text[6] = 'm';
			time_text[7] = '\0';
		} else {
			time_text[0] = prv_num_to_digit(hour_12);
			time_text[1] = ':';
			time_text[2] = prv_num_to_digit(tick_time->tm_min / 10);
			time_text[3] = prv_num_to_digit(tick_time->tm_min % 10);
			time_text[4] = am_pm;
			time_text[5] = 'm';
			time_text[6] = '\0';
			time_text[7] = '\0';
		}
	}

	text_layer_set_text(s_time_layer, time_text);
	prv_mark_progress_dirty();
}

#define TEXT_FG_COLOR GColorWhite
#define TEXT_BG_COLOR GColorBlack

static TextLayer * prv_init_text_layer(const GRect rect, const GTextAlignment align, const GFont font)
{
	TextLayer * const layer = text_layer_create(rect);

	text_layer_set_text_color(layer, TEXT_FG_COLOR);
	text_layer_set_background_color(layer, TEXT_BG_COLOR);
	text_layer_set_text_alignment(layer, align);
	text_layer_set_font(layer, font);
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(layer));

	return layer;
}

static void prv_window_load(Window * const window)
{
	Layer * const window_layer = window_get_root_layer(window);
	const GRect   bounds       = layer_get_bounds(window_layer);
	const int16_t ring_top     = 72;

	window_set_background_color(window, TEXT_BG_COLOR);

	s_date_font      = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ISO_DATE_23));
	s_time_font      = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ISO_TIME_32));
	s_steps_font     = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ISO_STEPS_18));
	s_date_layer     = prv_init_text_layer(GRect(0, 5, bounds.size.w, 27), GTextAlignmentCenter, s_date_font);
	s_time_layer     = prv_init_text_layer(GRect(0, 30, bounds.size.w, 36), GTextAlignmentCenter, s_time_font);
	s_progress_layer = layer_create(GRect(0, ring_top, bounds.size.w, bounds.size.h - ring_top));
	layer_set_update_proc(s_progress_layer, prv_progress_update_proc);
	layer_add_child(window_layer, s_progress_layer);

	// Get a tm structure
	time_t      temp      = time(NULL);
	struct tm * tick_time = localtime(&temp);

	// Display time immediately
	prv_tick_handler(tick_time, MINUTE_UNIT);

	// Subscribe to tick timer service
	tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
	health_service_events_subscribe(prv_health_handler, NULL);
}

static void prv_window_unload(Window * const window)
{
	tick_timer_service_unsubscribe();
	health_service_events_unsubscribe();
	layer_destroy(s_progress_layer);
	text_layer_destroy(s_date_layer);
	text_layer_destroy(s_time_layer);
	fonts_unload_custom_font(s_date_font);
	fonts_unload_custom_font(s_time_font);
	fonts_unload_custom_font(s_steps_font);
}

static void prv_init(void)
{
	prv_load_step_goal();

	s_window = window_create();
	window_set_window_handlers(s_window, (WindowHandlers) {
	                                         .load   = prv_window_load,
	                                         .unload = prv_window_unload,
	                                     });
	app_message_register_inbox_received(prv_inbox_received_handler);
	app_message_open(64, 64);
	const bool animated = true;
	window_stack_push(s_window, animated);
}

static void prv_deinit(void)
{
	window_destroy(s_window);
}

int main(void)
{
	prv_init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

	app_event_loop();
	prv_deinit();
}
