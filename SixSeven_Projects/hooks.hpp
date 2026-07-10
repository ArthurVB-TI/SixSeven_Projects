#ifndef HOOKS_HPP
#define HOOKS_HPP

#include "config.hpp"
#include "connection.hpp"

class Hooks {
    private:
        int id;
        int E_b;
        int E_r;
        int timer;

        String url(const char* path) {
            return "http://" + String(SERVER_HOST) + ":" + String(SERVER_PORT) + String(path);
        }

        int extractInt(const String& json, const String& key, int fallback) {
            String search = "\"" + key + "\"";
            int pos = json.indexOf(search);
            if (pos < 0) return fallback;
            pos = json.indexOf(':', pos);
            if (pos < 0) return fallback;
            pos++;
            while (pos < (int)json.length() && (json[pos] == ' ' || json[pos] == '"')) pos++;
            int end = pos;
            while (end < (int)json.length() && (isDigit(json[end]) || json[end] == '-')) end++;
            if (end == pos) return fallback;
            return json.substring(pos, end).toInt();
        }

    public:
        Hooks() {
            id = CONNECTION_ID;
            E_b = 0;
            E_r = 0;
            timer = SEND_INTERVAL;
        }

        int getId() { return this->id; }
        int getBaseEnergy() { return this->E_b; }
        int getReceivedEnergy() { return this->E_r; }
        int getTimer() { return this->timer; }

        void setReceivedEnergy(int value) { this->E_r = value; }

        bool push(const String& payload, Connection& conn) {
            if (!conn.wifiConnected()) {
                Serial.println("POST /data pulado: sem WiFi");
                return false;
            }
            HTTPClient http;
            http.begin(url(SERVER_PATH));
            http.setTimeout(HTTP_TIMEOUT);
            http.addHeader("Content-Type", "application/json");
            int code = http.POST(payload);
            http.end();
            // code negativo = erro do HTTPClient (-1 nao conectou,
            // -11 timeout); 200 = backend recebeu e gravou.
            Serial.print("POST ");
            Serial.print(url(SERVER_PATH));
            Serial.print(" -> ");
            Serial.println(code);
            return code >= 200 && code < 300;
        }

        bool pull(Connection& conn) {
            if (!conn.wifiConnected()) return false;
            HTTPClient http;
            http.begin(url(SERVER_CONFIG_PATH) + "?id=" + String(CONNECTION_ID));
            http.setTimeout(HTTP_TIMEOUT);
            int code = http.GET();
            Serial.print("GET ");
            Serial.print(url(SERVER_CONFIG_PATH));
            Serial.print("?id=");
            Serial.print(CONNECTION_ID);
            Serial.print(" -> ");
            Serial.println(code);
            if (code < 200 || code >= 300) {
                http.end();
                return false;
            }
            String body = http.getString();
            http.end();
            this->id = extractInt(body, "id", CONNECTION_ID);
            this->E_b = extractInt(body, "E_b", this->E_b);
            this->timer = extractInt(body, "timer", this->timer);
            return true;
        }
};

#endif
