
### CJSON example: 

Sample application showcasing how to manage JSON objects. Debug prints on **USB0**


**Features**


- How to read a JSON using cJSON library
- How to write a JSON
- How to manipulate JSON objects


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX
- Parse an example string into a JSON object and print the result in a formatted string
- Print some test outcomes (e.g. non existing item correctly not found)
- Retrieve single elements from the parsed JSON object and use them to format a descriptive string
- Delete the JSON object
- Create a new JSON object appending elements to it
- Print the result JSON string from the object

![](../../pictures/samples/cjson_bordered.png)

---------------------

