#ifndef HARDWARE_HPP
#define HARDWARE_HPP

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.hpp"
#include "connection.hpp"
#include "hooks.hpp"
#include "data.hpp"

class Hardware {
    private:
        DataRepository data;
        Connection conn;
        Hooks hooks;
        Adafruit_SSD1306 display;

        bool powered;
        bool lastButton;
        unsigned long lastDebounce;
        unsigned long beepEnd;
        bool oledOk;
        bool lastPushOk;
        unsigned long lastOled;

    public:
        Hardware();

        void begin();

        int readReceivedEnergy();
        int readBaseEnergy();
        bool readButton();

        void computeAll();

        void driveLed();
        void alert();
        void stopAlert();

        void showSerial();

        void beginOled();
        void drawOled();
        void displayer();

        void collector();
        void manipulator();
        void controller();
        void printer();

        bool push();
        bool pull();

        bool isPowered();
        void keepConnection();
        int getTimer();
};

#endif
