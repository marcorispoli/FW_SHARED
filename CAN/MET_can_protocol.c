#define _MET_CAN_PROTOCOL_C

#include "application.h"                // SYS function prototypes
#include "MET_can_protocol.h" 

#ifndef MET_CAN_APP_DEVICE_ID
    #warning "The application shall define the MET_CAN_APP_DEVICE_ID in the application.h header file" 
    #define MET_CAN_APP_DEVICE_ID 0
#endif
#ifndef MET_CAN_APP_MAJ_REV
    #warning "The application shall define the MET_CAN_APP_MAJ_REV in the application.h header file" 
    #define MET_CAN_APP_MAJ_REV 0
#endif
#ifndef MET_CAN_APP_MIN_REV
    #warning "The application shall define the MET_CAN_APP_MIN_REV in the application.h header file" 
    #define MET_CAN_APP_MIN_REV 0
#endif
#ifndef MET_CAN_APP_SUB_REV
    #warning "The application shall define the MET_CAN_APP_SUB_REV in the application.h header file" 
    #define MET_CAN_APP_SUB_REV 0
#endif
#ifndef MET_CAN_STATUS_REGISTERS
    #warning "The application shall define the MET_CAN_STATUS_REGISTERS in the application.h header file"
    #define MET_CAN_STATUS_REGISTERS 0
#endif
#ifndef MET_CAN_DATA_REGISTERS
    #warning "The application shall define the MET_CAN_DATA_REGISTERS in the application.h header file" 
    #define MET_CAN_DATA_REGISTERS 0
#endif
#ifndef MET_CAN_PARAM_REGISTERS
    #warning "The application shall define the MET_CAN_PARAM_REGISTERS in the application.h header file" 
    #define MET_CAN_PARAM_REGISTERS 0
#endif


/**
 * \defgroup metCanImplementation Implementation module
 * 
 * \ingroup metProtocolModule
 * 
 * This Module provides the implementation details
 * 
 *  @{ 
 */
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

            MET_Revision_Register_t     revisionRegister;        //!< Revision Register
            MET_Errors_Register_t       errorsRegister;          //!< Errors register
            MET_Command_Register_t      commandRegister;         //!< Command Execution  register
                        
            MET_Register_t  pApplicationStatusArray[MET_CAN_STATUS_REGISTERS]; //!< This is the Application Status Register array pointer
            uint8_t     applicationStatusArrayLen; //!< This is the Application Status Register array lenght

            MET_Register_t   pApplicationDataArray[MET_CAN_DATA_REGISTERS]; //!< This is the Application DATA Register array pointer
            uint8_t     applicationDataArrayLen; //!< This is the Application DATA Register array lenght

            MET_Register_t   pApplicationParameterArray[MET_CAN_PARAM_REGISTERS]; //!< This is the Application PARAMETER Register array pointer
            uint8_t     applicationParameterArrayLen; //!< This is the Application PARAMETER Register array lenght
                        
            MET_commandHandler_t applicationCommandHandler; //!< This is the application command handler
            
        } MET_Protocol_Data_t;
        
        static MET_Protocol_Data_t MET_Protocol_Data_Struct; //!< This is the internal protocol data structure
        
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
        static MET_Can_Protocol_RxTx_t MET_Can_Protocol_RxTx_Struct; //!< This is the structure handling the data transmitted and received
        
        
    /**
     * \defgroup metCanHarmony Harmony 3 necessary declarations
     * 
     * The declaration of this section are necessary for the usage of the CAN channel
     * based on the Harmony 3 configuration library.
     * 
     *  @{
     */ 
        /// Harmony 3 data declaration
        uint8_t Can0MessageRAM[CAN0_MESSAGE_RAM_CONFIG_SIZE] __attribute__((aligned (32))); 
        
        /// Attribute used into the CAN Rx/Tx functions
        CAN_MSG_RX_FRAME_ATTRIBUTE msgFrameAttr0 = CAN_MSG_RX_DATA_FRAME; 
        
    /** @}*/  // metCanHarmony
    
     /**
     * \defgroup metCanLocal Locale Module function declaration
     * 
     * 
     *  @{
     */        
    
        /// Error Callback routine
        static void MET_DefaultError_Callback(uint8_t event); 

        /// Interrupt routine
        static void MET_Can_Protocol_Reception_Callback(uintptr_t context); 

        /// Reception activation routine
        static void MET_Can_Protocol_Reception_Trigger(void);     

        static bool rxReceptionTrigger = false; //!< RX received frame flag
        static bool rxErrorTrigger = false;//!< TX received frame flag

    /** @}*/  // metCanLocal

        
