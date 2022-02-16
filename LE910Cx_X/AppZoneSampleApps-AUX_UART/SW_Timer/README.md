
### SW Timer (Software Timer)

The sample application shows how to use SW Timers M2MB API. Debug prints on **AUX UART**


**Features**


- How to open configure a SW timer
- How to use the timer to manage recurring events


#### Application workflow

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Print welcome message

- Create sw timer structure

- Configure it with 4 seconds timeout, periodic timer (auto fires when expires)

- Init the timer with the parameters

- Start the timer

- Wait 10 seconds

- Stop the timer

**`timerCb`**

- Print a message with inside the callback

![](../../pictures/samples/sw_timer_bordered.png)

---------------------

