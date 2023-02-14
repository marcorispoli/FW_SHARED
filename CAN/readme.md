<center><font size =6">Z190 SAME51 Shared Firmware Documentation</font></center>
<center><font size =5">Device Can Protocol</font></center>
<center><font size =4">REV : 1.0</font></center>


 <br/><br/>
 <br/><br/>
 <br/><br/>
 <br/><br/>

<center>
<font size =4">

|Date |Revision | Change log| Author|
|:---| :----: | :----: |:---|
|14/02/2023|1.0|First release| M. Rispoli|

</font>
</center>

<div style="page-break-after: always;"></div>

# Overview

This module implements the engine communication functionalities 
of the Z190 Device Communication protocol specifications.

The Module makes use of the CAN Channel 0.

# Communication settings

The CAN communication is based on the following   characteristics:

- Standard (not Flexible) communication;
- Standard frame (11 bit ID);
- Baude Rate: 1Mb/s; 
- Can ID reception address: 0x200 + deviceID;

The deviceID is a decimal value from 1 to 255.


# Harmony 3 Configurator Settings

This Module makes use of the following Processor modules:

- CAN0 peripheral module;
- Peripheral Can0 clock
- PIN Assignement

## Pin Assignement

+ The CAN TX Input shall be configured with CAN0-TX 
+ The CAN RX Input shall be configured with CAN0-TX 

## Peripheral Clock Settings

+ The CAN0 Peripheral clock shall be configured with a convenient Clock: 60MHz it is OK.

## CAN0 peripheral module

The following settings refers to only those settings that shall be modified.
Those settings that should not be reported shall be disabled or not modified.

+ CAN Operational Mode = NORMAL;
+ Interrupt Mode: Yes;
+ Bit Timing Calculation
+ Nominal Bit Timing
+ Automatic Nominal Bit Timing: Yes;
+ BIt Rate: 1000

+ Use RX FIFO 0: Yes
+ RX FIFO 0 Setting
+ Number of element: 1

+ Use TX FIFO: Yes
+ TX FIFO Setting
+ Number of element: 1
*
+ EnableTX Pause: YES
+ Standard Filters 
+ Number Of STandard Filters: 1
+ Standard Filter 1
+ Type: Range;
+ ID1: 0x200 + Device ID
+ ID12: 0x200 + Device ID
+ Element Configuration: Store in RX FIFO 0
+ Standard message No-Match disposion: Reject
+ Reject Standard Remote Frames: YES
+ Timestamp Enable: YES 

# Module Usage

This module implements the basic activities of the Device Can protocol.

In order to customize the protocol to the Applicaiton specific requirements 
this module should be included in a Container module that implements the 
specific registers and commands behavior.

## Module Setup

The Application Harmony 3 configurator shall be set according with the previous 
Harmony 3 setup description. This setup will auto generate the necessary code.

## Module Initialization

The Application shall instances the DATA STATUS and PARAMETER arrays into the Container module;

The Application should provides a Container Initialization function that shall be 
called into the main.c file in the initialization section (before the MainLoop()). This Initializator-Function shall call the MOdule initializer function: 

> void MET_Can_Protocol_Init(uint8_t deviceID, MET_Register_t* pStatusArray, uint8_t StatusLen, MET_Register_t* pDataArray, uint8_t DataLen);

where:
+ deviceID: is the device address assigned to the Device in the device network (1:255);
+ pStatusArray: is the pointer to the array of STATUS registers;
+ StatusLen: is the number of the STATUS registers implemented into the application;
+ pDataArray: is the pointer to the array of DATA registers;
+ DataLen: is the number of the DATA registers implemented into the application;

## Module Working routine

The Can Module makes use of a working routine to handle the CAN activities:
+ void *MET_Can_Protocol_Loop(void)*

The Application shall call this routine in the MainLoop.

When a can frame is received, an interrupt routine into the module sets an internal flag that will be processed by the *MET_Can_Protocol_Loop()* routine in the Main Loop. 

This approach prevents locking the interrupt routine for long time and
allows the other application modules to access the registers without facing 
with synchronization issues as long as those processes accesses registers into the same Main Loop and not in interrupt routines.

## STATUS Register usage

The STATUS registers are register that can be write only by the Firmware application 
and Read Only by the Gantry. The STATUS registers allow the Gantry 
to get Device internal GPIO status or volatile internal status.

The Application shall mandatory implement the following STATUS registers:

|INDEX|NAME|DESCRIPTION|
|:---:|:---|:---|
|0|RESERVED|DON'T SET THIS RERGISTER|
|1|REVISION|Application revision code|
|2|SYSTEM|Application system register|
|3|ERRORS|Application errors register|
|4|COMMAND|Application command register|
|||
|...|...|More Application related Status registers|
|||

See the protocol specification for details

In the Container Module initializator the application should mandatory set  
the REVISION status register with the Application revision code.

The Application may set the STATUS registers along the application modules 
without consider the Can activities: the CAN module automatically will 
handle remote Status Read command.

## DATA Register usage

The DATA registers are register that can be write only by the Gantry 
and Read Only by the Firmware application. The DATA registers allow the Gantry 
to activate GPIO into the Device or to set volatile internal status.

The Application shall mandatory implement the following DATA registers:

|INDEX|NAME|DESCRIPTION|
|:---:|:---|:---|
|0|RESERVED|DON'T READ THIS RERGISTER|
|1|....|Optionals application related DATA registers|
|||



