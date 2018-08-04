# Kernel - Event driven Delays and Intervals

## How it Works
There is a queue of functions.  They are in the ordered chronologically. Since it is a linked list,
the functions can be inserted in before other functions, but after others. Now all that Kernel
does is look at the next task coming up to see if it is the right time. If it is not time yet, it will
use delayMicroseconds to give it a precise start time. 

Repeating events (called intervals) will then be re-added to the queue in the correct
chronological order. Since the soonest event is always in the front of the queue, no events
will be passed over while using delay.

## Basic Usage
The example below shows how to add an interval. It is also possible to add a timeout by
using _setTimeout_ instead, which will only happen once.

```
#include <kernel.h>

Kernel kernel; //this should be a global, so other files can use it too.

void test() {
    Serial.println( micros() );
}

void setup() {
    Serial.begin(19200);
    
    //add a function to interval. It will run every 10,000 microseconds.
    kernel.setInterval(10000UL, test); // UL means the unsigned long
}

void loop() {
    //should avoid any code that could cause much delay here
    
    // runNext decides whether to delay or run the next function
    kernel.runNext(); 
}
```

## Advanced Usage

### Remove a task
Both _setInterval_ and _setTimeout_ return a unique integer PID. It can be used to later
remove a task from the queue.

```
int newPID = kernel.setInterval(100000UL, test);
kernel.clearInterval( newPID );
```

### Remove all tasks
Perhaps all current tasks need to stop happening. Maybe you've come accross an error and
want to stop everything.

```
kernel.stopALL();
```

### Break Up a Long Task
Sure, you could create a seperate task for a background task, but if your loop runs quickly
enough, you can essentially run kernel like you might a Metro, except it is only checking
the next pending task.

```
void loop() {
    for( int index = 0; i < hugeArray; i++ ) {
        //do some operations

        // will not delay until next task, tasks may be overdue
        kernel.runNow(); 
    }
}
```
You need to do either _runNow()_ frequently, or do _runNext()_ at the end. Otherwise,
no events will be called.

## Reasons to use Kernel

### _delay()_ is Bad
When you use _delay_ or _delayMicroseconds_ in Arduino, nothing else can happen until the
delay is finished, which can be a huge problem if you have more than one thing going on
simulteneously, as will always be the case when building a more advanced robot.

Imagine that you want to check a sensor once a second. Easy enough. Just delay then
check the sensor. What if you want to blink an LED on and off every 40ms, but still checking
that sensor. Any delays used by one will mess up the timing on the other. If you delay one
second after checking the sensor, the light will not blink during that time. You could make
a fixed number of blinks between sensor checks, but that is inflexible and accumulates error.

### Problems with Alternatives
Two alternatives already exist. Both are based on the idea of constantly asking
_can I go yet?_ in an if statement.

- You can check the times yourself, but that requires a lot of work and can make the code
  hard to read.

- You can also use Metro, which wraps the check up into a nice class where you just set an
  interval and run a .check() method.

I really like Metro, but it still requires the program to be written somewhat linearly instead of
in seperate functions, and it is not really meant to be used for one-time delays.

If you are using an Arduino Due, there the Scheduler library that provides an easier interface,
but is exclusive to the Due's ARM CPU.

### Events are Easy and Powerful
If you've ever dealt with GUIs or Javascript, you're probably comfortable with events. 
They're a nice way to make the most use out of a single threaded application
(like a browser window, or an Arduino) because, as long as you avoid any blocking, things
happen when they're supposed to.

Essentially, each _task_ is a function which gets called only when the _event_ happens. The
event, for example, can be a timing trigger or a user interaction.

Kernel takes care of timing the event calls. It can automatically maintain both run-once and
run-repeatedly events simulteneously.

Since Kernel is keeps track of which event needs to happen next, but still using
_delayMicroseconds_, better timing precision and consistency is achived.

### Other Potential Benefits
Since a Kernel is such an advanced framework, with a little additional programming, some
really useful features can be added:

- It could keep track of how long a task takes to complete, or give more important tasks
  priorities, and factor that into decisions of which task can go when, like a true kernel would
  for process management.

- A single _background_ process could be run, without interfering too much with the
  time-sensitive tasks. Which could be accomplished with the _runNow_ method.
