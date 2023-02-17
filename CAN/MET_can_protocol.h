#ifndef _MET_CAN_PROTOCOL_H
    #define _MET_CAN_PROTOCOL_H

#include "definitions.h"                // SYS function prototypes



#undef ext
#undef ext_static

#ifdef _MET_CAN_PROTOCOL_C
    #define ext
    #define ext_static static 
#else
    #define ext extern
    #define ext_static extern
#endif

/*!
 * \defgroup metProtocolModule  CAN Communication Engine Module
 *
 * \ingroup libraryModules
 * 
 * # Overview
 *
 * This module implements the engine communication functionalities 
 * of the Z190 Device Communication protocol specifications in the 
 * CAN CHannel 0.
 * 
 * 
 * 
 * # Communication settings
 * 
 * The CAN communication is based on the following 
 * characteristics:
 * 
 * - Standard (not Flexible) communication;
 * - Standard frame (11 bit ID);
 * - Baude Rate: 1Mb/s;
 * - Can ID reception address: 0x200 + deviceID;
 * 
 * The deviceID is a decimal value from 1 to 255.
 * 
 * # Harmony 3 Configurator Settings
 * 
 * This Module makes use of the following Processor modules:
 * - CAN0 peripheral module;
 * - Peripheral Can0 clock
 * - PIN Assignement
 * 
 * ## Pin Assignement
 * 
 * + The CAN TX Input shall be configured with CAN0-TX 
 * + The CAN RX Input shall be configured with CAN0-TX 
 * 
 * ## Peripheral Clock Settings
 * 
 *+ Activate the GLK4 Clock:
 *   + Set the DFLL (48MHz) as the Input clock;
 *   + Set the divisor to 2 to obtain a final 24MHz of GLK4 Clock;
 *   + The CAN0 Peripheral clock shall be configured with the GLK4 clock at 24 MHz.
 * 
 * ## CAN0 peripheral module
 * 
 * Follows only those settings that shall be activated.
 * What is not reported is disabled or not modified.
 * 
 * + CAN Operational Mode = NORMAL;
 * + Interrupt Mode: Yes;
 * + Bit Timing Calculation
 *  + Nominal Bit Timing
 *      + Automatic Nominal Bit Timing: Yes;
 *      + BIt Rate: 1000
 * 
 * + Use RX FIFO 0: Yes
 *   + RX FIFO 0 Setting
 *      + Number of element: 1
 * 
 * + Use TX FIFO: Yes
 *   + TX FIFO Setting
 *      + Number of element: 1
 *
 * + EnableTX Pause: YES
 * + Standard Filters 
 *  + Number Of STandard Filters: 1
 *  + Standard Filter 1
 *      + Type: Range;
 *      + ID1: 0x200 + Device ID
 *      + ID12: 0x200 + Device ID
 *      + Element Configuration: Store in RX FIFO 0
 *  + Standard message No-Match disposion: Reject
 *  + Reject Standard Remote Frames: YES
 *  + Timestamp Enable: YES 
 * 
 *  # Application Usage
 * 
 *  The Application shall mandatory define a header file application.h
 *  that shall be stored into the same directory of the main.c file.
 *  
 *  The application.h shall contain the following defines:
 * 
 *  + #define MET_CAN_APP_MAJ_REV : Revision Major Number
 *  + #define MET_CAN_APP_MIN_REV : Revision Minor Number
 *  + #define MET_CAN_APP_SUB_REV : Revision build Number
 *  + #define MET_CAN_APP_DEVICE_ID : Application DEVICE CAN Id address
 *  + #define MET_CAN_STATUS_REGISTERS : Defines the total number of implemented STATUS registers 
 *  + #define MET_CAN_DATA_REGISTERS : Defines the total number of implemented Application DATA registers 
 *  + #define MET_CAN_PARAM_REGISTERS :  Defines the total number of implemented PARAMETER registers 
 *
 *  The Application shall define a Callback routine to handle the Protocol Commands:
 *  + void ApplicationHandler(void): this functon shall be passed as parameter to the MET_Can_Protocol_Init();
 * 
 *  The Applicaiton shall call the following routines to activate the communication:
 *  + MET_Can_Protocol_Init() function in the Setup section of the main.c file;
 *  + MET_Can_Protocol_Loop() function in the main.c Loop;
 *   
 *  The Application shall use the following routines to handle with the Registers:
 * 
 *  + Functions to Get/Set the Application STATUS registers:
 *      + MET_Can_Protocol_SetStatusBit(): sets ON/OFF all the bits of a mask;
 *      + MET_Can_Protocol_SetStatusReg(): sets a byte of a STATUS register;
 *      + MET_Can_Protocol_GetStatus(): returns a byte value of a STATUS register;
 *      + MET_Can_Protocol_TestStatus(): test a condition on a STATUS register mask;
 * 
 *  + Functions to Get/Set the Application ERRORS register:
 *      + MET_Can_Protocol_SetErrorsBit(): sets ON/OFF all the bits of a mask into the ERRORS register;
 *      + MET_Can_Protocol_SetErrorsReg(): sets a byte of a ERRORS register;
 *      + MET_Can_Protocol_GetErrors(): returns a byte value of a ERRORS register;
 *      + MET_Can_Protocol_TestErrors(): test a condition on a ERRORS register mask;
 *
 *  
 *  + Functions to Test the Application DATA registers:
 *      + MET_Can_Protocol_GetData(): returns a byte value of a DATA register;
 *      + MET_Can_Protocol_TestData(): test a condition on a DATA register mask;
 *
 *  + Functions to Test the Application PARAMETER registers:
 *      + MET_Can_Protocol_GetParameter(): returns a byte value of a PARAMETER register;
 *      + MET_Can_Protocol_TestParameter(): test a condition on a PARAMETER register mask;
 * 
 * 
 *  + Functions to handle the Command Execution:
 *      + MET_Can_Protocol_getCommandCode(): return the requested command code;
 *      + MET_Can_Protocol_getCommandParam0(): requests for the command parameter 0;
 *      + MET_Can_Protocol_getCommandParam1(): requests for the command parameter 1;
 *      + MET_Can_Protocol_getCommandParam2(): requests for the command parameter 2;
 *      + MET_Can_Protocol_getCommandParam3(): requests for the command parameter 3;
 *      + MET_Can_Protocol_setReturnCommand(): set the return code should be returned after the command handling.
 * 
 *
 *    @{
 * 
 */

    /** 
     * \defgroup metCanData  Module data structures
     * 
     * This section implements the data structures of the module
     *  @{
     */
        #define _CAN_ID_BASE_ADDRESS 0x200 //!< This is the base address for the communication point to point
        #define _CAN_ID_LOADER_ADDRESS 0x100 //!< This is the base address for the Loader frames


        /**
         * @brief This is the enumeration class for the Frame Command codes
         */
        typedef enum{
            MET_CAN_PROTOCOL_READ_REVISION = 1, //!< Read Application Special Revision registers frame type
            MET_CAN_PROTOCOL_READ_ERRORS,       //!< Read Application Special Errors registers frame type
            MET_CAN_PROTOCOL_READ_COMMAND,      //!< Read Application Special Command registers frame type
            MET_CAN_PROTOCOL_READ_STATUS,       //!< Read Application Status registers frame type
            MET_CAN_PROTOCOL_READ_DATA,         //!< Read DATA registers frame type
            MET_CAN_PROTOCOL_READ_PARAM,        //!< Read PARAMETER registers frame type
            MET_CAN_PROTOCOL_WRITE_DATA,        //!< Write Data Registers frame type
            MET_CAN_PROTOCOL_WRITE_PARAM,       //!< Write Parameter Registers frame type
            MET_CAN_PROTOCOL_STORE_PARAMS,      //!< Store Parameters command frame 
            MET_CAN_PROTOCOL_COMMAND_EXEC       //!< Command Execution frame
        }MET_FRAME_CODES;
        
        /** 
         * @brief Can Protocol Error codes
         * 
         * Those errors are generated into the module in case
         * of some function should fail. When a error is detected, 
         * the error is provided to an internal error handler: MET_CanOpen_Error_Callback()
         * 
         */ 
        typedef enum
        {    
            MET_CAN_PROTOCOL_ERROR_OK=0, //!< No error
            MET_CAN_PROTOCOL_ERROR_RECEPTION_ACTIVATION, //!< Error in activating the reception
            MET_CAN_PROTOCOL_ERROR_TRANSMISSION, //!> Error during data transmission       
            MET_CAN_PROTOCOL_ERROR_INVALID_CRC, //!> Frame with invalid CRC received        
            MET_CAN_PROTOCOL_ERROR_INVALID_LENGHT, //!> Frame with invalid Lenght

        } MET_CAN_PROTOCOL_ERROR_DEFS;

        /**
         * This is the Protocol frame data content
         */
        typedef struct {
            uint8_t seq;        //!< Frame sequence number
            uint8_t frame_cmd;  //!< Frame command code  
            uint8_t idx;        //!< This is the IDX field
            uint8_t d[4];       //!< Frame data content
            uint8_t crc;        //!< Frame CRC
        }MET_Can_Frame_t;
        
        /**
         * Register Data type defining the content of a register
         *  
         */
        typedef struct {
            uint8_t d[4]; //!< Register data content
        }MET_Register_t;
        
        
        /**
         * @brief This is the Command Error eenumeration
         */
        typedef enum{
            MET_CAN_COMMAND_NO_ERROR = 0,       //!< No Error 
            MET_CAN_COMMAND_BUSY = 1,           //!< A command is executing
            MET_CAN_COMMAND_INVALID_DATA = 2,   //!< The Command data are invalid
            MET_CAN_COMMAND_NOT_ENABLED = 3,    //!< The Command cannot be momentary executed 
            MET_CAN_COMMAND_NOT_AVAILABLE = 4,  //!< The Command cannot be executed in this revision      
            MET_CAN_COMMAND_WRONG_RETURN_CODE = 5,  //!< The Command returned a non valid status code
            MET_CAN_COMMAND_APPLICATION_ERRORS  //!< Starting code for applicaiton specific errors                    
        }MET_CommandErrors_t;
        
        /**
         * @brief This type defines the possible Command Execution status
         * 
         */
        typedef enum{
            MET_CAN_COMMAND_COMPLETED = 0,  //!< The Command has been processed successfully
            MET_CAN_COMMAND_EXECUTING = 1,  //!< The Command is processing
            MET_CAN_COMMAND_ERROR = 2,      //!< The Command has been terminated with error condition
        }MET_CommandExecStatus_t;
        
        /**
         * @brief This is the data structure the application shall return from the 
         * command Handler routine
         */
        typedef struct{
            MET_CommandExecStatus_t status; //!< Returning execution status code
            uint8_t result[2];              //!< result command data
            uint8_t error;                  //!< Error code in case of unsuccess;        
        }MET_CommandExecReturn_t; 
        
        typedef struct {
         uint8_t command;   //!< D0 - This is the command code
         uint8_t param[4];  //!< This is the command parameter array
       }MET_Command_Data_t;

       
        /**
         * @brief This is the type definition for the Command callback function
         */
        typedef void (*MET_commandHandler_t)(void);
        
        
        /** 
        * ***REVISION STATUS REGISTER***
        * 
        * This is the structure of the Revision register content
        * 
        * The register data content is:
        * 
        * |BYTE|DESCRIPTION|
        * |:---:|:---|
        * |D0|Revision Major number|
        * |D1|Revision Minor number|
        * |D2|Revision Sub number|
        * |D3|-|
        * 
        * 
        */  
       typedef struct {
         uint8_t maj;  //!< D0 - This is the Major revision number
         uint8_t min;  //!< D1 - This is the minor revision number
         uint8_t sub;  //!< D2 - This is the Sub-minor revision number
         uint8_t d3;   //!< D3 - NA
       }MET_Revision_Status_t;
       
       /** 
        * ***ERRORS STATUS REGISTER***
        * 
        * This is the structure of the Errors register content
        * 
        * The register data content is:
        * 
        * |BYTE|DESCRIPTION|
        * |:---:|:---|
        * |D0|momentary-0|
        * |D1|momentary-1|
        * |D2|Persistent-0|
        * |D3|Persistent-1|
        * 
        * 
        */  
       typedef struct {
         uint8_t mom0;  //!< D0 - This is the Momentary error byte 0
         uint8_t mom1;  //!< D1 - This is the Momentary error byte 1
         uint8_t pers0;  //!< D2 - This is the Persisten error byte 0
         uint8_t pers1;   //!< D3 - This is the Persisten error byte 1
       }MET_Errors_Status_t;

        
        /** 
        * ***COMMAND STATUS REGISTER***
        * 
        * This is the structure of the Command register content.
        * 
        * The register data content is:
        * 
        * |BYTE|DESCRIPTION|
        * |:---:|:---|
        * |D0|Command code of the last command requested|
        * |D1|Sequence number of the command request frame|
        * |D2|Current execution status|
        * |D3|Error condition|
        * 
        * 
        */  
       typedef struct {
         uint8_t command;  //!< D0 - This is the last execution command
         uint8_t seq;      //!< D1 - This is the sequence number
         uint8_t stat;     //!< D2 - This is the execution status (see _CommandExecStatus_e )
         uint8_t error;    //!< D3 - This is a optiona error condition
       }_Command_Status_t;
           
       
       /**
        * This is the enumeration to access the content of the ERRORS register
        */
       typedef enum{
            MET_CAN_ERROR_MOM0 = 0, //!< byte index of the Momentary error 0
            MET_CAN_ERROR_MOM1,     //!< byte index of the Momentary error 1
            MET_CAN_ERROR_PERSISTENT0, //!< byte index of the Persisten error 0
            MET_CAN_ERROR_PERSISTENT1, //!< byte index of the Persisten error 1
        }MET_CAN_ERROR_BYTE_t;
            
    /** @}*/  // metCanData

