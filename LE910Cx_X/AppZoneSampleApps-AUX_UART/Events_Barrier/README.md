
### Events - Barrier (multi events)

Sample application showcasing how to setup and use multiple events to create a barrier. Debug prints on **AUX UART**


**Features**


- How to setup OS events to be used as a barrier
- How to wait for multiple events in the same point, and generate them in callback functions to synchronize blocks of code


#### Application workflow

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Create an event handler
- Create a timer to generate an event, with a 3 seconds expiration time
- Create another timer to generate an event, with a 6 seconds expiration time
- Start both timers
- Wait for both event bits on the event handler (each one will be set by one of the timers)
- At first timer expiration, set the first event bit and verify that the code flow does not procede.
- At second timer expiration, set the second event bit and verify that the code flow went through after the event (implementing a barrier).

![](../../pictures/samples/events_barrier_bordered.png)

---------------------

