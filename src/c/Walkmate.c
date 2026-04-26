/*
 * Walkmate
 * Copyright (c) 2026, Shinsuke Suzuki
 * All rights reserved.
 *
 * This file is part of Walkmate.
 * Licensed under the BSD 3-Clause License. See LICENSE for details.
 */

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
static Layer *     s_weather_layer;
static Layer *     s_battery_layer;
static GFont       s_date_font;
static GFont       s_time_font;
static GFont       s_steps_font;
static GFont       s_distance_font;

enum {
	APP_KEY_STEP_GOAL               = 10000,
	APP_KEY_RING_COLOR              = 10001,
	APP_KEY_WEATHER_UPDATE_INTERVAL = 10002,
	APP_KEY_WEATHER_REQUEST         = 10003,
	APP_KEY_WEATHER_TEMPERATURE     = 10004,
	APP_KEY_WEATHER_TEMPERATURE_MAX = 10005,
	APP_KEY_WEATHER_TEMPERATURE_MIN = 10006,
	APP_KEY_TEMPERATURE_DISPLAY_MAX = 10007,
	APP_KEY_TEMPERATURE_DISPLAY_MIN = 10008,
	APP_KEY_SHOW_AUX_GAUGES         = 10009,
	PERSIST_KEY_STEP_GOAL           = 1,
	PERSIST_KEY_RING_COLOR          = 2,
	PERSIST_KEY_WEATHER_UPDATE_INTERVAL,
	PERSIST_KEY_WEATHER_TEMPERATURE,
	PERSIST_KEY_WEATHER_TEMPERATURE_MAX,
	PERSIST_KEY_WEATHER_TEMPERATURE_MIN,
	PERSIST_KEY_TEMPERATURE_DISPLAY_MAX,
	PERSIST_KEY_TEMPERATURE_DISPLAY_MIN,
	PERSIST_KEY_SHOW_AUX_GAUGES,
	DEFAULT_STEP_GOAL               = 10000,
	DEFAULT_RING_COLOR              = 0xFFFFFF,
	DEFAULT_WEATHER_UPDATE_INTERVAL = 30,
	DEFAULT_TEMPERATURE_DISPLAY_MAX = 40,
	DEFAULT_TEMPERATURE_DISPLAY_MIN = -10,
	MIN_STEP_GOAL                   = 1000,
	MAX_STEP_GOAL                   = 99999,
	MIN_WEATHER_UPDATE_INTERVAL     = 5,
	MAX_WEATHER_UPDATE_INTERVAL     = 180,
	MIN_TEMPERATURE_DISPLAY         = -50,
	MAX_TEMPERATURE_DISPLAY         = 60,
	MAX_STEP_DISPLAY                = 99999,
	ARROWHEAD_ANGLE_OFFSET          = 12,
	MIN_ANGLE_DISPLAY_TEMP          = DEG_TO_TRIGANGLE(30),
	MAX_ANGLE_DISPLAY_TEMP          = DEG_TO_TRIGANGLE(150),
	MIN_ANGLE_DISPLAY_BATTERY       = DEG_TO_TRIGANGLE(210),
	MAX_ANGLE_DISPLAY_BATTERY       = DEG_TO_TRIGANGLE(330),
};

static uint32_t      s_step_goal                          = DEFAULT_STEP_GOAL;
static uint32_t      s_ring_color_hex                     = DEFAULT_RING_COLOR;
static uint32_t      s_weather_update_interval            = DEFAULT_WEATHER_UPDATE_INTERVAL;
static int32_t       s_temperature                        = INT32_MAX;
static int32_t       s_temperature_max                    = INT32_MAX;
static int32_t       s_temperature_min                    = INT32_MAX;
static time_t        s_last_weather_request               = 0;
static int32_t       s_temperature_display_max            = DEFAULT_TEMPERATURE_DISPLAY_MAX;
static int32_t       s_temperature_display_min            = DEFAULT_TEMPERATURE_DISPLAY_MIN;
static bool          s_show_aux_gauges                    = true;
static const int16_t s_progress_ring_outer_padding        = 1;
static const uint8_t s_progress_ring_width                = 16;
static const int16_t s_progress_arrow_base_extra          = 1;
static const uint8_t s_progress_arrow_overflow_line_width = 2;
static const int16_t s_temperature_ring_outer_offset      = 4;
static const uint8_t s_temperature_ring_thickness         = 2;
static const int16_t s_temperature_ring_display_thickness = 6;
static const int16_t s_temperature_display_thickness      = 9;
static const int16_t s_battery_ring_outer_offset          = 4;
static const uint8_t s_battery_ring_thickness             = 2;
static const int16_t s_battery_display_thickness          = 6;

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
static void prv_request_weather(void);

