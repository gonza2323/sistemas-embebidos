#include <Arduino_FreeRTOS.h>
#include <semphr.h>


// config
int ALARM_THRESHOLD = 80.0 / 100.0 * 1024;
int READ_INTERVAL = 200;
int SEND_INTERVAL = 3000;
int RECEIVE_MSG_INTERVAL = 200;
int INTERRUPT_INTERVAL = 100;

// tipos de mensajes
char READ_ON = 'Y';
char READ_OFF = 'N';
char ALARM_TRIGGERED = 'A';
char FILL = 'X';

// semáforos
SemaphoreHandle_t serialPortMutex;
SemaphoreHandle_t interruptSemaphore;
SemaphoreHandle_t readIlluminationSemaphore;
SemaphoreHandle_t alarmSemaphore;

int illumination = NULL;
bool alarmFiring = false;

// estructura para pasar argumentos a la tarea BlinkLED
typedef struct {
    int LED;
    int period;
    SemaphoreHandle_t semaphore;
} BlinkLEDTaskParams;


// inicialización
void setup() {
    // puerto serial y pines
    Serial.begin(115200);
    
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

    // tareas para lectura y envío de luminosidad, y manejo del botón
    xTaskCreate(TaskReadIllumination, "ReadIllumination", 96, NULL, 1, NULL);
    xTaskCreate(TaskSendIllumination, "SendIllumination", 96, NULL, 1, NULL);
    xTaskCreate(TaskHandleButtonPress, "HandleButton", 96, NULL, 2, NULL);

    // tareas para parpadeo de LEDs de lectura y alarma
    static BlinkLEDTaskParams readLED = {11, 1000, readIlluminationSemaphore};
    static BlinkLEDTaskParams alarmLED = {12, 100, alarmSemaphore};
    xTaskCreate(TaskBlinkLED, "BlinkReadLED", 72, &readLED, 1, NULL);
    xTaskCreate(TaskBlinkLED, "BlinkAlarmLED", 72, &alarmLED, 1, NULL);

    // tarea para leer el puerto serial
    xTaskCreate(TaskReadMessages, "ReadMessages", 96, NULL, 1, NULL);
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
            
            illumination = analogRead(A3);

            if (illumination > ALARM_THRESHOLD && !alarmFiring) {
                alarmFiring = true;
                xSemaphoreGive(alarmSemaphore);
                sendMessage(ALARM_TRIGGERED);
            }
            
            vTaskDelay(pdMS_TO_TICKS(READ_INTERVAL));
        }
    }
}

// enviar dato de iluminación por puerto serial
void TaskSendIllumination(void *pvParameters) {
    for(;;) {
        if (xSemaphoreTake(readIlluminationSemaphore, portMAX_DELAY) == pdTRUE) {
            xSemaphoreGive(readIlluminationSemaphore);
            sendInt(illumination);
            vTaskDelay(pdMS_TO_TICKS(SEND_INTERVAL));
        }
    }
}

void TaskReadMessages(void *pvParameters) {
    for(;;) {
        if (Serial.available()) {
            byte msg;
            if (xSemaphoreTake(serialPortMutex, portMAX_DELAY) == pdTRUE) {
                msg = Serial.read();
                xSemaphoreGive(serialPortMutex);
            }
            if (msg == READ_ON)
                turnOn();
            else if (msg == READ_OFF)
                turnOff();
        }
        vTaskDelay(pdMS_TO_TICKS(RECEIVE_MSG_INTERVAL));
    }
}

// control del botón: activa y desactiva lectura
void TaskHandleButtonPress(void *pvParameters) {
    for(;;) {
        if (xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS) {
            int isReadingIllumination = uxSemaphoreGetCount(readIlluminationSemaphore);
            if (isReadingIllumination) {
                turnOff();
                sendMessage(READ_OFF);
            } else {
                turnOn();
                sendMessage(READ_ON);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(INTERRUPT_INTERVAL));
    }
}

void turnOn() {
    xSemaphoreGive(readIlluminationSemaphore);
}

void turnOff() {
    xSemaphoreTake(readIlluminationSemaphore, 5);
    xSemaphoreTake(alarmSemaphore, 5);
    digitalWrite(11, LOW);
    digitalWrite(12, LOW);
    alarmFiring = false;
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

void sendInt(int value) {
    byte highByte = value >> 8;
    byte lowByte = value & 0xFF;

    if (xSemaphoreTake(serialPortMutex, portMAX_DELAY) == pdTRUE) {
        Serial.write(highByte);
        Serial.write(lowByte);
        xSemaphoreGive(serialPortMutex);
    }
}

void sendMessage(char message) {
    sendInt(256 * message + FILL);
}

void buttonHandler() {
    xSemaphoreGiveFromISR(interruptSemaphore, NULL);
}
