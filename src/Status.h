/**
    @author Yuske Goto
    Also thanks a lot for my old college Jochen for cleaning my code-chaos. Even though I'm eternaly C noob, I couldn't make it without your support
*/


#ifndef _ROBOT_STATUS_H_
#define _ROBOT_STATUS_H_

// #include <M5Stack.h>
#include <Arduino.h>
#include "Roomba.h"
#include "utils/Debug.h"

#include "def.h"


struct SensorData {
    // SensorData() : bumpR(false), bumpL(false), vWall(false),wheelDropR(false), wheelDropL(false), overCurrent(false), charger(false), batCritical(false), batLow(false), locked(false), buttonA_pressed(false), buttonB_pressed(false), buttonC_pressed(false){}

    // bool bumpR, bumpL, vWall, charger, wheelDropR, wheelDropL, overCurrent, batCritical, batLow, roombaLost, locked, buttonA_pressed, buttonB_pressed, buttonC_pressed;

    // //Roomba data
    // // Switch state of front bumper switch. L and R are avaliable
    // public:
    bool bumpR, bumpL;
    
    // // Enabled when infra-red virtual wall signal is detected
    // // Virtual wall and charger emit different signal, so there are two variables here: vWall and charger
    bool vWall, charger;

    // // Wheel drop detection
    bool wheelDropR, wheelDropL;
    // // Machine over current
    bool overCurrent;

    // // Battery level warning
    bool batCritical, batLow;
    // // Roomba condition alert
    bool roombaLost, locked;

    // IR sensor state of on the front bumper, there are 6 sensors installed on front bumper
    uint16_t lBumpL, lBumpR, lBumpFL, lBumpFR, lBumpCL, lBumpCR;
    uint8_t batteryLevel, cliff;

    // M5 button A - C status
    bool buttonA_pressed, buttonB_pressed, buttonC_pressed;

    SensorData(){
        bumpR= bumpL= vWall= wheelDropR= wheelDropL= overCurrent= charger= batCritical= batLow= locked= buttonA_pressed= buttonB_pressed= buttonC_pressed = false;
    }
};

    struct Motor {
        Motor() : action(false), left(0), right(0), duration(0), competenceLevel(-1){}
        bool action;
        int16_t left;
        int16_t right;
        uint16_t duration;
        uint8_t competenceLevel;
    };

class Status {

    public:
        Status();
        // Status(Roomba *r);
        SensorData read(void);


    private:
        // Roomba *roomba;
        void readPins(SensorData*);
        void readSensorBoard(SensorData*);
        void readRoomba(SensorData*);
        void readApp(SensorData*);
        float directionAdjust(uint8_t);
        void readRoombaStream(SensorData*);

        uint32_t signalLostTimer = 0;
        bool signalLostCounting = false;
        bool wheelDropCountR = false;
        uint32_t wheelDropTimerR = 0;
        bool wheelDropCountL = false;
        uint32_t wheelDropTimerL = 0;

        int16_t roombaSensorBackUp[19];

};

#endif