/**
 * @brief This function triggers the reception of a further CAN frame.
 * 
 * The Function firstly rearm the interrupt handler to be launched.
 * After the interrupt is armed, the function assignes the data pointer to the
 * receving buffer.
 * 
 * In case of an error in activating the Reception, the error  MET_CAN_PROTOCOL_ERROR_RECEPTION_ACTIVATION
 * is signaled to the Error Handler routine.
 *  
 */
void MET_Can_Protocol_Reception_Trigger(void){

    // Reception Event callback registered on the FIFO0
    CAN0_RxCallbackRegister( MET_Can_Protocol_Reception_Callback, 0 , CAN_MSG_ATTR_RX_FIFO0 );
    
    // Activate the reception buffer on the FIFO-0
    if (CAN0_MessageReceive(&MET_Can_Protocol_RxTx_Struct.rx_messageID,
            &MET_Can_Protocol_RxTx_Struct.rx_messageLength,
            MET_Can_Protocol_RxTx_Struct.rx_message,
            &MET_Can_Protocol_RxTx_Struct.rx_timestamp,
            CAN_MSG_ATTR_RX_FIFO0, &msgFrameAttr0) == false)  MET_DefaultError_Callback(MET_CAN_PROTOCOL_ERROR_RECEPTION_ACTIVATION);
    
    return;
}

/**
* 
* This function shall be called by the Application Implementing Protocol
* at the beginning of the program, in order to set the registers structure
* and to start the reception.
* 
*  
* @param deviceID this is the assigned deviceID (1:255)
* 
* @param pCommandHandler application command handler
* 
* 
*/
void MET_Can_Protocol_Init(MET_commandHandler_t pCommandHandler){
    
    // Assignes the current device ID
    MET_Protocol_Data_Struct.deviceID = MET_CAN_APP_DEVICE_ID;
    
    // Harmony 3 library call: Init memory of the CAN Bus module
    CAN0_MessageRAMConfigSet(Can0MessageRAM);
    
    // Assignes the Application Revision code to the revision register
    MET_Protocol_Data_Struct.revisionRegister.maj = MET_CAN_APP_MAJ_REV;
    MET_Protocol_Data_Struct.revisionRegister.min = MET_CAN_APP_MIN_REV;
    MET_Protocol_Data_Struct.revisionRegister.sub = MET_CAN_APP_SUB_REV;
    
    // Clears the Command register 
    MET_Protocol_Data_Struct.commandRegister.status = MET_CAN_COMMAND_EXECUTED;
    MET_Protocol_Data_Struct.commandRegister.result[0] = 0;
    MET_Protocol_Data_Struct.commandRegister.result[1] = 0;
    MET_Protocol_Data_Struct.commandRegister.error = MET_CAN_COMMAND_NO_ERROR;
    
    // Clears the Errors register
    MET_Protocol_Data_Struct.errorsRegister.mom0=0;
    MET_Protocol_Data_Struct.errorsRegister.mom1=0;
    MET_Protocol_Data_Struct.errorsRegister.pers0=0;
    MET_Protocol_Data_Struct.errorsRegister.pers1=0;
    
    // Add the external STATUS register array 
    MET_Protocol_Data_Struct.applicationStatusArrayLen = MET_CAN_STATUS_REGISTERS;
    
    // Add the external DATA register array
    MET_Protocol_Data_Struct.applicationDataArrayLen = MET_CAN_DATA_REGISTERS;
   
    // Add the external PARAMETER register array
    MET_Protocol_Data_Struct.applicationParameterArrayLen = MET_CAN_PARAM_REGISTERS;
    
    // Add the application command handler 
    MET_Protocol_Data_Struct.applicationCommandHandler = pCommandHandler;
    
    // Schedules the next reception interrupt
    MET_Can_Protocol_Reception_Trigger();      
    
    return ;
    
 }


