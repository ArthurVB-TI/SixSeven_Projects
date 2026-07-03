#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "config.hpp"

class Connection {
    private:
        BluetoothSerial bt;
        unsigned long lastAttempt;

    public:
        Connection() {
            lastAttempt = 0;
        }

        bool startWifi() {
            WiFi.mode(WIFI_STA);
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            unsigned long start = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
                delay(250);
            }
            return wifiConnected();
        }

        bool startBluetooth() {
            return bt.begin(BT_NAME);
        }

        bool start() {
            bool wifi = startWifi();
            bool blue = startBluetooth();
            return wifi || blue;
        }

        bool wifiConnected() {
            return WiFi.status() == WL_CONNECTED;
        }

        bool bluetoothConnected() {
            return bt.hasClient();
        }

        bool isConnected() {
            return wifiConnected() || bluetoothConnected();
        }

        void keepAlive() {
            if (wifiConnected()) return;
            if (millis() - lastAttempt >= RECONNECT_INTERVAL) {
                lastAttempt = millis();
                WiFi.disconnect();
                WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            }
        }

        BluetoothSerial& getBluetooth() {
            return bt;
        }

        void printStatus() {
            Serial.print("WiFi:");
            Serial.print(wifiConnected() ? "ok" : "--");
            Serial.print(" BT:");
            Serial.println(bluetoothConnected() ? "ok" : "--");
        }
};

#endif
