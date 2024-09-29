#define USBSerial Serial

#include <Arduino.h>
#include "matouch-pins.h"
#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include "touch.h"
#include "Globals.h"

// Specific to GFX hardware setup
#define TOUCH_RST -1 // 38
#define TOUCH_IRQ -1 // 0
#define DISPLAY_DC GFX_NOT_DEFINED 
#define DISPLAY_CS 1 
#define DISPLAY_SCK 46 
#define DISPLAY_MOSI 0 
#define DISPLAY_MISO GFX_NOT_DEFINED

#define DISPLAY_DE 2 
#define DISPLAY_VSYNC 42 
#define DISPLAY_HSYNC 3 
#define DISPLAY_PCLK 45 
#define DISPLAY_R0 11 
#define DISPLAY_R1 15 
#define DISPLAY_R2 12 
#define DISPLAY_R3 16 
#define DISPLAY_R4 21 
#define DISPLAY_G0 39 
#define DISPLAY_G1 7 
#define DISPLAY_G2 47 
#define DISPLAY_G3 8 
#define DISPLAY_G4 48 
#define DISPLAY_G5 9 
#define DISPLAY_B0 4 
#define DISPLAY_B1 41 
#define DISPLAY_B2 5 
#define DISPLAY_B3 40 
#define DISPLAY_B4 6 
#define DISPLAY_HSYNC_POL 1 
#define DISPLAY_VSYNC_POL 1 
#define DISPLAY_HSYNC_FRONT_PORCH 10 
#define DISPLAY_VSYNC_FRONT_PORCH 10 
#define DISPLAY_HSYNC_BACK_PORCH 50 
#define DISPLAY_VSYNC_BACK_PORCH 20 
#define DISPLAY_HSYNC_PULSE_WIDTH 8 
#define DISPLAY_VSYNC_PULSE_WIDTH 8

Arduino_DataBus *bus = new Arduino_SWSPI( DISPLAY_DC /* DC */, DISPLAY_CS /* CS */, DISPLAY_SCK /* SCK */, DISPLAY_MOSI /* MOSI */, DISPLAY_MISO /* MISO */);

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel( DISPLAY_DE, DISPLAY_VSYNC , DISPLAY_HSYNC, DISPLAY_PCLK, DISPLAY_R0, DISPLAY_R1, DISPLAY_R2, 
      DISPLAY_R3, DISPLAY_R4, DISPLAY_G0, DISPLAY_G1, DISPLAY_G2, DISPLAY_G3, DISPLAY_G4, DISPLAY_G5, DISPLAY_B0, DISPLAY_B1, DISPLAY_B2, DISPLAY_B3, 
      DISPLAY_B4, DISPLAY_HSYNC_POL, DISPLAY_HSYNC_FRONT_PORCH, DISPLAY_HSYNC_PULSE_WIDTH, DISPLAY_VSYNC_BACK_PORCH, DISPLAY_VSYNC_POL, DISPLAY_VSYNC_FRONT_PORCH, 
      DISPLAY_VSYNC_PULSE_WIDTH, DISPLAY_VSYNC_BACK_PORCH);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display( 480 /* width */, 480 /* height */, rgbpanel, 0 /* rotation */, true /* auto_flush */, bus, GFX_NOT_DEFINED /* RST */, 
      st7701_type5_init_operations, sizeof(st7701_type5_init_operations));

#define COLOR_NUM 5
int ColorArray[COLOR_NUM] = {WHITE, BLUE, GREEN, RED, YELLOW};

/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

// LVGL stuff
/*Change to your screen resolution*/
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 480;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

    lv_disp_flush_ready(disp);
} // end my_disp_flush

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    int touchX = 0, touchY = 0;

    if (read_touch(&touchX, &touchY) == 1)
    {
        data->state = LV_INDEV_STATE_PR;

        data->point.x = (uint16_t)touchX;
        data->point.y = (uint16_t)touchY;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
} // end my_touchpad_read

void my_encoder_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    if (counter > 0)
    {
        data->state = LV_INDEV_STATE_PRESSED;
        data->key = LV_KEY_LEFT;
        counter--;
        move_flag = 1;
    }

    else if (counter < 0)
    {
        data->state = LV_INDEV_STATE_PRESSED;
        data->key = LV_KEY_RIGHT;
        counter++;
        move_flag = 1;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    // if (enc_pressed())
    //     data->state = LV_INDEV_STATE_PRESSED;
    // else
    //     data->state = LV_INDEV_STATE_RELEASED;
} // end my_encoder_read

// end lvgl stuff