/** @}*/  // metProtocolModule
        


/******************************************************************************/
/*                      API DI ISTALLAZIONE PROTOCOLLO                        */
/******************************************************************************/
        

        
     /** 
     * \defgroup metCanApi  Module Installation API
     * 
     * \ingroup metProtocolModule
     * This section providides the API for the CAN protocol integration.
     * 
     *  @{
     */
        
        /// Can protocol initialization
        ext void MET_Can_Protocol_Init(MET_commandHandler_t pCommandHandler);
        
        /// Application Main Loop function handler
        void MET_Can_Protocol_Loop(void);  
     /** @}*/  // metCanApi
        
    /** 
     * \defgroup metCanRegApi  Module Register Access API
     * 
     * \ingroup metProtocolModule
     * This section providides the API for the CAN protocol integration.
     * 
     *  @{
     */
      
        /// Sets the bit-wise content of a STATUS register
        ext void  MET_Can_Protocol_SetStatusBit(uint8_t idx, uint8_t data_index, uint8_t mask, bool stat);
    
        /// Sets the whole content of a STATUS register
        ext void  MET_Can_Protocol_SetStatusReg(uint8_t idx, uint8_t data_index, uint8_t val );
        
        /// Returns the content of the STATUS register array
        uint8_t  MET_Can_Protocol_GetStatus(uint8_t idx, uint8_t data_index);
    
        // The function tests the content of a STATUS register with a mask byte
        bool  MET_Can_Protocol_TestStatus(uint8_t idx, uint8_t data_index, uint8_t mask);
        
        
        
        /// Sets the bit-wise content of the ERRORS register        
        ext void  MET_Can_Protocol_SetErrorsBit(MET_CAN_ERROR_BYTE_t data_index, uint8_t mask, bool stat);
        
        /// Sets the whole content of the ERRORS register        
        ext void  MET_Can_Protocol_SetErrorsReg(MET_CAN_ERROR_BYTE_t data_index, uint8_t val );
        
        /// Returns the content of the ERRORS register array
        ext uint8_t  MET_Can_Protocol_GetErrors(MET_CAN_ERROR_BYTE_t data_index);
        
        // The function tests the content of the ERRORS register with a mask byte
        ext bool  MET_Can_Protocol_TestErrors(MET_CAN_ERROR_BYTE_t data_index, uint8_t mask);
        
        
        /// Returns the pointer to the Application Data register array
        uint8_t  MET_Can_Protocol_GetData(uint8_t idx, uint8_t data_index);
    
        // The function tests the content of a Data register with a mask byte
        bool  MET_Can_Protocol_TestData(uint8_t idx, uint8_t data_index, uint8_t mask);
        
        /// Returns the pointer to the Application PARAMETER register array
        uint8_t  MET_Can_Protocol_GetParameter(uint8_t idx, uint8_t data_index);
    
        // The function tests the content of a PARAMETER register with a mask byte
        bool  MET_Can_Protocol_TestParameter(uint8_t idx, uint8_t data_index, uint8_t mask);
        
        
        /// Returns the current command execution code
        uint8_t MET_Can_Protocol_getCommandCode(void);
        
        /// Returns the current command parameter 0
        uint8_t MET_Can_Protocol_getCommandParam0(void);
        
        /// Returns the current command parameter 1
        uint8_t MET_Can_Protocol_getCommandParam1(void);
        
        /// Returns the current command parameter 2
        uint8_t MET_Can_Protocol_getCommandParam2(void);
        
        /// Returns the current command parameter 3
        uint8_t MET_Can_Protocol_getCommandParam3(void);

        /// Set the execution returning code
        void MET_Can_Protocol_setReturnCommand(MET_CommandExecStatus_t retstat, uint8_t ris0, uint8_t ris1, uint8_t error);
        
        /** @}*/  // metCanRegApi



#endif /* _MET_CAN_PROTOCOL_H */

