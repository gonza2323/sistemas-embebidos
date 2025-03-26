bool ledStatus = false;

void setup() {
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
    
    Serial.begin(9600);
}

void loop() {
    if (Serial.available() > 0) {
        String message = Serial.readStringUntil('\n');
        ledStatus = !ledStatus;
        digitalWrite(13, ledStatus);
    }
    delay(250);
}