/**
 * This function set the whole content of a STATUS register
 * 
 * 
 * @param idx index of the register
 * @param data_index index of the register data [0:3]
 * @param val value to be assigned
 * 
 */
void  MET_Can_Protocol_SetStatusReg(uint8_t idx, uint8_t data_index, uint8_t val ){

    if((idx < MET_Protocol_Data_Struct.applicationStatusArrayLen) && (data_index < 4)) {
        MET_Protocol_Data_Struct.pApplicationStatusArray[idx].d[data_index] = val;
    }
    return;
}

/**
 * This function set a bit-set of a sub register data section
 * 
 * The function perform the following setting:
 * REGISTER[IDX][DATA_INDEX] = (REGISTER[IDX][DATA_INDEX] &~ mask) | (val & mask);
 * 
 * @param idx index of the Status register array
 * @param data_index index of the register data [0:3]
 * @param mask data mask
 * @param val value to be set 
 */
void  MET_Can_Protocol_SetStatusBit(uint8_t idx, uint8_t data_index, uint8_t mask, bool stat){
    
    if((idx < MET_Protocol_Data_Struct.applicationStatusArrayLen) && (data_index < 4)) {
        uint8_t data = MET_Protocol_Data_Struct.pApplicationStatusArray[idx].d[data_index];
        data &= (~mask);
        if(stat) data |= mask;        
        MET_Protocol_Data_Struct.pApplicationStatusArray[idx].d[data_index] = data;  
    }
    
    return ;
}

/**
 * This function return the STATUS register content
 * 
 * @param idx index of the STATUS register
 * @param data_index index of the register data content [0:3]
 * @return the register content or 0 in case of out of index range
 * 
 */
uint8_t  MET_Can_Protocol_GetStatus(uint8_t idx, uint8_t data_index){
    if((idx < MET_Protocol_Data_Struct.applicationStatusArrayLen) && (data_index < 4)) {
        return MET_Protocol_Data_Struct.pApplicationStatusArray[idx].d[data_index];
    }    
    
    return 0;
}

 /**
 * This function tests a bit field condition on a STATUS register.
 * 
 * @param idx index of the STATUS register
 * @param data_index index of the register data content [0:3]
 * @param mask mask value to be tested
 * @return true if the mask select a true condition
 * 
 */
bool  MET_Can_Protocol_TestStatus(uint8_t idx, uint8_t data_index, uint8_t mask){
    
    if((idx < MET_Protocol_Data_Struct.applicationStatusArrayLen) && (data_index < 4)) {
        uint8_t data = MET_Protocol_Data_Struct.pApplicationStatusArray[idx].d[data_index];
        data &= (~mask);
        data |= (mask);
        if(data) return true;
    }
      
    return false;

}



/**
 * This function returns the current active errors
 * 
 * @param mom0: momentary mom0 pointer
 * @param mom1: momentary mom1 pointer
 * @param pers0: persisten pers0 pointer
 * @param pers1: persisten pers1 pointer
 */
void  MET_Can_Protocol_GetErrors(uint8_t* mom0, uint8_t* mom1, uint8_t* pers0, uint8_t* pers1){
    if(mom0) *mom0 = MET_Protocol_Data_Struct.errorsRegister.mom0;
    if(mom1) *mom1 = MET_Protocol_Data_Struct.errorsRegister.mom1;
    if(pers0) *pers0 = MET_Protocol_Data_Struct.errorsRegister.pers0;
    if(pers1) *pers1 = MET_Protocol_Data_Struct.errorsRegister.pers1;
    return;
 }

/**
 * This function sets a given error
 *  
 * Only the not null error field pointers will be updated. 
 * 
 * @param mom0: momentary mom0 pointer
 * @param mom1: momentary mom1 pointer
 * @param pers0: persisten pers0 pointer
 * @param pers1: persisten pers1 pointer
 */
void  MET_Can_Protocol_SetErrors(uint8_t* mom0, uint8_t* mom1, uint8_t* pers0, uint8_t* pers1){
    if(mom0) MET_Protocol_Data_Struct.errorsRegister.mom0 = *mom0;
    if(mom1) MET_Protocol_Data_Struct.errorsRegister.mom1 = *mom1;
    if(pers0) MET_Protocol_Data_Struct.errorsRegister.pers0 = *pers0;
    if(pers1) MET_Protocol_Data_Struct.errorsRegister.pers1 = *pers1;
    
    return;
}
    

