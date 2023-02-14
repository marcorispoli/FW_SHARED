#define _MET_CAN_PROTOCOL_C

#include "MET_can_protocol.h"                // SYS function prototypes
#include "application.h"

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

void MET_Can_Protocol_Init(uint8_t deviceID, MET_Register_t* pStatusArray, uint8_t StatusLen,MET_Register_t* pDataArray, uint8_t DataLen){
    
    // Assignes the current device ID
    deviceID = deviceID;
    
    // Harmony 3 library call: Init memory of the CAN Bus module
    CAN0_MessageRAMConfigSet(Can0MessageRAM);
    
    // Add the external STATUS register array
    MET_Protocol_Data_Struct.pStatusArray = pStatusArray;
    MET_Protocol_Data_Struct.statusArrayLen = StatusLen;
    
    // Add the external DATA register array
    MET_Protocol_Data_Struct.pDataArray = pDataArray;
    MET_Protocol_Data_Struct.dataArrayLen = DataLen;

    // Schedules the next reception interrupt
    MET_Can_Protocol_Reception_Trigger();      
    return ;
 }


/**
 * @brief 
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
    MET_Can_Protocol_Command_t* cmdFrame;

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
        cmdFrame = (MET_Can_Protocol_Command_t*) &MET_Can_Protocol_RxTx_Struct.rx_message;        
        
        // Copy the received to the data that will be retransmitted 
        memcpy(MET_Can_Protocol_RxTx_Struct.tx_message, MET_Can_Protocol_RxTx_Struct.rx_message,8);
             
        // Identifies the Protocol command
        switch(cmdFrame->frame_cmd){
            case MET_CAN_PROTOCOL_READ_STATUS:
                
                // Read the Status register
                if( cmdFrame->idx < MET_Protocol_Data_Struct.statusArrayLen){
                    memcpy(&MET_Can_Protocol_RxTx_Struct.tx_message[3], MET_Protocol_Data_Struct.pStatusArray[cmdFrame->idx].d, sizeof(MET_Register_t));                   
                }else{
                    // Error index out of range
                    MET_Can_Protocol_RxTx_Struct.tx_message[2] = 0; 
                }                
                break;
                
            case MET_CAN_PROTOCOL_WRITE_DATA:
                
                // Write data Status register
                if( cmdFrame->idx < MET_Protocol_Data_Struct.dataArrayLen){
                    
                    memcpy(MET_Protocol_Data_Struct.pDataArray[cmdFrame->idx].d, cmdFrame->data.d, sizeof(MET_Register_t));
                }else{
                    // Error index out of range
                    MET_Can_Protocol_RxTx_Struct.tx_message[2] = 0; 
                }                
                break;

            case MET_CAN_PROTOCOL_WRITE_PARAM:
                break;
            case MET_CAN_PROTOCOL_STORE_PARAMS:
                break;
            case MET_CAN_PROTOCOL_COMMAND_EXEC:
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