#define METAR_KEY 0

#include <pebble.h>

static Window *s_main_window;
static TextLayer *name_layer;
static TextLayer *s_time_layer;
static TextLayer *metar_layer;

static void update_time(){
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          //"%H:%M" : "%I:%M", tick_time);
                                          "%H%MR" : "%I%MR", tick_time);
  
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  if(units_changed == MINUTE_UNIT && tick_time->tm_min / 10 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_cstring(iter, 0, "0");
  
    // Send the message!
    app_message_outbox_send();
  }
}

static void main_window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  name_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(0,10), bounds.size.w, 15));
  text_layer_set_background_color(name_layer, GColorClear);
  text_layer_set_text_color(name_layer, GColorWhite);
  text_layer_set_text_alignment(name_layer, GTextAlignmentCenter);
  text_layer_set_text(name_layer, "For Natalia Godwinska");
  layer_add_child(window_layer, text_layer_get_layer(name_layer));
  
  s_time_layer = text_layer_create(GRect(0,PBL_IF_ROUND_ELSE(58, 30),bounds.size.w, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  metar_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(125,80), bounds.size.w, 100));
  text_layer_set_background_color(metar_layer, GColorClear);
  text_layer_set_text_color(metar_layer, GColorWhite);
  text_layer_set_text_alignment(metar_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(metar_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(metar_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(metar_layer));
}

static void main_window_unload(Window *window){
  text_layer_destroy(s_time_layer);
  text_layer_destroy(metar_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context){
  static char METAR_buffer[512];
  Tuple *METAR_tuple = dict_find(iterator, METAR_KEY);
  
  if (METAR_tuple){
    snprintf(METAR_buffer, sizeof(METAR_buffer), "%s", METAR_tuple -> value -> cstring);
    text_layer_set_text(metar_layer, METAR_buffer);
    
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context){
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context){
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send fail!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context){
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void init(){
  
  tick_timer_service_subscribe(MINUTE_UNIT,tick_handler);
  s_main_window = window_create();
  
  window_set_background_color(s_main_window, GColorBlack);
  
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  const int inbox_size = 128;
  const int outbox_size = 128;
  
  app_message_open(inbox_size, outbox_size);
    
  window_stack_push(s_main_window, true);
  update_time();
}

void deinit(){
  window_destroy(s_main_window);
}

int main(){
  init();
  
  app_event_loop();
  
  deinit();  
  
  return 0;
}