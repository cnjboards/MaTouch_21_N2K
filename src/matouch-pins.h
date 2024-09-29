#pragma once

// defines for MaTouch and daughter card N2K tx and rx
#define ESP32_CAN_TX_PIN GPIO_NUM_43 // on MaTouch this is on J2, pin 4
#define ESP32_CAN_RX_PIN GPIO_NUM_44 // on MaTouch this is on J2, pin 3

// disoply backlight
#define TFT_BL 38

// dial button function
#define BUTTON_PIN 14 // dial press button gpio

// dial rotate encoder
#define ENCODER_CLK 13 // dial rotate CLK (pulses)
#define ENCODER_DT 10  // dial rotate DT (direction)

// display I2C pins
#define I2C_SDA_PIN 17 
#define I2C_SCL_PIN 18 
