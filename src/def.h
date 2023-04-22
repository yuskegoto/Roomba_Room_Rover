#include "utils/Debug.h"

////////////////////// System /////////////////////////
#define UPDATE_RATE_ms 100

////////////////////// Debug Config ///////////////////
#define MONITOR_BAUD 115200
#define DEBUG_LEVEL DEBUG_GENERAL | DEBUG_CONTROL | DEBUG_SPIFFS | DEBUG_WIFI
// #define DEBUG_ONLY_WEB
// #define ENABLE_DNS
// #define ENABLE_WIFI_AP

////////////////////// Pin config ///////////////////
// M5-Roomba module pins
// #define LED_BUILT_IN 5
// #define ROOMBA_BRC 26    // Baud Rate Change Pin
// And Serial2 RX: 16, TX:17 are taken for the unit

//////////////////////Sensing set up/////////////////////////
#define WHEELDROP_TIMER 10000
#define BATTERY_LEVEL_CRITICAL 128
#define BATTERY_LEVEL_LOW 160
#define ROOMBA_LOST_LIMIT 60000

////////////////// Roomba set up //////////////////////////
#define MOTOR_SPEED_FACTOR 3.5f
#define MOTOR_SPEED_STEP 20
#define SPEED_LIMIT 100
#define DISTANCE_LIMIT 10000

#define VACUUM_ON_THRESHOLD 30

////////////////// AP set up //////////////////////////
#define ACCESSPOINT_IP1 192
#define ACCESSPOINT_IP2 168
#define ACCESSPOINT_IP3 4
#define ACCESSPOINT_IP4 1