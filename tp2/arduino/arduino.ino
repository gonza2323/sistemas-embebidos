#include <Arduino_FreeRTOS.h>
#include <semphr.h>


int illumination = NULL;
SemaphoreHandle_t serialPortMutex;
SemaphoreHandle_t interruptSemaphore;
SemaphoreHandle_t readIlluminationSemaphore;


void setup() {
    Serial.begin(9600);
    
    pinMode(12, OUTPUT);
    pinMode(A3, INPUT);

    serialPortMutex = xSemaphoreCreateMutex();
    interruptSemaphore = xSemaphoreCreateBinary();
    readIlluminationSemaphore = xSemaphoreCreateBinary();

    // xSemaphoreGive(readIlluminationSemaphore);

    xTaskCreate(
    TaskReadIllumination
    ,  "ReadIllumination"
    ,  128
    ,  NULL
    ,  1
    ,  NULL );

    xTaskCreate(
    TaskSendIllumination
    ,  "ReadIllumination"
    ,  128
    ,  NULL
    ,  1
    ,  NULL );

    xTaskCreate(
    TaskHandleButtonPress
    ,  "ReadIllumination"
    ,  128
    ,  NULL
    ,  2
    ,  NULL );

    attachInterrupt(digitalPinToInterrupt(3), buttonHandler, RISING);
}

void loop() { }

void TaskReadIllumination(void *pvParameters) {
    for(;;) {
        if (xSemaphoreTake(readIlluminationSemaphore, portMAX_DELAY) == pdTRUE) {
            int analogValue = analogRead(A3);
            illumination = calculateIllumination(analogValue);
            Serial.println("read illumination");
            xSemaphoreGive(readIlluminationSemaphore);
            vTaskDelay(pdMS_TO_TICKS(300));
        }
    }
}

void TaskSendIllumination(void *pvParameters) {
    for(;;) {
        if (xSemaphoreTake(readIlluminationSemaphore, portMAX_DELAY) == pdTRUE) {
            byte highByte = illumination >> 8;
            byte lowByte = illumination & 0xFF;
            
            if (xSemaphoreTake(serialPortMutex, portMAX_DELAY) == pdTRUE) {
                // Serial.write(highByte);
                // Serial.write(lowByte);
                Serial.println("send illumination");
                xSemaphoreGive(serialPortMutex);
            }

            xSemaphoreGive(readIlluminationSemaphore);
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
    }
}

void TaskHandleButtonPress(void *pvParameters) {
    for(;;) {
        if (xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS) {
            int isReadingIllumination = uxSemaphoreGetCount(readIlluminationSemaphore);
            if (isReadingIllumination) {
                xSemaphoreTake(readIlluminationSemaphore, portMAX_DELAY);
            } else {
                xSemaphoreGive(readIlluminationSemaphore);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
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

void buttonHandler() {
    xSemaphoreGiveFromISR(interruptSemaphore, NULL);
}
