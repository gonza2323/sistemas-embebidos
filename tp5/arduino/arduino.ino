
#define READ_FREQ_MS 5000

bool ledStatus = false;

void setup() {
    Serial.begin(115200);
    pinMode(A3, INPUT);
    pinMode(10, OUTPUT);
}

void loop() {
    int analogValue = analogRead(A3);
    Serial.write((uint8_t*)&analogValue, sizeof(int));
    ledStatus = !ledStatus;
    digitalWrite(10, ledStatus);
    delay(READ_FREQ_MS);
}
