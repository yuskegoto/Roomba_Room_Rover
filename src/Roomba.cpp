/**
    This code is based on Belvedere serving robot by wolffan
    http://www.instructables.com/id/Belvedere-A-Butler-Robot/

    @author Yuske Goto
    Also thanks a lot for my old college Jochen for cleaning my code-chaos. Even though I'm eternaly C noob, I couldn't make it without your support
*/

#include <Arduino.h>

#include "oi.h"
#include "Roomba.h"
#include "utils/Debug.h"
#include "def.h"

extern Debug debug;

Roomba::Roomba(Stream *s, int8_t b):stream(s), brc(b)
{
    sleeping = false;
    encL = 0;
    encR = 0;
    x = 0;
    y = 0;
    direction = 0;
    deviationLogging = false;
    oiMode = 0;
    interacting = false;
    chargerInternalFlag = false;
    streamLength = 0;
}
//Baudrate setter over UART communication.
//baud code 7: 19200, 5: 9600
void Roomba::setBaud(uint8_t baud_code)
{
  stream->write(CmdBaud);
  stream->write(baud_code);
  delay(100);
}

//change baud rate from 115200 to 19200 using BRC (Baud Rate Change) pin, so that arduino won't overwhelmed.
// void Roomba::set19200(void){
//   for (uint8_t i = 0; i <= 3; i++){
//     digitalWrite(brc,HIGH);
//     debug.ledOn();
//     delay(50);
//     digitalWrite(brc,LOW);
//     debug.ledOff();
//     delay(50);
//   }
// }

void Roomba::reset(void){
  stream->write(CmdReset);
}

void Roomba::stop(void){
  stream->write(CmdStart);
  stream->write(CmdSafe);

  drive(0, RadStraight);
  //stream->write(CmdPower);
  stream->write(CmdStop);
  //reset();
}

void Roomba::sleep(void){
    debug.println(F("I gonna sleep..."), DEBUG_ACTION);
    stop();
    sleeping = true;
}

// void Roomba::keepSleeping(void){
//   //if roomba is out of control, try to get it to safe mode again
//   if(oiMode == 0xFF){
//     for(uint8_t i = 0; i < 3; i++){
//       digitalWrite(brc,LOW);
//       debug.ledOff();
//       delay(100);
//       digitalWrite(brc,HIGH);
//       debug.ledOn();
//       delay(100);
//       digitalWrite(brc,LOW);

//       debug.ledOff();
//     }
//   }

//   stream->write(CmdStart);
//   stream->write(CmdSafe);

//   //stop uncontrolled driving
//   stream->write(CmdDriveWheels);
//   uint8_t spd = 0;
//   stream->write(spd);
//   stream->write(spd);
//   stream->write(spd);
//   stream->write(spd);

//   stream->write(CmdStop);
// }

// void Roomba::wakeUp(void){
//     debug.println(F("Wake up!"), DEBUG_ACTION);
//     //wake roomba from passive mode
//     sleeping = false;

//     digitalWrite(brc,HIGH);
//     debug.ledOn();
//     delay(100);
//     digitalWrite(brc,LOW);
//     debug.ledOff();
//     delay(100);
//     digitalWrite(brc,HIGH);
//     debug.ledOn();
//     delay(100);
//     digitalWrite(brc,LOW);
//     debug.ledOff();

//     stream->write(CmdStart);
//     delay(200);
//     stream->write(CmdSafe);
//     drive(0, RadStraight);

// }

//load encoder values
void Roomba::encInitialize(void){
  int16_t pkt[4] = {};

  stream->write(CmdSensorList);
  stream->write(2);
  stream->write(PktLeftEnc);
  stream->write(PktRightEnc);
  delay(100);

  while(stream->available() > 0){
    for(uint8_t i = 0; i<4; i++){
      pkt[i] = stream->read();
      }
      stream->read();
    }

  encL = pkt[0] << 8 | pkt[1];
  encR = pkt[2] << 8 | pkt[3];
}

