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
static TextLayer *s_output_layer;
static double q[10];
static int qidx = 0;

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
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Punch detected");
  switch (axis) {
  case ACCEL_AXIS_X:
    if (direction > 0) {
      text_layer_set_text(s_output_layer, "X axis positive.");
    } else {
      text_layer_set_text(s_output_layer, "X axis negative.");
    }
    break;
  case ACCEL_AXIS_Y:
    if (direction > 0) {
      text_layer_set_text(s_output_layer, "Y axis positive.");
    } else {
      text_layer_set_text(s_output_layer, "Y axis negative.");
    }
    break;
  case ACCEL_AXIS_Z:
    if (direction > 0) {
      text_layer_set_text(s_output_layer, "Z axis positive.");
    } else {
      text_layer_set_text(s_output_layer, "Z axis negative.");
    }
    break;
  }
  qidx = 0;
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
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "LOG");
  
  if (qidx < 10) {
    q[qidx++] = sqrt(cur_ax * cur_ax + cur_ay * cur_ay + cur_az * cur_az);
  }
  
  if (qidx == 10) {
    
    snprintf(s_buffer, sizeof(s_buffer), "PQ: %d", sumq());
    text_layer_set_text(s_output_layer, s_buffer);
    qidx++;
  }
  
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 10, window_bounds.size.h));
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_output_layer, "No data yet.");
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
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