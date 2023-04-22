#ifndef _ROBOT_ACTION_H_
#define _ROBOT_ACTION_H_

#include <Arduino.h>

class Action{
  public:
    Action():updated(false), cleaning(false), motorR(0), motorL(0){}
    bool updated;
    bool cleaning;
    int16_t motorR;
    int16_t motorL;

    void forward(void);
    void back(void);
    void stop(void);
    void turnRight(void);
    void turnLeft(void);
    void setXY2Speed(int x, int y);
    void toggleCleaningMotors(void);
    void stopCleaningMotors(void);
};

#endif