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
 * of the Z190 Device Communication protocol specifications.
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
 * + The CAN0 Peripheral clock shall be configured with
 * a convenient Clock: 60MHz it is OK.
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
 *  @{
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
         * Register Data type defining the content of a register
         *  
         */
        typedef struct {
            uint8_t d[4]; //!< Register data content
        }MET_Register_t;
        
        
        /**
         * @brief Structure for module customization
         * 
         * This structure is filled with the initialization data 
         * passed by the Application Protocol Implementing Module.
         * 
         * The pStatusArray is a pointer to the Status Register Array:
         * + The Status Register Array is an array[N][4] where N is the number 
         * of implemented registers and 4 is the number of register bytes
         * 
         * The pDataArray is a pointer to the DATA Register Array:
         * + The DATA Register Array is an array[N][4] where N is the number 
         * of implemented registers and 4 is the number of register bytes
         */
        typedef struct {
            uint8_t deviceID; //!< This is the device ID from 1:255

            MET_Register_t*   pStatusArray; //!< This is the Status Register array pointer
            uint8_t     statusArrayLen; //!< This is the Status Register array lenght
            MET_Register_t*   pDataArray; //!< This is the DATA Register array pointer
            uint8_t     dataArrayLen; //!< This is the DATA Register array lenght

        } MET_Protocol_Data_t;
        ext_static MET_Protocol_Data_t MET_Protocol_Data_Struct; //!< This is the internal protocol data structure
        
        /** 
         * @brief Rx and Tx communication data
         * 
         * This structure is internally used to handle the communication 
         * data frames. The structure is local and is not shared with the 
         * application. The Received data are stroeed into the rx_message[]
         * array as soon as the data is received into the FIFO. 
         * The tx_message[] array is passed to the Can Sender function 
         * when a frame shall be sent.
         * 
         */  
        typedef struct {

            uint32_t rx_messageID; //!< Received ID frame (11bit)
            uint8_t rx_message[8]; //!< Received data byte
            uint8_t rx_messageLength;//!< Received data lenght
            uint16_t rx_timestamp; //!< Received Time stamp

            uint32_t tx_messageID; //!< Transmitting ID (11 bit)
            uint8_t tx_message[8]; //!< Transmitting data byte
            uint8_t tx_messageLength;//!< transmitting data lenght

        } MET_Can_Protocol_RxTx_t;        
        ext_static MET_Can_Protocol_RxTx_t MET_Can_Protocol_RxTx_Struct; //!< This is the structure handling the data transmitted and received

        /**
         * @brief This is the enumeration class for the Frame Command codes
         */
        typedef enum{
            MET_CAN_PROTOCOL_READ_STATUS = 1, //!< Read Status registers frame type
            MET_CAN_PROTOCOL_WRITE_PARAM, //!<  Write Parameter Registers frame type
            MET_CAN_PROTOCOL_WRITE_DATA, //!< Write Data Registers frame type
            MET_CAN_PROTOCOL_STORE_PARAMS, //!< Store Parameters command frame 
            MET_CAN_PROTOCOL_COMMAND_EXEC //!< Command Execution frame
        }MET_FRAME_CODES;
        
        
        
        
        /**
         * @brief Helper class user to cast the received can frame
         * 
         * The Can data frame is coded with the following data field:
         * 
         * |Byte|Name|Description|
         * |:---:|:---:|:---|
         * |0|SEQ|Frame sequence number set by the sender|
         * |1|Frame Command Code|defines the frame type|
         * |2:6|D[0:4]|Frame data content|
         * |7|CRC|Crc frame code|
         * ||||
         * 
         */
        typedef struct {
            uint8_t seq; //!< Frame sequence number
            MET_FRAME_CODES frame_cmd;//!< Frame command code
            uint8_t idx; //!< Register index
            MET_Register_t data; //!< Register data content
            uint8_t crc;  //! CRC filed
        } MET_Can_Protocol_Command_t;

        
        
    /** @}*/  // metCanData

/** @}*/  // metProtocolModule
        


/******************************************************************************/
/*                      API DI ISTALLAZIONE PROTOCOLLO                        */
/******************************************************************************/
        

        
     /** 
     * \defgroup metCanApi  Module API
     * 
     * \ingroup metProtocolModule
     * This section providides the API for the CAN protocol integration.
     * 
     *  @{
     */
        /**
         * @brief Can protocol initialization
         * 
         * This function shall be called by the Application Implementing Protocol
         * at the beginning of the program, in order to set the registers structure
         * and to start the reception.
         * 
         *  
         * @param deviceID this is the assigned deviceID (1:255)
         * @param pStatusArray pointer to the Status Register's Array
         *  The structure is array[N][4]: 
         *  + N is the number of the Status registers;
         *  + [4] is the register data content;
         * 
         * @param StatusLen number of Status Registers pointed
         * NOTE: the array shall be instantiated into the implementing class!
         * 
         * @param pDataArray pointer to the DATA Register's Array
         *  The structure is array[N][4]: 
         *  + N is the number of the DATA registers;
         *  + [4] is the register data content;
         * 
         * @param DataLen number of DATA Registers pointed
         * NOTE: the array shall be instantiated into the implementing class!
         * 
         */
        void MET_Can_Protocol_Init(uint8_t deviceID, MET_Register_t* pStatusArray, uint8_t StatusLen, MET_Register_t* pDataArray, uint8_t DataLen);
        
        /**
         * @brief Application Main Loop function handler
         * 
         * This function shall be called into the Application Main Loop.
         * 
         * NOTE: when a can frame is received, an interrupt is generated
         * and handled into the module. The interrupt will set an internal 
         * flag that will be processed by this routine in the Main Loop. 
         * This approach prevents to lock the interrupt routine for long time and
         * allows the processes sharing the access with the data registers to
         * don't face synchronization issues as long as those processes will 
         * access data into the same Main Loop.
         * 
         */
        void MET_Can_Protocol_Loop(void);  
        
    /** @}*/  // metCanApi



#endif /* _MET_CAN_PROTOCOL_H */

