#include <pebble.h>
#include "stdio.h"
#include "string.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_tim.h"
#include "stm32f4x_i2c.h"

#define REG_BUCK2_CFG  0x0F
#define REG_BUCK2_VSET 0x10
void max14690_initr(void);
void hw_power_init(void);
typedef struct {
    
    // I2C Stuff
    uint8_t Address;   
    GPIO_TypeDef *PortI2C;
    uint16_t PinSCL;
    uint16_t PinSDA;

    uint16_t PinIntn;   // power interrupt
    uint16_t PinReset;  // reset the max14690 (low)
    uint16_t PinMPC0;   // external peripheral control 0
    uint16_t PinMPC1;   // external peripheral control 1
    uint16_t PinPFN1;   // Fn1
    uint16_t PinPFN2;   // Fn2
    uint16_t PinMON;    // Monitor
} max14690_t;
static Window *s_window;
static TextLayer *s_text_layer;

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Dumping");
  hw_power_init();
  text_layer_set_text(s_text_layer, "Done");
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Up");
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Down");
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
  text_layer_set_text(s_text_layer, "Press a button");
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

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}

// useful?
// https://developer.mbed.org/users/switches/code/MAX14690/file/666b6c505289/MAX14690.h

// massive work in progress

max14690_t max1690 = {
    .Address    = 0x50,
    .PinIntn    = GPIO_Pin_1,
    .PinReset   = GPIO_Pin_1, // unknown
    .PinSCL     = GPIO_Pin_6,
    .PinSDA     = GPIO_Pin_9,
    // unknown
    .PinMPC0    = GPIO_Pin_8,
    .PinMPC1    = GPIO_Pin_8,
    .PinPFN1    = GPIO_Pin_8,
    .PinPFN2    = GPIO_Pin_8,
    .PinMON     = GPIO_Pin_8,
};

I2C_conf_t I2C_conf = {
    .I2Cx                      = I2C1,
    .RCC_APB1Periph_I2Cx       = RCC_APB1Periph_I2C1,
    .RCC_AHB1Periph_GPIO_SCL   = RCC_AHB1Periph_GPIOB,
    .RCC_AHB1Periph_GPIO_SDA   = RCC_AHB1Periph_GPIOB,
    .GPIO_AF_I2Cx              = GPIO_AF_I2C1,
    .GPIO_SCL                  = GPIOB,
    .GPIO_SDA                  = GPIOB,
    .GPIO_Pin_SCL              = GPIO_Pin_6,
    .GPIO_Pin_SDA              = GPIO_Pin_9,
    .GPIO_PinSource_SCL        = GPIO_PinSource6,
    .GPIO_PinSource_SDA        = GPIO_PinSource9,
};

void max14690_initr(void)
{
    /*
     *
     * These were set in bootloader. at some point.
     * TODO
     GPIO_SetBits(GPIOD, GPIO_Pin_2);
    GPIO_SetBits(GPIOD, GPIO_Pin_4);
    
    GPIO_SetBits(GPIOF, GPIO_Pin_3);
    GPIO_SetBits(GPIOF, GPIO_Pin_2);
    */
    //GPIO_InitTypeDef GPIO_InitStructure;
    
//     stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOF);
    
    /*GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    GPIO_SetBits(GPIOF, GPIO_Pin_3);
    GPIO_SetBits(GPIOF, GPIO_Pin_2);*/

//     stm32_power_release(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOF);
    
    /*uint8_t buf[0x1F];
    I2C_read_reg(&I2C_conf, 0x50, 0x00, buf, 0x1F + 1);
    
     for (uint16_t i = 0; i < 0x1F + 1; i++)
     {
        char *cbuf;
        size_t sz;
        sz = snprintf(NULL, 0, "R: %02x %02x", i, buf[i]);
        cbuf = (char *)malloc(sz + 1);  make sure you check for != NULL in real code
        snprintf(cbuf, sz+1, "R: %02x %02x", i, buf[i]);
        APP_LOG(APP_LOG_LEVEL_DEBUG, cbuf, NULL);
     }*/

//     printf("****I2C 0x16**\n");    
volatile uint8_t *p8 = (volatile uint8_t *)0x08000000;
  int i;
  for (i = 0; i < 16384; i += 16) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
      p8[i   ], p8[i+ 1], p8[i+ 2], p8[i+ 3],
      p8[i+ 4], p8[i+ 5], p8[i+ 6], p8[i+ 7],
      p8[i+ 8], p8[i+ 9], p8[i+10], p8[i+11],
      p8[i+12], p8[i+13], p8[i+14], p8[i+15]);

  }
}

void hw_power_init(void)
{   
    I2C_init(&I2C_conf);
    max14690_initr();
}