/**
 * This function return the Data register content
 * 
 * @param idx index of the Data register
 * @param data_index index of the register data content [0:3]
 * @return the register content or 0 in case of out of index range
 * 
 */
uint8_t  MET_Can_Protocol_GetData(uint8_t idx, uint8_t data_index){
    if((idx < MET_Protocol_Data_Struct.applicationDataArrayLen) && (data_index < 4)) {
        return MET_Protocol_Data_Struct.pApplicationDataArray[idx].d[data_index];
    }    
    
    return 0;
}

/**
 * This function tests a bit field condition on a Data register.
 * 
 * @param idx index of the Data register
 * @param data_index index of the register data content [0:3]
 * @param mask mask value to be tested
 * @return true if the mask select a true condition
 * 
 */
bool  MET_Can_Protocol_TestData(uint8_t idx, uint8_t data_index, uint8_t mask){
    
    if((idx < MET_Protocol_Data_Struct.applicationDataArrayLen) && (data_index < 4)) {
        uint8_t data = MET_Protocol_Data_Struct.pApplicationDataArray[idx].d[data_index];
        data &= (~mask);
        data |= (mask);
        if(data) return true;
    }
      
    return false;

}

/**
 * This function return the PARAMETER register content
 * 
 * @param idx index of the PARAMETER register
 * @param data_index index of the register data content [0:3]
 * @return the register content or 0 in case of out of index range
 * 
 */
uint8_t  MET_Can_Protocol_GetParameter(uint8_t idx, uint8_t data_index){
    if((idx < MET_Protocol_Data_Struct.applicationParameterArrayLen) && (data_index < 4)) {
        return MET_Protocol_Data_Struct.pApplicationParameterArray[idx].d[data_index];
    }    
    
    return 0;
}

/**
 * This function tests a bit field condition on a PARAMETER register.
 * 
 * @param idx index of the PARAMETER register
 * @param data_index index of the register data content [0:3]
 * @param mask mask value to be tested
 * @return true if the mask select a true condition
 * 
 */
bool  MET_Can_Protocol_TestParameter(uint8_t idx, uint8_t data_index, uint8_t mask){
    
    if((idx < MET_Protocol_Data_Struct.applicationParameterArrayLen) && (data_index < 4)) {
        uint8_t data = MET_Protocol_Data_Struct.pApplicationParameterArray[idx].d[data_index];
        data &= (~mask);
        data |= (mask);
        if(data) return true;
    }
      
    return false;

}


void MET_Can_Protocol_returnCommandExecuting(void){
    MET_Protocol_Data_Struct.commandRegister.status = MET_CAN_COMMAND_EXECUTING;
    MET_Protocol_Data_Struct.commandRegister.result[0] = 0;
    MET_Protocol_Data_Struct.commandRegister.result[1] = 0;
    MET_Protocol_Data_Struct.commandRegister.error = 0;
    return;
}

void MET_Can_Protocol_returnCommandExecuted(uint8_t ris0, uint8_t ris1){
    MET_Protocol_Data_Struct.commandRegister.status = MET_CAN_COMMAND_EXECUTED;
    MET_Protocol_Data_Struct.commandRegister.result[0] = ris0;
    MET_Protocol_Data_Struct.commandRegister.result[1] = ris1;
    MET_Protocol_Data_Struct.commandRegister.error = 0;
    return;
}

void MET_Can_Protocol_returnCommandError(uint8_t err){
    MET_Protocol_Data_Struct.commandRegister.status = MET_CAN_COMMAND_ERROR;
    MET_Protocol_Data_Struct.commandRegister.error = err;
    return;
}
        
void MET_Can_Protocol_returnCommandAborted(void){
    MET_Protocol_Data_Struct.commandRegister.status = MET_CAN_COMMAND_ERROR;
    MET_Protocol_Data_Struct.commandRegister.error = MET_CAN_COMMAND_ABORT_CODE;
    return;
}


        
/**
 * 
 * The function checks the flags rxReceptionTrigger or rxErrorTrigger.
 * 
 * In case of reception the function checks the CRC, data Lenght
 * in order to proceed with the protocol decoding.
 * 
 * With the correct frame checked, the protocol is identified:
 * - The Status Register is handled;
 * - The Parameter Register is handled;
 * - The Data Register is handled;
 * - The Command frame is Handled;
 * 
 * The function activate the proper TX frame based on the
 * received processed frame.
 * 
 */
