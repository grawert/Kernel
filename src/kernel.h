#ifndef Kernel_h
#define Kernel_h

/*
 *  Copyright (C) 2013  Evan Boldt <robotic-controls.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/




/*

Add timeouts by accessing the Kernel methods.

To use kernel among several files or classes, use a global and 
decleare the kernel as an extern.


Global in .ino:
    Kernel kernel;

Add Function:
    kernel.setInterval(800UL, funcName ); //run funcName 800 microseconds

Add Method:
    extern Kernel kernel; // get the global from the main .ino file
    
    //make a fast delegate
    FuncDelegate f_delegate;
    f_delegate = MakeDelegate(this, & ClassName::memberFunctionName);
    kernel.setInterval( 700UL, f_delegate);
    
    

*/





#include "FastDelegate.h"
#include "QueueList.h"

using namespace fastdelegate;
typedef FastDelegate0<> FuncDelegate;





/*

The Timeout class is like a single process.

It has:
  a function (or function delegate).
  a unique PID or "process ID"


To keep track of the timing, it needs:
  next: the next time in microseconds
  intervalDelay: if it is an interval, it will need to keep track
                  of how frequent it should happen
  overflows: the number of overflows of the microseconds clock to wait


To help the Kernel class, it needs to be able to be compared, so 
the operators are overloaded.

*/
class Timeout {
    public: 
        Timeout() {};
        Timeout(unsigned long wait, FuncDelegate f, unsigned int npid ); //make a timeout
        Timeout(unsigned long wait, FuncDelegate f, unsigned int npid, boolean ); //make an interval
        
        unsigned long next;
        FuncDelegate func;
        unsigned long intervalDelay;
        unsigned int pid;
        
        unsigned int overflows;
        
        boolean operator<(const Timeout &rhs);
        boolean operator<(const unsigned long time);
        boolean operator==(const unsigned int);
};



/*

The kernel keeps track of the processes. It's like a task manager.

ALl interaction goes though here.


*/
class Kernel {
    private:
        // a queue for things to do
        QueueList<Timeout> timeouts;
        
        // the current PID number
        unsigned int curPID;
        
        void reAddInterval( Timeout&);
        
        unsigned long actualTime;
        unsigned long lastTimeCheck;
        long delayAdjustment;        
        
        unsigned int fallingBehind;
    public:
        Kernel();
                
        //add a timeout to the queue
        int setTimeout( unsigned long wait, void (*f)(void) );
        int setTimeout( unsigned long delay, FuncDelegate &f );
        int setInterval( unsigned long delay, FuncDelegate &f );
        int setInterval( unsigned long wait, void (*f)(void) );
        
        //remove a timeout from the queue
        boolean clearTimeout(  unsigned int tpid ) { return timeouts.remove( tpid ); };
        boolean clearInterval( unsigned int tpid ) { return timeouts.remove( tpid ); };
        
        //stop everything and empty the queue
        void stopALL() { timeouts.empty(); }
        
        
        static void printTimout(Timeout & t) {
            Serial.print(t.pid);
            Serial.print("\t");
            Serial.println(t.next);
        }
        
        //print the upcoming delays
        void debug() {
            Serial.println("\nKERNEL DEBUG:");
            
            Serial.print("Stored Count:\t");
            Serial.println( timeouts.count() );
            
            Serial.print("Real Count:\t");
            Serial.println( countQueue() );
            
            timeouts.applyAll( printTimout );
            
            /*
            Serial.println("pending:");
            while ( ! timeouts.isEmpty() ) {
                Serial.println( timeouts.pop().next );
            }
            */
        };
        
        static void decreaseOverflow(Timeout & t );
        void decreaseOverflows();
        
        static void countItem(Timeout & t );
        unsigned int countQueue();
        
        //execute any pending timeouts due to run now
        void runNow();
        void runNow(Timeout &t);
        
        //allow sleep, called from loop() usuaully
        void runNext();
};

#endif