// objects for display
static lv_style_t border_style;
static lv_style_t smallBorder_style;
static lv_style_t style_btn;
static lv_style_t body_style;
static lv_style_t indicator_style;
static lv_style_t smallIndicator_style;
static lv_style_t vertBarStyle;
static lv_style_t vertBarStyleIndic;
static lv_obj_t *wifiLabel;
static lv_obj_t *ipLabel;
static lv_obj_t *ssidLabel;
static lv_timer_t *updateMainScreenTimer;
static lv_timer_t *updateMainScreen_testTimer;
static lv_timer_t *updateStatusBarTimer;
static lv_obj_t *Screen1;
static lv_obj_t *Screen2;
static lv_obj_t *Screen3;
static lv_obj_t *Screen4;
static lv_obj_t *Screen5;
static lv_obj_t *engineRpmGauge;
static lv_meter_indicator_t *engineRpmIndic;
static lv_obj_t *engineOilGauge;
static lv_meter_indicator_t *engineOilIndic;
static lv_obj_t *engineRPMIndicator;
static lv_obj_t *engineRPMText;
static lv_obj_t *sogIndicator;
static lv_obj_t *sogText;
static lv_obj_t *engineOilIndicator;
static lv_obj_t *engineOilText;
static lv_obj_t *engineOilTitleText;
static lv_obj_t *tempGuageObject;
static lv_obj_t *tempGuageBar;
static lv_obj_t *tempGuageBar_shadow;
static lv_obj_t *tempGuageIndicator;
static lv_obj_t *tempGuageText;
static lv_obj_t *tempGuageTitleText;
static lv_obj_t *tempGuageBarTextLower;
static lv_obj_t *tempGuageBarTextMid;
static lv_obj_t *tempGuageBarTextUpper;
static lv_obj_t *battGuageObject;
static lv_obj_t *battGuageBar;
static lv_obj_t *battGuageBar_shadow;
static lv_obj_t *battGuageIndicator;
static lv_obj_t *battGuageText;
static lv_obj_t *battGuageTitleText;
static lv_obj_t *battGuageBarTextLower;
static lv_obj_t *battGuageBarTextMid;
static lv_obj_t *battGuageBarTextUpper;
static lv_obj_t *screenList;
static lv_obj_t *screen2Text;
static lv_obj_t *screen3Text;
static lv_obj_t *screen4Text;
static lv_obj_t *screen5Text;

// list of screen names, simple but works
#define SCREEN_1_NAME "Motoring"
#define SCREEN_2_NAME "Sailing"
#define SCREEN_3_NAME "Tanks"
#define SCREEN_4_NAME "Battery"
#define SCREEN_5_NAME "Configuration"

// forward declarations
static void setStyle();
static void buildScreen1(void);
static void buildScreen2(void);
static void buildScreen3(void);
static void buildScreen4(void);
static void buildScreen5(void);
static void buildRPMGuage();
static void buildOilGuage();
static void buildTempGuage();
static void updateMainScreen(lv_timer_t *);
static void buildScreenList(lv_obj_t *);


// init lvgl graphics
void doLvglInit(){

    // print out version
    String LVGL_Arduino = "LVGL: ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial.println( LVGL_Arduino );
    
    // Init display hw
    gfx->begin();

    // init graphics lib
    lv_init();
    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );

    // start the main UI code here
    setStyle();

    // build all the screens here, for now support 5 screens, dumb ut simple :-)
    buildScreen1();
    buildScreen2();
    buildScreen3();
    buildScreen4();
    buildScreen5();

    // activate main screen (screen 1) on startup
    lv_scr_load(Screen1);
    // build a list for screen navigation and attach to screen 1
    buildScreenList(Screen1);
    // start timer for screen processing
    updateMainScreenTimer = lv_timer_create(updateMainScreen, 250, NULL);
} // end do_lvgl_init

