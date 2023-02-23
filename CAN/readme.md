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
    - [5.1.1. *application.h* header file setup](#511-applicationh-header-file-setup)
    - [5.1.2. Executing Command handler declaration](#512-executing-command-handler-declaration)
  - [5.2. Module Initialization and activation](#52-module-initialization-and-activation)
  - [5.3. Command Execution implementation](#53-command-execution-implementation)
    - [5.3.1. Command return function and rule](#531-command-return-function-and-rule)
    - [5.3.2. Command delayed termination rules](#532-command-delayed-termination-rules)
  - [5.4. Register data access](#54-register-data-access)
    - [5.4.1. STATUS registers access functions](#541-status-registers-access-functions)
    - [5.4.2. ERRORS register access functions](#542-errors-register-access-functions)
    - [5.4.3. DATA registers access functions](#543-data-registers-access-functions)
    - [5.4.4. PARAMETER registers access functions](#544-parameter-registers-access-functions)
- [Work in progress](#work-in-progress)

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
- Peripheral SYSTEM;

## SYSTEM Peripheral module setup
  
  This Module makes use of the Smart EEPROM feature of the microcontroller.
  The module shall provides 256 32-bit word to be assigned to parameters.
  According with this requisite the SYSTEM peripheral module shall be set as follow:
  + SYSTEM/Device & Project Configuration/ATSAME51J20A Device Configuration/Fuse settings:
   + Number Of Phisical NVM Blocks Composing a SmartEEPROM Sector = 1;
   + Size Of SmartEEPROM Page = 1;

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

### 5.1.1. *application.h* header file setup

The Device Application shall ***mandatory*** implement an application.h header file stored into 
the same directory where the main.c file is located (the root of the source project).
In the application.h file the application shall define the following macros:

   + #define MET_CAN_APP_MAJ_REV : Revision Major Number
   + #define MET_CAN_APP_MIN_REV : Revision Minor Number
   + #define MET_CAN_APP_SUB_REV : Revision build Number
   + #define MET_CAN_APP_DEVICE_ID : Application DEVICE CAN Id address
   + #define MET_CAN_STATUS_REGISTERS : Defines the total number of implemented STATUS registers 
   + #define MET_CAN_DATA_REGISTERS : Defines the total number of implemented Application DATA registers 
   + #define MET_CAN_PARAM_REGISTERS :  Defines the total number of implemented PARAMETER registers 

### 5.1.2. Executing Command handler declaration

The Application that should implement Execution Commands shall declare and instance 
the Command Execution Callback routine that the CAN Module will call whenever a
Command request should be received. The prototype of such callback is:
  + void (*ApplicationCommandHandler)(uint8_t cmd, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) .

The callback routine will be passed to the Module initializer. See next section.

## 5.2. Module Initialization and activation

The Application shall call the module initializer routine usually in the setup section of the main.c file:
+ MET_Can_Protocol_Init(ApplicationCommandHandler): this function initialize the module data structure and the CAN registers.

The Application shall call the Module Worker routine into the main.c loop():
+ MET_Can_Protocol_Loop(): this routine handles the reception frames according with the MET protocol;


## 5.3. Command Execution implementation

In order to properly handle with the application Command execution the application should 
implement the following template handler routine:

  void ApplicationCommandHandler(uint8_t cmd, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3){

    switch(cmd){ // Gets the requested command code

      case MET_COMMAND_ABORT: // MANDATORY TO IMPLEMENT THE ABORT COMMAND HERE ....        
        .. do abort procedures ...
        MET_Can_Protocol_returnCommandAborted();        
        break;

      case APP_DELAYED_COMMAND_XX: // Tipical application command that takes longer time
        .. activate the command here ..
        MET_Can_Protocol_returnCommandExecuting(); // Returns the EXECUTING command code
        break;
      
      case APP_INSTANT_COMMAND_XX: // Tipical application command that is instantly executed
        .. executes and termines here ..
        .. the command produces two return bytes, b0 and b1
        MET_Can_Protocol_returnCommandExecuted(b0, b1); // Returns the EXECUTED command code
        break;
      
      default: // TYPICAL ERROR CODE DUE TO A UNIMPLEMENTED COMMAND CODE
        MET_Can_Protocol_returnCommandError(MET_CAN_COMMAND_NOT_AVAILABLE);    
      }
      
      
      return;
  }


In the template there are few important items:

+ The function parameters:
  + cmd: is the command code to be execute;
  + d0 to d3: they are optional data received with the command frame;
  
+ **MET_COMMAND_ABORT**: this is a special Command code, usually 0, that every Device application 
  shall handle. The Device shall activate the aborting procedures to stop the current execution 
  command. When the command is aborted (or the abort sequence is started) the Device application shall 
  call the MET_Can_Protocol_returnCommandAborted() function.

>NOTE: the Command Handler routine should never take more than 2ms!

### 5.3.1. Command return function and rule

When a Command has been processed into the Command Handler routine 
the application shall mandatory use one of the following function:

+ MET_Can_Protocol_returnCommandAborted(): this shall be called after processing the special Command ABORT;
+ MET_Can_Protocol_returnCommandExecuting(): this function shall be called when the command will take time to complete;
+ MET_Can_Protocol_returnCommandExecuted(uint8_t b0,uint8_t b1): this function shall be called in the case the command should be instantly executed. In this case optional returned b0 and b1 byte will be passed to the function;
+ MET_Can_Protocol_returnCommandError(uint8_t error): this function shall be called in case an error should prevent to execute the requested command;
  
There are a set of standard protocol error that should be preferred:

+ MET_CAN_COMMAND_NO_ERROR: No Error 
+ MET_CAN_COMMAND_BUSY:   A command is executing
+ MET_CAN_COMMAND_INVALID_DATA: The Command data are invalid
+ MET_CAN_COMMAND_NOT_ENABLED: The Command cannot be momentary executed 
+ MET_CAN_COMMAND_NOT_AVAILABLE: The Command cannot be executed in this revision      
+ MET_CAN_COMMAND_WRONG_RETURN_CODE: The Command returned a non valid status code
+ MET_CAN_COMMAND_ABORT_CODE: the code that the protocol automatically set whenever an abort request is processed;
+ MET_CAN_COMMAND_APPLICATION_ERRORS: Starting code for applicaiton specific errors     

If the appliction should add other error codes, the user code should be set as:
+ MET_CAN_COMMAND_APPLICATION_ERRORS + user code

### 5.3.2. Command delayed termination rules

In case a command execution should take long time to complete, 
then the device shall use the function MET_Can_Protocol_returnCommandExecuting() in the Command Handler.
This actually informs the MASTER that the command has been correctly initiated.

When the command termines:
+ If the command should successfully termines, the application shall finally call the function MET_Can_Protocol_returnCommandExecuted(uint8_t b0,uint8_t b1) that will properly set the content of the COMMAND register with the resulting data b0 and b1;
+ If the command should termine with error, the application shall finally call the function MET_Can_Protocol_returnCommandError(uint8_t error) that will properly set the content of the COMMAND register with the resulting error code;


## 5.4. Register data access

The Module automatically update the module Register  
in a transparent mode: the application shall never account with 
the CAN communication aspect regarding the Register data exchange with 
the MAIN-CPU. 

The Module provides special interface function to access 
in Read-only mode the DATA registers and PARAMETER registers and in Read/Write mode 
the STATUS register array.

### 5.4.1. STATUS registers access functions

In this section let assume the Application STATUS register array as: STATUS[index][byte_pos]
where:
+ index points to a given register;
+ byte_pos: 0 to 3, point to a byte in the four byte array of the register;


+ MET_Can_Protocol_SetStatusBit(uint8_t idx, uint8_t data_index, uint8_t mask, bool stat):
  + If stat == TRUE:
    + STATUS[idx][data_index] = (STATUS[idx][data_index] &= ~mask)  | mask;
  + If stat == FALSE:
    + STATUS[idx][data_index] = (STATUS[idx][data_index] &= ~mask);
  + The bits of the *mask* are set or clear into the *data_index* byte of the idx STATUS register according with the *stat* parameter;
  
+ MET_Can_Protocol_SetStatusReg(uint8_t idx, uint8_t data_index, uint8_t val):
  +  STATUS[idx][data_index] = val;
  +  The *data_index* byte of the idx STATUS register is set to *val*;
  
+ uint8_t MET_Can_Protocol_GetStatus(uint8_t idx, uint8_t data_index):
  + return STATUS[idx][data_index];

+ bool  MET_Can_Protocol_TestStatus(uint8_t idx, uint8_t data_index, uint8_t mask):
  + return STATUS[idx][data_index] & mask;

### 5.4.2. ERRORS register access functions

The ERROR register are accessed with the following read and write functions:
+ MET_Can_Protocol_SetErrors(uint8_t* mom0, uint8_t* mom1, uint8_t* pers0, uint8_t* pers1): sets the errors;
+ MET_Can_Protocol_GetErrors(uint8_t* mom0, uint8_t* mom1, uint8_t* pers0, uint8_t* pers1): gets the errors
        

### 5.4.3. DATA registers access functions

In this section let assume the Application DATA register array as: DATA[index][byte_pos]
where:
+ index points to a given register;
+ byte_pos: 0 to 3, point to a byte in the four byte array of the register;


+ uint8_t MET_Can_Protocol_GetData(uint8_t idx, uint8_t data_index):
  + return DATA[idx][data_index];

+ bool  MET_Can_Protocol_TestData(uint8_t idx, uint8_t data_index, uint8_t mask):
  + return DATA[idx][data_index] & mask;

### 5.4.4. PARAMETER registers access functions

In this section let assume the Application PARAMETER register array as: PARAMETER[index][byte_pos]
where:
+ index points to a given register;
+ byte_pos: 0 to 3, point to a byte in the four byte array of the register;


+ uint8_t MET_Can_Protocol_GetParameter(uint8_t idx, uint8_t data_index):
  + return PARAMETER[idx][data_index];

+ bool  MET_Can_Protocol_TestParameter(uint8_t idx, uint8_t data_index, uint8_t mask):
  + return PARAMETER[idx][data_index] & mask;


# Work in progress

+ The PARAMETER storage is not yet implemented;
+ The Loader section of the protocoll is not yet implemented;
        