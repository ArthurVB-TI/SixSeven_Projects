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

    ledcAttach(PIN_OUTPUT, PWM_FREQ, PWM_RES);
    ledcAttach(PIN_BUZZER, PWM_FREQ, PWM_RES);
    ledcWrite(PIN_BUZZER, 0);
    ledcWrite(PIN_OUTPUT, 0);

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

void Hardware::driveOutput() {
    if (!powered) {
        ledcWrite(PIN_OUTPUT, 0);
        return;
    }
    int target = data.getBaseEnergy();
    int measured = data.getMeanReceived();
    int correction = target - measured;
    int output = measured + correction;
    if (output < 0) output = 0;
    if (output > ADC_MAX) output = ADC_MAX;
    int pwm = map(output, 0, ADC_MAX, 0, PWM_MAX);
    ledcWrite(PIN_OUTPUT, pwm);
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
    driveOutput();
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
