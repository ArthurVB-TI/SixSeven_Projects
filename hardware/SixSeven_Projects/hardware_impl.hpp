#ifndef HARDWARE_IMPL_HPP
#define HARDWARE_IMPL_HPP

#include "config.hpp"
#include "hardware.hpp"

Hardware::Hardware() {
    powered = false;
    lastButton = false;
    lastDebounce = 0;
    beepEnd = 0;
}

void Hardware::begin() {
    Serial.begin(115200);
    delay(300);

    pinMode(PIN_POT_RECEIVED, INPUT);
    pinMode(PIN_POT_BASE, INPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    ledcAttach(PIN_LED_A, PWM_FREQ, PWM_RES);
    ledcAttach(PIN_LED_B, PWM_FREQ, PWM_RES);
    ledcAttach(PIN_BUZZER, PWM_FREQ, PWM_RES);
    ledcWrite(PIN_BUZZER, 0);
    ledcWrite(PIN_LED_A, 0);
    ledcWrite(PIN_LED_B, 0);

    conn.start();
}

int Hardware::readReceivedEnergy() {
    int value = analogRead(PIN_POT_RECEIVED);
    data.setReceivedEnergy(value);
    hooks.setReceivedEnergy(value);
    return value;
}

int Hardware::readBaseEnergy() {
    int value = analogRead(PIN_POT_BASE);
    data.setBaseEnergy(value);
    return value;
}

bool Hardware::readButton() {
    bool pressed = digitalRead(PIN_BUTTON) == LOW;
    if (pressed && !lastButton && millis() - lastDebounce >= DEBOUNCE_INTERVAL) {
        lastDebounce = millis();
        powered = !powered;
    }
    lastButton = pressed;
    return powered;
}

void Hardware::computeAll() {
    data.computeMeanReceived(data.getReceivedEnergy());
    data.computeVariation();
    data.computeMeanVariation();
}

void Hardware::driveLed() {
    if (!powered) {
        ledcWrite(PIN_LED_A, 0);
        ledcWrite(PIN_LED_B, 0);
        return;
    }
    int level = data.getMeanReceived();
    if (data.isStable() && level >= LED_HIGH) {
        ledcWrite(PIN_LED_A, 0);
        ledcWrite(PIN_LED_B, PWM_MAX);
    } else if (level >= LED_LOW) {
        ledcWrite(PIN_LED_A, PWM_MAX);
        ledcWrite(PIN_LED_B, PWM_MAX);
    } else {
        ledcWrite(PIN_LED_A, PWM_MAX);
        ledcWrite(PIN_LED_B, 0);
    }
}

void Hardware::alert() {
    ledcWriteTone(PIN_BUZZER, ALERT_TONE);
    beepEnd = millis() + BEEP_DURATION;
}

void Hardware::stopAlert() {
    if (beepEnd != 0 && millis() >= beepEnd) {
        ledcWrite(PIN_BUZZER, 0);
        beepEnd = 0;
    }
}

void Hardware::showSerial() {
    data.printSerial();
    conn.printStatus();
}

void Hardware::collector() {
    readReceivedEnergy();
    readBaseEnergy();
    readButton();
}

void Hardware::manipulator() {
    computeAll();
}

void Hardware::controller() {
    driveLed();
    if (powered && !data.isStable()) alert();
    stopAlert();
}

void Hardware::printer() {
    showSerial();
}

bool Hardware::push() {
    return hooks.push(data.toJson(), conn);
}

bool Hardware::pull() {
    bool ok = hooks.pull(conn);
    if (ok) data.applyHooks(hooks);
    return ok;
}

bool Hardware::isPowered() {
    return powered;
}

void Hardware::keepConnection() {
    conn.keepAlive();
}

int Hardware::getTimer() {
    return data.getTimer();
}

#endif
