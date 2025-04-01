#include <Arduino_FreeRTOS.h>


int illumination = NULL;

void setup() {
    Serial.begin(9600);
    
    pinMode(12, OUTPUT);
    pinMode(A3, INPUT);

    xTaskCreate(
    TaskReadIllumination
    ,  "ReadIllumination"
    ,  128  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL );

    xTaskCreate(
    TaskSendIllumination
    ,  "ReadIllumination"
    ,  128  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL );
}

void loop() { }

void TaskReadIllumination(void *pvParameters) {
    for(;;) {
        int analogValue = analogRead(A3);
        illumination = calculateIllumination(analogValue);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void TaskSendIllumination(void *pvParameters) {
    for(;;) {
        byte highByte = illumination >> 8;
        byte lowByte = illumination & 0xFF;
    
        // TODO: Should lock serial port
        Serial.write(highByte);
        Serial.write(lowByte);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
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
