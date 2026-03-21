#include <pebble.h>

static Window *    s_window;
static TextLayer * s_time_layer;

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
	static char s_time_buffer[16];

	strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M:%S" : "%I:%M:%S", tick_time);
	text_layer_set_text(s_time_layer, s_time_buffer);
}

static void prv_window_load(Window * const window)
{
	Layer * const window_layer = window_get_root_layer(window);
	const GRect bounds         = layer_get_bounds(window_layer);

	s_time_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

	// Get a tm structure
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);

	// Display time immediately
	prv_tick_handler(tick_time, SECOND_UNIT);

	// Subscribe to tick timer service
	tick_timer_service_subscribe(SECOND_UNIT, prv_tick_handler);
}

static void prv_window_unload(Window * const window)
{
	text_layer_destroy(s_time_layer);
	tick_timer_service_unsubscribe();
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
