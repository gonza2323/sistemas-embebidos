
#define FIRST_LED 6
#define LAST_LED 13

#define ILLUMINATION_UPDATE_PERIOD 500

float volume = 0;
uint32_t lastSentUpdate;

union FloatUnion {
    uint32_t i;
    float f;
};

void setup() {
    pinMode(A3, INPUT);
    for (int i = FIRST_LED; i <= LAST_LED; i++)
        pinMode(i, OUTPUT);
    
    lastSentUpdate = millis();
    updateLEDs();
    
    Serial.begin(9600);
}

void loop() {
    if (Serial.available() >= 4) {
        readVolume();
        updateLEDs();
    }

    uint32_t now = millis();
    if (now - lastSentUpdate > ILLUMINATION_UPDATE_PERIOD) {
        sendIllumination();
        lastSentUpdate = now;
    }
    delay(100);
}

void readVolume() {
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

void sendIllumination() {
    int analogValue = analogRead(A3);
    Serial.write((uint8_t*)&analogValue, sizeof(int));
}
