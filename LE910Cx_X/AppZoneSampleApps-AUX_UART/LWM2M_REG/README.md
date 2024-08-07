
### LWM2M REG

Sample application showcasing LWM2M client registration management using M2MB API. Debug prints on **AUX UART**


**Features**


- Configure LWM2M client and enable it

- Get LWM2M server information using REG apis

- Deregister from LWM2M server using REG apis

- Register to the LWM2M server usign REG apis

- Update registration on LWM2M server using REG apis

- Deregister from LWM2M server using REG apis

**Requirements**

This application expects the user to configure the PDP context ID 1 with the proper APN.
it can be done with the following AT command:

`AT+CGDCONT=1,"IPV4V6","<user apn>"`

Depending on the Mobiler Network Operator and Access Technology, the APN might be automatically set by the network itself. In this case, nothing must be done by the user.


#### Application workflow

**`M2MB_main.c`**

- Open USB/UART/UART_AUX

- Create a task to manage the LWM2M client and start it


**`lwm2m_demo.c`**

**`msgLWM2MTask`**
- Check registration status

- Initialize LWM2M client, 

  - Check for XML file fo custom object
  
  - Enable LwM2M client
  
  - Wait for client to register to Portal
  
  - Get the LWM2M server information

  - Performs client portal deregistration
  
  - Performs client portal registration
  
  - Performs client portal registration Update
  
  - Performs client portal deregistration
  

**`lwm2mIndicationCB`**

- Manage events arriving from client \(operations completion status and unsolicited events\)

#### Application execution example

![](../../pictures/samples/lwm2m_reg_bordered.png)

#### Device Profile upload

**LWM2M resources demo** device profile must be imported to have a real-time update of resources values on the LWM2M browser. 

To do so, import the file `json/lwm2m_resources_demo.json` (provided with the sample files) on section `Developer` > `Device profiles` of OneEdge IoT portal:

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



Also, the application requires the XML file `xml/object_32010.xml` (provided with the sample files) to be stored in module's `/XML/` folder. 
It can be done with 

`AT#M2MWRITE=/XML/object_32010.xml,<size_in_bytes>`

To load the XML file in the module, Telit AT Controller (TATC) can be used. Once the command above is issued, press the load content button:

![](../../pictures/samples/lwm2m_xml_5_load_xml_bordered.png)


Select the file from your computer

![](../../pictures/samples/lwm2m_xml_6_file_select_bordered.png)


The file is successfully loaded on the module

![](../../pictures/samples/lwm2m_xml_7_done_bordered.png)

#### Onboard the device

**Get the Telit ID**

To retrieve the Telit ID data, issue `AT#TID` to get the Telit ID. The command response will be similar to

\#TID: **xxxxxxxxxxxxxxxxxxxxxxxxxxx**,1
OK


Take note of the Telit ID highlighted in **bold** above (or copy it on a text editor): this ID it will be needed for the onboarding process.

**Create a new Thing**

From the OneEdge portal, on **"Things"** section, click **"New Thing"** button in the top right corner.

![](../../pictures/samples/lwm2m_new_thing_bordered.png)

In the Create a new thing dialog, select "Telit Module"

![](../../pictures/samples/lwm2m_telit_module_bordered.png)

A dialog appears: select â€œDefaultâ€� thing definition

![](../../pictures/samples/lwm2m_fota_ack_default_thing_bordered.png)

In the following screen, provide the Telit ID as â€œIdentifierâ€�
Click on â€œFindâ€� and make sure that model, firmware and the other details are properly
populated.

Click on lwm2m tab and set the device profile previously imported as shown in the screenshot below

![](../../pictures/samples/lwm2m_demo_device_profile_bordered.png)

Click **"Add"** to complete the new thing creation procedure.

**If the Thing already exists, its device profile can be changed by following the steps shown in the picture below**

![](../../pictures/samples/lwm2m_change_device_profile_bordered.png)

---------------------