void Roomba::startStream(void){
  stream->write(CmdStream);
  stream->write(STREAM_PACKET);
  stream->write(PktBumpWall); //1
  stream->write(PktVirtualWall);  //1
  stream->write(PktOverCurrent);  //1
  stream->write(PktCharger);  //1
  stream->write(PktOIMode);// 1

  stream->write(PktLBumperL);//2
  stream->write(PktLBumperFL);//2
  stream->write(PktLBumperCL);//2
  stream->write(PktLBumperCR);//2
  stream->write(PktLBumperFR);//2
  stream->write(PktLBumperR);//2

  stream->write(PktBatLevel); //2
  stream->write(PktBatCapacity); //2

  stream->write(PktLeftEnc); //2
  stream->write(PktRightEnc); //2

  stream->write(PktCliffL); //1
  stream->write(PktCliffFL); //1
  stream->write(PktCliffFR); //1
  stream->write(PktCliffR); //1

  for(uint8_t i = 0; i < ARRAY_SIZE(dataLength); i ++){
    streamLength += dataLength[i] + 1;
  }
}

int16_t * Roomba::getStream(){
  static int16_t values[ARRAY_SIZE(dataLength)] = {};
  values[4] = 0xFFFF;  //OI Mode, this value should overwritten if roomba answers

  bool streamRead = false;
  uint8_t buf[(streamLength + 3)];
  uint8_t bufAligned[(streamLength + 3)];
  uint8_t bufferSize = ARRAY_SIZE(buf);
  uint16_t checkSum = 0;
  uint8_t loop = 0;

  if(stream->available() > 0){
    while(!streamRead && loop < 16){
      for(uint8_t i = 0; i < bufferSize; i++){
        buf[i] = stream->read();
      }

      for(uint8_t i = 0; i < bufferSize-2; i++){

        if(buf[i] ==19){ // 19 is header of stream
          //check whether it's not the last byte of buffer
          if(buf[i + 1] == streamLength){ // next packet is stream length
              uint8_t pointer = i;
              for(uint8_t j = 0; j < bufferSize; j++){
                if(pointer + j < bufferSize){
                  bufAligned[j] = buf[pointer + j];
                }
                else{//start from zero
                  bufAligned[j] = buf[pointer + j - bufferSize];
                }
              }
              checkSum = 19 + streamLength;
            }

            for(uint8_t j = 0; j < streamLength; j ++){
              checkSum += bufAligned[2 + j];
            }
              //this is check sum
              checkSum += bufAligned[2 + streamLength];

              //Lower byte of checksum must be zero...
              if((checkSum & 0xFF) == 0){
                uint8_t k = 1;
                for(uint8_t j = 0; j < ARRAY_SIZE(dataLength); j ++){
                  if(dataLength[j] == 1){
                    values[j] = bufAligned[2 + k];
                    k += 2;
                  }
                  //in the case of two byte packet...
                  else if(dataLength[j] == 2){
                    values[j] = bufAligned[2 + k] << 8 | bufAligned[3 + k];
                    k += 3;
                  }
                }
                streamRead = true;
                i = bufferSize;
                break;
              }
            }
          }
      loop ++;
    }
  }

  if(!streamRead){
    values[0] = 0xFF00;
    oiMode = 0xFF;
  }
  else oiMode = values[4];
  //check if roomba is on charger
  if(values[3] == B10){
    chargerInternalFlag = true;
  }
  else if(chargerInternalFlag && (values[3] != 0)){
    values[3] = B10;
    chargerInternalFlag = true;
  }
  else if(values[3] == 0) chargerInternalFlag = false;
  //if there is no answer (= OI Mode has wrong value) and roomba is not charging
  if((values[4] == 0xFFFF) && !chargerInternalFlag){
    values[0] = 0xFF00;
  }

return values;
}

void Roomba::stopStream(){
  uint8_t streamPause = 0;
  stream->write(CmdStreamPause);
  stream->write(streamPause); //write zero to pause data stream
}

