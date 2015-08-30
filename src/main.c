#include <pebble.h>
#include <ctype.h>


static Window *s_main_window;

static InverterLayer *s_all_inverter_layer;

static GBitmap *s_bars_bitmap;
static BitmapLayer *s_bars_bitmap_layer;

static GBitmap *s_day_bitmap;
static BitmapLayer *s_day_bitmap_layer;

static GBitmap *s_datebg_bitmap;
static BitmapLayer *s_datebg_bitmap_layer; 

static GFont s_custom_font_time;
static GFont s_custom_font_date;

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static bool s_is_inverted = false;


static void invert(Window *window) {
	if (!s_is_inverted) {
		Layer *window_layer = window_get_root_layer(window);
		GRect bounds = layer_get_frame(window_layer);

		#ifdef PBL_PLATFORM_APLITE 
			s_all_inverter_layer = inverter_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
			layer_add_child(window_layer, inverter_layer_get_layer(s_all_inverter_layer));
		#else
			//todo: basalt invert
		#endif
			
		s_is_inverted = true;
	}
}


static void uninvert() {
	if (s_is_inverted) {
		inverter_layer_destroy(s_all_inverter_layer);		
		s_is_inverted = false;
	}
}


static void minute_tick(struct tm* tick_time, TimeUnits units_changed) {
	//weekday
	static char s_day_text[] = "0";
	strftime(s_day_text, sizeof(s_day_text), "%w", tick_time);
	gbitmap_destroy(s_day_bitmap);
	if (s_day_text[0] == '0') {
		s_day_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DAY_0);
	} else if (s_day_text[0] == '1') {
		s_day_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DAY_1);
	} else if (s_day_text[0] == '2') {
		s_day_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DAY_2);
	} else if (s_day_text[0] == '3') {
		s_day_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DAY_3);
	} else if (s_day_text[0] == '4') {
		s_day_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DAY_4);
	} else if (s_day_text[0] == '5') {
		s_day_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DAY_5);
	} else if (s_day_text[0] == '6') {
		s_day_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DAY_6);
	}
	bitmap_layer_set_bitmap(s_day_bitmap_layer, s_day_bitmap);
	
	//time
	static char s_time_text[] = "00:00";
	if (clock_is_24h_style() == true) {
		//24 hour
		strftime(s_time_text, sizeof(s_time_text), "%H:%M", tick_time);
	} else {
		//12 hour
		strftime(s_time_text, sizeof(s_time_text), "%I:%M", tick_time);
	}
	text_layer_set_text(s_time_layer, s_time_text);
	
	//date
	static char s_date_text[] = "00/00/00";
	strftime(s_date_text, sizeof(s_date_text), "%m/%d/%y", tick_time);
	text_layer_set_text(s_date_layer, s_date_text);
}


static void main_window_load(Window *window) {
	//main window & bounds
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	//bars img
	s_bars_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BARS);
	s_bars_bitmap_layer = bitmap_layer_create(GRect(3, 3, 138, 162));
	bitmap_layer_set_bitmap(s_bars_bitmap_layer, s_bars_bitmap);
	
	//weekday img
	s_day_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DAY_0);
	s_day_bitmap_layer = bitmap_layer_create(GRect(3, 11, 138, 57));
	bitmap_layer_set_bitmap(s_day_bitmap_layer, s_day_bitmap);
	
	//date background img
	s_datebg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DATEBG);
	s_datebg_bitmap_layer = bitmap_layer_create(GRect(3, 74, 138, 98));
	bitmap_layer_set_bitmap(s_datebg_bitmap_layer, s_datebg_bitmap);
 
 	//custom fonts
	s_custom_font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CAPTUREITNUMMONO_REGULAR_56));
	s_custom_font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CAPTUREITNUMMONO_REGULAR_28));
	
	//time
	s_time_layer = text_layer_create(GRect(0, 50, bounds.size.w, 106));
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_font(s_time_layer, s_custom_font_time);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	
	//date
	s_date_layer = text_layer_create(GRect(0, 105, bounds.size.w, 134));
	text_layer_set_text_color(s_date_layer, GColorBlack);
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_font(s_date_layer, s_custom_font_date);
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

	//setup timer & initial run to ensure immediate display
	time_t now = time(NULL);
	struct tm *current_time = localtime(&now);
	minute_tick(current_time, MINUTE_UNIT);
	tick_timer_service_subscribe(MINUTE_UNIT, minute_tick);

	//actually add the layers
	layer_add_child(window_layer, bitmap_layer_get_layer(s_bars_bitmap_layer));
	layer_add_child(window_layer, bitmap_layer_get_layer(s_day_bitmap_layer)); 
	layer_add_child(window_layer, bitmap_layer_get_layer(s_datebg_bitmap_layer)); 
	layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
	
	//todo: make this a config option
	//invert(s_main_window);
}


static void main_window_unload(Window *window) {
	tick_timer_service_unsubscribe();
	
	inverter_layer_destroy(s_all_inverter_layer);

	bitmap_layer_destroy(s_bars_bitmap_layer); 
	gbitmap_destroy(s_bars_bitmap);

	bitmap_layer_destroy(s_day_bitmap_layer); 
	gbitmap_destroy(s_day_bitmap);

	bitmap_layer_destroy(s_datebg_bitmap_layer); 
	gbitmap_destroy(s_datebg_bitmap);
	
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_date_layer);
	
	fonts_unload_custom_font(s_custom_font_time); 
	fonts_unload_custom_font(s_custom_font_date);
}


static void init(void) {
	s_main_window = window_create();

	window_set_background_color(s_main_window, GColorBlack);
	
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload,
	});
	
	window_stack_push(s_main_window, true);
}


static void deinit(void) {
	window_destroy(s_main_window);
}


int main(void) {
	init();
	app_event_loop();
	deinit();
}