#include <Arduino_FreeRTOS.h>
#include <semphr.h>


// config
int ALARM_THRESHOLD = 800;
int ILM_READ_INTERVAL = 200;
int ILM_UPDATE_INTERVAL = 3000;
int INTERRUPT_INTERVAL = 100;

// semáforos
SemaphoreHandle_t serialPortMutex;
SemaphoreHandle_t interruptSemaphore;
SemaphoreHandle_t readIlluminationSemaphore;
SemaphoreHandle_t alarmSemaphore;

int illumination = NULL;

// estructura para pasar argumentos a la tarea BlinkLED
typedef struct {
    int LED;
    int period;
    SemaphoreHandle_t semaphore;
} BlinkLEDTaskParams;


// inicialización
void setup() {
    // puerto serial y pines
    Serial.begin(9600);
    
    pinMode(3, INPUT);
    pinMode(A3, INPUT);
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(3), buttonHandler, RISING);

    // inicializar semáforos
    serialPortMutex = xSemaphoreCreateMutex();
    interruptSemaphore = xSemaphoreCreateBinary();
    readIlluminationSemaphore = xSemaphoreCreateBinary();
    alarmSemaphore = xSemaphoreCreateBinary();

    xSemaphoreGive(readIlluminationSemaphore);

    // crear tareas para lectura, envío y manejo del botón
    xTaskCreate(TaskReadIllumination, "ReadIllumination", 96, NULL, 1, NULL);
    xTaskCreate(TaskSendIllumination, "SendIllumination", 96, NULL, 1, NULL);
    xTaskCreate(TaskHandleButtonPress, "HandleButton", 96, NULL, 2, NULL);

    // tareas para parpadeo de LEDs de lectura y alarma
    static BlinkLEDTaskParams readLED = {11, 1000, readIlluminationSemaphore};
    static BlinkLEDTaskParams alarmLED = {12, 100, alarmSemaphore};
    xTaskCreate(TaskBlinkLED, "BlinkReadLED", 96, &readLED, 1, NULL);
    xTaskCreate(TaskBlinkLED, "BlinkAlarmLED", 96, &alarmLED, 1, NULL);

}

void loop() { }


// ---------------------------------------------- //
// ------------------- TAREAS ------------------- //
// ---------------------------------------------- //


// lectura del sensor de iluminación
void TaskReadIllumination(void *pvParameters) {
    for(;;) {
        if (xSemaphoreTake(readIlluminationSemaphore, portMAX_DELAY) == pdTRUE) {
            xSemaphoreGive(readIlluminationSemaphore);
            
            int analogValue = analogRead(A3);
            illumination = calculateIllumination(analogValue);

            if (illumination > ALARM_THRESHOLD) {
                xSemaphoreGive(alarmSemaphore);
            }
            
            vTaskDelay(pdMS_TO_TICKS(ILM_READ_INTERVAL));
        }
    }
}

// enviar dato de iluminación por puerto serial
void TaskSendIllumination(void *pvParameters) {
    for(;;) {
        if (xSemaphoreTake(readIlluminationSemaphore, portMAX_DELAY) == pdTRUE) {
            xSemaphoreGive(readIlluminationSemaphore);
            
            byte highByte = illumination >> 8;
            byte lowByte = illumination & 0xFF;
            
            if (xSemaphoreTake(serialPortMutex, portMAX_DELAY) == pdTRUE) {
                // Serial.write(highByte);
                // Serial.write(lowByte);
                xSemaphoreGive(serialPortMutex);
            }

            vTaskDelay(pdMS_TO_TICKS(ILM_UPDATE_INTERVAL));
        }
    }
}

// control del botón: activa y desactiva lectura
void TaskHandleButtonPress(void *pvParameters) {
    for(;;) {
        if (xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS) {
            int isReadingIllumination = uxSemaphoreGetCount(readIlluminationSemaphore);
            if (isReadingIllumination) {
                xSemaphoreTake(readIlluminationSemaphore, 5);
                xSemaphoreTake(alarmSemaphore, 5);
                digitalWrite(11, LOW);
                digitalWrite(12, LOW);
            } else {
                xSemaphoreGive(readIlluminationSemaphore);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(INTERRUPT_INTERVAL));
    }
}

// control del parpadeo de un LED
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


// ------------------- AUX -------------------- //


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
