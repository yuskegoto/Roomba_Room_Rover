/**
    @author Yuske Goto
    Also thanks a lot for my old college Jochen for cleaning my code-chaos. Even though I'm eternaly C noob, I couldn't make it without your support
*/

#include <math.h>
#include <Arduino.h>

#include "Debug.h"
#include "Status.h"
#include "def.h"

#define DEBUG
//#define PROCESSING_TRACKING

bool angle_logged = false;
uint8_t azimuth_history;
int16_t angle_history;

// extern Roomba roomba;

Debug::Debug(Stream *s, uint8_t debuglevel):stream(s)
{
    outputLevel = debuglevel;
    // outputLevel |= DEBUG_GENERAL;
    //outputLevel |= DEBUG_CONTROL;
    //outputLevel |= DEBUG_SENSOR;
    //outputLevel |= DEBUG_ACTION;
    //outputLevel |= DEBUG_MOVE;
    //outputLevel |= DEBUG_FENCING;
    //outputLevel |= DEBUG_APP;
}

int Debug::getFreeRam()
{
    extern int __heap_start, *__brkval;
    int v;

    v = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);

    #ifdef DEBUG
        if ( DEBUG_GENERAL & outputLevel ){
            stream->print(F("Free RAM = "));
            stream->println(v, DEC);

        }
    #endif
    return v;
}
size_t Debug::printf(uint8_t level, const char *format, ...)
{
    int len = 0;
#ifdef DEBUG
    if (level & outputLevel)
    {
        char loc_buf[64];
        char *temp = loc_buf;
        va_list arg;
        va_list copy;
        va_start(arg, format);
        va_copy(copy, arg);
        len = vsnprintf(temp, sizeof(loc_buf), format, copy);
        va_end(copy);
        if (len < 0)
        {
            va_end(arg);
            return 0;
        };
        if (len >= sizeof(loc_buf))
        {
            temp = (char *)malloc(len + 1);
            if (temp == NULL)
            {
                va_end(arg);
                return 0;
            }
            len = vsnprintf(temp, len + 1, format, arg);
        }
        va_end(arg);
        // len = write((uint8_t *)temp, len);
        stream->print(temp);
        if (temp != loc_buf)
        {
            free(temp);
        }
    }
#endif
    return len;
}

void Debug::print(uint8_t val, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel) {
            stream->print(val);
        }
    #endif
}

void Debug::println(uint8_t val, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel) {
            stream->println(val);
        }
    #endif
}

void Debug::print(int8_t val, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel) {
            stream->print(val);
        }
    #endif
}

void Debug::println(int8_t val, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel) {
            stream->println(val);
        }
    #endif
}

void Debug::print(int16_t val, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel) {
            stream->print(val);
        }
    #endif
}

void Debug::println(int16_t val, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel) {
            stream->println(val);
        }
    #endif
}

void Debug::print(uint16_t val, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel) {
            stream->print(val);
        }
    #endif
}

void Debug::println(uint16_t val, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel) {
            stream->println(val);
        }
    #endif
}

void Debug::println(const __FlashStringHelper* string, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel ){
            stream->println(string);
        }
    #endif
}

void Debug::print(const __FlashStringHelper* string, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel ){
            stream->print(string);
        }
    #endif
}

void Debug::println(String string, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel ){
            stream->println(string);
        }
    #endif
}

void Debug::print(String string, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel ){
            stream->print(string);
        }
    #endif
}

void Debug::println(const char* string, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel ){
            stream->println(string);
        }
    #endif
}

void Debug::print(const char* string, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel ){
            stream->print(string);
        }
    #endif
}

void Debug::print(char string, uint8_t level) {
    #ifdef DEBUG
        if ( level & outputLevel ){
            stream->print(string);
        }
    #endif
}

void Debug::ledOn(){
    #ifdef DEBUG
    // pinMode(LED_BUILT_IN, OUTPUT);
    // digitalWrite(LED_BUILT_IN, HIGH);
    #endif
}

void Debug::ledOff(){
    #ifdef DEBUG
    // pinMode(LED_BUILT_IN, OUTPUT);
    // digitalWrite(LED_BUILT_IN, LOW);
    #endif
}

// void Debug::print(SensorData *data) {
//     #ifdef DEBUG
//         if ( DEBUG_SENSOR & outputLevel ){
//             stream->print(F("LB (L|FL|CL|CR|FR|R): "));
//             stream->print(data->lBumpL);
//             stream->print(F(" "));
//             stream->print(data->lBumpFL);
//             stream->print(F(" "));
//             stream->print(data->lBumpCL);
//             stream->print(F(" "));
//             stream->print(data->lBumpCR);
//             stream->print(F(" "));
//             stream->print(data->lBumpFR);
//             stream->print(F(" "));
//             stream->print(data->lBumpR);
//             stream->print(F("  "));
//         }

//         if (DEBUG_GENERAL & outputLevel ){
//           stream->print(F(" Loop:"));
//           stream->print(millis() - loopTimer);
//           stream->print(F(" "));
//           loopTimer = millis();
//         }
//         stream->println(F(""));
//     #endif
// }

// void Debug::print(Motor *data) {
//     #ifdef DEBUG
//         if ( DEBUG_MOVE & outputLevel ){
//             char output[130];
//             sprintf(output, "Move (R|L):\t%d\t%d", data->right, data->left);
//             stream->println(output);
//         }
//     #endif
// }

// void Debug::writeXY(float x, float y, float ang, float dist){
//   #ifdef DEBUG
//     if(DEBUG_FENCING & outputLevel){
//       //char output[4];
//       int16_t x16 = (int16_t) x;
//       int16_t y16 = (int16_t) y;
//       int16_t ang16 = (int16_t) (ang * 180 /3.1415);
//       int16_t extraVal = 0;
//     }
//   #endif
// }

#define PACKET_LENGTH 24