// setup all the lvgl styles for this project
static void setStyle() {
  
  // overall style 
  lv_style_init(&border_style);
  lv_style_set_border_width(&border_style, 2);
  lv_style_set_border_color(&border_style, lv_color_black());
  lv_style_set_text_font(&border_style, &lv_font_montserrat_20);

  // body style 
  lv_style_init(&body_style);
  lv_style_set_border_width(&body_style, 2);
  lv_style_set_border_color(&body_style, lv_color_black());
  lv_style_set_text_font(&body_style, &lv_font_montserrat_20);

  // small text version 
  lv_style_init(&smallBorder_style);
  lv_style_set_border_width(&smallBorder_style, 2);
  lv_style_set_border_color(&smallBorder_style, lv_color_black());
  lv_style_set_text_font(&smallBorder_style, &lv_font_montserrat_8);

  // style for indicator readouts
  lv_style_init(&indicator_style);
  lv_style_set_border_width(&indicator_style, 2);
  lv_style_set_radius(&indicator_style,4);
  lv_style_set_border_color(&indicator_style, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_text_font(&indicator_style, &lv_font_montserrat_20);

  // style for indicator readouts
  lv_style_init(&smallIndicator_style);
  lv_style_set_border_width(&smallIndicator_style, 1);
  lv_style_set_radius(&smallIndicator_style,2);
  lv_style_set_border_color(&smallIndicator_style, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_text_font(&indicator_style, &lv_font_montserrat_10);

  // status bar, wifi text (changes color with connectivity)
  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, lv_color_hex(0xC5C5C5));
  lv_style_set_bg_opa(&style_btn, LV_OPA_50);
  lv_style_set_text_color(&style_btn, lv_color_make(255, 0, 0));

  lv_style_init(&vertBarStyle);
  lv_style_set_border_color(&vertBarStyle, lv_palette_main(LV_PALETTE_BLUE_GREY));
  lv_style_set_border_width(&vertBarStyle, 2);
  lv_style_set_pad_all(&vertBarStyle, 2); /*To make the indicator smaller*/
  lv_style_set_radius(&vertBarStyle, 1);
  lv_style_set_anim_time(&vertBarStyle, 1000);

  lv_style_init(&vertBarStyleIndic);
  lv_style_set_bg_opa(&vertBarStyleIndic, LV_OPA_COVER);
  lv_style_set_bg_color(&vertBarStyleIndic, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_radius(&vertBarStyleIndic, 0);

} // end setstyle

// decide later
#define MS_HIEGHT (screenHeight - 0)
#define MS_WIDTH (screenWidth - 0) 

// screen 1 builder
static void buildScreen1(void) {
  // main screen frame
  Screen1 = lv_obj_create(NULL);
  lv_obj_add_style(Screen1, &border_style, 0);
  lv_obj_set_size(Screen1, MS_WIDTH, MS_HIEGHT);
  lv_obj_align(Screen1, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_scrollbar_mode(Screen1, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(Screen1, LV_OBJ_FLAG_SCROLLABLE);

  // Components on screen 1
  buildRPMGuage();
  buildOilGuage();
  buildTempGuage();

} // end buildScreen1

// screen 2 builder
static void buildScreen2(void) {
  // main screen frame
  Screen2 = lv_obj_create(NULL);
  lv_obj_add_style(Screen2, &border_style, 0);
  lv_obj_set_size(Screen2, MS_WIDTH, MS_HIEGHT);
  lv_obj_align(Screen2, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_scrollbar_mode(Screen2, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(Screen2, LV_OBJ_FLAG_SCROLLABLE);

  // Build screen 2 components here
  // TBD
  // placeholder label
  screen2Text = lv_label_create(Screen2);
  lv_obj_align_to(screen2Text, Screen2, LV_ALIGN_CENTER, -50, 25);
  lv_obj_set_style_text_font(screen2Text, &lv_font_montserrat_28, 0);
  lv_label_set_text(screen2Text,SCREEN_2_NAME);

} // end buildScreen2

// screen 3 builder
static void buildScreen3(void) {
  // main screen frame
  Screen3 = lv_obj_create(NULL);
  lv_obj_add_style(Screen3, &border_style, 0);
  lv_obj_set_size(Screen3, MS_WIDTH, MS_HIEGHT);
  lv_obj_align(Screen3, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_scrollbar_mode(Screen3, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(Screen3, LV_OBJ_FLAG_SCROLLABLE);

  // Build screen 3 components here
  // TBD
  // placeholder label
  screen3Text = lv_label_create(Screen3);
  lv_obj_align_to(screen3Text, Screen3, LV_ALIGN_CENTER, -50, 25);
  lv_obj_set_style_text_font(screen3Text, &lv_font_montserrat_28, 0);
  lv_label_set_text(screen3Text,SCREEN_3_NAME);

} // end buildScreen3

// screen 4 builder
static void buildScreen4(void) {
  // main screen frame
  Screen4 = lv_obj_create(NULL);
  lv_obj_add_style(Screen4, &border_style, 0);
  lv_obj_set_size(Screen4, MS_WIDTH, MS_HIEGHT);
  lv_obj_align(Screen4, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_scrollbar_mode(Screen4, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(Screen4, LV_OBJ_FLAG_SCROLLABLE);

  // Build screen 4 components here
  // TBD
  // placeholder label
  screen4Text = lv_label_create(Screen4);
  lv_obj_align_to(screen4Text, Screen4, LV_ALIGN_CENTER, -50, 25);
  lv_obj_set_style_text_font(screen4Text, &lv_font_montserrat_28, 0);
  lv_label_set_text(screen4Text,SCREEN_4_NAME);

} // end buildScreen4

// screen 5 builder
static void buildScreen5(void) {
  // main screen frame
  Screen5 = lv_obj_create(NULL);
  lv_obj_add_style(Screen5, &border_style, 0);
  lv_obj_set_size(Screen5, MS_WIDTH, MS_HIEGHT);
  lv_obj_align(Screen5, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_scrollbar_mode(Screen2, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(Screen5, LV_OBJ_FLAG_SCROLLABLE);

  // Build screen 5 components here
  // TBD
  // placeholder label
  screen5Text = lv_label_create(Screen5);
  lv_obj_align_to(screen5Text, Screen5, LV_ALIGN_CENTER, -50, 25);
  lv_obj_set_style_text_font(screen5Text, &lv_font_montserrat_28, 0);
  lv_label_set_text(screen5Text,SCREEN_5_NAME);

} // end buildScreen5

// worker code to process the display
void processDisplay(void){
  
  // must call from display
  lv_timer_handler(); 
  
  // check if PB pressed and bring up the screen selection list
  if ( shortButtonStateLatched == true){
    // reset the latched pb
    shortButtonStateLatched = false;
    if (lv_obj_has_flag(screenList, LV_OBJ_FLAG_HIDDEN)){
      // unhide
      lv_obj_clear_flag(screenList, LV_OBJ_FLAG_HIDDEN);  
    } else {
      // hide the list
      lv_obj_add_flag(screenList, LV_OBJ_FLAG_HIDDEN);
    } // end else
  } // end if

  #ifdef SERIALDEBUG
  // debug stuff
  if (read_touch(&x, &y) == 1)
  {
    USBSerial.print("Touch ");
    USBSerial.print(x);
    USBSerial.print("\t");
    USBSerial.println(y);

    flesh_flag = 1;
  }
  if (digitalRead(BUTTON_PIN) == 0)
  {
    if (button_flag != 1)
    {
      button_flag = 1;
      flesh_flag = 1;
    }

    USBSerial.println("Button Press");
  } else {
    if (button_flag != 0)
    {
      button_flag = 0;
      flesh_flag = 1;
    } // end if
  } // end if

  if (move_flag == 1)
  {
    USBSerial.print("Position: ");
    USBSerial.println(counter);
    move_flag = 0;
    flesh_flag = 1;
  } // end if
  #endif

  delay(100);
} // end processdisplay



// locate engine rpm guage on the main screen relative to centre of Screen1 object
#define RPM_GUAGE_HIEGHT 440
#define RPM_GUAGE_WIDTH 440
#define ENGINE_RPM_GUAGE_XOFFSET -155
#define ENGINE_RPM_GUAGE_YOFFSET -150

// build the rpm guage on the main screen
static void buildRPMGuage() {
  // create and rpm guage on main display
  engineRpmGauge = lv_meter_create(Screen1);
  // this removes the outline, allows for more usable space
  lv_obj_remove_style(engineRpmGauge, NULL, LV_PART_MAIN);
  
  // Locate and set size of rpm guage
  lv_obj_align_to(engineRpmGauge, Screen1, LV_ALIGN_CENTER, ENGINE_RPM_GUAGE_XOFFSET, ENGINE_RPM_GUAGE_YOFFSET);
  lv_obj_set_size(engineRpmGauge, RPM_GUAGE_WIDTH, RPM_GUAGE_HIEGHT);
  
  /*Add a scale first*/
  lv_meter_scale_t * scale = lv_meter_add_scale(engineRpmGauge);
  lv_meter_set_scale_range(engineRpmGauge, scale, 0, 50, 210, 165);
  lv_meter_set_scale_ticks(engineRpmGauge, scale, 51, 4, 20, lv_palette_main(LV_PALETTE_GREY));
  lv_meter_set_scale_major_ticks(engineRpmGauge, scale, 10, 6, 30, lv_color_black(), 15);
  
  /* Green range - arc */
  engineRpmIndic = lv_meter_add_arc(engineRpmGauge, scale, 3, lv_palette_main(LV_PALETTE_GREEN), 0);
  lv_meter_set_indicator_start_value(engineRpmGauge, engineRpmIndic, 0);
  lv_meter_set_indicator_end_value(engineRpmGauge, engineRpmIndic, 30);

  /* Green range - ticks */
  engineRpmIndic = lv_meter_add_scale_lines(engineRpmGauge, scale, lv_palette_main(LV_PALETTE_GREEN), lv_palette_main(LV_PALETTE_GREEN),false, 0);
  lv_meter_set_indicator_start_value(engineRpmGauge, engineRpmIndic, 0);
  lv_meter_set_indicator_end_value(engineRpmGauge, engineRpmIndic, 30);

  /* Red range - arc */
  engineRpmIndic = lv_meter_add_arc(engineRpmGauge, scale, 3, lv_palette_main(LV_PALETTE_BLUE), 0);
  lv_meter_set_indicator_start_value(engineRpmGauge, engineRpmIndic, 30);
  lv_meter_set_indicator_end_value(engineRpmGauge, engineRpmIndic, 50);

  /* Red range - arc */
  engineRpmIndic = lv_meter_add_scale_lines(engineRpmGauge, scale, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false,0);
  lv_meter_set_indicator_start_value(engineRpmGauge, engineRpmIndic, 35);
  lv_meter_set_indicator_end_value(engineRpmGauge, engineRpmIndic, 50);

  /* Add the indicator needle and set initial value*/
  engineRpmIndic = lv_meter_add_needle_line(engineRpmGauge, scale, 6, lv_palette_main(LV_PALETTE_BLUE_GREY), -60);
  lv_meter_set_indicator_value(engineRpmGauge, engineRpmIndic, 0);

  /* some static text on the display */
  engineRPMText = lv_label_create(engineRpmGauge);
  lv_obj_align_to(engineRPMText, engineRpmGauge, LV_ALIGN_CENTER, -10, -135);
  lv_obj_set_style_text_font(engineRPMText, &lv_font_montserrat_24, 0);
  lv_label_set_text(engineRPMText,"RPM \nx100");

  /* display the rpm in numerical format as well */
  engineRPMIndicator = lv_label_create(engineRpmGauge);
  lv_obj_add_style(engineRPMIndicator, &indicator_style, 0);
  lv_obj_align_to(engineRPMIndicator, engineRpmGauge, LV_ALIGN_CENTER, -38, 18);
  lv_obj_set_style_text_font(engineRPMIndicator, &lv_font_montserrat_34, 0);
  lv_label_set_text_fmt(engineRPMIndicator, " %04d ", 0);

/* some static text on the display */
  sogText = lv_label_create(engineRpmGauge);
  lv_obj_align_to(sogText, engineRpmGauge, LV_ALIGN_CENTER, -120, -45);
  lv_obj_set_style_text_font(sogText, &lv_font_montserrat_24, 0);
  lv_label_set_text(sogText,"SOG");

  /* display the sog in numerical format as well */
  sogIndicator = lv_label_create(engineRpmGauge);
  lv_obj_add_style(sogIndicator, &indicator_style, 0);
  lv_obj_align_to(sogIndicator, engineRpmGauge, LV_ALIGN_CENTER, -150, -20);
  lv_obj_set_style_text_font(sogIndicator, &lv_font_montserrat_24, 0);
  lv_label_set_text_fmt(sogIndicator, "%03.1f knts", (float)(locSOG * 1.944));

} // end buildrpmguage

// locate engine oil guage on the main screen relative to centre of Screen1 object
#define ENGINE_OIL_GUAGE_HIEGHT 150
#define ENGINE_OIL_GUAGE_WIDTH 150
#define ENGINE_OIL_GUAGE_XOFFSET 82
#define ENGINE_OIL_GUAGE_YOFFSET -20

// build the oil pressure on the main screen
static void buildOilGuage() {
  // create and rpm guage on main display
  engineOilGauge = lv_meter_create(Screen1);
  lv_obj_add_style(engineOilGauge, &smallBorder_style, 0);
  lv_obj_remove_style(engineOilGauge, NULL, LV_PART_MAIN);

  // set the size
  lv_obj_set_size(engineOilGauge, ENGINE_OIL_GUAGE_WIDTH, ENGINE_OIL_GUAGE_HIEGHT);
  lv_obj_align_to(engineOilGauge, Screen1, LV_ALIGN_CENTER, ENGINE_OIL_GUAGE_XOFFSET, ENGINE_OIL_GUAGE_YOFFSET);
  
  /*Add a scale first*/
  lv_meter_scale_t * scale = lv_meter_add_scale(engineOilGauge);
  lv_meter_set_scale_range(engineOilGauge, scale, 0, 80, 180, 180);
  lv_meter_set_scale_ticks(engineOilGauge, scale, 9, 3, 6, lv_color_black());
  lv_meter_set_scale_major_ticks(engineOilGauge, scale, 2, 5, 8, lv_color_black(), 13);
  
  /* Green range - arc */
  engineOilIndic = lv_meter_add_arc(engineOilGauge, scale, 2, lv_palette_main(LV_PALETTE_GREEN), 0);
  lv_meter_set_indicator_start_value(engineOilGauge, engineOilIndic, 20);
  lv_meter_set_indicator_end_value(engineOilGauge, engineOilIndic, 80);

  /* Green range - ticks */
  engineOilIndic = lv_meter_add_scale_lines(engineOilGauge, scale, lv_palette_main(LV_PALETTE_GREEN), lv_palette_main(LV_PALETTE_GREEN),false, 0);
  lv_meter_set_indicator_start_value(engineOilGauge, engineRpmIndic, 20);
  lv_meter_set_indicator_end_value(engineOilGauge, engineRpmIndic, 80);

  /* Red range - arc */
  engineOilIndic = lv_meter_add_arc(engineOilGauge, scale, 2, lv_palette_main(LV_PALETTE_BLUE), 0);
  lv_meter_set_indicator_start_value(engineOilGauge, engineOilIndic, 0);
  lv_meter_set_indicator_end_value(engineOilGauge, engineOilIndic, 20);

  /* Red range - arc */
  engineOilIndic = lv_meter_add_scale_lines(engineOilGauge, scale, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_BLUE), false,0);
  lv_meter_set_indicator_start_value(engineOilGauge, engineOilIndic, 0);
  lv_meter_set_indicator_end_value(engineOilGauge, engineOilIndic, 20);

  /* Add the indicator needle and set initial value*/
  engineOilIndic = lv_meter_add_needle_line(engineOilGauge, scale, 3, lv_palette_main(LV_PALETTE_BLUE_GREY), -25);
  lv_meter_set_indicator_value(engineOilGauge, engineOilIndic, 0);

  /* some static text on the display */
  engineOilText = lv_label_create(engineOilGauge);
  lv_obj_align_to(engineOilText, engineOilGauge, LV_ALIGN_CENTER, 30, 22);
  lv_obj_set_style_text_font(engineOilText, &lv_font_montserrat_20, 0);
  lv_label_set_text(engineOilText,"Psi");

  /* display the rpm in numerical format as well */
  engineOilIndicator = lv_label_create(engineOilGauge);
  lv_obj_align_to(engineOilIndicator, engineOilGauge, LV_ALIGN_CENTER, -40, 25);
  lv_obj_set_style_text_font(engineOilIndicator, &lv_font_unscii_16, 0);
  lv_label_set_text_fmt(engineOilIndicator, " %03d ", 0);

  /* display the rpm in numerical format as well */
  engineOilTitleText = lv_label_create(Screen1);
  lv_obj_align_to(engineOilTitleText, engineOilGauge, LV_ALIGN_CENTER, -18, -110);
  lv_obj_set_style_text_font(engineOilTitleText, &lv_font_montserrat_18, 0);
  lv_label_set_text(engineOilTitleText, "    Oil  \nPressure");

} // end buildoilguage

// temperature guage stuff
#define TEMP_GUAGE_HIEGHT 150
#define TEMP_GUAGE_WIDTH 80
#define ENGINE_TEMP_GUAGE_XOFFSET -50
#define ENGINE_TEMP_GUAGE_YOFFSET 125
#define TEMP_GUAGE_YOFFSET_TXT 15
#define GUAGE_SEPARATION_X 70

static void tempGuageBar_event_cb(lv_event_t * e)
{
    lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
    if(dsc->part != LV_PART_INDICATOR) return;

    lv_obj_t * obj = lv_event_get_target(e);

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.font = &lv_font_montserrat_10;
    label_dsc.font = LV_FONT_DEFAULT;

    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d", (int)lv_bar_get_value(tempGuageBar));

    lv_point_t txt_size;
    lv_txt_get_size(&txt_size, buf, label_dsc.font, label_dsc.letter_space, label_dsc.line_space, LV_COORD_MAX,
                    label_dsc.flag);

    lv_area_t txt_area;
    /*If the indicator is long enough put the text inside on the right*/
    if(lv_area_get_width(dsc->draw_area) > txt_size.x + 20) {
        txt_area.x2 = dsc->draw_area->x2 - 5;
        txt_area.x1 = txt_area.x2 - txt_size.x + 1;
        label_dsc.color = lv_color_white();
    }
    /*If the indicator is still short put the text out of it on the right*/
    else {
        txt_area.x1 = dsc->draw_area->x2 + 5;
        txt_area.x2 = txt_area.x1 + txt_size.x - 1;
        label_dsc.color = lv_color_black();
    }

    txt_area.y1 = dsc->draw_area->y1 + (lv_area_get_height(dsc->draw_area) - txt_size.y) / 2;
    txt_area.y2 = txt_area.y1 + txt_size.y - 1;

    lv_draw_label(dsc->draw_ctx, &label_dsc, &txt_area, buf, NULL);
} // end tempGuageBar_event_cb

// screen list handler - manage screen navigation
static void listEventHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        String selectedItem = String(lv_list_get_btn_text(screenList, obj));
        Serial.printf("Clicked: %s", selectedItem);
        // close sleceted so just close the screen list
        if (selectedItem.equalsIgnoreCase("Close")){
          // hide the list
          lv_obj_add_flag(screenList, LV_OBJ_FLAG_HIDDEN);      
        // screen 1 selected
        } else if (selectedItem.equalsIgnoreCase(SCREEN_1_NAME)){
          // hide the list
          lv_obj_add_flag(screenList, LV_OBJ_FLAG_HIDDEN);
          // activate Engine Screen
          lv_scr_load(Screen1);
          // attach screen list
          lv_obj_set_parent(screenList, Screen1);
        } else if (selectedItem.equalsIgnoreCase(SCREEN_2_NAME)){
          // hide the list
          lv_obj_add_flag(screenList, LV_OBJ_FLAG_HIDDEN);
          // activate screen 2
          lv_scr_load(Screen2);
          // attach screen list
          lv_obj_set_parent(screenList, Screen2);
        } else if (selectedItem.equalsIgnoreCase(SCREEN_3_NAME)){
          // hide the list
          lv_obj_add_flag(screenList, LV_OBJ_FLAG_HIDDEN);
          // activate screen 3
          lv_scr_load(Screen3);
          // attach screen list
          lv_obj_set_parent(screenList, Screen3);
        } else if (selectedItem.equalsIgnoreCase(SCREEN_4_NAME)){
          // hide the list
          lv_obj_add_flag(screenList, LV_OBJ_FLAG_HIDDEN);
          // activate screen 4
          lv_scr_load(Screen4);
          // attach screen list
          lv_obj_set_parent(screenList, Screen4);
        } else if (selectedItem.equalsIgnoreCase(SCREEN_5_NAME)){
          // hide the list
          lv_obj_add_flag(screenList, LV_OBJ_FLAG_HIDDEN);
          // activate screen 5
          lv_scr_load(Screen5);
          // attach screen list
          lv_obj_set_parent(screenList, Screen5);
        } // end if
    } // end if
} // end listEventHandler

// build the vertical guages on the main screen
static void buildTempGuage() {

  // start with base object, turn on border. Everything is within object so can be moved
  tempGuageObject = lv_obj_create(Screen1);
  lv_obj_align_to(tempGuageObject, Screen1, LV_ALIGN_CENTER, ENGINE_TEMP_GUAGE_XOFFSET, ENGINE_TEMP_GUAGE_YOFFSET);
  lv_obj_set_size(tempGuageObject, TEMP_GUAGE_WIDTH, TEMP_GUAGE_HIEGHT);
  lv_obj_add_style(tempGuageObject, &body_style, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_scrollbar_mode(tempGuageObject, LV_SCROLLBAR_MODE_OFF);

  // add animated bar
  lv_obj_t * tempGuageBar = lv_bar_create(tempGuageObject);
  tempGuageBar_shadow = tempGuageBar; // work around since tempGuageBar is getting cloberred

  lv_obj_remove_style_all(tempGuageBar);  /* To have a clean start*/
  lv_obj_add_style(tempGuageBar, &vertBarStyle, 0);
  lv_obj_add_style(tempGuageBar, &vertBarStyleIndic, LV_PART_INDICATOR);
  
  lv_bar_set_range(tempGuageBar, 0, 300);
  lv_obj_set_size(tempGuageBar, 15, TEMP_GUAGE_HIEGHT-30);
  lv_obj_align_to(tempGuageBar, tempGuageObject, LV_ALIGN_BOTTOM_MID, 10, -10);
  lv_bar_set_value(tempGuageBar, 0, LV_ANIM_ON);

  /* some static text on the display */
  tempGuageText = lv_label_create(tempGuageObject);
  lv_obj_align_to(tempGuageText, tempGuageObject, LV_ALIGN_BOTTOM_MID, 36, TEMP_GUAGE_YOFFSET_TXT+1);
  lv_label_set_text(tempGuageText,"°F");

  /* display the rpm in numerical format as well */
  tempGuageIndicator = lv_label_create(tempGuageObject);
  //lv_obj_add_style(tempGuageIndicator, &indicator_style, 0);
  lv_obj_set_style_text_font(tempGuageIndicator, &lv_font_unscii_16, 0);
  lv_obj_align_to(tempGuageIndicator, tempGuageObject, LV_ALIGN_BOTTOM_MID, -20, TEMP_GUAGE_YOFFSET_TXT);
  lv_label_set_text_fmt(tempGuageIndicator, " %03d ", 0);

  /* meter scale */
  tempGuageBarTextLower = lv_label_create(tempGuageObject);
  lv_obj_set_style_text_font(tempGuageBarTextLower, &lv_font_montserrat_14, 0);
  lv_obj_align_to(tempGuageBarTextLower, tempGuageObject, LV_ALIGN_BOTTOM_MID, -4, -10);
  lv_label_set_text(tempGuageBarTextLower,"0");
  
  tempGuageBarTextMid = lv_label_create(tempGuageObject);
  lv_obj_set_style_text_font(tempGuageBarTextMid, &lv_font_montserrat_14, 0);
  lv_obj_align_to(tempGuageBarTextMid, tempGuageObject, LV_ALIGN_BOTTOM_MID, -10, -((ENGINE_OIL_GUAGE_HIEGHT-40)/2)/* -75 */);
  lv_label_set_text(tempGuageBarTextMid,"150");

  tempGuageBarTextUpper = lv_label_create(tempGuageObject);
  lv_obj_set_style_text_font(tempGuageBarTextUpper, &lv_font_montserrat_14, 0);
  lv_obj_align_to(tempGuageBarTextUpper, tempGuageObject, LV_ALIGN_BOTTOM_MID, -10, -(ENGINE_OIL_GUAGE_HIEGHT-30-10));
  lv_label_set_text(tempGuageBarTextUpper,"300");

  /* title for temperature guage */
  tempGuageTitleText = lv_label_create(Screen1);
  //lv_obj_add_style(engineOilIndicator, &indicator_style, 0);
  lv_obj_align_to(tempGuageTitleText, tempGuageObject, LV_ALIGN_CENTER, -100, -55);
  lv_obj_set_style_text_font(tempGuageTitleText, &lv_font_montserrat_18, 0);
  lv_label_set_text(tempGuageTitleText, "Coolant\nTemp.");

  // start with base object, turn on border. Everything is within object so can be moved
  battGuageObject = lv_obj_create(Screen1);
  lv_obj_align_to(battGuageObject, Screen1, LV_ALIGN_CENTER, ENGINE_TEMP_GUAGE_XOFFSET + TEMP_GUAGE_WIDTH + GUAGE_SEPARATION_X, ENGINE_TEMP_GUAGE_YOFFSET);
  lv_obj_set_size(battGuageObject, TEMP_GUAGE_WIDTH, TEMP_GUAGE_HIEGHT);
  lv_obj_add_style(battGuageObject, &smallBorder_style, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_scrollbar_mode(battGuageObject, LV_SCROLLBAR_MODE_OFF);

  // add animated bar
  lv_obj_t * battGuageBar = lv_bar_create(battGuageObject);
  battGuageBar_shadow = battGuageBar; // work around since battGuageBar is getting cloberred
  lv_obj_remove_style_all(battGuageBar);  /*To have a clean start*/
  lv_obj_add_style(battGuageBar, &vertBarStyle, 0);
  lv_obj_add_style(battGuageBar, &vertBarStyleIndic, LV_PART_INDICATOR);

  lv_bar_set_range(battGuageBar, 90, 160);
  //lv_obj_add_event_cb(battGuageBar, battGuageBar_event_cb, LV_EVENT_DRAW_PART_END, NULL);
  lv_obj_set_size(battGuageBar, 15, TEMP_GUAGE_HIEGHT-30);
  lv_obj_align_to(battGuageBar, battGuageObject, LV_ALIGN_BOTTOM_MID, 10, -10);
  lv_bar_set_value(battGuageBar, 105, LV_ANIM_ON);

  /* some static text on the display */
  battGuageText = lv_label_create(battGuageObject);
  lv_obj_align_to(battGuageText, battGuageObject, LV_ALIGN_BOTTOM_MID, 44, TEMP_GUAGE_YOFFSET_TXT+1);
  lv_label_set_text(battGuageText,"V");

  /* display the rpm in numerical format as well */
  battGuageIndicator = lv_label_create(battGuageObject);
  //lv_obj_add_style(battGuageIndicator, &indicator_style, 0);
  lv_obj_set_style_text_font(battGuageIndicator, &lv_font_unscii_16, 0);
  lv_obj_align_to(battGuageIndicator, battGuageObject, LV_ALIGN_BOTTOM_MID, -25, TEMP_GUAGE_YOFFSET_TXT);
  lv_label_set_text_fmt(battGuageIndicator, " %4.1f ", 0);

  
  /* meter scale */
  battGuageBarTextLower = lv_label_create(battGuageObject);
  lv_obj_set_style_text_font(battGuageBarTextLower, &lv_font_montserrat_14, 0);
  lv_obj_align_to(battGuageBarTextLower, battGuageObject, LV_ALIGN_BOTTOM_MID, -5, -10);
  lv_label_set_text(battGuageBarTextLower,"9.0");
  
  battGuageBarTextMid = lv_label_create(battGuageObject);
  lv_obj_set_style_text_font(battGuageBarTextMid, &lv_font_montserrat_14, 0);
  lv_obj_align_to(battGuageBarTextMid, battGuageObject, LV_ALIGN_BOTTOM_MID, -10, -((ENGINE_OIL_GUAGE_HIEGHT-40)/2)/* -75 */);
  lv_label_set_text(battGuageBarTextMid,"12.5");

  battGuageBarTextUpper = lv_label_create(battGuageObject);
  lv_obj_set_style_text_font(battGuageBarTextUpper, &lv_font_montserrat_14, 0);
  lv_obj_align_to(battGuageBarTextUpper, battGuageObject, LV_ALIGN_BOTTOM_MID, -10, -(ENGINE_OIL_GUAGE_HIEGHT-30-10));
  lv_label_set_text(battGuageBarTextUpper,"16.0");

  /* title for temperature guage */
  battGuageTitleText = lv_label_create(Screen1);
  //lv_obj_add_style(engineOilIndicator, &indicator_style, 0);
  lv_obj_align_to(battGuageTitleText, battGuageObject, LV_ALIGN_CENTER, 67, -55);
  lv_obj_set_style_text_font(battGuageTitleText, &lv_font_montserrat_18, 0);
  lv_label_set_text(battGuageTitleText, "Alternator\nVolts");
} // end buildTempGuage

// helper to update the main display including header bar
static void updateMainScreen(lv_timer_t *timer)
{
    // needle is rpm/100 since guage is in 100's of rpm
    // grab a current copy from datatable
    lv_meter_set_indicator_value(engineRpmGauge, engineRpmIndic, (uint32_t)(locEngRPM/100));
    // display engine rpm
    lv_label_set_text_fmt(engineRPMIndicator, " %04d ", (uint32_t)(locEngRPM));
    
    // Update SOG
    lv_label_set_text_fmt(sogIndicator, "%03.1f knts", (float)(locSOG * 1.944));

    // needle is oil pressure  psi
    uint32_t locOilP = (uint32_t)(locEngOilPres/6894.75);
    lv_meter_set_indicator_value(engineOilGauge, engineOilIndic, locOilP);
    // display oil pressure psi
    lv_label_set_text_fmt(engineOilIndicator, " %03d ", locOilP);

    // Engine temp degF
    // (296K − 273.15) × 9/5 + 32
    int32_t locEngTemp = (int32_t)((((locEngCoolTemp-273.15)*(9.0/5.0))+32.0));
    if (tempGuageBar_shadow != NULL) {
      lv_bar_set_value(tempGuageBar_shadow, locEngTemp, LV_ANIM_ON);
      lv_obj_invalidate(tempGuageBar_shadow);
    }
    lv_label_set_text_fmt(tempGuageIndicator, " %03d ", locEngTemp);

    // Alternator Voltage
    int32_t locEngVoltage = (int32_t)(locEngAltVolt * 10);
    if (locEngVoltage <= 90)
      locEngVoltage = 90;
    if (battGuageBar_shadow != NULL) {
      lv_bar_set_value(battGuageBar_shadow, locEngVoltage, LV_ANIM_ON);
      lv_obj_invalidate(battGuageBar_shadow);
    }
    lv_label_set_text_fmt(battGuageIndicator, " %4.1f ", (float)(locEngVoltage/10.0));
    
} // end setvalue

// list to handle screen management
void buildScreenList(lv_obj_t * obj)
{
    /* Create a list */
    //screenList = lv_list_create(lv_scr_act());
    screenList = lv_list_create(obj);
    lv_obj_set_size(screenList, 240, 320);
    lv_obj_center(screenList);
    // hide for now
    lv_obj_add_flag(screenList, LV_OBJ_FLAG_HIDDEN);

    /*Add buttons to the list*/
    lv_obj_t * btn;

    lv_list_add_text(screenList, "Select Screen");
    btn = lv_list_add_btn(screenList, LV_SYMBOL_HOME, SCREEN_1_NAME);
    lv_obj_add_event_cb(btn, listEventHandler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(screenList, LV_SYMBOL_IMAGE, SCREEN_2_NAME);
    lv_obj_add_event_cb(btn, listEventHandler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(screenList, LV_SYMBOL_IMAGE, SCREEN_3_NAME);
    lv_obj_add_event_cb(btn, listEventHandler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(screenList, LV_SYMBOL_IMAGE, SCREEN_4_NAME);
    lv_obj_add_event_cb(btn, listEventHandler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(screenList, LV_SYMBOL_SETTINGS, SCREEN_5_NAME);
    lv_obj_add_event_cb(btn, listEventHandler, LV_EVENT_CLICKED, NULL);

    lv_list_add_text(screenList, "Exit");
    btn = lv_list_add_btn(screenList, LV_SYMBOL_CLOSE, "Close");
    lv_obj_add_event_cb(btn, listEventHandler, LV_EVENT_CLICKED, NULL);
} // end build 


// old stuff
#if 0

void page_1()
{
  String temp = "";
  gfx->fillScreen(ColorArray[((unsigned)counter / 2 % COLOR_NUM)]);
  gfx->setTextSize(4);
  gfx->setTextColor(BLACK);
  gfx->setCursor(120, 100);
  gfx->println(F("Makerfabs"));

  gfx->setTextSize(3);
  gfx->setCursor(30, 160);
  gfx->println(F("2.1inch TFT with Touch "));

  gfx->setTextSize(4);
  gfx->setCursor(60, 200);
  temp = temp + "Encoder: " + counter;
  gfx->println(temp);

  gfx->setTextSize(4);
  gfx->setCursor(60, 240);
  temp = "";
  temp = temp + "BUTTON: " + button_flag;
  gfx->println(temp);

  gfx->setTextSize(4);
  gfx->setCursor(60, 280);
  temp = "";
  temp = temp + "Touch X: " + x;
  gfx->println(temp);

  gfx->setTextSize(4);
  gfx->setCursor(60, 320);
  temp = "";
  temp = temp + "Touch Y: " + y;
  gfx->println(temp);

  // gfx->fillRect(240, 400, 30, 30, ColorArray[(((unsigned)counter / 2 + 1) % COLOR_NUM)]);

  flesh_flag = 0;
} // end page_1

void test()
{
    while (1)
    {
        // if (wifi_flag != 1)
        //     if (WiFi.status() == WL_CONNECTED)
        //     {
        //         USBSerial.println(WiFi.localIP());
        //         wifi_flag = 1;
        //         flesh_flag = 1;
        //     }

        if (read_touch(&x, &y) == 1)
        {
            USBSerial.print("Touch ");
            USBSerial.print(x);
            USBSerial.print("\t");
            USBSerial.println(y);

            flesh_flag = 1;
        }

        if (digitalRead(BUTTON_PIN) == 0)
        {

            USBSerial.println("Button Press");
            delay(100);
            if (digitalRead(BUTTON_PIN) == 0)
                return;
        }

        if (move_flag == 1)
        {
            USBSerial.print("Position: ");
            USBSerial.println(counter);
            move_flag = 0;
            flesh_flag = 1;
        }
        if (flesh_flag == 1)
            page_1();

        delay(100);
    }
    return;
} // end test
#endif
