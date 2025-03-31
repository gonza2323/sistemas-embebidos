
void setup() {
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(13, OUTPUT);
    pinMode(A3, INPUT);
    
    Serial.begin(9600);
}

void loop() {
    if (Serial.available() >= 4) {
        updateLEDs();
    }
    sendCurrentIllumination();
    delay(250);
}

void updateLEDs() {
    byte led09brightness = Serial.read();
    byte led10brightness = Serial.read();
    byte led11brightness = Serial.read();
    byte led13status = Serial.read();

    analogWrite(9, led09brightness);
    analogWrite(10, led10brightness);
    analogWrite(11, led11brightness);
    digitalWrite(13, led13status);
}

void sendCurrentIllumination() {
    int analogValue = analogRead(A3);
    int illumination = calculateIllumination(analogValue);

    byte highByte = illumination >> 8;
    byte lowByte = illumination & 0xFF;
    
    Serial.write(highByte);
    Serial.write(lowByte);
}

int calculateIllumination(int analogValue) {
    // These constants should match the photoresistor's attributes
    const float GAMMA = 0.7;
    const float RL10 = 50;

    float voltage = analogValue / 1024. * 5;
    float resistance = 2000 * voltage / (1 - voltage / 5);
    float illumination = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));

    return illumination;
}
