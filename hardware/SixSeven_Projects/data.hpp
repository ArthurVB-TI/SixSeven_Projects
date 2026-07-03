#ifndef DATA_HPP
#define DATA_HPP

#include "config.hpp"
#include "hooks.hpp"

class Data {
    private:
        void initData() {
            this->id = CONNECTION_ID;
            this->E_b = 0;
            this->E_r = 0;
            this->timer = SEND_INTERVAL;
            this->M_er = 0;
            this->V_mer = 0;
            this->M_vmer = 0;
        }
    protected:
        int id;
        int E_b;
        int E_r;
        int timer;
        int M_er;
        int V_mer;
        int M_vmer;
    public:
        Data() { initData(); }

        int getId() { return this->id; }
        int getBaseEnergy() { return this->E_b; }
        int getReceivedEnergy() { return this->E_r; }
        int getTimer() { return this->timer; }
        int getMeanReceived() { return this->M_er; }
        int getVariation() { return this->V_mer; }
        int getMeanVariation() { return this->M_vmer; }

        void setId(int id) { this->id = id; }
        void setBaseEnergy(int E_b) { this->E_b = E_b; }
        void setReceivedEnergy(int E_r) { this->E_r = E_r; }
        void setTimer(int timer) { this->timer = timer; }
        void setMeanReceived(int M_er) { this->M_er = M_er; }
        void setVariation(int V_mer) { this->V_mer = V_mer; }
        void setMeanVariation(int M_vmer) { this->M_vmer = M_vmer; }
};

class DataRepository : public Data {
    private:
        int receivedHistory[WINDOW_SIZE];
        int receivedCount;
        int receivedSum;

        int variationHistory[WINDOW_SIZE];
        int variationCount;
        int variationSum;

        void initRepository() {
            receivedCount = 0;
            receivedSum = 0;
            variationCount = 0;
            variationSum = 0;
            for (int i = 0; i < WINDOW_SIZE; i++) {
                receivedHistory[i] = 0;
                variationHistory[i] = 0;
            }
        }

    public:
        DataRepository() { initRepository(); }

        void create(int id, int E_b, int E_r, int timer, int M_er, int V_mer, int M_vmer) {
            this->id = id;
            this->E_b = E_b;
            this->E_r = E_r;
            this->timer = timer;
            this->M_er = M_er;
            this->V_mer = V_mer;
            this->M_vmer = M_vmer;
        }

        void update(int id, int E_b, int E_r, int timer, int M_er, int V_mer, int M_vmer) {
            create(id, E_b, E_r, timer, M_er, V_mer, M_vmer);
        }

        void read() {
            printSerial();
        }

        void reset() {
            setId(CONNECTION_ID);
            setBaseEnergy(0);
            setReceivedEnergy(0);
            setTimer(SEND_INTERVAL);
            setMeanReceived(0);
            setVariation(0);
            setMeanVariation(0);
            initRepository();
        }

        void applyHooks(Hooks& hooks) {
            this->id = hooks.getId();
            this->E_b = hooks.getBaseEnergy();
            this->E_r = hooks.getReceivedEnergy();
            this->timer = hooks.getTimer();
        }

        int computeMeanReceived(int reading) {
            int index = receivedCount % WINDOW_SIZE;
            receivedSum -= receivedHistory[index];
            receivedHistory[index] = reading;
            receivedSum += reading;
            if (receivedCount < WINDOW_SIZE) receivedCount++;
            int n = receivedCount < WINDOW_SIZE ? receivedCount : WINDOW_SIZE;
            this->M_er = receivedSum / n;
            return this->M_er;
        }

        int computeVariation() {
            this->V_mer = this->E_r - this->M_er;
            if (this->V_mer < 0) this->V_mer = -this->V_mer;
            return this->V_mer;
        }

        int computeMeanVariation() {
            int index = variationCount % WINDOW_SIZE;
            variationSum -= variationHistory[index];
            variationHistory[index] = this->V_mer;
            variationSum += this->V_mer;
            if (variationCount < WINDOW_SIZE) variationCount++;
            int n = variationCount < WINDOW_SIZE ? variationCount : WINDOW_SIZE;
            this->M_vmer = variationSum / n;
            return this->M_vmer;
        }

        bool isStable() {
            return this->V_mer <= STABLE_THRESHOLD;
        }

        String toJson() {
            String json = "{";
            json += "\"id\":" + String(this->id) + ",";
            json += "\"E_b\":" + String(this->E_b) + ",";
            json += "\"E_r\":" + String(this->E_r) + ",";
            json += "\"timer\":" + String(this->timer) + ",";
            json += "\"M_er\":" + String(this->M_er) + ",";
            json += "\"V_mer\":" + String(this->V_mer) + ",";
            json += "\"M_vmer\":" + String(this->M_vmer);
            json += "}";
            return json;
        }

        void printSerial() {
            Serial.print("id:");      Serial.print(this->id);
            Serial.print(" E_b:");    Serial.print(this->E_b);
            Serial.print(" E_r:");    Serial.print(this->E_r);
            Serial.print(" M_er:");   Serial.print(this->M_er);
            Serial.print(" V_mer:");  Serial.print(this->V_mer);
            Serial.print(" M_vmer:"); Serial.print(this->M_vmer);
            Serial.print(" timer:");  Serial.println(this->timer);
        }
};

#endif
