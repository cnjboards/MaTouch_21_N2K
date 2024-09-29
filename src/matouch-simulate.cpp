#include <Arduino.h>
#include "Globals.h"

// Task handle for OneWire read
TaskHandle_t simulateDataTaskHandle;

// forward declarationa
void simulateDataTask (void * parameter);

// initialize code to simulate variable data
void simulateDataInit(){

    // kickoff a task to create simulated data
    xTaskCreatePinnedToCore (
        simulateDataTask , // Function to implement the task
        "Simulate" ,        // Name of the task
        4096 ,          // Stack size in words
        NULL ,           // Task input parameter
        1 ,              // Priority of the task
        & simulateDataTaskHandle ,        // Task handle.
        1);              // Core where the task should run
    
    // small delay to allow task to startup
    delay(200);

} // end simulateInit()

// task to periodically create data for display simulation
void simulateDataTask(void * parameter){

  // loop forecver since we are a task
  for ( ; ; ) {

    // temp is in Kelvin
    locEngCoolTemp = (float)(random ( 291 , 420)); // kelvin
    locEngAltVolt = (float)(random (1000 , 1480)/100); // volts
    locEngRPM = (float)(random(1200, 3500));
    locEngOilPres = (float)(random ( 0 , 551600 ));

    // do 500 msec updates
    vTaskDelay(500);

  } // end for loop

  // should never get here, cleanup if we do
  vTaskDelete(NULL);
} // end simulateDataTask