void MET_Can_Protocol_Loop(void){
    MET_Can_Frame_t* cmdFrame;
    static uint8_t lastSequence = 0;

    uint8_t crc = 0;
    uint8_t i;
    
    if(rxReceptionTrigger){
        rxReceptionTrigger = false;                          
        
        
        // Verify the Lenght: it shall be 8 byte
        if(MET_Can_Protocol_RxTx_Struct.rx_messageLength != 8) {
            MET_DefaultError_Callback(MET_CAN_PROTOCOL_ERROR_INVALID_LENGHT);
            MET_Can_Protocol_Reception_Trigger(); // Reschedule the new data reception
            return;
        }
        
        // Verify the CRC code
        for(i=0; i<8; i++) crc ^=  MET_Can_Protocol_RxTx_Struct.rx_message[i];
        if(crc){
            MET_DefaultError_Callback(MET_CAN_PROTOCOL_ERROR_INVALID_CRC);
            MET_Can_Protocol_Reception_Trigger(); // Reschedule the new data reception
            return;
        }
        
        
        // Cast pointer to help the received data decoding
        cmdFrame = (MET_Can_Frame_t*) &MET_Can_Protocol_RxTx_Struct.rx_message;        
        
        // Veries if the sequence number is changed        
        if(cmdFrame->seq == lastSequence) {
            MET_Can_Protocol_Reception_Trigger(); // Reschedule the new data reception
            return;
        }
        
        lastSequence = cmdFrame->seq;
        
        // Copy the received to the data that will be retransmitted 
        memcpy(MET_Can_Protocol_RxTx_Struct.tx_message, MET_Can_Protocol_RxTx_Struct.rx_message,8);
        
        
        // Identifies the Protocol command
        switch(cmdFrame->frame_cmd){
            case MET_CAN_PROTOCOL_READ_REVISION:
                memcpy(&MET_Can_Protocol_RxTx_Struct.tx_message[3], &MET_Protocol_Data_Struct.revisionRegister, sizeof(MET_Register_t));
                break;
            
            case MET_CAN_PROTOCOL_READ_ERRORS:
                memcpy(&MET_Can_Protocol_RxTx_Struct.tx_message[3], &MET_Protocol_Data_Struct.errorsRegister, sizeof(MET_Register_t));                    
                
                // Clears the momentary error bytes
                MET_Protocol_Data_Struct.errorsRegister.mom0 = 0;
                MET_Protocol_Data_Struct.errorsRegister.mom1 = 0;                
                break;
            
            case MET_CAN_PROTOCOL_READ_COMMAND:
                memcpy(&MET_Can_Protocol_RxTx_Struct.tx_message[2], &MET_Protocol_Data_Struct.commandRegister, sizeof(MET_Command_Register_t));                
                break;
                
            case MET_CAN_PROTOCOL_READ_STATUS:
                
                if(cmdFrame->idx < MET_Protocol_Data_Struct.applicationStatusArrayLen){
                    memcpy(&MET_Can_Protocol_RxTx_Struct.tx_message[3], MET_Protocol_Data_Struct.pApplicationStatusArray[cmdFrame->idx].d, sizeof(MET_Register_t));
                }else{
                    // Error index out of range
                    MET_Can_Protocol_RxTx_Struct.tx_message[1] = 0; 
                    MET_Can_Protocol_RxTx_Struct.tx_message[2] = MET_CAN_PROTOCOL_READ_STATUS;
                    MET_Can_Protocol_RxTx_Struct.tx_message[3] = 1; // Index out of range
                }
                
            break;
            
            case MET_CAN_PROTOCOL_READ_DATA:
                
                if(cmdFrame->idx < MET_Protocol_Data_Struct.applicationDataArrayLen){
                    memcpy(&MET_Can_Protocol_RxTx_Struct.tx_message[3], MET_Protocol_Data_Struct.pApplicationDataArray[cmdFrame->idx].d, sizeof(MET_Register_t));
                }else{
                    // Error index out of range
                    MET_Can_Protocol_RxTx_Struct.tx_message[1] = 0; 
                    MET_Can_Protocol_RxTx_Struct.tx_message[2] = MET_CAN_PROTOCOL_READ_DATA;
                    MET_Can_Protocol_RxTx_Struct.tx_message[3] = 1; // Index out of range
                }
                
            break;
            
            case MET_CAN_PROTOCOL_READ_PARAM:
                
                if(cmdFrame->idx < MET_Protocol_Data_Struct.applicationParameterArrayLen){
                    memcpy(&MET_Can_Protocol_RxTx_Struct.tx_message[3], MET_Protocol_Data_Struct.pApplicationParameterArray[cmdFrame->idx].d, sizeof(MET_Register_t));
                }else{
                    // Error index out of range
                    MET_Can_Protocol_RxTx_Struct.tx_message[1] = 0; 
                    MET_Can_Protocol_RxTx_Struct.tx_message[2] = MET_CAN_PROTOCOL_READ_PARAM;
                    MET_Can_Protocol_RxTx_Struct.tx_message[3] = 1; // Index out of range
                }
                
            break;
            
            case MET_CAN_PROTOCOL_WRITE_DATA:
                
                // Write data Status register
                if( cmdFrame->idx < MET_Protocol_Data_Struct.applicationDataArrayLen){
                    memcpy(MET_Protocol_Data_Struct.pApplicationDataArray[cmdFrame->idx].d, &MET_Can_Protocol_RxTx_Struct.tx_message[3], sizeof(MET_Register_t));
                }else{
                     // Error index out of range
                    MET_Can_Protocol_RxTx_Struct.tx_message[1] = 0; 
                    MET_Can_Protocol_RxTx_Struct.tx_message[2] = MET_CAN_PROTOCOL_WRITE_DATA;
                    MET_Can_Protocol_RxTx_Struct.tx_message[3] = 1; // Index out of range
                }                
                break;

            case MET_CAN_PROTOCOL_WRITE_PARAM:
                
                // Write data Status register
                if( cmdFrame->idx <  MET_Protocol_Data_Struct.applicationParameterArrayLen){
                    memcpy(MET_Protocol_Data_Struct.pApplicationParameterArray[cmdFrame->idx].d, &MET_Can_Protocol_RxTx_Struct.tx_message[3], sizeof(MET_Register_t));
                }else{
                     // Error index out of range
                    MET_Can_Protocol_RxTx_Struct.tx_message[1] = 0; 
                    MET_Can_Protocol_RxTx_Struct.tx_message[2] = MET_CAN_PROTOCOL_WRITE_PARAM;
                    MET_Can_Protocol_RxTx_Struct.tx_message[3] = 1; // Index out of range
                }                
                break;
                
            case MET_CAN_PROTOCOL_STORE_PARAMS:
                break;
            
            case MET_CAN_PROTOCOL_COMMAND_EXEC:
                
                // Command execution handler not assigned by the application 
                if(MET_Protocol_Data_Struct.applicationCommandHandler == 0){        
                        MET_Protocol_Data_Struct.commandRegister.command = cmdFrame->idx;
                        MET_Protocol_Data_Struct.commandRegister.status = MET_CAN_COMMAND_ERROR;
                        MET_Protocol_Data_Struct.commandRegister.result[0] = 0;
                        MET_Protocol_Data_Struct.commandRegister.result[1] = 0;                        
                        MET_Protocol_Data_Struct.commandRegister.error = MET_CAN_COMMAND_NOT_AVAILABLE;
                        memcpy(&MET_Can_Protocol_RxTx_Struct.tx_message[2], &MET_Protocol_Data_Struct.commandRegister, sizeof(MET_Command_Register_t));    
                        break;
                }
                
                // Busy condition: command already in execution
                if((cmdFrame->idx != MET_COMMAND_ABORT) && ( MET_Protocol_Data_Struct.commandRegister.status == MET_CAN_COMMAND_EXECUTING)){
                    // The Command Register shall not be modified in this case
                    ((MET_Command_Register_t*)&MET_Can_Protocol_RxTx_Struct.tx_message[2])->command = cmdFrame->idx; // Command code
                    ((MET_Command_Register_t*)&MET_Can_Protocol_RxTx_Struct.tx_message[2])->status = MET_CAN_COMMAND_ERROR; 
                    ((MET_Command_Register_t*)&MET_Can_Protocol_RxTx_Struct.tx_message[2])->result[0] = 0; // Command code
                    ((MET_Command_Register_t*)&MET_Can_Protocol_RxTx_Struct.tx_message[2])->result[1] = 0; // Sequence code
                    ((MET_Command_Register_t*)&MET_Can_Protocol_RxTx_Struct.tx_message[2])->error = MET_CAN_COMMAND_BUSY; // Sequence code
                    break;
                }
               
                 
                // In case of Abort request, the command code shall not be changed 
                if(cmdFrame->idx != MET_COMMAND_ABORT) MET_Protocol_Data_Struct.commandRegister.command = cmdFrame->idx;
                
                // Pre assign a wrong status to check if the Application returns wih a correct code
                MET_Protocol_Data_Struct.commandRegister.status = MET_CAN_COMMAND_STATUS_UNASSIGNED;
               
                // Pre assign the command data results               
                MET_Protocol_Data_Struct.commandRegister.result[0] = 0;
                MET_Protocol_Data_Struct.commandRegister.result[1] = 0; 
                
                // Calls the Application command handler
                MET_Protocol_Data_Struct.applicationCommandHandler(cmdFrame->idx, cmdFrame->d[0], cmdFrame->d[1], cmdFrame->d[2], cmdFrame->d[3]);

                // The Application should have assigned the correct returning code to the Command Register
                if(MET_Protocol_Data_Struct.commandRegister.status > MET_CAN_COMMAND_ERROR){
                    MET_Protocol_Data_Struct.commandRegister.status = MET_CAN_COMMAND_ERROR;
                    MET_Protocol_Data_Struct.commandRegister.error  = MET_CAN_COMMAND_WRONG_RETURN_CODE;                
                }
                
                memcpy(&MET_Can_Protocol_RxTx_Struct.tx_message[2], &MET_Protocol_Data_Struct.commandRegister, sizeof(MET_Command_Register_t));                                    
                break;
            
        }
    
        
        // Calcs the CRC of the buffer to be sent 
        crc = 0;
        for(i=0; i<7; i++) crc ^=  MET_Can_Protocol_RxTx_Struct.tx_message[i];
        MET_Can_Protocol_RxTx_Struct.tx_message[7] = crc;

        // Sends the buffer to the caller
        CAN0_MessageTransmit(_CAN_ID_BASE_ADDRESS + MET_Protocol_Data_Struct.deviceID, 8, MET_Can_Protocol_RxTx_Struct.tx_message, CAN_MODE_NORMAL, CAN_MSG_ATTR_TX_FIFO_DATA_FRAME);  
        MET_Can_Protocol_Reception_Trigger(); // Reschedule the new data reception
        
    }else if(rxErrorTrigger){
        rxErrorTrigger = false;
        MET_Can_Protocol_Reception_Trigger();
    }

    // Rescedule the reception for a next frame
    
}   


