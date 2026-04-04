#include <pebble.h>

static Window *    s_window;
static TextLayer * s_date_layer;
static TextLayer * s_time_layer;

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

static inline char num_to_digit(int n)
{
	return '0' + n;
}

static void prv_tick_handler(struct tm * tick_time, TimeUnits units_changed)
{
	static char date_text[] = "MMM/99";
	static char time_text[] = "99:99am";

	(void)units_changed;

	// Date
	strcpy(date_text, month[tick_time->tm_mon]);
	date_text[3] = '/';
	if (tick_time->tm_mday >= 10) {
		date_text[4] = num_to_digit(tick_time->tm_mday / 10);
		date_text[5] = num_to_digit(tick_time->tm_mday % 10);
		date_text[6] = '\0';
	} else {
		date_text[4] = num_to_digit(tick_time->tm_mday);
		date_text[5] = '\0';
	}
	text_layer_set_text(s_date_layer, date_text);

	// Time
	if (clock_is_24h_style()) {
		time_text[0] = num_to_digit(tick_time->tm_hour / 10);
		time_text[1] = num_to_digit(tick_time->tm_hour % 10);
		time_text[2] = ':';
		time_text[3] = num_to_digit(tick_time->tm_min / 10);
		time_text[4] = num_to_digit(tick_time->tm_min % 10);
		time_text[5] = '\0';
	} else {
		const int  hour_12 = (tick_time->tm_hour % 12 == 0) ? 12 : (tick_time->tm_hour % 12);
		const char am_pm   = (tick_time->tm_hour >= 12) ? 'p' : 'a';

		if (hour_12 >= 10) {
			time_text[0] = '1';
			time_text[1] = num_to_digit(hour_12 % 10);
			time_text[2] = ':';
			time_text[3] = num_to_digit(tick_time->tm_min / 10);
			time_text[4] = num_to_digit(tick_time->tm_min % 10);
			time_text[5] = am_pm;
			time_text[6] = 'm';
			time_text[7] = '\0';
		} else {
			time_text[0] = num_to_digit(hour_12);
			time_text[1] = ':';
			time_text[2] = num_to_digit(tick_time->tm_min / 10);
			time_text[3] = num_to_digit(tick_time->tm_min % 10);
			time_text[4] = am_pm;
			time_text[5] = 'm';
			time_text[6] = '\0';
			time_text[7] = '\0';
		}
	}

	text_layer_set_text(s_time_layer, time_text);
}

#define TEXT_FG_COLOR GColorWhite
#define TEXT_BG_COLOR GColorBlack

static TextLayer * prv_init_text_layer(const GRect rect, const GTextAlignment align, const uint32_t font_res_id)
{
	TextLayer * const layer = text_layer_create(rect);

	text_layer_set_text_color(layer, TEXT_FG_COLOR);
	text_layer_set_background_color(layer, TEXT_BG_COLOR);
	text_layer_set_text_alignment(layer, align);
	text_layer_set_font(layer, fonts_load_custom_font(resource_get_handle(font_res_id)));
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(layer));

	return layer;
}

static void prv_window_load(Window * const window)
{
	Layer * const window_layer = window_get_root_layer(window);
	const GRect   bounds       = layer_get_bounds(window_layer);

	window_set_background_color(window, TEXT_BG_COLOR);

	s_date_layer = prv_init_text_layer(GRect(0, 5, bounds.size.w, 27), GTextAlignmentCenter, RESOURCE_ID_FONT_ISO_DATE_23);
	s_time_layer = prv_init_text_layer(GRect(0, 30, bounds.size.w, 36), GTextAlignmentCenter, RESOURCE_ID_FONT_ISO_TIME_32);

	// Get a tm structure
	time_t      temp      = time(NULL);
	struct tm * tick_time = localtime(&temp);

	// Display time immediately
	prv_tick_handler(tick_time, MINUTE_UNIT);

	// Subscribe to tick timer service
	tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
}

static void prv_window_unload(Window * const window)
{
	tick_timer_service_unsubscribe();
	text_layer_destroy(s_date_layer);
	text_layer_destroy(s_time_layer);
}

static void prv_init(void)
{
	s_window = window_create();
	window_set_window_handlers(s_window, (WindowHandlers) {
	                                         .load   = prv_window_load,
	                                         .unload = prv_window_unload,
	                                     });
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
