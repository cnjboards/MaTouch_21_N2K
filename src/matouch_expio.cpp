#include <Arduino.h>
#include <Wire.h>
#include "matouch_expio.h"
#include "Globals.h"
#include "TCA9554.h"
#include "matouch-pins.h"

// Task handle for expIO handling
TaskHandle_t expIOTaskHandle;

// instantiate TCA9554
TCA9554 TCA(TCA9554_Addr, &Wire);

// forward declarationa
void expIOTask (void * parameter);

// initialize expIO connection and task
void expIOInit(){

    // instantiate the expansion IO PCA9554PWR, for now use default 8 x input
    if (TCA.begin()) {
        // success
        Serial.println("TCA9554 found");
    } else {
        // not found so return, no expIO task
        Serial.println("TCA9554 not found");
        return;
    } // end if

    // kickoff expIo task
    xTaskCreatePinnedToCore (
        expIOTask , // Function to implement the task
        "expIO" ,        // Name of the task
        4096 ,          // Stack size in words
        NULL ,           // Task input parameter
        1 ,              // Priority of the task
        & expIOTaskHandle ,        // Task handle.
        1);              // Core where the task should run
    
    // small delay to allow task to startup
    delay(200);

} // end simulateInit()

// task to handle expIO
void expIOTask(void * parameter){

  // loop forever since we are a task
  for ( ; ; ) {

    // for now just print out 8 digital input states
    Serial.println("TEST read1(pin)");
    for (int pin = 0; pin < 8; pin++) {
        int val = TCA.read1(pin);
        Serial.print(val);
        Serial.print('\t');
    } // end for

    // do 500 msec updates, effectively poll expIO every 500 msec
    vTaskDelay(500);
  } // end for loop

  // should never get here, cleanup if we do
  vTaskDelete(NULL);
} // end expIOTask