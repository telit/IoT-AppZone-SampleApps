
### LWM2M OBJ_GET AND OBJ_SET

Sample application showcasing LWM2M client m2mb_lwm2m_objget and m2mb_lwm2m_objset M2MB APIs usage. Debug prints on **MAIN UART**


**Features**


- Configure LWM2M client and enable it

- Create an instance of a custom object

- Create a Json string

- Set string, integer, float, boolean, timestamp and opaque values with `m2mb_lwm2m_objset`

- Get all resources values with `m2mb_lwm2m_objget`

- Manage write, read and monitoring resources changed from the portal

**Requirements**

This application expects the user to configure the PDP context ID 1 with the proper APN.
it can be done with the following AT command:

`AT+CGDCONT=1,"IPV4V6","<user apn>"`

Depending on the Mobiler Network Operator and Access Technology, the APN might be automatically set by the network itself. In this case, nothing must be done by the user.


**Application workflow**

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Create a task to manage the LWM2M client and start it


**`lwm2m_demo.c`**

**`msgLWM2MTask`**
- Check registration status

- Initialize LWM2M client,

  - Check for XML file fo custom object

  - Enable unsolicited messages from client

  - Create a task \(lwm2m_taskCB is its callback function \)to manage events from Portal

  - Enable LwM2M client

  - Create a new instance for the custom object

  - Wait for client to register to Portal

  - Performs obj_set and obj_get operations on the related resources

  - Wait for events from server


**`lwm2mIndicationCB`**

- Manage events arriving from client \(operations completion status and unsolicited events\)
- Run lwm2m_taskCB when a monitored resource changes, to manage the action to be done

#### Device Profile upload

**LWM2M resources demo** device profile must be imported to have a real-time update of resources values on the LWM2M browser.

To do so, import the file `lwm2m_resources_demo.json` on section `Developer` > `Device profiles` of OneEdge IoT portal:

![](../../pictures/samples/lwm2m_device_profile_bordered.png)


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



Also, the application requires the XML file `/xml/object_32011.xml` (provided with the sample files) to be stored in module's `/XML/` folder.
It can be done with

`AT#M2MWRITE=/XML/object_32011.xml,<size_in_bytes>`

To load the XML file in the module, Telit AT Controller (TATC) can be used. Once the command above is issued, press the load content button:

![](../../pictures/samples/lwm2m_xml_5_load_xml_bordered.png)


Select the file from your computer

![](../../pictures/samples/lwm2m_xml_6_file_select_bordered.png)


The file is successfully loaded on the module

![](../../pictures/samples/lwm2m_xml_7_done_bordered.png)


#### Application execution example

![](../../pictures/samples/lwm2m_1_bordered.png)


![](../../pictures/samples/lwm2m_obj_set_get_1_bordered.png)
![](../../pictures/samples/lwm2m_obj_set_get_2_bordered.png)
![](../../pictures/samples/lwm2m_obj_set_get_3_bordered.png)



After the Demo completes the initialization, it is possible to access the object resources from the Portal Object Browser

![](../../pictures/samples/lwm2m_portal_object_browser_bordered.png)

An instance of the object will be present and the resources can be modified.

![](../../pictures/samples/lwm2m_obj_set_get_portal_bordered.png)

---------------------

