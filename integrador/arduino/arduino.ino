
#define FIRST_LED 6
#define LAST_LED 13

#define UPDATE_INTERVAL 500
#define BUTTON_DEBOUNCE_PERIOD 50

#define ILLUMINATION_MSG 'L'
#define BUTTON_MSG 'B'

float volume = 0;
uint32_t lastIlluminationUpdateTime;

typedef struct {
    uint8_t pin;
    uint32_t lastUpdateTime;
    bool lastStableState;
} Button;

Button button2 = {2, NULL, NULL};
Button button3 = {3, NULL, NULL};

union FloatUnion {
    uint32_t i;
    float f;
};


void setup() {
    pinMode(A3, INPUT);
    pinMode(2, INPUT);
    pinMode(3, INPUT);
    for (int i = FIRST_LED; i <= LAST_LED; i++)
        pinMode(i, OUTPUT);
    
    lastIlluminationUpdateTime = millis();
    
    button2.lastUpdateTime = millis();
    button3.lastUpdateTime = millis();
    button2.lastStableState = digitalRead(button2.pin);
    button2.lastStableState = digitalRead(button3.pin);

    updateLEDs();

    attachInterrupt(digitalPinToInterrupt(2), processButton2, CHANGE);
    attachInterrupt(digitalPinToInterrupt(3), processButton3, CHANGE);
    
    Serial.begin(9600);
}

void loop() {
    if (Serial.available() >= 4) {
        readData();
        updateLEDs();
    }

    uint32_t now = millis();
    if (now - lastIlluminationUpdateTime > UPDATE_INTERVAL) {
        sendIlluminationUpdate();
        sendButtonUpdate(&button2);
        sendButtonUpdate(&button3);
        lastIlluminationUpdateTime = now;
    }
    delay(100);
}

void readData() {
    uint32_t data = 0;
    data |= (uint32_t)Serial.read();
    data |= (uint32_t)Serial.read() << 8;
    data |= (uint32_t)Serial.read() << 16;
    data |= (uint32_t)Serial.read() << 24;

    FloatUnion fu;
    fu.i = data;
    volume = fu.f;
}

void updateLEDs() {
    float ledRange = volume * (LAST_LED - FIRST_LED + 1);
    int fullLitLEDAmount = (int)ledRange;
    int lastLitLED = FIRST_LED + fullLitLEDAmount;
    
    float lastLitLEDBrightness = ledRange - fullLitLEDAmount;
    int pwm = (int)(lastLitLEDBrightness * 255);

    for (int i = FIRST_LED; i < lastLitLED; i++)
        digitalWrite(i, HIGH);
    
    analogWrite(lastLitLED, pwm);
    
    for (int i = lastLitLED + 1; i <= LAST_LED; i++)
        digitalWrite(i, LOW);
}

void sendIlluminationUpdate() {
    int analogValue = analogRead(A3);
    Serial.write(ILLUMINATION_MSG);
    Serial.write((uint8_t*)&analogValue, sizeof(int));
}

void sendButtonUpdate(Button* button) {
    uint32_t now = millis();
    if (now - button->lastUpdateTime < BUTTON_DEBOUNCE_PERIOD) { return; }
    
    Serial.write(BUTTON_MSG);
    Serial.write(button->pin);
    Serial.write(button->lastStableState);
}

void processButton(Button* button) {
    uint32_t now = millis();
    if (now - button->lastUpdateTime < BUTTON_DEBOUNCE_PERIOD) { return; }

    button->lastStableState = !(button->lastStableState);
    button->lastUpdateTime = now;
    sendButtonUpdate(button);
    Serial.write(BUTTON_MSG);
    Serial.write(button->pin);
    Serial.write(button->lastStableState);
}

void processButton2() { processButton(&button2); }
void processButton3() { processButton(&button3); }