bool Roomba::checkAlive(void){
  bool roboAlive = false;
  uint8_t check[3] = {};

  stream->write(CmdStart);
  delay(100);   //This is just for show blink of power button
  //stream->write(CmdSafe);
  stopStream();

  //flush all serial buffer from roomba
  while(stream->available() > 0)
  {
    stream->read();
  }
  //flush serial buffer of arduino
  stream->flush();
  delay(100);

  stream->write(CmdSensors);
  stream->write(PktBatCapacity);
  delay(50);
  while(stream->available() > 0){
    for(uint8_t i = 0; i < 2; i ++){
      check[i] = stream->read();
    }
  }

  stream->write(CmdSensors);
  stream->write(PktOIMode);
  delay(50);
  while(stream->available() > 0){
    check[2] = stream->read();
  }

  for(uint8_t i = 0; i < 3; i ++){
      debug.print(check[i], DEBUG_GENERAL);
      debug.print(F(" "), DEBUG_GENERAL);
  }

  if(check[0] != B0 && check[1] != B0 && check[2] < 4){
  debug.println(F(" Robo answered "), DEBUG_GENERAL);
    roboAlive = true;
  }
  else debug.println(F(" No answer... "), DEBUG_GENERAL);

  return roboAlive;
}

bool Roomba::setup(void){
  bool start = false;

  // pinMode(brc, OUTPUT);
  // pinMode(LED_BUILT_IN, OUTPUT);
  // digitalWrite(brc, LOW);
  // debug.ledOff();

  if(checkAlive()){
    start = true;

    defineBeep();
    encInitialize();

    //flush all serial buffer from roomba
    while(stream->available() > 0)
    {
      stream->read();
    }
    //flush serial buffer of arduino
    stream->flush();
    delay(10);
  }
  return start;
}

// Send Create drive commands in terms of velocity and radius
void Roomba::drive(int16_t vel, int16_t rad){
  if(!sleeping){
    stream->write(CmdStart);
    stream->write(CmdSafe);

    stream->write(CmdDrive);
    stream->write((uint8_t)((vel >> 8) & 0x00FF));
    stream->write((uint8_t)(vel & 0x00FF));
    stream->write((uint8_t)((rad >> 8) & 0x00FF));
    stream->write((uint8_t)(rad & 0x00FF));
  }
}

//speed should more than 11mm/s
void Roomba::driveDirect(int16_t rightVelocity, int16_t leftVelocity){
  if(!sleeping){
    stream->write(CmdStart);
    stream->write(CmdSafe);

    stream->write(CmdDriveWheels);
    stream->write((uint8_t)((rightVelocity >> 8) & 0x00FF));
    stream->write((uint8_t)(rightVelocity & 0x00FF));
    stream->write((uint8_t)((leftVelocity >> 8) & 0x00FF));
    stream->write((uint8_t)(leftVelocity & 0x00FF));
  }
}

void Roomba::toggleCleaning(bool state){
  if(!sleeping){
    uint8_t cmd = 0;
    if (state){
      cmd = SideBrushOn | MainBrushOn | VacuumOn;
    }

    stream->write(CmdStart);
    stream->write(CmdSafe);

    stream->write(CmdCleanMotors);
    stream->write(cmd);
  }
}

bool Roomba::roboInit(){
  bool started = setup();
  playBeep(START_BEEP);
  driveDirect(0, 0);
  return started;
}

