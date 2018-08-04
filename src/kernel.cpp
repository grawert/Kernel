#include "kernel.h"

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


//timeout 
Timeout::Timeout(unsigned long wait, FuncDelegate f, unsigned int npid ) {
    next = micros() + wait;
    
    if ( next < micros() ) {
        overflows = 1;
    }
    else {
        overflows = 0;
    }
    
    func = f;
    intervalDelay = 0;
    pid = npid;
}

//interval
Timeout::Timeout(unsigned long wait, FuncDelegate f, unsigned int npid, boolean isIntervalInit ) {
    if ( isIntervalInit ) {
        next = micros() + wait;
    }
    else {
        next = wait;
    }
    
    if ( next < micros() ) {
        overflows = 1;
    }
    else {
        overflows = 0;
    }
    
    func = f;
    intervalDelay = wait;
    pid = npid;
}

boolean Timeout::operator<(const Timeout &rhs) {
    if (overflows != rhs.overflows) {
        return overflows < rhs.overflows;
    }
    else {
        return next < rhs.next;
    }
}
boolean Timeout::operator<(const unsigned long time) {
    return next < time;
}
boolean Timeout::operator==(const unsigned int tpid) {
    return pid == tpid;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////


Kernel::Kernel() {
    //initialize interval array
    curPID = 1;
    
    actualTime = 0;
    lastTimeCheck = 0;
    
    delayAdjustment = -30;
    
    fallingBehind = 0;
    
    timeouts.setPrinter( Serial );
}


int Kernel::setTimeout( unsigned long wait, void (*f)(void) ) {
    FastDelegate0<> ff( f );
    Timeout temp(wait, ff, curPID);
    timeouts.insert( temp );
    return curPID++;
}
int Kernel::setTimeout( unsigned long wait, FuncDelegate &f ) {
    Timeout temp(wait, f, curPID);
    timeouts.insert( temp );
    return curPID++;
}
int Kernel::setInterval( unsigned long wait, void (*f)(void) ) {
    FastDelegate0<> ff( f );
    Timeout temp(wait, ff, curPID, true);
    timeouts.insert( temp );
    return curPID++;
}
int Kernel::setInterval( unsigned long wait, FuncDelegate &f ) {
    Timeout temp(wait, f, curPID, true);
    timeouts.insert( temp );
    return curPID++;
}
void Kernel::reAddInterval( Timeout& t ) {
    if ( t.intervalDelay != 0 ) {
      
      //DROP intervals if falling behind - or at least add some lag
      //kinda bad, but keeps fast intervals from blocking out slower ones
      if ( actualTime > t.next + 1000UL ) {
        fallingBehind += 10;
        t.next = micros();
      }   
      
      if ( t.next > t.next + t.intervalDelay ) {
            t.overflows = 1;
        }
        t.next += t.intervalDelay;
        
        timeouts.insert( t );
    }
}


void Kernel::decreaseOverflow(Timeout & t ) {
    if ( t.overflows > 0 )  {
        t.overflows --;
    }
    else {
        t.overflows = 0;
    }
}
void Kernel::decreaseOverflows() {
    timeouts.applyAll( decreaseOverflow );
}


/*
unsigned int countItems;
void Kernel::countItem( Timeout & t ) {
    countItems++;
}
*/
unsigned int Kernel::countQueue() {
    /*countItems = 0;
    timeouts.applyAll( countItem );
    return countItems;
    */
    return timeouts.countReal();
}


void Kernel::runNow() {
    Timeout next = timeouts.peek();
    runNow( next );
}
void Kernel::runNow(Timeout &next) {
    while (next.overflows == 0 && next.next <= micros() + 15UL ) {
        actualTime = micros();
        
        next.func();
        
        timeouts.pop();
        
        reAddInterval( next );
        
        if ( timeouts.isEmpty() ) { 
            Serial.println( "Timouts is empty." );
            break; 
        }
        
        next = timeouts.peek();
        /*
        if ( next.overflows == 0 && next.next <= micros() + 15UL ) {
            Serial.print(micros());
            Serial.print("\tDO ANOTHER\t");
            Serial.println( next.next );
        }
        */
    }
}


void Kernel::runNext() {
    
    
    //if we are now LESS than the last time it was run, overflow happened
    if ( micros() <  lastTimeCheck ) {
        decreaseOverflows();
    }
    lastTimeCheck = micros(); 
    
    
    
    //no tasks are waiting. Something's probably wrong!
    if ( timeouts.isEmpty() ) {
        Serial.print( micros() );
        Serial.print( "\t" );
        Serial.println( "No tasks waiting" );
        delay( 1000 );
        return;
    }
    
    
    long wait;
    Timeout nextTimeout = timeouts.peek();
    
    
    //unsigned long targetTime = nextTimeout.next;
    
    if ( nextTimeout.overflows > 1 ) {
        wait = 67108864;
    }
    if ( nextTimeout.overflows == 1 ) {
        //Serial.println( "\n\n\n\n\n\n\n\n\noverflow111111111111111111111111111111111111111111111111\n" );
        
        unsigned long maxlong = 0;
        maxlong--;
        
        wait = nextTimeout.next;
        maxlong -= micros();
        wait += maxlong;
    } 
    else {
        wait = nextTimeout.next - micros();
    }
    //wait += delayAdjustment;
    
    
    
    
    if( wait >= 16383 ) {
        delay( wait / 1000UL - 8UL );
    }
    else if ( wait < 5 ) {
        runNow(nextTimeout);
    }
    else {
        delayMicroseconds(  wait );
        
        runNow(nextTimeout);
        
        if ( fallingBehind > 100 ) {
          Serial.println( "KERNEL WARNING: CANNOT KEEP UP. LAG ADDED" );
          fallingBehind = -1;
        }
        else if ( fallingBehind > 0 ) {
          fallingBehind--;
        }
        
        /*
        if ( targetTime > actualTime ) {
            delayAdjustment += targetTime - actualTime;
        }
        else {
            delayAdjustment -= actualTime - targetTime;
        }
        */
    }
}