/**
 * @brief CAN Reception Interrupt Handler
 * 
 * This function is assigned in the MET_Can_Protocol_Reception_Trigger() function
 * when the reception is activated. When the interrupt is executed it needs to be 
 * rescheduled in order to be executed again.
 * 
 * The function determines if the interrupt has been generated 
 * because of an error condition or because of a correct 
 * frame received. In both cases a given flag is set so it can be 
 * handled into the MET_Can_Protocol_Loop() function out of the Interrupt context.
 * 
 * 
 * @param context
 */
void MET_Can_Protocol_Reception_Callback(uintptr_t context)
{
 
    uint32_t  status = CAN0_ErrorGet();

    if (((status & CAN_PSR_LEC_Msk) == CAN_ERROR_NONE) || 
	((status & CAN_PSR_LEC_Msk) == CAN_ERROR_LEC_NC))
    {
       // SUCCESSO
        rxReceptionTrigger = true;
        
    }    else
    {
        // ERROR
        rxErrorTrigger = true;
        
    }
}

/**
 * @brief Module Error Handler
 * 
 * @param errEvent This is the error code signaled
 */
void MET_DefaultError_Callback(uint8_t errEvent)
{
    switch(errEvent){
        case 0:
            break;
        default:
            break;
    }
    
    return;
}

/** @}*/  // metCanLocal