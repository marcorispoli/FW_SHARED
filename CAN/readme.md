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

# 1. Tabel Of Content

- [1. Tabel Of Content](#1-tabel-of-content)
- [2. Overview](#2-overview)
- [3. CAN Communication settings](#3-can-communication-settings)
- [4. Harmony 3 Configurator Settings](#4-harmony-3-configurator-settings)
  - [4.1. Pin Assignement](#41-pin-assignement)
  - [4.2. Peripheral Clock Settings](#42-peripheral-clock-settings)
  - [4.3. CAN0 peripheral module](#43-can0-peripheral-module)
- [5. Module Usage](#5-module-usage)
  - [5.1. Module Setup](#51-module-setup)
  - [5.2. Module Initialization](#52-module-initialization)
  - [5.3. Command Execution implementation](#53-command-execution-implementation)
    - [5.3.1. Command Execution parameter access](#531-command-execution-parameter-access)
    - [5.3.2. Command return function and rule](#532-command-return-function-and-rule)
  - [5.4. Register data access](#54-register-data-access)

# 2. Overview

This module implements the Device Protocol communication basics. 

See the **Gantry Software Detailed Documentation - Device Protocol Specification** for details.

# 3. CAN Communication settings

The CAN communication is based on the following characteristics:

- Standard (not Flexible) communication mode;
- Standard frame with 11 bit Identifier;
- Baude Rate: 1Mb/s; 
- Can ID reception address: 0x200 + deviceID;

The deviceID is a decimal value from 1 to 255.


# 4. Harmony 3 Configurator Settings

> NOTE: this module has been developed for the SAME51 CAN Channel 0 only. It cannot be applied to the Channel 1.

This Module makes use of the following Processor modules:

- CAN0 peripheral module;
- Peripheral Can0 clock;
- PIN Assignement;

## 4.1. Pin Assignement

+ The CAN TX Input shall be configured with CAN0-TX 
+ The CAN RX Input shall be configured with CAN0-TX 

## 4.2. Peripheral Clock Settings

+ Activate the GLK4 Clock:
    + Set the DFLL (48MHz) as the Input clock;
    + Set the divisor to 2 to obtain a final 24MHz of GLK4 Clock;
+ The CAN0 Peripheral clock shall be configured with the GLK4 clock at 24 MHz.

## 4.3. CAN0 peripheral module

The following settings refers to only those settings that shall be modified.
Those settings that should not be reported shall be disabled or not modified.

+ CAN Operational Mode = NORMAL;
+ Interrupt Mode: Yes;
+ Bit Timing Calculation
+ Nominal Bit Timing
+ Automatic Nominal Bit Timing: Yes;
+ Bit Rate: 1000

+ Use RX FIFO 0: Yes
+ RX FIFO 0 Setting
+ Number of element: 1

+ Use TX FIFO: Yes
+ TX FIFO Setting
+ Number of element: 1

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

# 5. Module Usage

This module implements the basic activities of the Device Can protocol.

The Device application should implement a Container Module in order to 
implement the application dependent protocol parts, as well providing 
a unique module interface to the Device application.


## 5.1. Module Setup

The Application Harmony 3 configurator shall be set according with the previous 
Harmony 3 setup description. This setup will auto generate the necessary code.

The Application shall mandatory implement a application.h header file stored into 
the same directory where the main.c file is lockated (the root of the source project).
In the application.h file the application shall define the following macros:

   + #define MET_CAN_APP_MAJ_REV : Revision Major Number
   + #define MET_CAN_APP_MIN_REV : Revision Minor Number
   + #define MET_CAN_APP_SUB_REV : Revision build Number
   + #define MET_CAN_APP_DEVICE_ID : Application DEVICE CAN Id address
   + #define MET_CAN_STATUS_REGISTERS : Defines the total number of implemented STATUS registers 
   + #define MET_CAN_DATA_REGISTERS : Defines the total number of implemented Application DATA registers 
   + #define MET_CAN_PARAM_REGISTERS :  Defines the total number of implemented PARAMETER registers 

The Application that shall implement Execution Commands shall declare and instance 
the COmmand Execution Callback routine that the CAN Module will call whenever a
Command request should be received:
  + void (*ApplicationCommandHandler)(void) : this routine will further passed as parameter 
  to the MET_Can_Protocol_Init() routine.

   
## 5.2. Module Initialization

The Application shall call the module initializer routine usually in the setup section of the main.c file:
+ MET_Can_Protocol_Init(ApplicationCommandHandler);

The Application shall call the Module Worker routine into the main.c loop():
+ MET_Can_Protocol_Loop()

## 5.3. Command Execution implementation

In order to properly handle with the application Command execution the application should 
implement the following template handler routine:

  void ApplicationCommandHandler(void){

    switch(MET_Can_Protocol_getCommandCode()){ // Gets the requested command code
      case COMMAND-XX:
        // Do something ....

        MET_Can_Protocol_setReturnCommand(....); // MANDATORY Returns a result
        break;
      
      default:
        MET_Can_Protocol_setReturnCommand(MET_CAN_COMMAND_ERROR,0,0,MET_CAN_COMMAND_NOT_AVAILABLE);    
      }
      
      
      return;
  }


> The function mandatory shall to call the function MET_Can_Protocol_setReturnCommand()
before to return!

### 5.3.1. Command Execution parameter access

In order to access the Command parameters the Application can use:
+ MET_Can_Protocol_getCommandCode(): returns the requested command code;
+ MET_Can_Protocol_getCommandParam0(): returns the optional parameter byte 0;
+ MET_Can_Protocol_getCommandParam1(): returns the optional parameter byte 1;
+ MET_Can_Protocol_getCommandParam2(): returns the optional parameter byte 2;
+ MET_Can_Protocol_getCommandParam3(): returns the optional parameter byte 3;

### 5.3.2. Command return function and rule

When a Command has been processed into the Command Handler routine 
the application shall mandatory use the function:

+ MET_Can_Protocol_setReturnCommand(MET_CommandExecStatus_t retstat, uint8_t ris0, uint8_t ris1, uint8_t error), where:
  + retstat: is the command execution status:
    + MET_CAN_COMMAND_COMPLETED: the command is successfully completed;
    + MET_CAN_COMMAND_EXECUTING: the command is started;
    + MET_CAN_COMMAND_ERROR: the command is failed;
  + ris0, ris1: they are two bytes of optional command result;
  + error: in case of MET_CAN_COMMAND_ERROR, this is the error cause.

There are a set of standard error that should be preferred:

+ MET_CAN_COMMAND_NO_ERROR: No Error 
+ MET_CAN_COMMAND_BUSY:   A command is executing
+ MET_CAN_COMMAND_INVALID_DATA: The Command data are invalid
+ MET_CAN_COMMAND_NOT_ENABLED: The Command cannot be momentary executed 
+ MET_CAN_COMMAND_NOT_AVAILABLE: The Command cannot be executed in this revision      
+ MET_CAN_COMMAND_WRONG_RETURN_CODE: The Command returned a non valid status code
+ MET_CAN_COMMAND_APPLICATION_ERRORS: Starting code for applicaiton specific errors     

If the appliction should add other error codes, the user code should be set as:
+ MET_CAN_COMMAND_APPLICATION_ERRORS + user code



## 5.4. Register data access

