#include <Arduino.h>

#define SERIAL_SPEED 9600
#define BUTTON_DEBOUNCE_TIMEOUT 150
#define PRESSED_BUTTON_STATE 1

#define BUTTON_PIN 6
#define VPNLED_PIN 10

#define CHANNEL1_PIN A3
#define CHANNEL2_PIN A2
#define CHANNEL3_PIN A1
#define CHANNEL4_PIN A0

bool currentOutputState = 0; //readButtonRoutine()

void ledSmoothSwitcher(const bool action);
void readButtonRoutine();
void readSerialRoutine();

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
}

void readButtonRoutine()
{
    static bool previousButtonState = !PRESSED_BUTTON_STATE;
    static unsigned long previuosChangeTime = 0;

    bool currentButtonState = 0;

    if ((millis() - previuosChangeTime) < BUTTON_DEBOUNCE_TIMEOUT) {
        return;
    }

    currentButtonState = digitalRead(BUTTON_PIN);

    if (currentButtonState == previousButtonState) {
        return;
    }

    if (currentButtonState == PRESSED_BUTTON_STATE) {
        currentOutputState = !currentOutputState;
        digitalWrite(CHANNEL1_PIN, currentOutputState);
    }

    previousButtonState = currentButtonState;
    previuosChangeTime = millis();
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

void ledSmoothSwitcher(const bool action)
{
    static uint8_t angle = 0;
    int8_t multiplier = 0;

    if (action == 0) {
        angle = 90;
        multiplier = -1;
    } else {
        angle = 0;
        multiplier = 1;
    }

    for(;;) {
        angle += multiplier;
        analogWrite(VPNLED_PIN, 255*sin(angle*(PI/180)));
        if (angle ==  0 || angle == 90) {
            break;
        }
        delay(10); //Fix me
    }
}
