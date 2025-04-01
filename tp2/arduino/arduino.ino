#include <Arduino_FreeRTOS.h>
#include <semphr.h>


int ILM_READ_INTERVAL = 200;
int ILM_UPDATE_INTERVAL = 3000;
int INTERRUPT_INTERVAL = 100;

int illumination = NULL;

SemaphoreHandle_t serialPortMutex;
SemaphoreHandle_t interruptSemaphore;
SemaphoreHandle_t readIlluminationSemaphore;
SemaphoreHandle_t alarmSemaphore;

typedef struct {
    int LED;
    int period;
    SemaphoreHandle_t semaphore;
} BlinkLEDTaskParams;

void setup() {
    Serial.begin(9600);
    
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(A3, INPUT);

    serialPortMutex = xSemaphoreCreateMutex();
    interruptSemaphore = xSemaphoreCreateBinary();
    readIlluminationSemaphore = xSemaphoreCreateBinary();
    alarmSemaphore = xSemaphoreCreateBinary();

    xSemaphoreGive(readIlluminationSemaphore);

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

    static BlinkLEDTaskParams readLED = {11, 1000, readIlluminationSemaphore};
    static BlinkLEDTaskParams alarmLED = {12, 100, alarmSemaphore};

    xTaskCreate(
    TaskBlinkLED
    ,  "BlinkReadLED"
    ,  128
    ,  &readLED
    ,  1
    ,  NULL );

    attachInterrupt(digitalPinToInterrupt(3), buttonHandler, RISING);
}

void loop() { }

void TaskReadIllumination(void *pvParameters) {
    for(;;) {
        if (xSemaphoreTake(readIlluminationSemaphore, portMAX_DELAY) == pdTRUE) {
            Serial.write("read");
            int analogValue = analogRead(A3);
            illumination = calculateIllumination(analogValue);
            xSemaphoreGive(readIlluminationSemaphore);
            vTaskDelay(pdMS_TO_TICKS(ILM_READ_INTERVAL));
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
                Serial.write("send");
                xSemaphoreGive(serialPortMutex);
            }

            xSemaphoreGive(readIlluminationSemaphore);
            vTaskDelay(pdMS_TO_TICKS(ILM_UPDATE_INTERVAL));
        }
    }
}

void TaskHandleButtonPress(void *pvParameters) {
    for(;;) {
        if (xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS) {
            int isReadingIllumination = uxSemaphoreGetCount(readIlluminationSemaphore);
            if (isReadingIllumination) {
                xSemaphoreTake(readIlluminationSemaphore, portMAX_DELAY);
                digitalWrite(11, LOW);
            } else {
                xSemaphoreGive(readIlluminationSemaphore);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(INTERRUPT_INTERVAL));
    }
}

void TaskBlinkLED(void *pvParameters) {
    BlinkLEDTaskParams *params = (BlinkLEDTaskParams*) pvParameters;

    for(;;) {
        if (xSemaphoreTake(params->semaphore, portMAX_DELAY) == pdTRUE) {
            xSemaphoreGive(params->semaphore);
            digitalWrite(params->LED, HIGH);
            vTaskDelay(pdMS_TO_TICKS(params->period/2));
            digitalWrite(params->LED, LOW);
            vTaskDelay(pdMS_TO_TICKS(params->period/2));
        }
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
