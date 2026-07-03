#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "config.hpp"

class Connection {
    private:
        unsigned long lastAttempt;

    public:
        Connection() {
            lastAttempt = 0;
        }

        bool startWifi() {
            WiFi.mode(WIFI_STA);
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            unsigned long start = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - start < BOOT_WIFI_WAIT) {
                delay(250);
            }
            return wifiConnected();
        }

        bool start() {
            return startWifi();
        }

        bool wifiConnected() {
            return WiFi.status() == WL_CONNECTED;
        }

        bool isConnected() {
            return wifiConnected();
        }

        void keepAlive() {
            if (wifiConnected()) return;
            if (millis() - lastAttempt >= RECONNECT_INTERVAL) {
                lastAttempt = millis();
                WiFi.disconnect();
                WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            }
        }

        void printStatus() {
            Serial.print("WiFi:");
            Serial.println(wifiConnected() ? "ok" : "--");
        }
};

#endif
