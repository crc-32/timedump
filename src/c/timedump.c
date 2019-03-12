#include <pebble.h>
#include <stdbool.h>

static Window *s_window;
static TextLayer *s_text_layer;
SmartstrapServiceId ssid;
bool writeConfirm = false;
bool override = false;

void strap_availability_handler(SmartstrapServiceId service_id,
                                       bool is_available) {
  // A service's availability has changed
  APP_LOG(APP_LOG_LEVEL_INFO, "Service %d is %s available",
                                (int)service_id, is_available ? "now" : "NOT");
  ssid = service_id;
}

void dump(bool strap) {
  volatile uint8_t *p8 = (volatile uint8_t *)0x08000000;
  if(strap){
    return; //TODO: Strap mode
  }else{
    int i;
    for (i = 0; i < 16384; i += 16) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        p8[i   ], p8[i+ 1], p8[i+ 2], p8[i+ 3],
        p8[i+ 4], p8[i+ 5], p8[i+ 6], p8[i+ 7],
        p8[i+ 8], p8[i+ 9], p8[i+10], p8[i+11],
        p8[i+12], p8[i+13], p8[i+14], p8[i+15]);
    }
  }
}

bool isRbl(volatile uint8_t *p8) {
  char rbl[4] = "rbl\0";
  char check[4] = {p8[0x1A1], p8[0x1A2], p8[0x1A3], p8[0x1A4]};
  return *rbl == *check;
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Dumping via logs");
  dump(false);
  text_layer_set_text(s_text_layer, "Done");
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  /*text_layer_set_text(s_text_layer, "Dumping via smartstrap");
  dump(true);
  text_layer_set_text(s_text_layer, "Done");*/
  if(writeConfirm)
  {
    writeConfirm = false;
  }
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(!writeConfirm)
  {
    text_layer_set_text(s_text_layer, "DOWN: CONFIRM");
    writeConfirm = true;
    return;
  }else{
    text_layer_set_text(s_text_layer, "!!WRITING!!");
    volatile uint8_t *p8 = (volatile uint8_t *)0x08000000;
    if(!isRbl(p8) || override) {
      // Open bootloader bin
      ResHandle handle = resource_get_handle(RESOURCE_ID_SNOWY_BOOT_RBL);
      size_t res_size = resource_size(handle);
      uint8_t *s_buffer = (uint8_t*)malloc(res_size);
      resource_load(handle, s_buffer, res_size);
      // Calc crc of bootloader bin
      //uint32_t crcFile = crc_32(s_buffer, res_size);
      // Write new bootloader
      memcpy((void*)p8, (void*)s_buffer, res_size);
      // Check crc of new bootloader is same
      //uint32_t crcWrote = crc_32(p8, res_size);
      //if(crcWrote != crcFile) {
      if(false) {//TODO
        text_layer_set_text(s_text_layer, "!!CHECKSUM MISMATCH!!");
        writeConfirm = false;
        override = true;
        return;
      }else{
        text_layer_set_text(s_text_layer, "Success!");
      }

    }else {
      text_layer_set_text(s_text_layer, "BL is already Rebble!");
      override = true;
    }
  }
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
  text_layer_set_text(s_text_layer, "Dwn:Write Sel:Dump");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void prv_init(void) {
  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}