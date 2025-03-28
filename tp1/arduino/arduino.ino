
byte led09brightness = 0;
byte led10brightness = 0;
byte led11brightness = 0;
boolean led13status = false;

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
        updateStatus();
    }
    sendIlluminationUpdate();
    delay(250);
}

void updateLEDs() {
    analogWrite(9, led09brightness);
    analogWrite(10, led10brightness);
    analogWrite(11, led11brightness);
    digitalWrite(13, led13status);
}

void updateStatus() {
    led09brightness = Serial.read();
    led10brightness = Serial.read();
    led11brightness = Serial.read();
    led13status = Serial.read();

    updateLEDs();
}

void sendIlluminationUpdate() {
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