bool Roomba::roboInitSequence(){
  bool roboStart = false;

  // setBaud(Baud115200);
  // Serial2.flush();
  // delay(50);
  // Serial2.end();
  // delay(50);
  // Serial2.begin(115200);
  // delay(50);

  if(!roboInit()){
    for (uint8_t i = 1; i <= SERIAL_RETRY_COUNT; i++)
    {
      // while(!roboInit()){
      debug.printf(DEBUG_GENERAL, "retry:%d\n", i);
      // M5.Lcd.printf("retry:%d\n", i);
        // delay(1000);
        reset();
        delay(1000);
        if(roboInit()){
          roboStart = true;
          break;
      }
    }

    // //if roomba doesn't answer, try to communicate on 19200 bps
    // if(!roboStart){
    //   debug.println(F("try to connect robo in 19200 bps..."), DEBUG_CONTROL);
    //   // M5.Lcd.println(F("Connect robo in 19200 bps..."));
    //   // Serial2.flush();
    //   delay(100);
    //   // Serial2.end();
    //   delay(100);
    // //   Serial2.begin(19200);
    // //   delay(100);
    // }

    // for (uint8_t k = 1; k <= SERIAL_RETRY_COUNT; k++)
    // {
    //   debug.printf(DEBUG_GENERAL, "Baud change: %d\n", k);
    //   // M5.Lcd.printf("Baud change:%d\n", k);
    //   // setBaud(Baud19200);
    //   // set19200();
    //   delay(1000);
    //   //check robot answers now
    //   if(roboInit()){
    //     roboStart = true;
    //     // k = 8;
    //     //if roomba answered, change baud rate to 115200
    //     // setBaud(Baud115200);
    //     // Serial2.flush();
    //     // delay(50);
    //     // Serial2.end();
    //     // delay(50);
    //     // Serial2.begin(115200);
    //     // delay(50);
    //     debug.println("Connection successful", DEBUG_GENERAL);
    //     // M5.Lcd.println("Robo connection successful");
    //     break;
    //   }
    // }
  }
  else{
    roboStart = true;
  }

  return roboStart;
}

void Roomba::defineBeep(){

stream->write(CmdSong);
stream->write(uint8_t(BUMP_BEEP));
stream->write(1);
stream->write(33);
stream->write(4);

stream->write(CmdSong);
stream->write(START_BEEP);
stream->write(2);
stream->write(74);
stream->write(6);
stream->write(74);
stream->write(6);

stream->write(CmdSong);
stream->write(RESET_BEEP);
stream->write(3);
stream->write(100);
stream->write(6);
stream->write(100);
stream->write(6);
stream->write(100);
stream->write(6);

stream->write(CmdSong);
stream->write(APP_BEEP);
stream->write(2);
stream->write(33);
stream->write(4);
stream->write(33);
stream->write(4);
}


void Roomba::playBeep(uint8_t beepNo){
  stream->write(CmdPlay);
  stream->write(beepNo); //Song No
}

void Roomba::getXY(int16_t encoderL, int16_t encoderR){
  int16_t encHistory[2] = {encL, encR};

  int16_t enc[2]={encoderL, encoderR};
  float dist[2]={};

  for(uint8_t i = 0; i < 2; i++){

      //rollover
      int32_t diff = enc[i] - encHistory[i];
      diff = abs(diff);
      if(diff > 1000){
        dist[i] = (65535 - encHistory[i] + enc[i]) * (PI * 72 / 508.8);
      }
      else{
        dist[i] = (enc[i] - encHistory[i]) * (PI * 72 / 508.8);
      }
  }

  //save current values
  encL = enc[0];
  encR = enc[1];

  //filter out unusual data
  float distTemp = abs(dist[0]);
  if(distTemp > DISTANCE_LIMIT){
    dist[0] = 0;
  }
  distTemp = abs(dist[1]);
  if(distTemp > 500){
    dist[1] = 0;
  }
  //distance
  float distance = (dist[0] + dist[1]) / 2;

  //Angle in radian
  float angle = (dist[0] - dist[1]) / 235;

  //filter out unusual data
  float angTemp = abs(angle);
  if(angTemp > 2 * PI){
    angle = 0;
  }

  angle += direction;
  //radian rollover
  if(angle > PI){
    direction = angle - (2 * PI);
  }
  else if(angle < -PI){
    direction = angle + (2 * PI);
  }
  else{
    direction = angle;
  }
  x += distance * cos(direction);
  y += distance * sin(direction);
}
