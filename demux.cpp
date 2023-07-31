#include <Arduino.h>

#define SERIAL_SPEED 9600

#define BUTTON_PUSH_DURATION 50    //ms
#define BUTTON_ACTION_TIMEOUT 100  //ms

#define PRESSED_BUTTON_STATE 1
#define BUTTON_PIN 6

#define VPNLED_PIN 10
#define VPNLED_SMOOTH_SPEED 40 //milliseconds

#define CHANNEL1_PIN A3
#define CHANNEL2_PIN A2
#define CHANNEL3_PIN A1
#define CHANNEL4_PIN A0

bool currentOutputState = 0; //readButtonRoutine()

void ledSmoothSwitcher(const int8_t = -1);
void readButtonRoutine();
void readSerialRoutine();
//void speedTest();

void setup()
{
    Serial.begin(SERIAL_SPEED);

    pinMode(VPNLED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(CHANNEL1_PIN, OUTPUT);

    digitalWrite(CHANNEL1_PIN, LOW);
}

void loop()
{
    readButtonRoutine();
    readSerialRoutine();
    ledSmoothSwitcher();
    //speedTest(); //72K calls per second ("idle")
}


void readButtonRoutine() //strict EMI protection and debounce
{
    static int16_t pushDuration = 0;
    static uint32_t previousTime = millis(); //in case multiple buttons, could be moved level up

    bool currentButtonState = digitalRead(BUTTON_PIN);

    //unlock pushDuration timer, start action if button pressed
    if (currentButtonState == PRESSED_BUTTON_STATE && pushDuration == 0) {
        ++pushDuration;
        previousTime = millis();
        return;
    }

    //idle guard
    if (pushDuration == 0) {
        return;
    }

    uint16_t timeDifference = millis() - previousTime;

    //minimum time chunk is 1ms
    if (timeDifference == 0) {
        return;
    }

    previousTime = millis();

    //timeout between actions
    if (pushDuration < 0) {
        pushDuration += timeDifference;

        //in case heavy duty external operations which takes more than 1ms. Flapping timer protection.
        if (pushDuration > 0) {
            pushDuration = 0; //lock pushDuration timer, action completely done
        }

        //prevent continuously toggling, if button held pressed
        if (currentButtonState == PRESSED_BUTTON_STATE) {
            pushDuration = -BUTTON_ACTION_TIMEOUT;
        }

        return;
    }

    //EMI protection, including severe ones
    if (currentButtonState == PRESSED_BUTTON_STATE) {
        pushDuration += timeDifference;
    } else {
        pushDuration -= timeDifference;
    }

    //toggle state, finally...
    if (pushDuration > BUTTON_PUSH_DURATION) {
        currentOutputState = !currentOutputState;
        digitalWrite(CHANNEL1_PIN, currentOutputState);

        pushDuration = -BUTTON_ACTION_TIMEOUT;
    }

}

void readSerialRoutine()
{
    while (Serial.available() > 0) {
        int state = Serial.parseInt();

        if (Serial.read() == '\n') {
            switch(state) {
                case 0:
                    ledSmoothSwitcher(0);
                    break;
                case 1:
                    ledSmoothSwitcher(1);
                    break;
                case 5:
                    currentOutputState = !currentOutputState;
                    digitalWrite(CHANNEL1_PIN, currentOutputState);
                    break;
            }
            Serial.println("OK");
        }
    }
}

void ledSmoothSwitcher(const int8_t action)
{
    static uint8_t angle = 0;
    static int8_t step = 1;
    static unsigned long previuosChangeTime = millis();

    if (angle == 0 && action == -1) {
        return;
    }

    if (action == 0) {
        angle = 90;
        step = -2;
    }

    if (action == 1) {
        angle = 1;
        step = 1;
    }

    if (millis() - previuosChangeTime < VPNLED_SMOOTH_SPEED) {
            return;
    }

    angle += step;
    analogWrite(VPNLED_PIN, 255 * sin(angle * (PI / 180)));

    if (angle == 90) {
        angle = 0;
    }

    previuosChangeTime = millis();
}

/*
void speedTest()
{
    static unsigned long previuosChangeTime = millis();
    static unsigned long counter = 0;

    if (millis() - previuosChangeTime > 1000) {
        Serial.println(counter);
        counter = 0;
        previuosChangeTime = millis();
        return;
    }

    ++counter;
}
*/

