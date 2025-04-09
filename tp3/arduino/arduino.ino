#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <EEPROM.h>

// config
#define DEBOUNCE_DELAY 30
#define RECEIVE_MSG_INTERVAL 20
#define MAX_EVENT_AMOUNT 147

// tipos de mensajes
#define ERASE_MEMORY_MSG 'M'
#define REQUEST_EVENTS_MSG 'R'
#define SYNC_MSG 'S'
#define SINGLE_EVENT_MSG 255

// variables
uint32_t unixTimeSecondsAtSync = 0;
uint16_t unixTimeMilisAtSync = 0;
TickType_t tickCountAtSync = 0;
uint8_t eventAmount = 0;
uint8_t checksum = 0;

// estructura Button
typedef struct {
    uint8_t pin;
    SemaphoreHandle_t semaphore;
    TaskHandle_t task;
} Button;

// botones
Button button2 = {2, NULL, NULL};
Button button3 = {3, NULL, NULL};

// estructura evento
typedef struct {
    uint8_t pin;
    uint32_t tickCount;
} Event;

typedef struct {
    uint8_t pin;    
    uint32_t unixTimeSeconds;  
    uint16_t unixTimeMilis;
} EventData;

// control
QueueHandle_t eventSaveQueue;

// handles de tareas
TaskHandle_t blinkLedTaskHandle;


// inicialización
void setup() {
    // puerto serial y pines
    Serial.begin(9600);
    pinMode(2, INPUT);
    pinMode(3, INPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(11, OUTPUT);

    if (checkEepromHasValidState()) {
        eventAmount = EEPROM.read(EEPROM.length() - 1);
        checksum = EEPROM.read(EEPROM.length() - 2);
    } else {
        EEPROM.write(EEPROM.length() - 1, 0);
        EEPROM.write(EEPROM.length() - 2, 0);
    }

    eventSaveQueue = xQueueCreate(4, sizeof(Event));
    button2.semaphore = xSemaphoreCreateBinary();
    button3.semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(button2.semaphore);
    xSemaphoreGive(button3.semaphore);

    xTaskCreate(TaskProcessEvent, "ProcessEvents", 128, NULL, 1, NULL);
    xTaskCreate(TaskDebounceButton, "DebounceButton2", 96, &button2, 1, &button2.task);
    xTaskCreate(TaskDebounceButton, "DebounceButton3", 96, &button3, 1, &button3.task);
    xTaskCreate(TaskBlinkOnceLED, "BlinkLEDOnce", 96, NULL, 1, &blinkLedTaskHandle);
    xTaskCreate(TaskReadMessages, "ReadMsgs", 96, NULL, 2, NULL);

    attachInterrupt(digitalPinToInterrupt(2), button2ISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(3), button3ISR, CHANGE);
}

void loop() { }


// ---------------------------------------------- //
// ------------------- TAREAS ------------------- //
// ---------------------------------------------- //

// parpadear LED una vez
void TaskBlinkOnceLED(void *pvParameters) {
    for(;;) {
        uint32_t ledPin;
        if (xTaskNotifyWait(0, 0, &ledPin, portMAX_DELAY) == pdTRUE) {
            digitalWrite(ledPin, HIGH);
            vTaskDelay(pdMS_TO_TICKS(100));
            digitalWrite(ledPin, LOW);
        }
    }
}

void TaskDebounceButton(void *pvParameters) {
    Button* button = pvParameters;
    TickType_t tickCountAtEvent;
    
    for(;;) {
        if (xTaskNotifyWait(0, 0, (uint32_t*)&tickCountAtEvent, pdMS_TO_TICKS(20 * 1000)) == pdTRUE) {
            if (digitalRead(button->pin)) {
                Event event = {button->pin, tickCountAtEvent};
                xQueueSend(eventSaveQueue, &event, portMAX_DELAY);
            }
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY));
            xSemaphoreGive(button->semaphore);
        }
    }
}

