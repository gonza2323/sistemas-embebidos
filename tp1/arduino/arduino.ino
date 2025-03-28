
int led09brightness = 0;
int led10brightness = 0;
int led11brightness = 0;
bool led13Status = false;

void setup() {
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(13, OUTPUT);
    
    digitalWrite(9, led09brightness);
    digitalWrite(10, led10brightness);
    digitalWrite(11, led11brightness);
    digitalWrite(13, LOW);
    
    Serial.begin(9600);
}

void loop() {
    if (Serial.available() > 0) {
        handleMessage();
    }
    sendIlluminationUpdate();
    
    delay(250);
}

void handleMessage() {
    char msgType = Serial.read();
    
    if (msgType == 'r')
        sendStatusUpdate();
    else if (msgType == 'u')
        processStatusUpdate();
}

void sendStatusUpdate() {
    char msgType = 'u';
    Serial.write(msgType);    
    Serial.write(led09brightness);    
    Serial.write(led10brightness);    
    Serial.write(led11brightness);    
    Serial.write(led13Status);
}

void processStatusUpdate() {
    return;
}

void sendIlluminationUpdate() {
    char msgType = 'l';
    int analogValue = analogRead(A3);
    int illumination = calculateIllumination(analogValue);

    byte highByte = illumination >> 8;
    byte lowByte = illumination & 0xFF;
    
    Serial.write(msgType);
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
