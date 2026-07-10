#ifndef HARDWARE_IMPL_HPP
#define HARDWARE_IMPL_HPP

#include "config.hpp"
#include "hardware.hpp"

Hardware::Hardware() : display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1) {
    powered = false;
    lastButton = false;
    lastDebounce = 0;
    beepEnd = 0;
    oledOk = false;
    lastPushOk = false;
    lastOled = 0;
}

void Hardware::begin() {
    Serial.begin(115200);
    // Espera o USB CDC do S3 enumerar para nao perder o banner de boot.
    delay(1000);

    pinMode(PIN_POT_RECEIVED, INPUT);
    pinMode(PIN_POT_BASE, INPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    ledcAttach(PIN_LED_A, PWM_FREQ, PWM_RES);
    ledcAttach(PIN_LED_B, PWM_FREQ, PWM_RES);
    ledcAttach(PIN_BUZZER, PWM_FREQ, PWM_RES);
    ledcWrite(PIN_BUZZER, 0);
    ledcWrite(PIN_LED_A, 0);
    ledcWrite(PIN_LED_B, 0);

    Serial.println();
    Serial.println("== SixSeven boot ==");
    beginOled();
    Serial.print("WiFi \"");
    Serial.print(WIFI_SSID);
    Serial.print("\"... ");
    bool wifiOk = conn.start();
    Serial.println(wifiOk ? "conectado" : "FALHOU (vai tentando em background)");
    if (wifiOk) {
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    }
    Serial.print("backend: http://");
    Serial.print(SERVER_HOST);
    Serial.print(":");
    Serial.println(SERVER_PORT);
    Serial.println("power OFF — aperte o botao para iniciar leitura/envio");
    drawOled();
}

void Hardware::beginOled() {
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    oledOk = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
    Serial.print("OLED: ");
    Serial.println(oledOk ? "ok" : "nao encontrado (segue sem display)");
}

void Hardware::drawOled() {
    if (!oledOk) return;
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("SixSeven   WiFi:");
    display.println(conn.wifiConnected() ? "ok" : "--");

    if (!powered) {
        display.setCursor(0, 24);
        display.println("power: OFF");
        display.println();
        display.println("aperte o botao");
    } else {
        display.setCursor(0, 12);
        display.print("power:ON   envio:");
        display.println(lastPushOk ? "ok" : "--");
        display.setTextSize(2);
        display.setCursor(0, 26);
        display.print("Er ");
        display.println(data.getReceivedEnergy());
        display.setTextSize(1);
        display.setCursor(0, 46);
        display.print("Eb:");
        display.print(data.getBaseEnergy());
        display.print("  M:");
        display.println(data.getMeanReceived());
        display.setCursor(0, 56);
        display.print(data.isStable() ? "estavel " : "INSTAVEL");
        display.print("  V:");
        display.print(data.getVariation());
    }
    display.display();
}

void Hardware::displayer() {
    if (millis() - lastOled >= OLED_INTERVAL) {
        lastOled = millis();
        drawOled();
    }
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
        Serial.println(powered ? "power: ON" : "power: OFF");
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
    lastPushOk = hooks.push(data.toJson(), conn);
    return lastPushOk;
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
