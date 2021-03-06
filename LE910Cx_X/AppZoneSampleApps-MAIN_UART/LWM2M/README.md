
### LWM2M

Sample application showcasing LWM2M client usage with M2MB API. Debug prints on **MAIN UART**


**Features**


- Configure LWM2M client and enable it

- Create an instance of a custom object

- Set an integer value on a read only resource

- Set two integer values on a multi-instance read only resource

- write a string on a read/write resource

- Manage exec requests from the portal

- Manage write, read and monitoring resources changed from the portal

**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Create a task to manage the LWM2M client and start it


**`lwm2m_demo.c`**

**`msgLWM2MTask`**
- Check registration status

- Configure APN to the correct one for CID 1

- Initialize LWM2M client, 

  - Check for XML file fo custom object
  
  - Enable unsolicited messages from client
  
  - Create a task \(lwm2m_taskCB is its callback function \)to manage events from Portal
  
  - Enable LwM2M client
  
  - Create a new instance for the custom object
  
  - Wait for client to register to Portal
  
  - Send integer and string values
  
  - Wait for events from server


**`lwm2mIndicationCB`**

- Manage events arriving from client \(operations completion status and unsolicited events\)
- Run lwm2m_taskCB when a monitored resource changes, to manage the action to be done

#### Custom Object configuration

The XML file content must be loaded on the Telit IoT Portal for the demo application to be fully executed.

First, enter Developer section from the top menu

![](../../pictures/samples/lwm2m_xml_1_developer_bordered.png)


Choose Object Registry

![](../../pictures/samples/lwm2m_xml_2_object_registry_bordered.png)


Create a New Object

![](../../pictures/samples/lwm2m_xml_3_new_object_bordered.png)


Copy the xml file content and paste it in the new Object form

![](../../pictures/samples/lwm2m_xml_4_paste_content_bordered.png)



Also, the application requires the XML file `/xml/object_35000.xml` (provided with the sample files) to be stored in module's `/XML/` folder. 
It can be done with 

`AT#M2MWRITE=/XML/object_35000.xml,<size_in_bytes>`

To load the XML file in the module, Telit AT Controller (TATC) can be used. Once the command above is issued, press the load content button:

![](../../pictures/samples/lwm2m_xml_5_load_xml_bordered.png)


Select the file from your computer

![](../../pictures/samples/lwm2m_xml_6_file_select_bordered.png)


The file is successfully loaded on the module

![](../../pictures/samples/lwm2m_xml_7_done_bordered.png)


#### Application execution example

![](../../pictures/samples/lwm2m_1_bordered.png)

![](../../pictures/samples/lwm2m_2_bordered.png)


After the Demo completes the initialization, it is possible to access the object resources from the Portal Object Browser

![](../../pictures/samples/lwm2m_portal_object_browser_bordered.png)

An instance of the object will be present and the resources can be modified. 

![](../../pictures/samples/lwm2m_portal_object_bordered.png)

For example, executing the two Exec Resources at the bottom of the list, the application will react accordingly:

![](../../pictures/samples/lwm2m_3_exec_bordered.png)

Writing a string resource (id /35000/0/11 ), the application will notify the change

![](../../pictures/samples/lwm2m_4_write_bordered.png)

---------------------

