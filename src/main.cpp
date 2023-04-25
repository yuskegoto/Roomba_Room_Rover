#include <Arduino.h>

#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#ifdef ENABLE_WIFI_AP
#include <WiFiAP.h>
#endif

#include "utils/Debug.h"
#include "Roomba.h"
#include "Status.h"
#include "Action.h"

// Project config
#include "def.h"

Debug debug(&Serial, DEBUG_LEVEL);

Roomba roomba(&Serial2, 0);
Status status;
Action action;

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15 
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
// #define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
// #define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
// #define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD
#define CAMERA_M5_TIMER_CAM
#include "camera_pins.h"

// ===========================
// Enter your WiFi credentials
// ===========================
#ifdef ENABLE_WIFI_AP
const char *SSID = "rbcam";
IPAddress apIP(192, 168, 4, 1);
#else
const char *ssid = "LAB3";
const char *password = "aabbccddeeff";
#endif

#define RXD2 Ext_PIN_1
#define TXD2 Ext_PIN_2

void startCameraServer();

#define USER_BTN_PIN 37

void setup() {
    Serial.begin(MONITOR_BAUD);

    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable   detector
    // bat_init();
    // bat_hold_output(); // If bat hold is high, you can't use USER_SWC button
    Serial.setDebugOutput(true);
    Serial.println();
    pinMode(CAMERA_LED_GPIO, OUTPUT);
    debug.ledOff();

    pinMode(USER_BTN_PIN, INPUT); // GPIO37 does not have pull up circuitry, it is pulled up internally to VBAT_IN

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG; // for streaming
    //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 4;//12;
    config.fb_count = 1;

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
    //                      for larger pre-allocated frame buffer.
    if(config.pixel_format == PIXFORMAT_JPEG){
    if(psramFound()){
        config.jpeg_quality = 4;//10;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
        // Limit the frame size when PSRAM is not available
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
    }
    } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
    #if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
    #endif
    }

    #if defined(CAMERA_MODEL_ESP_EYE)
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
    #endif

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
    }

    sensor_t * s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
    // s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
    }
    // drop down frame size for higher initial frame rate
    if(config.pixel_format == PIXFORMAT_JPEG){
    s->set_framesize(s, FRAMESIZE_QVGA);
    }

    // flip camera orientation
    s->set_vflip(s, 0);
    s->set_hmirror(s, 1);

#ifdef ENABLE_WIFI_AP
    WiFi.disconnect();   // added to start with the wifi off, avoid crashing
    WiFi.mode(WIFI_OFF); // added to start with the wifi off, avoid crashing
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(SSID);
    #else
    WiFi.begin(ssid, password);
    WiFi.setSleep(false);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    debug.printf(DEBUG_WIFI, ".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    #endif

    startCameraServer();

    #ifdef ENABLE_WIFI_AP
    debug.printf(DEBUG_WIFI, "Camera Ready! Use 'http://%s' to connect\n", WiFi.softAPIP().toString().c_str());
    #else
    debug.printf(DEBUG_WIFI, "Camera Ready! Use 'http://%s' to connect\n", WiFi.localIP().toString().c_str());
    #endif

    // Init Roomba
    debug.println(F("Roomba Camera"), DEBUG_GENERAL);

    // delay(100);
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);


    uint8_t retryCount = 0;
    while (!roomba.roboInitSequence())
    {
    debug.printf(DEBUG_GENERAL, "Retry sequence:%d\n", retryCount++);
    delay(1000);
    }
}

void loop() {
    // SensorData data;

    // Check machine state
    // data = status.read();

    if (action.updated)
    {
        debug.ledOff();
        roomba.driveDirect(action.motorR, action.motorL);

        roomba.toggleCleaning(action.cleaning);
        action.updated = false;
    }
    else{
        debug.ledOn();
    }

    // if (data.button_pressed)
    if (digitalRead(USER_BTN_PIN) == LOW)
    {
        debug.printf(DEBUG_GENERAL, "Button pressed\n");
        delay(1000);
        if (digitalRead(USER_BTN_PIN) == LOW){
            ESP.restart();
        }
    }

    delay(UPDATE_RATE_ms);
}
