/**
    @author Yuske Goto
    Also thanks a lot for my old college Jochen for cleaning my code-chaos. Even though I'm eternaly C noob, I couldn't make it without your support
*/

#ifndef _ROBOT_ROOMBA_H_
#define _ROBOT_ROOMBA_H_

#include <Arduino.h>
#include "oi.h"
#include "def.h"

#define STREAM_PACKET 19
#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))
#define SERIAL_RETRY_COUNT 4

class Roomba{

  public:
    bool sleeping, deviationLogging, interacting;
    float x, y, direction, distance;
    uint8_t oiMode;

    Roomba(Stream *s, int8_t b);
    bool setup(void);
    bool roboInitSequence(void);
    void reset(void);
    void stop(void);
    void wakeUp(void);
    void sleep(void);
    void keepSleeping(void);
    void drive(int16_t velocity, int16_t radius);
    void driveDirect(int16_t rightVelocity, int16_t leftVelocity);
    void toggleCleaning(bool);
    void defineBeep(void);
    void playBeep(uint8_t beep);
    void startStream(void);
    int16_t * getStream();
    void stopStream();
    void setBaud(uint8_t baud_code);
    void getXY(int16_t encoderL, int16_t encoderR);

    bool checkAlive(void);
    void set19200(void);

  private:
    Stream    *stream;      // serial to Roomba

    void encInitialize(void);
    bool roboInit(void);

    bool chargerInternalFlag;
    int8_t brc;
    int16_t encL;
    int16_t encR;
    uint8_t dataLength[STREAM_PACKET] = {1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1};  //table of data length
    uint8_t streamLength;

};

#endif
