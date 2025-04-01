#include <Arduino_FreeRTOS.h>
#include <semphr.h>


int illumination = NULL;
SemaphoreHandle_t serialPortMutex;


void setup() {
    Serial.begin(9600);
    
    pinMode(12, OUTPUT);
    pinMode(A3, INPUT);

    serialPortMutex = xSemaphoreCreateMutex();

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
        
        if (xSemaphoreTake(serialPortMutex, portMAX_DELAY) == pdTRUE) {
            Serial.write(highByte);
            Serial.write(lowByte);
            xSemaphoreGive(serialPortMutex);
        }

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
