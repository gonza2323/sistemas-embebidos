
void setup() {
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(13, OUTPUT);
    pinMode(A3, INPUT);
    
    Serial.begin(115200);
}

void loop() {
    if (Serial.available() >= 4) {
        updateLEDs();
    }
    sendIllumination();
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

void sendIllumination() {
    int analogValue = analogRead(A3);
    Serial.write((uint8_t*)&analogValue, sizeof(int));
}
