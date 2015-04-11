/*
 * main.c
 * Constructs a Window housing an output TextLayer to show data from 
 * either modes of operation of the accelerometer.
 */

#include <pebble.h>

#define TAP_NOT_DATA true
#define THRESHOLD 20
#define SAMPLE_RATE 10

static Window *s_main_window;
static TextLayer *s_pq_layer, *s_pq_label_layer;
static TextLayer *s_average_pq_layer, *s_best_pq_layer, *s_total_punches_layer;
static double q[10];
static int qidx = 11;
static int punches = 0;
static int best_pq = 0;
static int avg_pq = 0;
static int total_pq = 0;

static double sqrt( double num )
{
  double a, p, e = 0.001, b;
  
  a = num;
  p = a * a;
  while( p - num >= e )
  {
  b = ( a + ( num / a ) ) / 2;
  a = b;
  p = a * a;
  }
  return a;
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  static char s_buffer[128];
  text_layer_set_text(s_pq_layer, "Punch Detected!");
  punches += 1;
  qidx = 0;
  
  snprintf(s_buffer, sizeof(s_buffer), "# Punches: %d", punches);
  text_layer_set_text(s_total_punches_layer, s_buffer);
}

static int sumq() {
  int res = 0;
  for (int i = 0; i < 10; i++) {
    res += q[i];
  }
  return res;
}

static void data_handler(AccelData *data, uint32_t num_samples) {
  // Long lived buffer
  static char s_buffer[128];
  static char s_buffer2[128];
  static char s_buffer3[128];
  
  double cur_ax = 0;
  double cur_ay = 0;
  double cur_az = 0;
  
  for (int i = 0; i < SAMPLE_RATE; i++) {
    cur_ax += data[i].x;
    cur_ay += data[i].y;
    cur_az += data[i].z;
  }
  
  cur_ax /= 10.0;
  cur_ay /= 10.0;
  cur_az /= 10.0;
  
  if (qidx < 10) {
    q[qidx++] = sqrt(cur_ax * cur_ax + cur_ay * cur_ay + cur_az * cur_az);
  }
  
  
  
  if (qidx == 10) {
    
  int pq = sumq();
  
  if (pq > best_pq) {
    best_pq = pq;
    snprintf(s_buffer2, sizeof(s_buffer2), "Best PQ: %d", best_pq);
    text_layer_set_text(s_best_pq_layer, s_buffer2);
  }
  
  total_pq += pq;
  snprintf(s_buffer3, sizeof(s_buffer3), "Avg PQ: %d", total_pq/punches);
  text_layer_set_text(s_average_pq_layer, s_buffer3);
    
    snprintf(s_buffer, sizeof(s_buffer), "%d", pq);
    text_layer_set_text(s_pq_layer, s_buffer);
    qidx++;
  }
  
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create PQ Score TextLayer
  s_pq_layer = text_layer_create(GRect(5, 30, window_bounds.size.w - 10, 30));
  text_layer_set_font(s_pq_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_pq_layer, "No data yet.");
  text_layer_set_overflow_mode(s_pq_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(s_pq_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_pq_layer));
  
  // Create PQ Label
  s_pq_label_layer = text_layer_create(GRect(5, 5, window_bounds.size.w - 10, 30));
  text_layer_set_font(s_pq_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_pq_label_layer, "Last PQ:");
  text_layer_set_overflow_mode(s_pq_label_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(s_pq_label_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_pq_label_layer));
  
  s_best_pq_layer = text_layer_create(GRect(5, 65, window_bounds.size.w - 10, 30));
  text_layer_set_font(s_best_pq_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_best_pq_layer, "Best PQ: 0");
  text_layer_set_overflow_mode(s_best_pq_layer, GTextOverflowModeWordWrap);
    text_layer_set_text_alignment(s_best_pq_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_best_pq_layer));
  
  s_average_pq_layer = text_layer_create(GRect(5, 90, window_bounds.size.w - 10, 30));
  text_layer_set_font(s_average_pq_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_average_pq_layer, "Avg PQ: 0");
  text_layer_set_overflow_mode(s_average_pq_layer, GTextOverflowModeWordWrap);
    text_layer_set_text_alignment(s_average_pq_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_average_pq_layer));
  
  s_total_punches_layer = text_layer_create(GRect(5, 120, window_bounds.size.w - 10, 30));
  text_layer_set_font(s_total_punches_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_total_punches_layer, "# Punches: 0");
  text_layer_set_overflow_mode(s_total_punches_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(s_total_punches_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_total_punches_layer));
  
 
  
 
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_pq_layer);
}

static void init() {
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
    
    int num_samples = SAMPLE_RATE;
    accel_data_service_subscribe(num_samples, data_handler);

    // Choose update rate
    accel_service_set_sampling_rate(ACCEL_SAMPLING_100HZ);
  
  accel_tap_service_subscribe(tap_handler);
}

//Handles single click center
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  //Window *window = (Window *)context;
  //something here
}


//Handles Clicks
static void config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
}

static void deinit() {
  // Destroy main Window
  window_destroy(s_main_window);


    accel_tap_service_unsubscribe();

    accel_data_service_unsubscribe();

  
  window_set_click_config_provider(s_main_window, (ClickConfigProvider) config_provider);

}

int main(void) {
  init();
  app_event_loop();
  deinit();
}