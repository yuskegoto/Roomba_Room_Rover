# About
This is a project to turn your Roomba into WiFi-RC car. Based on my previous projects.

[RoombaWifiController](https://github.com/yuskegoto/RoombaWifiController): This project does not use camera streaming.

[MKZ4Camera](https://github.com/cerevo/MKZ4/tree/master/custom/MKZ4Camera): This is a project to run WiFi RC car MKZ-4 with camera streaming, which I worked on when I was at Cerevo Inc.

![Roomba and device from top](https://raw.githubusercontent.com/yuskegoto/Roomba_Room_Rover/main/Pics/top.jpg)
![Device from front](https://raw.githubusercontent.com/yuskegoto/Roomba_Room_Rover/main/Pics/device_front.jpg)
![Decice rear side](https://raw.githubusercontent.com/yuskegoto/Roomba_Room_Rover/main/Pics/device_rear.jpg)

# System
The camera streaming and communication to the Roomba is done by ESP32 with camera module. Basically This project should run on any ESP32 with camera module, but I have used Timer Camera F from M5 since it has fish eye lens.

The communication between ESP32 and Roomba is done over UART, which you can access from the Mini-DIN connector on the Roomba. ESP32 module is also powered from Roomba's battery via tiny DC-DC converter.

The movement is controlled from web interface served from ESP32's simple http server. Control value is communicated over Websocket.

# Environment
PlatformIO with Arduino
Built on PlatformIO Version 6.1.6
ESP32 for Arduino v6.1.0

# How to build
Run "platformio run" at project root directory.

## Circuitry and parts
- [FXMA2102](https://akizukidenshi.com/catalog/g/gM-05825/)
    - Due to circuit level of ESP32(3.3V) and Roomba(5.0V) I needed to place level converter. Actually I am supplying 5V to ESP32 side too, since there is no 3.3V out from Timer CAM F module. ESP32 is 5V tolerant, so this module is simply working as a UART buffer.
- [VX07805-500](https://akizukidenshi.com/catalog/g/gM-15209/)
    - Power supply for the ESP32 module. This needs to be a DCDC, since the Roomba side can only supply up to 200mA and ESP32 can sometimes consumes much larger current.

![Circuit Board Front](https://raw.githubusercontent.com/yuskegoto/Roomba_Room_Rover/main/Pics/circuitboard_front.jpg)
![Circuit Board Rear](https://raw.githubusercontent.com/yuskegoto/Roomba_Room_Rover/main/Pics/circuitboard_rear.jpg)

## Project structure
- oi.h
    - iRobot Open Interface packet definition. Based on the [Roomba-Butler Belvedere project by wolffan;](https://www.instructables.com/id/Belvedere-A-Butler-Robot/).
- Roomba.h / Roomba.cpp
    - Roomba interface
- app_httpd.cpp
    - Http and Websocket server.
- camera_index.h / camera_index.html
    - Web interface html text stored as string.

No additional library is required.

# References
[iRobot Open Interface](https://edu.irobot.com/learning-library/create-2-oi-spec)

[The official M5 Timer Camera F info](https://docs.m5stack.com/en/unit/timercam_f)

[official CameraWebServer.ino sample](https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/Camera/CameraWebServer)

[WS Server sample](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_http_server.html#websocket-server)

The original oi.h was referenced from the [Roomba-Butler Belvedere project by wolffan](https://www.instructables.com/Belvedere-A-Butler-Robot/).

# Todo
[] Clean up app_httpd.cpp and html file.
[] Detailed documentation for circuit board.
[] Build instruction?