void TaskProcessEvent(void *pvParameters) {
    Event event;

    while (1) {
        if (xQueueReceive(eventSaveQueue, &event, portMAX_DELAY) == pdTRUE) {
            uint32_t millisAtEvent = (event.tickCount - tickCountAtSync) * 1000UL / configTICK_RATE_HZ + unixTimeMilisAtSync;
            uint32_t unixTimestampSeconds = unixTimeSecondsAtSync + millisAtEvent / 1000;
            uint16_t unixTimestampMilis = millisAtEvent % 1000;
            
            EventData eventData = {event.pin, unixTimestampSeconds, unixTimestampMilis};

            if ((eventAmount + 1) * sizeof(EventData) < EEPROM.length() - 2) {
                EEPROM.put(eventAmount++ * sizeof(EventData), eventData);
                
                incrementChecksum(&eventData);
                saveState();
                Serial.write(SINGLE_EVENT_MSG);
                Serial.write((uint8_t*)&eventData, sizeof(EventData));
            }
        }
    }
}

void TaskReadMessages(void *pvParameters) {
    for(;;) {
        if (Serial.available()) {
            byte msg;
            msg = Serial.read();
            switch (msg) {
                case SYNC_MSG:
                    syncTime();
                    break;
                case REQUEST_EVENTS_MSG:
                    sendEvents();
                    break;
                case ERASE_MEMORY_MSG:
                    eraseMemory();
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(RECEIVE_MSG_INTERVAL));
    }
}

void syncTime() {
    tickCountAtSync = xTaskGetTickCount();
    
    while (Serial.available() < 6) { }

    unixTimeSecondsAtSync = 0;
    unixTimeSecondsAtSync |= (uint32_t)Serial.read();
    unixTimeSecondsAtSync |= (uint32_t)Serial.read() << 8;
    unixTimeSecondsAtSync |= (uint32_t)Serial.read() << 16;
    unixTimeSecondsAtSync |= (uint32_t)Serial.read() << 24;

    unixTimeMilisAtSync = 0;
    unixTimeMilisAtSync |= (uint16_t)Serial.read();
    unixTimeMilisAtSync |= (uint16_t)Serial.read() << 8;
}

void sendEvents() {
    Serial.write(eventAmount);
    for (int i = 0; i < eventAmount; i++) {
        EventData eventData;
        eventData = EEPROM.get(i * sizeof(EventData), eventData);
        Serial.write((uint8_t*)&eventData, sizeof(EventData));
    }
}

void eraseMemory() {
    eventAmount = 0;
    checksum = 0;
    saveState();
}

bool checkEepromHasValidState() {
    uint8_t storedEventAmount = EEPROM.read(EEPROM.length() - 1);
    uint8_t storedChecksum = EEPROM.read(EEPROM.length() - 2);

    if (storedEventAmount > MAX_EVENT_AMOUNT)
        return false;
    
    uint8_t checksum = storedEventAmount;

    for (int i = 0; i < storedEventAmount * sizeof(EventData); i++)
        checksum += EEPROM.read(i);
    
    return checksum == storedChecksum;
}

void incrementChecksum(EventData* eventData) {
    uint8_t sum = 0;
    const uint8_t *p = (const uint8_t*)eventData;
    for (uint8_t i = 0; i < sizeof(EventData); i++)
        sum += p[i];
    checksum += sum + 1;
}

void saveState() {
    EEPROM.update(EEPROM.length() - 1, eventAmount);
    EEPROM.update(EEPROM.length() - 2, checksum);
}

// manejo de botones

void buttonISR(Button* button) {
    TickType_t currentTicks = xTaskGetTickCountFromISR();
    
    if (xSemaphoreTakeFromISR(button->semaphore, 0) == pdTRUE) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTaskNotifyFromISR(button->task, currentTicks, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            taskYIELD();
        }
    }
}

void button2ISR() { buttonISR(&button2); }
void button3ISR() { buttonISR(&button3); }
