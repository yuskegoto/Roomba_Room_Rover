/**
 * @author Yuske Goto
 * Also thanks a lot for my old college Jochen for cleaning my code-chaos. Even though I'm eternaly C noob, I couldn't make it without your support
 */


#include "Status.h"
extern Debug debug;
extern Roomba roomba;

Status::Status() {
    // init condition
    pinMode(BUTTON_PIN, INPUT);
}

/**
 * read sensor data from all sources (board, roomba)
 */
SensorData Status::read(void) {
    SensorData data;
    readPins(&data);
    readRoombaStream(&data);
    return data;
}

void Status::readPins(SensorData* data) {
  if(digitalRead(BUTTON_PIN) == LOW) data->button_pressed = true;
  else data->button_pressed = false;
}

void Status::readRoombaStream(SensorData* data) {

  int16_t * roombaSensors = roomba.getStream();

  //data evaluation
  if((* roombaSensors & 0xFF00) == 0){
    data->roombaLost = false;
    signalLostCounting = false;

    //light bumper limiter
    for(uint8_t i = 0; i < 6; i++){
    if(*(roombaSensors + 5 + i) > 4095){
      if(roombaSensorBackUp[i] < 4095){
        *(roombaSensors + 5 + i) = roombaSensorBackUp[i];
      }
      else *(roombaSensors + 5 + i) = 0;
    }
    if(roombaSensorBackUp[i] < 10 && *(roombaSensors + 5 + i) < 1000){
        *(roombaSensors + 5 + i) = roombaSensorBackUp[i];
      }
    }

    //back up sensor data
    for(uint8_t i = 0; i < 18; i++){
      roombaSensorBackUp[i] = *(roombaSensors + i);
    }

  }
  //if roomba doesn't answer for certain times, we regard the communication is lost
  else{
    if(signalLostCounting && millis() - signalLostTimer > ROOMBA_LOST_LIMIT){
      data->roombaLost = true;
      //debug.print(F("lost roomba "),DEBUG_GENERAL);
      signalLostCounting = false;
    }
    else{
      if(!signalLostCounting && !data->roombaLost){
        //debug.print(F(" lost counting "),DEBUG_GENERAL);
        data->roombaLost = false;
        signalLostCounting = true;
        signalLostTimer = millis();
      }
    }

    //retreave sensor values
    *roombaSensors = *roombaSensorBackUp;

  }

  //sensor value 0: |15-8 siglnal lost count |3 drop left |2 drop right |1 bump left |0 bump right
  //1: |0 virtual wall
  //2: |4 over current left |3 over current right
  //3: |1 home base station
  //4: OIMode
  //5 - 10: light bumper values
  data->bumpL = (*roombaSensors & B10);
  data->bumpR = (*roombaSensors & B1);
  data->vWall = (*(roombaSensors + 1) & B1);
  data->charger = (*(roombaSensors + 3) & B10);

  if(!roomba.sleeping){
    //over current check, only if roomba is not on charge station,
    //since it may produce wrong overcurrent signal during charge
    data->overCurrent = (*(roombaSensors + 2) & B10000) | (*(roombaSensors + 2) & B1000);

    //wheel drop errors    
    // if((*roombaSensors & B100)>>2 == 0x01) data->wheelDropR = true;
    if((*roombaSensors & B100) >> 2 == 0x01){
      if(!wheelDropCountR){
          wheelDropTimerR = millis();
          wheelDropCountR = true;
      }
      if(millis() - wheelDropTimerR > WHEELDROP_TIMER){
        data->wheelDropR = true;
      }
    }
    else{
      data->wheelDropR = false;
      wheelDropCountR = false;
    }

    // detecting whell drop left
    // if((*roombaSensors & B1000)>>3 == 0x01) data->wheelDropL = true;
    if((*roombaSensors & B1000) >> 3 == 0x01){
      if(!wheelDropCountL){
          wheelDropTimerL = millis();
          wheelDropCountL = true;
      }
      if(millis() - wheelDropTimerL > WHEELDROP_TIMER){
        data->wheelDropL = true;
      }
    }
    else{
      data->wheelDropL = false;
      wheelDropCountL = false;
    }
  }

  data->lBumpL = *(roombaSensors + 5);
  data->lBumpFL = *(roombaSensors + 6);
  data->lBumpCL = *(roombaSensors + 7);
  data->lBumpCR = *(roombaSensors + 8);
  data->lBumpFR = *(roombaSensors + 9);
  data->lBumpR = *(roombaSensors + 10);

  uint16_t batLevel = ((float) *(roombaSensors + 11)) / ((float) *(roombaSensors + 12)) * 255;

  int16_t encoderL = *(roombaSensors + 13);
  int16_t encoderR = *(roombaSensors + 14);

  data->cliff = *(roombaSensors + 15);
  data->cliff += *(roombaSensors + 16);
  data->cliff += *(roombaSensors + 17);
  data->cliff += *(roombaSensors + 18);

  //max battery capacity: 255
  //estimated max battery capacity: 2696mAh
//  float batCap = (float) *(roombaSensors + 11);
//  float batFullCap = (float) *(roombaSensors + 12);

  if(batLevel > 0xFF || batLevel == 0) batLevel = 0xFF;

  data->batteryLevel = (uint8_t) batLevel;
  if(!roomba.sleeping || !data->charger){
    if(data->batteryLevel < BATTERY_LEVEL_CRITICAL){
      data->batCritical = true;
    }
    else if(data->batteryLevel < BATTERY_LEVEL_LOW){
      data->batLow = true;
    }
    else {
      data->batCritical = false;
      data->batLow = false;
    }
  }

  roomba.getXY(encoderL, encoderR);
}

