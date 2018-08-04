#include "kernel.h"
//may need to use <> instead of "", depending on if you install Kernel.


//this should be a global, so other files can use it too.
Kernel kernel;


void test() {
    Serial.println(  micros()  );
}


void setup()  {
    Serial.begin(19200);
    
    //add a function to interval. It will run every 10,000 microseconds.
    kernel.setInterval(10000UL, test);
}


void loop(){
    
    //runNext decides whether to delay or run the next function
    kernel.runNext();
}
