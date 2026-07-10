#include "config.hpp"
#include "connection.hpp"
#include "hooks.hpp"
#include "data.hpp"
#include "hardware.hpp"
#include "hardware_impl.hpp"

Hardware hardware;

unsigned long lastRead = 0;
unsigned long lastSend = 0;

void setup() {
    hardware.begin();
}

void loop() {
    unsigned long now = millis();

    hardware.keepConnection();
    hardware.readButton();
    hardware.displayer();

    if (!hardware.isPowered()) {
        hardware.controller();
        return;
    }

    if (now - lastRead >= READ_INTERVAL) {
        lastRead = now;
        hardware.collector();
        hardware.manipulator();
        hardware.controller();
    }

    if (now - lastSend >= (unsigned long)hardware.getTimer()) {
        lastSend = now;
        hardware.pull();
        hardware.push();
        hardware.printer();
    }
}
