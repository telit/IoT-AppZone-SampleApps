
### C++  method to function pointer

Sample application showing how to manage class methods as function pointers. Debug prints on MAIN_UART


**Features**


- how to define a class object with a generic method with the same prototype as a m2mb callback function (in this case, a hw timer callback)
- how to use a single static function in the class workspace to call multiple class instances method by using "this" as argument in the timer creation
- how to configure the static function to convert the input parameter with a static cast and call the input class instance method


**Application workflow**

**`M2MB_main.c`**

- Call C++ main function

**`main.cpp`**

- Create two HwTimer class instance with different timeouts
- Start both timers.
- Each will expire at a different time, and both m2mb timers will call the static function, which will run the appropriate class instance method as callback.


![](../../pictures/samples/cpp_method_bordered.png)

---------------------