static inline char prv_num_to_digit(uint32_t n)
{
	return '0' + n;
}

static bool prv_is_valid_step_goal(const uint32_t step_goal)
{
	return step_goal >= MIN_STEP_GOAL && step_goal <= MAX_STEP_GOAL;
}

static bool prv_is_valid_ring_color_hex(const uint32_t color_hex)
{
	switch (color_hex) {
	case 0xFFFFFF:
	case 0xAAAAAA:
	case 0x55AAFF:
	case 0x55FF55:
	case 0xFFFF55:
	case 0xFFAA55:
	case 0xFF5555:
	case 0xAA55FF:
	case 0xFF55AA:
		return true;
	default:
		return false;
	}
}

static bool prv_is_valid_weather_update_interval(const uint32_t weather_update_interval)
{
	return weather_update_interval >= MIN_WEATHER_UPDATE_INTERVAL && weather_update_interval <= MAX_WEATHER_UPDATE_INTERVAL;
}

static bool prv_is_valid_temperature_display_range(const int32_t display_max, const int32_t display_min)
{
	return display_min >= MIN_TEMPERATURE_DISPLAY && display_max <= MAX_TEMPERATURE_DISPLAY && display_min < display_max;
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

static void prv_set_ring_color_hex(const uint32_t color_hex)
{
	if (!prv_is_valid_ring_color_hex(color_hex)) {
		return;
	}

	s_ring_color_hex = color_hex;
	persist_write_int(PERSIST_KEY_RING_COLOR, s_ring_color_hex);
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

static void prv_load_ring_color_hex(void)
{
	if (persist_exists(PERSIST_KEY_RING_COLOR)) {
		const uint32_t persisted_ring_color = persist_read_int(PERSIST_KEY_RING_COLOR);
		if (prv_is_valid_ring_color_hex(persisted_ring_color)) {
			s_ring_color_hex = persisted_ring_color;
			return;
		}
	}

	s_ring_color_hex = DEFAULT_RING_COLOR;
}

static void prv_set_weather_update_interval(const uint32_t weather_update_interval)
{
	if (!prv_is_valid_weather_update_interval(weather_update_interval)) {
		return;
	}

	s_weather_update_interval = weather_update_interval;
	persist_write_int(PERSIST_KEY_WEATHER_UPDATE_INTERVAL, s_weather_update_interval);
}

static void prv_load_weather_update_interval(void)
{
	if (persist_exists(PERSIST_KEY_WEATHER_UPDATE_INTERVAL)) {
		const uint32_t persisted_weather_update_interval = persist_read_int(PERSIST_KEY_WEATHER_UPDATE_INTERVAL);
		if (prv_is_valid_weather_update_interval(persisted_weather_update_interval)) {
			s_weather_update_interval = persisted_weather_update_interval;
			return;
		}
	}

	s_weather_update_interval = DEFAULT_WEATHER_UPDATE_INTERVAL;
}

static void prv_set_temperature_display_range(const int32_t display_max, const int32_t display_min)
{
	if (!prv_is_valid_temperature_display_range(display_max, display_min)) {
		return;
	}

	s_temperature_display_max = display_max;
	s_temperature_display_min = display_min;
	persist_write_int(PERSIST_KEY_TEMPERATURE_DISPLAY_MAX, s_temperature_display_max);
	persist_write_int(PERSIST_KEY_TEMPERATURE_DISPLAY_MIN, s_temperature_display_min);

	if (s_weather_layer != NULL) {
		layer_mark_dirty(s_weather_layer);
	}
}

static void prv_load_temperature_display_range(void)
{
	if (persist_exists(PERSIST_KEY_TEMPERATURE_DISPLAY_MAX) && persist_exists(PERSIST_KEY_TEMPERATURE_DISPLAY_MIN)) {
		const int32_t persisted_display_max = persist_read_int(PERSIST_KEY_TEMPERATURE_DISPLAY_MAX);
		const int32_t persisted_display_min = persist_read_int(PERSIST_KEY_TEMPERATURE_DISPLAY_MIN);
		if (prv_is_valid_temperature_display_range(persisted_display_max, persisted_display_min)) {
			s_temperature_display_max = persisted_display_max;
			s_temperature_display_min = persisted_display_min;
			return;
		}
	}

	s_temperature_display_max = DEFAULT_TEMPERATURE_DISPLAY_MAX;
	s_temperature_display_min = DEFAULT_TEMPERATURE_DISPLAY_MIN;
}

static void prv_set_show_aux_gauges(const bool show_aux_gauges)
{
	s_show_aux_gauges = show_aux_gauges;
	persist_write_bool(PERSIST_KEY_SHOW_AUX_GAUGES, s_show_aux_gauges);

	if (s_weather_layer != NULL) {
		layer_mark_dirty(s_weather_layer);
	}
	if (s_battery_layer != NULL) {
		layer_mark_dirty(s_battery_layer);
	}
}

static void prv_load_show_aux_gauges(void)
{
	if (persist_exists(PERSIST_KEY_SHOW_AUX_GAUGES)) {
		s_show_aux_gauges = persist_read_bool(PERSIST_KEY_SHOW_AUX_GAUGES);
		return;
	}

	s_show_aux_gauges = true;
}

static void prv_set_weather(const int32_t temperature, const int32_t temperature_max, const int32_t temperature_min)
{
	s_temperature     = temperature;
	s_temperature_max = temperature_max;
	s_temperature_min = temperature_min;

	persist_write_int(PERSIST_KEY_WEATHER_TEMPERATURE, s_temperature);
	persist_write_int(PERSIST_KEY_WEATHER_TEMPERATURE_MAX, s_temperature_max);
	persist_write_int(PERSIST_KEY_WEATHER_TEMPERATURE_MIN, s_temperature_min);

	if (s_weather_layer != NULL) {
		layer_mark_dirty(s_weather_layer);
	}
}

static void prv_load_weather(void)
{
	if (persist_exists(PERSIST_KEY_WEATHER_TEMPERATURE)) {
		s_temperature = persist_read_int(PERSIST_KEY_WEATHER_TEMPERATURE);
	}

	if (persist_exists(PERSIST_KEY_WEATHER_TEMPERATURE_MAX)) {
		s_temperature_max = persist_read_int(PERSIST_KEY_WEATHER_TEMPERATURE_MAX);
	}

	if (persist_exists(PERSIST_KEY_WEATHER_TEMPERATURE_MIN)) {
		s_temperature_min = persist_read_int(PERSIST_KEY_WEATHER_TEMPERATURE_MIN);
	}
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

static uint32_t prv_get_today_distance_meters(void)
{
	const time_t start = time_start_of_today();
	const time_t end   = time(NULL);

	const HealthServiceAccessibilityMask mask =
	    health_service_metric_accessible(HealthMetricWalkedDistanceMeters, start, end);
	if ((mask & HealthServiceAccessibilityMaskAvailable) == 0) {
		return 0;
	}

	return (uint32_t) health_service_sum_today(HealthMetricWalkedDistanceMeters);
}

static void prv_mark_progress_dirty(void)
{
	if (s_progress_layer != NULL) {
		layer_mark_dirty(s_progress_layer);
	}
}

static void prv_mark_battery_dirty(void)
{
	if (s_battery_layer != NULL) {
		layer_mark_dirty(s_battery_layer);
	}
}

static void prv_battery_state_handler(BatteryChargeState charge_state)
{
	(void) charge_state;

	prv_mark_battery_dirty();
}

static void prv_inbox_received_handler(DictionaryIterator * const iter, void * const context)
{
	(void) context;

	const Tuple * const step_goal_tuple = dict_find(iter, APP_KEY_STEP_GOAL);
	if (step_goal_tuple != NULL && step_goal_tuple->type == TUPLE_INT) {
		prv_set_step_goal(step_goal_tuple->value->int32);
	}

	const Tuple * const ring_color_tuple = dict_find(iter, APP_KEY_RING_COLOR);
	if (ring_color_tuple != NULL && ring_color_tuple->type == TUPLE_INT) {
		prv_set_ring_color_hex(ring_color_tuple->value->int32);
	}

	const Tuple * const weather_update_interval_tuple = dict_find(iter, APP_KEY_WEATHER_UPDATE_INTERVAL);
	if (weather_update_interval_tuple != NULL && weather_update_interval_tuple->type == TUPLE_INT) {
		prv_set_weather_update_interval(weather_update_interval_tuple->value->int32);
	}

	const Tuple * const show_aux_gauges_tuple = dict_find(iter, APP_KEY_SHOW_AUX_GAUGES);
	if (show_aux_gauges_tuple != NULL && show_aux_gauges_tuple->type == TUPLE_INT) {
		prv_set_show_aux_gauges(show_aux_gauges_tuple->value->int32 != 0);
	}

	const Tuple * const temperature_display_max_tuple = dict_find(iter, APP_KEY_TEMPERATURE_DISPLAY_MAX);
	const Tuple * const temperature_display_min_tuple = dict_find(iter, APP_KEY_TEMPERATURE_DISPLAY_MIN);
	if (temperature_display_max_tuple != NULL && temperature_display_min_tuple != NULL && temperature_display_max_tuple->type == TUPLE_INT && temperature_display_min_tuple->type == TUPLE_INT) {
		prv_set_temperature_display_range(temperature_display_max_tuple->value->int32,
		                                  temperature_display_min_tuple->value->int32);
	}

	const Tuple * const temperature_tuple     = dict_find(iter, APP_KEY_WEATHER_TEMPERATURE);
	const Tuple * const temperature_max_tuple = dict_find(iter, APP_KEY_WEATHER_TEMPERATURE_MAX);
	const Tuple * const temperature_min_tuple = dict_find(iter, APP_KEY_WEATHER_TEMPERATURE_MIN);

	if (temperature_tuple != NULL && temperature_max_tuple != NULL && temperature_min_tuple != NULL && temperature_tuple->type == TUPLE_INT && temperature_max_tuple->type == TUPLE_INT && temperature_min_tuple->type == TUPLE_INT) {
		prv_set_weather(temperature_tuple->value->int32,
		                temperature_max_tuple->value->int32,
		                temperature_min_tuple->value->int32);
	}
}

static void prv_request_weather(void)
{
	AppMessageResult     result;
	DictionaryIterator * out_iter;

	result = app_message_outbox_begin(&out_iter);
	if (result != APP_MSG_OK || out_iter == NULL) {
		return;
	}

	dict_write_uint8(out_iter, APP_KEY_WEATHER_REQUEST, 1);
	result = app_message_outbox_send();
	if (result != APP_MSG_OK) {
		APP_LOG(APP_LOG_LEVEL_WARNING, "Failed to request weather: %d", result);
		return;
	}

	s_last_weather_request = time(NULL);
}

static void prv_fill_ring_segment(GContext * const ctx, const GRect rect, const GColor color, const int32_t start_angle, const int32_t end_angle)
{
	graphics_context_set_fill_color(ctx, color);
	graphics_fill_radial(ctx, rect, GOvalScaleModeFitCircle, s_progress_ring_width, start_angle, end_angle);
}

static GPoint prv_gpoint_from_center_radius(const GRect rect, const int16_t radius, const int32_t angle)
{
	const GPoint  center     = grect_center_point(&rect);
	const int16_t diameter   = radius * 2;
	const GRect   polar_rect = GRect(center.x - radius, center.y - radius, diameter, diameter);

	return gpoint_from_polar(polar_rect, GOvalScaleModeFitCircle, angle);
}

static void prv_get_ring_arrowhead_points(const GRect rect, const int32_t angle, GPoint * const points)
{
	const int16_t base_radius  = rect.size.w / 2;
	const int16_t outer_radius = base_radius + s_progress_arrow_base_extra;
	const int16_t inner_radius = base_radius - s_progress_ring_width - s_progress_arrow_base_extra;
	const int16_t mid_radius   = base_radius - (s_progress_ring_width / 2);

	points[0] = prv_gpoint_from_center_radius(rect, inner_radius, angle);
	points[1] = prv_gpoint_from_center_radius(rect, mid_radius, angle + DEG_TO_TRIGANGLE(ARROWHEAD_ANGLE_OFFSET));
	points[2] = prv_gpoint_from_center_radius(rect, outer_radius, angle);
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
	static char distance_text[16];

	uint32_t steps = prv_get_today_steps();

	if (steps > 0U) {
		const GRect    bounds          = layer_get_bounds(layer);
		const uint32_t distance_meters = prv_get_today_distance_meters();

		const int32_t raw_angle = (int32_t) (((uint64_t) TRIG_MAX_ANGLE * steps) / s_step_goal);
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
		const GColor  ring_color = GColorFromHEX(s_ring_color_hex);

		prv_fill_ring_segment(ctx, ring_rect, ring_color, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(0) + angle);
		prv_fill_ring_arrowhead(ctx, ring_rect, ring_color, DEG_TO_TRIGANGLE(-2) + angle);
		prv_draw_ring_arrowhead(ctx, ring_rect, DEG_TO_TRIGANGLE(1) + overflow_angle);

		const uint32_t display_steps = (steps < MAX_STEP_DISPLAY) ? steps : MAX_STEP_DISPLAY;
		snprintf(steps_text, sizeof(steps_text), "%" PRIu32, display_steps);
		snprintf(distance_text,
		         sizeof(distance_text),
		         "%" PRIu32 ".%01" PRIu32 "km",
		         distance_meters / 1000,
		         (distance_meters % 1000) / 100);
		graphics_context_set_text_color(ctx, GColorWhite);
		graphics_draw_text(ctx,
		                   steps_text,
		                   s_steps_font,
		                   GRect(0, bounds.size.h / 2 - 18, bounds.size.w, 26),
		                   GTextOverflowModeTrailingEllipsis,
		                   GTextAlignmentCenter,
		                   NULL);
		graphics_draw_text(ctx,
		                   distance_text,
		                   s_distance_font,
		                   GRect(0, bounds.size.h / 2, bounds.size.w, 22),
		                   GTextOverflowModeTrailingEllipsis,
		                   GTextAlignmentCenter,
		                   NULL);
	}
}

static int32_t prv_weather_calc_temperature_to_angle(int32_t temperature)
{
	int32_t angle;

	if (temperature <= s_temperature_display_min) {
		angle = MAX_ANGLE_DISPLAY_TEMP;
	} else if (temperature >= s_temperature_display_max) {
		angle = MIN_ANGLE_DISPLAY_TEMP;
	} else {
		angle = MAX_ANGLE_DISPLAY_TEMP - (MAX_ANGLE_DISPLAY_TEMP - MIN_ANGLE_DISPLAY_TEMP) * (temperature - s_temperature_display_min) / (s_temperature_display_max - s_temperature_display_min);
	}

	return angle;
}

static bool prv_weather_has_temperature(void)
{
	return s_temperature != INT32_MAX && s_temperature_max != INT32_MAX && s_temperature_min != INT32_MAX;
}

static void prv_weather_update_proc(Layer * const layer, GContext * const ctx)
{
	if (!s_show_aux_gauges) {
		return;
	}

	const GRect   bounds           = layer_get_bounds(layer);
	const int16_t diameter         = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
	const GRect   temperature_rect = GRect((bounds.size.w - diameter) / 2 - s_temperature_ring_outer_offset,
	                                       (bounds.size.h - diameter) / 2 - s_temperature_ring_outer_offset,
	                                       diameter + s_temperature_ring_outer_offset * 2,
	                                       diameter + s_temperature_ring_outer_offset * 2);

	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_radial(ctx, temperature_rect, GOvalScaleModeFillCircle, s_temperature_ring_thickness, DEG_TO_TRIGANGLE(30), DEG_TO_TRIGANGLE(150));

	if (!prv_weather_has_temperature()) {
		return;
	}

	const GRect temperature_ring_display_rect = GRect((bounds.size.w - diameter) / 2 - s_temperature_ring_outer_offset - s_temperature_ring_display_thickness,
	                                                  (bounds.size.h - diameter) / 2 - s_temperature_ring_outer_offset - s_temperature_ring_display_thickness,
	                                                  diameter + (s_temperature_ring_outer_offset + s_temperature_ring_display_thickness) * 2,
	                                                  diameter + (s_temperature_ring_outer_offset + s_temperature_ring_display_thickness) * 2);

	const int32_t start_angle = prv_weather_calc_temperature_to_angle(s_temperature_max);
	const int32_t end_angle   = prv_weather_calc_temperature_to_angle(s_temperature_min);

	graphics_context_set_fill_color(ctx, GColorDarkGray);
	graphics_fill_radial(ctx, temperature_ring_display_rect, GOvalScaleModeFillCircle, s_temperature_ring_display_thickness, start_angle, end_angle);

	const GRect temperature_display_rect = GRect((bounds.size.w - diameter) / 2 - s_temperature_ring_outer_offset - s_temperature_display_thickness,
	                                             (bounds.size.h - diameter) / 2 - s_temperature_ring_outer_offset - s_temperature_display_thickness,
	                                             diameter + (s_temperature_ring_outer_offset + s_temperature_display_thickness) * 2,
	                                             diameter + (s_temperature_ring_outer_offset + s_temperature_display_thickness) * 2);

	const int32_t temperature_angle = prv_weather_calc_temperature_to_angle(s_temperature);

	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_radial(ctx, temperature_display_rect, GOvalScaleModeFillCircle, s_temperature_display_thickness, temperature_angle - DEG_TO_TRIGANGLE(1), temperature_angle + DEG_TO_TRIGANGLE(1));

	for (int32_t temp = (s_temperature_display_min / 10 + 1) * 10; temp < (s_temperature_display_max / 10) * 10; temp += 10) {
		const int32_t temp_angle = prv_weather_calc_temperature_to_angle(temp);
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_radial(ctx, temperature_ring_display_rect, GOvalScaleModeFillCircle, s_temperature_ring_display_thickness, temp_angle - DEG_TO_TRIGANGLE(1), temp_angle + DEG_TO_TRIGANGLE(1));
	}
}

static int32_t prv_battery_calc_charge_to_angle(const uint8_t charge_percent)
{
	return MIN_ANGLE_DISPLAY_BATTERY + (MAX_ANGLE_DISPLAY_BATTERY - MIN_ANGLE_DISPLAY_BATTERY) * charge_percent / 100;
}

static void prv_battery_update_proc(Layer * const layer, GContext * const ctx)
{
	if (!s_show_aux_gauges) {
		return;
	}

	const GRect   bounds       = layer_get_bounds(layer);
	const int16_t diameter     = bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h;
	const GRect   battery_rect = GRect((bounds.size.w - diameter) / 2 - s_battery_ring_outer_offset,
	                                   (bounds.size.h - diameter) / 2 - s_battery_ring_outer_offset,
	                                   diameter + s_battery_ring_outer_offset * 2,
	                                   diameter + s_battery_ring_outer_offset * 2);

	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_radial(ctx, battery_rect, GOvalScaleModeFillCircle, s_battery_ring_thickness, MIN_ANGLE_DISPLAY_BATTERY, MAX_ANGLE_DISPLAY_BATTERY);

	const BatteryChargeState charge_state       = battery_state_service_peek();
	const int32_t            battery_angle      = prv_battery_calc_charge_to_angle(charge_state.charge_percent);
	const GRect              battery_gauge_rect = GRect((bounds.size.w - diameter) / 2 - s_battery_ring_outer_offset - s_battery_display_thickness,
	                                                    (bounds.size.h - diameter) / 2 - s_battery_ring_outer_offset - s_battery_display_thickness,
	                                                    diameter + (s_battery_ring_outer_offset + s_battery_display_thickness) * 2,
	                                                    diameter + (s_battery_ring_outer_offset + s_battery_display_thickness) * 2);

	graphics_context_set_fill_color(ctx, charge_state.is_charging ? GColorWhite : GColorDarkGray);
	graphics_fill_radial(ctx, battery_gauge_rect, GOvalScaleModeFillCircle, s_battery_display_thickness, MIN_ANGLE_DISPLAY_BATTERY, battery_angle);

	for (int32_t percent = 20; percent < charge_state.charge_percent; percent += 20) {
		const int32_t angle = prv_battery_calc_charge_to_angle(percent);
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_radial(ctx, battery_gauge_rect, GOvalScaleModeFillCircle, s_battery_display_thickness, angle - DEG_TO_TRIGANGLE(1), angle + DEG_TO_TRIGANGLE(1));
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

	if (s_last_weather_request == 0 || time(NULL) - s_last_weather_request >= (time_t) (s_weather_update_interval * 60)) {
		prv_request_weather();
	}
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
	const int16_t ring_top     = 64;

	window_set_background_color(window, TEXT_BG_COLOR);

	s_date_font      = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ISO_DATE_23));
	s_time_font      = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ISO_TIME_32));
	s_steps_font     = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ISO_STEPS_20));
	s_distance_font  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ISO_DISTANCE_16));
	s_date_layer     = prv_init_text_layer(GRect(0, 0, bounds.size.w, 27), GTextAlignmentCenter, s_date_font);
	s_time_layer     = prv_init_text_layer(GRect(0, 25, bounds.size.w, 36), GTextAlignmentCenter, s_time_font);
	s_progress_layer = layer_create(GRect(0, ring_top, bounds.size.w, bounds.size.h - ring_top));
	layer_set_update_proc(s_progress_layer, prv_progress_update_proc);
	layer_add_child(window_layer, s_progress_layer);
	s_weather_layer = layer_create(GRect(0, ring_top, bounds.size.w, bounds.size.h - ring_top));
	layer_set_update_proc(s_weather_layer, prv_weather_update_proc);
	layer_add_child(window_layer, s_weather_layer);
	s_battery_layer = layer_create(GRect(0, ring_top, bounds.size.w, bounds.size.h - ring_top));
	layer_set_update_proc(s_battery_layer, prv_battery_update_proc);
	layer_add_child(window_layer, s_battery_layer);

	// Get a tm structure
	time_t      temp      = time(NULL);
	struct tm * tick_time = localtime(&temp);

	// Display time immediately
	prv_tick_handler(tick_time, MINUTE_UNIT);

	// Subscribe to tick timer service
	tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
	battery_state_service_subscribe(prv_battery_state_handler);
}

static void prv_window_unload(Window * const window)
{
	battery_state_service_unsubscribe();
	tick_timer_service_unsubscribe();
	layer_destroy(s_progress_layer);
	layer_destroy(s_weather_layer);
	layer_destroy(s_battery_layer);
	text_layer_destroy(s_date_layer);
	text_layer_destroy(s_time_layer);
	fonts_unload_custom_font(s_date_font);
	fonts_unload_custom_font(s_time_font);
	fonts_unload_custom_font(s_steps_font);
	fonts_unload_custom_font(s_distance_font);
}

static void prv_init(void)
{
	prv_load_step_goal();
	prv_load_ring_color_hex();
	prv_load_weather_update_interval();
	prv_load_temperature_display_range();
	prv_load_show_aux_gauges();
	prv_load_weather();

	s_window = window_create();
	window_set_window_handlers(s_window, (WindowHandlers) {
	                                         .load   = prv_window_load,
	                                         .unload = prv_window_unload,
	                                     });
	app_message_register_inbox_received(prv_inbox_received_handler);
	app_message_open(128, 128);
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
