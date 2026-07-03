#ifndef HARDWARE_HPP
#define HARDWARE_HPP

#include "config.hpp"
#include "connection.hpp"
#include "hooks.hpp"
#include "data.hpp"

class Hardware {
    private:
        DataRepository data;
        Connection conn;
        Hooks hooks;

        bool powered;
        bool lastButton;
        unsigned long lastDebounce;
        unsigned long beepEnd;

    public:
        Hardware();

        void begin();

        int readReceivedEnergy();
        int readBaseEnergy();
        bool readButton();

        void computeAll();

        void driveOutput();
        void alert();
        void stopAlert();

        void showSerial();

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
