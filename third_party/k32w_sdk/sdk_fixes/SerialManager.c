/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2021 NXP
* All rights reserved.
*
* \file
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */


/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */

#include "SerialManager.h"
#include "Panic.h"
#include "FunctionLib.h"
#if (gSerialMgrUseSPI_c)
#include "GPIO_Adapter.h"
#endif

#include "fsl_device_registers.h"
#if USE_SDK_OSA
#include "fsl_os_abstraction.h"
#else
    extern void OSA_InterruptEnable(void);
    extern void OSA_InterruptDisable(void);
#endif
#include "fsl_common.h"
#include "pin_mux.h"
#include <string.h>

#if gNvStorageIncluded_d
#include "NVM_Interface.h"
#endif


#if gSerialMgr_DisallowMcuSleep_d
#include "PWR_Interface.h"
#endif

#if (gSerialMgrUseUart_c)
#include "UART_Serial_Adapter.h"
#endif

#if (gSerialMgrUseIIC_c)
#include "I2C_Serial_Adapter.h"
#endif

#if (gSerialMgrUseSPI_c)
#include "SPI_Serial_Adapter.h"
#endif

#if (gSerialMgrUseUSB_c)
#include "VirtualComInterface.h"
#endif

#if (gSerialMgrUseUSB_VNIC_c)
#include "VirtualNicInterface.h"
#endif

#if gSerialMgrUseFSCIHdr_c
#include "FsciInterface.h"
#include "FsciCommunication.h"
#endif

/*! *********************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
********************************************************************************** */
#ifndef gSMGR_UseOsSemForSynchronization_c
#if defined (USE_SDK_OSA) && (!USE_SDK_OSA)
#define gSMGR_UseOsSemForSynchronization_c 0
#else
#define gSMGR_UseOsSemForSynchronization_c (USE_RTOS)
#endif
#endif

#define mSerial_IncIdx_d(idx, max) if( ++(idx) >= (max) ) { (idx) = 0; }

#define mSerial_DecIdx_d(idx, max) if( (idx) > 0 ) { (idx)--; } else  { (idx) = (max) - 1; }

#define mSerial_AddIdx_d(idx, val, max) {idx += val; if(idx >= max) idx -= max;}

#define gSMRxBufSize_c (gSerialMgrRxBufSize_c + 1)

#define mSMGR_DapIsrPrio_c    (0x80)

#if gSerialMgrUseFSCIHdr_c
#define mSMGR_FSCIHdrLen_c  sizeof(clientPacketHdr_t)
#endif

/*! *********************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
********************************************************************************** */
#if (gSerialManagerMaxInterfaces_c)
/*
 * Set the size of the Rx buffer indexes
 */
#if gSMRxBufSize_c < 255
typedef uint8_t bufIndex_t;
#else
typedef uint16_t bufIndex_t;
#endif

/*
 * Defines events recognized by the SerialManager's Task
 * Message used to enqueue asynchronous tx data
 */
typedef struct SerialManagetMsg_tag{
    pSerialCallBack_t txCallback;
    void             *pTxParam;
    uint8_t          *pData;
    uint16_t          dataSize;
}SerialMsg_t;

/*
 * Defines the serial interface structure
 */
typedef struct serial_tag{
    serialInterfaceType_t  serialType;
    uint8_t                serialChannel;
    /* Rx parameters */
    volatile bufIndex_t    rxIn;
    volatile bufIndex_t    rxOut;
    pSerialCallBack_t      rxCallback;
    void                  *pRxParam;
    uint8_t                rxBuffer[gSMRxBufSize_c];
    /* Tx parameters */
    SerialMsg_t            txQueue[gSerialMgrTxQueueSize_c];
#if gSMGR_UseOsSemForSynchronization_c
    osaSemaphoreId_t       txSyncSemId;
#if gSerialMgr_BlockSenderOnQueueFull_c
    osaSemaphoreId_t       txQueueSemId;
    uint8_t                txBlockedTasks;
#endif
#endif
#if gSerialMgrUseFSCIHdr_c
    fsciLen_t              rxFsciIn;
    fsciLen_t              rxFsciLen;
    uint8_t                rxFsciPkt;
#endif
    volatile uint8_t       txIn;
    volatile uint8_t       txOut;
    volatile uint8_t       txCurrent;
    volatile uint8_t       txNo;
    volatile uint8_t       events;
    volatile uint8_t       state;
    volatile bool_t        txMntOngoing;
}serial_t;

/*
 * SMGR task event flags
 */
typedef enum{
    gSMGR_Rx_c     = (1<<0),
    gSMGR_TxDone_c = (1<<1),
    gSMGR_TxNew_c  = (1<<2)
}serialEventType_t;

/*
 * Common driver data structure union
 */
typedef union smgrDrvData_tag
{
#if (gSerialMgrUseUart_c)
  uartState_t uartState;
#endif /* #if (gSerialMgrUseUart_c) */
#if (gSerialMgrUseIIC_c)
  i2cState_t i2cState;
#endif
#if (gSerialMgrUseSPI_c)
  spiState_t spiState;
#endif
  void *pDrvData;
}smgrDrvData_t;
#endif /* #if (gSerialManagerMaxInterfaces_c) */

/*! *********************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
********************************************************************************** */
#if (gSerialManagerMaxInterfaces_c)
/*
 * SMGR internal functions
 */
#if USE_SDK_OSA
void SerialManagerTask(osaTaskParam_t argument);
#else
void SerialManagerTask(void);
#endif

void  SerialManager_RxNotify(uint32_t i);
void  SerialManager_TxNotify(uint32_t i);
#if gSMGR_UseOsSemForSynchronization_c
static void  Serial_SyncTxCallback(void *pSer);
#endif
static void  Serial_TxQueueMaintenance(serial_t *pSer);
static serialStatus_t Serial_WriteInternal (uint8_t InterfaceId);
#if (gSerialMgrUseSPI_c) || (gSerialMgrUseIIC_c)
static uint32_t Serial_GetInterfaceIdFromType(serialInterfaceType_t type);
#endif

#if (gSerialMgrUseSPI_c)
static void Serial_SpiSendDummyByte(uint32_t i);
#endif

/*
 * UART, LPUART and LPSCI specific functions
 */
#if (gSerialMgrUseUart_c)
static void Serial_UartRxCb(uartState_t* state);
static void Serial_UartTxCb(uartState_t* state);
#endif

/*
 * SPI specific functions
 */
#if (gSerialMgrUseSPI_c)
static void SpiMasterDapISR(void);
static void SpiCallback(uint32_t flags, spiState_t* pState);
#endif

/*
 * IIC specific functions
 */
#if (gSerialMgrUseIIC_c)
static void I2cMasterDapISR(void);
static void Serial_I2cRxCb(i2cState_t* state);
static void Serial_I2cTxCb(i2cState_t* state);
#endif

#endif /* #if (gSerialManagerMaxInterfaces_c) */

#if defined(FWK_SMALL_RAM_CONFIG)
void FwkInit(void);
#endif

/*! *********************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
#if USE_SDK_OSA
extern const uint8_t gUseRtos_c;
#endif
/*! *********************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
********************************************************************************** */
#if gSerialManagerMaxInterfaces_c

/*
 * RTOS objects definition
 */
#if defined(FWK_SMALL_RAM_CONFIG)
extern osaEventId_t  gFwkCommonEventId;
extern osaTaskId_t gFwkCommonTaskId;
#define gSerialManagerTaskId gFwkCommonTaskId
#define mSMTaskEventId gFwkCommonEventId

#else
#if USE_SDK_OSA
OSA_TASK_DEFINE( SerialManagerTask, gSerialTaskPriority_c, 1, gSerialTaskStackSize_c, FALSE );
osaTaskId_t gSerialManagerTaskId;
osaEventId_t        mSMTaskEventId;
#endif
#endif /* defined(FWK_SMALL_RAM_CONFIG) */

/*
 * SMGR internal data
 */
static serial_t      mSerials[gSerialManagerMaxInterfaces_c];
static smgrDrvData_t mDrvData[gSerialManagerMaxInterfaces_c];

/*
 * Default configuration for IIC driver
 */
#if (gSerialMgrUseIIC_c)
gpioInputPinConfig_t mI2cMasterDapCfg = {
    .gpioPort = gI2cMasterDap_Port_d,
    .gpioPin = gI2cMasterDap_Pin_d,
#if gSerialMgrSlaveDapTxLogicOne_c
    .pullSelect = pinPull_Down_c,
    .interruptSelect = pinInt_LogicOne_c,
#else
    .pullSelect = pinPull_Up_c,
    .interruptSelect = pinInt_LogicZero_c,
#endif
};

gpioOutputPinConfig_t mI2cSlaveDapCfg = {
    .gpioPort = gI2cSlaveDap_Port_d,
    .gpioPin = gI2cSlaveDap_Pin_d,
#if gSerialMgrSlaveDapTxLogicOne_c
    .outputLogic = 0,
#else
    .outputLogic = 1,
#endif
    .slewRate = pinSlewRate_Fast_c,
    .driveStrength = pinDriveStrength_Low_c
};
#endif /* #if (gSerialMgrUseIIC_c) */

/*
 * Default configuration for SPI driver
 */
#if (gSerialMgrUseSPI_c)
spiBusConfig_t gSpiConfig = {
    .bitsPerSec = 1000000,
    .master = FALSE,
    .clkActiveHigh = TRUE,
    .clkPhaseFirstEdge = TRUE,
    .MsbFirst = TRUE
};

gpioInputPinConfig_t mSpiMasterDapCfg = {
    .gpioPort = gSpiMasterDap_Port_d,
    .gpioPin = gSpiMasterDap_Pin_d,
#if gSerialMgrSlaveDapTxLogicOne_c
    .pullSelect = pinPull_Down_c,
    .interruptSelect = pinInt_LogicOne_c,
#else
    .pullSelect = pinPull_Up_c,
    .interruptSelect = pinInt_LogicZero_c,
#endif
};


gpioOutputPinConfig_t mSpiSlaveDapCfg = {
    .gpioPort = gSpiSlaveDap_Port_d,
    .gpioPin = gSpiSlaveDap_Pin_d,
#if gSerialMgrSlaveDapTxLogicOne_c
    .outputLogic = 0,
#else
    .outputLogic = 1,
#endif
    .slewRate = pinSlewRate_Fast_c,
    .driveStrength = pinDriveStrength_Low_c
};

#endif

#endif /* #if gSerialManagerMaxInterfaces_c */

/*! *********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
* \brief   Creates the SerialManager's task and initializes internal data structures
*
********************************************************************************** */
void Serial_InitManager( void )
{
#if (gSerialManagerMaxInterfaces_c)
    static uint8_t initialized = FALSE;

    /* Check if SMGR is already initialized */
    if( FALSE == initialized )
    {
        initialized = TRUE;

        /* Fill the structure with zeros */
        FLib_MemSet( mSerials, 0x00, sizeof(mSerials) );
#if defined(FWK_SMALL_RAM_CONFIG)
        FwkInit();
#else
#if USE_SDK_OSA
        mSMTaskEventId = OSA_EventCreate( TRUE );
        if( NULL == mSMTaskEventId )
        {
            panic(0,0,0,0);
        }
        else
        {
            gSerialManagerTaskId = OSA_TaskCreate(OSA_TASK(SerialManagerTask), NULL);
            if( NULL == gSerialManagerTaskId )
            {
                panic(0,0,0,0);
            }
        }
#endif /* USE_SDK_OSA */
#endif /* #if defined(FWK_SMALL_RAM_CONFIG) */
    }
#endif /* #if (gSerialManagerMaxInterfaces_c) */
}

/*! *********************************************************************************
* \brief   The main task of the Serial Manager
*
* \param[in] initialData unused
*
********************************************************************************** */
#if (gSerialManagerMaxInterfaces_c)

#if USE_SDK_OSA
void SerialManagerTask(osaTaskParam_t argument)
#else
void SerialManagerTask(void)
#endif
{
    uint16_t i;
    uint8_t ev;

#if defined(FWK_SMALL_RAM_CONFIG)
    {
#else
#if USE_SDK_OSA
    osaEventFlags_t  mSMTaskEventFlags = 0;
#endif
    while( 1 )
    {
#if USE_SDK_OSA
        /* Wait for an event. The task will block here. */
        (void)OSA_EventWait(mSMTaskEventId, osaEventFlagsAll_c, FALSE, osaWaitForever_c ,&mSMTaskEventFlags);
#endif
#endif
        for( i = 0; i < gSerialManagerMaxInterfaces_c; i++ )
        {
            OSA_InterruptDisable();
            ev = mSerials[i].events;
            mSerials[i].events = 0;
            OSA_InterruptEnable();

            if ( (ev & gSMGR_Rx_c) &&
                 (NULL != mSerials[i].rxCallback) )
            {
                mSerials[i].rxCallback( mSerials[i].pRxParam );
            }

            if( ev & gSMGR_TxDone_c )
            {
                Serial_TxQueueMaintenance(&mSerials[i]);
            }

            /* If the Serial is IDLE and there is data to tx */
            if( (mSerials[i].state == 0) && (mSerials[i].txQueue[mSerials[i].txCurrent].dataSize > 0) )
            {
                (void)Serial_WriteInternal( i );
            }
#if gSerialMgrUseSPI_c
            /* If the SPI Slave has more data to transmit, restart the transfer */
#if gSerialMgrSlaveDapTxLogicOne_c
            if( (GpioReadPinInput(&mSpiMasterDapCfg)) && (mSerials[i].serialType == gSerialMgrSPIMaster_c) && (0 == mSerials[i].state) )
#else
            if( (!GpioReadPinInput(&mSpiMasterDapCfg)) && (mSerials[i].serialType == gSerialMgrSPIMaster_c) && (0 == mSerials[i].state) )
#endif
            {
                if( (0 == mSerials[i].txQueue[mSerials[i].txIn].dataSize) && (NULL == mSerials[i].txQueue[mSerials[i].txIn].txCallback) )
                {
                    Serial_SpiSendDummyByte(i);
                }
            }
#endif

        }

#if !defined(FWK_SMALL_RAM_CONFIG)
#if USE_SDK_OSA
        /* For BareMetal break the while(1) after 1 run */
        if (gUseRtos_c == 0)
#endif
        {
            break;
        }
#endif
    } /* while(1) */
}
#endif


void Serial_RingInit(serialRingState_t *ring, uint8_t * buffer, uint16_t ring_sz)
{
    ring->buffer = buffer;
    ring->in = 0;
    ring->out = 0;
    ring->space_left = ring_sz;
    ring->max = ring_sz;
}


void Serial_RingConsume(serialRingState_t *ring, uint8_t * buffer, uint16_t nb_to_consume)
{
    for (int i = 0; i < nb_to_consume; i++)
    {
        OSA_InterruptDisable();
        buffer[i] = ring->buffer[ring->out];
        ring->out++;
        if (ring->out == ring->max)
            ring->out =  0;
        ring->space_left ++;
        OSA_InterruptEnable();
    }
}

void Serial_RingProduceCharProtected(serialRingState_t *ring, uint8_t byte)
{
    ring->buffer[ring->in] = byte;
    ring->in++;
    if (ring->in == ring->max)
        ring->in = 0;
    ring->space_left--;
}

void Serial_RingProduceChar(serialRingState_t *ring, uint8_t byte)
{
    OSA_InterruptDisable();
    Serial_RingProduceCharProtected(ring, byte);
    OSA_InterruptEnable();
}

/*! *********************************************************************************
* \brief   Initialize a communication interface.
*
* \param[in] pInterfaceId   pointer to a location where the interface Id will be stored
* \param[in] interfaceType  the type of the interface: UART/SPI/IIC/USB
* \param[in] instance       the instance of the HW module (ex: if UART1 is used, this value should be 1)
*
* \return The interface number if success or gSerialManagerInvalidInterface_c if an error occurred.
*
********************************************************************************** */
serialStatus_t Serial_InitInterface( uint8_t *pInterfaceId,
                                     serialInterfaceType_t interfaceType,
                                     uint8_t instance )
{
    serialStatus_t status = gSerial_Success_c;
#if gSerialManagerMaxInterfaces_c
    uint32_t i;
    serial_t *pSer;

    *pInterfaceId = gSerialMgrInvalidIdx_c;

    for ( i=0; i<gSerialManagerMaxInterfaces_c; i++ )
    {
        pSer = &mSerials[i];

        if ( (pSer->serialType == interfaceType) &&
            (pSer->serialChannel == instance) )
        {
            /* The Interface is already opened. */
            status = gSerial_InterfaceInUse_c;
            break;
        }

        if ( pSer->serialType == gSerialMgrNone_c )
        {
            OSA_InterruptDisable();
            pSer->serialChannel = instance;
            switch ( interfaceType )
            {
            case gSerialMgrUart_c:
#if gSerialMgrUseUart_c && FSL_FEATURE_SOC_UART_COUNT
                UART_Initialize(instance, &mDrvData[i].uartState);
//                mDrvData[i].uartState.pRxData = pSer->rxBuffer;
                Serial_RingInit(&mDrvData[i].uartState.rx_ring,
                                pSer->rxBuffer,
                                gSMRxBufSize_c);
                UART_InstallRxCalback(instance, Serial_UartRxCb, i);
                UART_InstallTxCalback(instance, Serial_UartTxCb, i);
#endif
                break;

            case gSerialMgrLpuart_c:
#if gSerialMgrUseUart_c && FSL_FEATURE_SOC_LPUART_COUNT
                LPUART_Initialize(instance, &mDrvData[i].uartState);
//                mDrvData[i].uartState.pRxData = pSer->rxBuffer;
                Serial_RingInit(&mDrvData[i].uartState.rx_ring,
                                pSer->rxBuffer,
                                gSMRxBufSize_c);
                LPUART_InstallRxCalback(instance, Serial_UartRxCb, i);
                LPUART_InstallTxCalback(instance, Serial_UartTxCb, i);
#endif
                break;

            case gSerialMgrLpsci_c:
#if gSerialMgrUseUart_c && FSL_FEATURE_SOC_LPSCI_COUNT
                LPSCI_Initialize(instance, &mDrvData[i].uartState);
//                mDrvData[i].uartState.pRxData = pSer->rxBuffer;
                Serial_RingInit(&mDrvData[i].uartState.rx_ring,
                                pSer->rxBuffer,
                                gSMRxBufSize_c);
                LPSCI_InstallRxCalback(instance, Serial_UartRxCb, i);
                LPSCI_InstallTxCalback(instance, Serial_UartTxCb, i);
#endif
                break;

            case gSerialMgrUSB_c:
#if gSerialMgrUseUSB_c
                mDrvData[i].pDrvData = VirtualCom_Init(i);
                if (NULL == mDrvData[i].pDrvData)
                {
                    status =  gSerial_InternalError_c;
                }
#endif
                break;
            case gSerialMgrUSB_VNIC_c:
#if gSerialMgrUseUSB_VNIC_c
                mDrvData[i].pDrvData = VirtualNic_Init(i);
                if (NULL == mDrvData[i].pDrvData)
                {
                    status = gSerial_InternalError_c;
                }
#endif
                break;
            case gSerialMgrIICMaster_c:
#if gSerialMgrUseIIC_c
                mDrvData[i].i2cState.master = TRUE;
                mDrvData[i].i2cState.address = gSerialMgrIICAddress_c;
                I2C_Initialize(instance, &mDrvData[i].i2cState);
                I2C_InstallRxCalback(instance, Serial_I2cRxCb, i);
                I2C_InstallTxCalback(instance, Serial_I2cTxCb, i);
                GpioInstallIsr(I2cMasterDapISR,
                               gGpioIsrPrioNormal_c,
                               mSMGR_DapIsrPrio_c,
                               &mI2cMasterDapCfg);
                GpioInputPinInit(&mI2cMasterDapCfg, 1);
#endif
                break;

            case gSerialMgrIICSlave_c:
#if gSerialMgrUseIIC_c
                GpioOutputPinInit(&mI2cSlaveDapCfg, 1);
                mDrvData[i].i2cState.master = FALSE;
                mDrvData[i].i2cState.address = gSerialMgrIICAddress_c;
                I2C_Initialize(instance, &mDrvData[i].i2cState);
                mDrvData[i].i2cState.pRxData = &mSerials[i].rxBuffer[mSerials[i].rxIn];
                I2C_InstallRxCalback(instance, Serial_I2cRxCb, i);
                I2C_InstallTxCalback(instance, Serial_I2cTxCb, i);
#endif
                break;

            case gSerialMgrSPIMaster_c:
#if gSerialMgrUseSPI_c
                Spi_Init(instance, &(mDrvData[i].spiState), SpiCallback, (void*)i );
                gSpiConfig.master = TRUE;
                mDrvData[i].spiState.signalRxByte = TRUE;
                mDrvData[i].spiState.pRxData = &pSer->rxBuffer[pSer->rxIn];
                Spi_Configure(instance, &gSpiConfig);
                GpioInstallIsr(SpiMasterDapISR, gGpioIsrPrioNormal_c, mSMGR_DapIsrPrio_c, &mSpiMasterDapCfg);
                GpioInputPinInit(&mSpiMasterDapCfg, 1);
#endif
                break;

            case gSerialMgrSPISlave_c:
#if gSerialMgrUseSPI_c
                Spi_Init(instance, &(mDrvData[i].spiState), SpiCallback, (void*)i );
                gSpiConfig.master = FALSE;
                mDrvData[i].spiState.signalRxByte = TRUE;
                mDrvData[i].spiState.pRxData = &pSer->rxBuffer[pSer->rxIn];
                Spi_Configure(instance, &gSpiConfig);
                GpioOutputPinInit(&mSpiSlaveDapCfg, 1);
#endif
                break;

            case gSerialMgrUsart_c:
#if gSerialMgrUseUart_c && FSL_FEATURE_SOC_USART_COUNT
                USART_Initialize(instance, &mDrvData[i].uartState);
//                mDrvData[i].uartState.pRxData = &pSer->rxBuffer[pSer->rxIn];
                Serial_RingInit(&mDrvData[i].uartState.rx_ring,
                                pSer->rxBuffer,
                                gSMRxBufSize_c);
                USART_InstallRxCalback(instance, Serial_UartRxCb, i);
                USART_InstallTxCalback(instance, Serial_UartTxCb, i);
#endif
                break;

            case gSerialMgrCustom_c:
                /* Nothing to do here. The initialization is done outside SerialManager */
                break;

            default:
                status = gSerial_InvalidInterface_c;
                break;
            }

#if gSMGR_UseOsSemForSynchronization_c
            if( (status == gSerial_Success_c) &&
                ((pSer->txSyncSemId = OSA_SemaphoreCreate(0)) == NULL) )
            {
                status = gSerial_SemCreateError_c;
            }

#if gSerialMgr_BlockSenderOnQueueFull_c
            if( (status == gSerial_Success_c) &&
                ((pSer->txQueueSemId = OSA_SemaphoreCreate( 0)) == NULL) )
            {
                status = gSerial_SemCreateError_c;
            }
#endif /* gSerialMgr_BlockSenderOnQueueFull_c */
#endif /* gSMGR_UseOsSemForSynchronization_c */

            if( status == gSerial_Success_c )
            {
                pSer->serialType = interfaceType;
                *pInterfaceId = i;
            }
            OSA_InterruptEnable();
            break;
        }
    }

    /* There are no more free interfaces. */
    if( i == gSerialManagerMaxInterfaces_c )
    {
        status = gSerial_MaxInterfacesReached_c;
    }
#else
    (void)interfaceType;
    (void)instance;
    (void)pInterfaceId;
#endif
    return status;
}

/*! *********************************************************************************
* \brief   Close a communication interface.
*
* \param[in] pInterfaceId   pointer to a location where the interface Id will be stored
*
* \return gSerial_Success_c if ok, other values in case of failure
*
********************************************************************************** */
serialStatus_t Serial_CloseInterface( uint8_t InterfaceId)
{
    serialStatus_t status = gSerial_Success_c;
#if gSerialManagerMaxInterfaces_c
    serial_t *pSer = &mSerials[InterfaceId];

    OSA_InterruptDisable();
    do {
        if ( pSer->serialType == gSerialMgrNone_c ) break;

        serialInterfaceType_t interfaceType = pSer->serialType;
        uint8_t instance = pSer->serialChannel;
        switch ( interfaceType )
        {
        case gSerialMgrUart_c:
#if gSerialMgrUseUart_c && FSL_FEATURE_SOC_UART_COUNT
            UART_InstallRxCalback(instance, NULL, InterfaceId);
            UART_InstallTxCalback(instance, NULL, InterfaceId);
#endif
            break;

        case gSerialMgrLpuart_c:
#if gSerialMgrUseUart_c && FSL_FEATURE_SOC_LPUART_COUNT
            LPUART_InstallRxCalback(instance, NULL, InterfaceId);
            LPUART_InstallTxCalback(instance, NULL, InterfaceId);
#endif
            break;

        case gSerialMgrLpsci_c:
#if gSerialMgrUseUart_c && FSL_FEATURE_SOC_LPSCI_COUNT
            LPSCI_InstallRxCalback(instance, NULL, InterfaceId);
            LPSCI_InstallTxCalback(instance, NULL, InterfaceId);
#endif
            break;

        case gSerialMgrUSB_c:
        case gSerialMgrUSB_VNIC_c:
            break;
        case gSerialMgrIICMaster_c:
        case gSerialMgrIICSlave_c:
#if gSerialMgrUseIIC_c
            I2C_InstallRxCalback(instance, NULL, InterfaceId);
            I2C_InstallTxCalback(instance, NULL, InterfaceId);
#endif
            break;

        case gSerialMgrSPIMaster_c:
        case gSerialMgrSPISlave_c:
#if gSerialMgrUseSPI_c
            Spi_Init(instance, &(mDrvDataInterfaceId].spiState), NULL, (void*)InterfaceId );
#endif
            break;

        case gSerialMgrUsart_c:
#if gSerialMgrUseUart_c && FSL_FEATURE_SOC_USART_COUNT
            USART_InstallRxCalback(instance, NULL, InterfaceId);
            USART_InstallTxCalback(instance, NULL, InterfaceId);
#endif
            break;

        case gSerialMgrCustom_c:
            /* Nothing to do here. The initialization is done outside SerialManager */
            break;

        default:
            status = gSerial_InvalidInterface_c;
            break;
        }
#if gSMGR_UseOsSemForSynchronization_c
            OSA_SemaphoreDestroy(pSer->txSyncSemId);
#if gSerialMgr_BlockSenderOnQueueFull_c
            OSA_SemaphoreDestroy(pSer->txQueueSemId);
#endif /* gSerialMgr_BlockSenderOnQueueFull_c */
#endif /* gSMGR_UseOsSemForSynchronization_c */
    } while (0);
    OSA_InterruptEnable();

#else
    (void)InterfaceId;
#endif
    return status;
}


/*! *********************************************************************************
* \brief   Transmit a data buffer asynchronously
*
* \param[in] InterfaceId the interface number
* \param[in] pBuf pointer to data location
* \param[in] bufLen the number of bytes to be sent
* \param[in] pSerialRxCallBack pointer to a function that will be called when
*            a new char is available
*
* \return The status of the operation
*
********************************************************************************** */
serialStatus_t Serial_AsyncWrite( uint8_t InterfaceId,
                                  uint8_t *pBuf,
                                  uint16_t bufLen,
                                  pSerialCallBack_t cb,
                                  void *pTxParam )
{
    serialStatus_t status = gSerial_Success_c;
    SERIAL_DBG_LOG("bufLen=%d",  bufLen);

#if gSerialManagerMaxInterfaces_c
    SerialMsg_t *pMsg = NULL;
    serial_t *pSer = &mSerials[InterfaceId];

#if gSerialMgr_ParamValidation_d
    if( (NULL == pBuf) || (0 == bufLen) || (InterfaceId >= gSerialManagerMaxInterfaces_c) ||
        (pSer->serialType == gSerialMgrNone_c) )
    {
        status = gSerial_InvalidParameter_c;
    }
    else
#endif
    {

#if (gSerialMgr_BlockSenderOnQueueFull_c == 0) || ((gSerialMgr_BlockSenderOnQueueFull_c) && (gSMGR_UseOsSemForSynchronization_c))
        osaTaskId_t taskHandler = OSA_TaskGetId();
#endif

#if (gSerialMgr_BlockSenderOnQueueFull_c == 0)
        if( taskHandler == gSerialManagerTaskId )
        {
            Serial_TxQueueMaintenance(pSer);
        }
#endif

        /* Check if slot is free */
        do {
            OSA_InterruptDisable();

            if( (0 == pSer->txQueue[pSer->txIn].dataSize) && (NULL == pSer->txQueue[pSer->txIn].txCallback) && (pSer->txNo < gSerialMgrTxQueueSize_c) )
            {
                pMsg = &pSer->txQueue[pSer->txIn];
                pMsg->dataSize   = bufLen;
                pMsg->pData      = (void*)pBuf;
                pMsg->txCallback = cb;
                pMsg->pTxParam   = pTxParam;
                mSerial_IncIdx_d(pSer->txIn, gSerialMgrTxQueueSize_c)
                pSer->txNo++;
            }
#if (gSerialMgr_BlockSenderOnQueueFull_c) && (gSMGR_UseOsSemForSynchronization_c)
            else
            {
                if(taskHandler != gSerialManagerTaskId)
                {
                    pSer->txBlockedTasks++;
                }
            }
#endif
            OSA_InterruptEnable();

            if( pMsg )
            {
                status = Serial_WriteInternal( InterfaceId );
                break;
            }
            else
            {
                status = gSerial_OutOfMemory_c;
#if gSerialMgr_BlockSenderOnQueueFull_c
#if gSMGR_UseOsSemForSynchronization_c
                if(taskHandler != gSerialManagerTaskId)
                {
                    (void)OSA_SemaphoreWait(pSer->txQueueSemId, osaWaitForever_c);
                }
                else
#endif
                {
                    Serial_TxQueueMaintenance(pSer);
                }
#else
                break;
#endif
            }
        } while( status != gSerial_Success_c );
    }
#else
    (void)InterfaceId;
    (void)pBuf;
    (void)bufLen;
    (void)cb;
    (void)pTxParam;
#endif /* gSerialManagerMaxInterfaces_c */
    return status;
}


/*! *********************************************************************************
* \brief Transmit a data buffer synchronously. The task will block until the Tx is done
*
* \param[in] pBuf pointer to data location
* \param[in] bufLen the number of bytes to be sent
* \param[in] InterfaceId the interface number
*
* \return The status of the operation
*
********************************************************************************** */
serialStatus_t Serial_SyncWrite( uint8_t InterfaceId,
                                 uint8_t *pBuf,
                                 uint16_t bufLen )
{
    serialStatus_t status = gSerial_Success_c;
#if gSerialManagerMaxInterfaces_c
    pSerialCallBack_t cb = NULL;
    serial_t *pSer = &mSerials[InterfaceId];
    SERIAL_DBG_LOG("bufLen=%d",  bufLen);

#if gSMGR_UseOsSemForSynchronization_c
    /* If the calling task is SMGR do not block on semaphore */
    if( OSA_TaskGetId() != gSerialManagerTaskId )
         cb = Serial_SyncTxCallback;
#endif

    status  = Serial_AsyncWrite(InterfaceId, pBuf, bufLen, cb, pSer);

    if( (gSerial_InvalidParameter_c != status) &&
        (gSerial_OutOfMemory_c != status) )
    {
        /* Wait until Tx finishes. The semaphore will be released by the SMGR task */
#if gSMGR_UseOsSemForSynchronization_c
        if( cb )
        {
            (void)OSA_SemaphoreWait(pSer->txSyncSemId, osaWaitForever_c);
        }
        else
#endif
        {
            while(pSer->state) {}
        }
#if (gSerialMgrUseUart_c)
        switch (pSer->serialType)
        {
#if FSL_FEATURE_SOC_UART_COUNT
        case gSerialMgrUart_c:
            while(UART_IsTxActive(pSer->serialChannel)) {}
            break;
#endif
#if FSL_FEATURE_SOC_LPUART_COUNT
        case gSerialMgrLpuart_c:
            while(LPUART_IsTxActive(pSer->serialChannel)) {}
            break;
#endif
#if FSL_FEATURE_SOC_LPSCI_COUNT
        case gSerialMgrLpsci_c:
            while(LPSCI_IsTxActive(pSer->serialChannel)) {}
            break;
#endif
#if FSL_FEATURE_SOC_USART_COUNT
            case gSerialMgrUsart_c:
                while (USART_IsTxActive(pSer->serialChannel))
                {}
                break;
#endif
        default:
            break;
        }
#endif
    }
#else
    (void)pBuf;
    (void)bufLen;
    (void)InterfaceId;
#endif /* gSerialManagerMaxInterfaces_c */
    return status;
}

/*! *********************************************************************************
* \brief   Returns a specified number of characters from the Rx buffer
*
* \param[in] InterfaceId the interface number
* \param[out] pData pointer to location where to store the characters
* \param[in] dataSize the number of characters to be read
* \param[out] bytesRead the number of characters read
*
* \return The status of the operation
*
********************************************************************************** */
serialStatus_t Serial_Read( uint8_t InterfaceId, uint8_t *pData, uint16_t dataSize, uint16_t *bytesRead )
{
    serialStatus_t status = gSerial_Success_c;
#if (gSerialManagerMaxInterfaces_c)
    serial_t *pSer = &mSerials[InterfaceId];
    uint16_t i, bytes;

#if gSerialMgr_ParamValidation_d
    if ( (InterfaceId >= gSerialManagerMaxInterfaces_c) || (NULL == pData) || (0 == dataSize) )
    {
        status = gSerial_InvalidParameter_c;
    }
    else
#endif
    {
        Serial_RxBufferByteCount(InterfaceId, & bytes);


        if( bytes > 0 )
        {
            if( bytes > dataSize )
            {
                bytes = dataSize;
            }
#if gSerialMgrUseUart_c && (FSL_FEATURE_SOC_USART_COUNT>0)

            if (gSerialMgrUsart_c == pSer->serialType)
            {
                bool_t rx_full;

                serialRingState_t * rx_ring  = &mDrvData[InterfaceId].uartState.rx_ring;
                rx_full = (0 == rx_ring->space_left);

                Serial_RingConsume(rx_ring, pData, bytes);
                SERIAL_DBG_LOG("consumed bytes=%d", bytes);

                /* was full so enable Rx Interrupt again since room has been done */
                if (bytes > 0 && rx_full)
                {
                    USART_RxIntCtrl(InterfaceId, true);
                }
            }
            else
#endif
            {
                /* Copy data */
                for( i=0; i<bytes; i++ )
                {
                    OSA_InterruptDisable();
                    *pData++ = pSer->rxBuffer[pSer->rxOut];
                    mSerial_IncIdx_d(pSer->rxOut, gSMRxBufSize_c);
                    OSA_InterruptEnable();
                }

            }
            dataSize -= bytes;

            /* Additional processing depending on interface */
            switch ( pSer->serialType )
            {
#if gSerialMgrUseUSB_c
            case gSerialMgrUSB_c:
                VirtualCom_SMReadNotify( mDrvData[InterfaceId].pDrvData );
                break;
#endif

#if gSerialMgrUseUSB_VNIC_c
            case gSerialMgrUSB_VNIC_c:
                VirtualNic_SMReadNotify( mDrvData[InterfaceId].pDrvData );
                break;
#endif
            default:
                break;
            }
        }

        if( bytesRead )
        {
            *bytesRead = bytes;
        }
    }
#else
    (void)InterfaceId;
    (void)pData;
    (void)dataSize;
    bytesRead = 0;
    (void)bytesRead;
#endif
    return status;
}

/*! *********************************************************************************
* \brief   Returns a the number of bytes available in the RX buffer
*
* \param[in] InterfaceId the interface number
* \param[out] bytesCount the number of bytes available
*
* \return The status of the operation
*
********************************************************************************** */
serialStatus_t Serial_RxBufferByteCount( uint8_t InterfaceId, uint16_t *bytesCount )
{
    serialStatus_t status = gSerial_Success_c;
#if (gSerialManagerMaxInterfaces_c)
#if gSerialMgr_ParamValidation_d
    if ( (InterfaceId >= gSerialManagerMaxInterfaces_c) || (NULL == bytesCount) )
    {
        status = gSerial_InvalidParameter_c;
    }
    else
#endif
    {
        int delta;
        OSA_InterruptDisable();
#if FSL_FEATURE_SOC_USART_COUNT
        if (gSerialMgrUsart_c == mSerials[InterfaceId].serialType)
        {
            serialRingState_t * rx_ring  = &mDrvData[InterfaceId].uartState.rx_ring;
            delta = (rx_ring->max - rx_ring->space_left);
        }
        else
#endif
        {
            delta = mSerials[InterfaceId].rxIn - mSerials[InterfaceId].rxOut;
            if (delta < 0)
            {
                delta += gSMRxBufSize_c; /* since delta is negative ! */
            }
        }
        OSA_InterruptEnable();
        SERIAL_DBG_LOG("bytes=%d", delta);
        *bytesCount = (uint16_t)delta;
    }
#else
    (void)bytesCount;
    (void)InterfaceId;
#endif
    return status;
}

/*! *********************************************************************************
* \brief   Sets a pointer to a function that will be called when data is received
*
* \param[in] InterfaceId the interface number
* \param[in] pfCallBack pointer to the function to be called
* \param[in] pRxParam pointer to a parameter which will be passed to the CB function
*
* \return The status of the operation
*
********************************************************************************** */
serialStatus_t Serial_SetRxCallBack( uint8_t InterfaceId, pSerialCallBack_t cb, void *pRxParam )
{
    serialStatus_t status = gSerial_Success_c;
#if (gSerialManagerMaxInterfaces_c)
#if gSerialMgr_ParamValidation_d
    if ( InterfaceId >= gSerialManagerMaxInterfaces_c )
    {
        status = gSerial_InvalidParameter_c;
    }
    else
#endif
    {
        mSerials[InterfaceId].rxCallback = cb;
        mSerials[InterfaceId].pRxParam = pRxParam;
    }
#else
    (void)InterfaceId;
    (void)cb;
    (void)pRxParam;
#endif
    return status;
}

/*! *********************************************************************************
* \brief   Set the communication speed for an interface
*
* \param[in] baudRate communication speed
* \param[in] InterfaceId the interface number
*
* \return The status of the operation
*
********************************************************************************** */
serialStatus_t Serial_SetBaudRate( uint8_t InterfaceId, uint32_t baudRate  )
{
    serialStatus_t status = gSerial_Success_c;
#if gSerialManagerMaxInterfaces_c

#if gSerialMgr_ParamValidation_d
    if ( (InterfaceId >= gSerialManagerMaxInterfaces_c) || (0 == baudRate) )
    {
        status = gSerial_InvalidParameter_c;
    }
    else
#endif
    {
        switch ( mSerials[InterfaceId].serialType )
        {
#if (gSerialMgrUseUart_c)
#if FSL_FEATURE_SOC_UART_COUNT
        case gSerialMgrUart_c:
            UART_SetBaudrate(mSerials[InterfaceId].serialChannel, baudRate);
            break;
#endif
#if FSL_FEATURE_SOC_LPUART_COUNT
        case gSerialMgrLpuart_c:
            LPUART_SetBaudrate(mSerials[InterfaceId].serialChannel, baudRate);
            break;
#endif
#if FSL_FEATURE_SOC_LPSCI_COUNT
        case gSerialMgrLpsci_c:
            LPSCI_SetBaudrate(mSerials[InterfaceId].serialChannel, baudRate);
            break;
#endif
#if FSL_FEATURE_SOC_USART_COUNT
            case gSerialMgrUsart_c:
                USART_SetBaudrate(mSerials[InterfaceId].serialChannel, baudRate);
                break;
#endif
#endif /* #if (gSerialMgrUseUart_c) */

#if gSerialMgrUseIIC_c
        case gSerialMgrIICMaster_c:
            I2C_MasterSetSpeed(mSerials[InterfaceId].serialChannel, baudRate);
            break;
#endif

#if gSerialMgrUseSPI_c
        case gSerialMgrSPIMaster_c:
            gSpiConfig.bitsPerSec = baudRate;
            Spi_Configure(mSerials[InterfaceId].serialChannel, &gSpiConfig);
            break;
#endif

#if gSerialMgrUseUSB_c
        case gSerialMgrUSB_c:
            /* Nothing to do here. */
            break;
#endif

        case gSerialMgrCustom_c:
            /* Nothing to do here. */
            break;

        default:
            status = gSerial_InvalidInterface_c;
            break;
        }
    }
#endif
    return status;
}

/*! *********************************************************************************
* \brief   Prints a string to the serial interface
*
* \param[in] InterfaceId the interface number
* \param[in] pString pointer to the string to be printed
* \param[in] allowToBlock specify if the task will wait for the tx to finish or not.
*
* \return The status of the operation
*
********************************************************************************** */
serialStatus_t Serial_Print( uint8_t InterfaceId, const char* pString, serialBlock_t allowToBlock )
{
    serialStatus_t status = gSerial_Success_c;
#if gSerialManagerMaxInterfaces_c
    if ( allowToBlock )
    {
        status = Serial_SyncWrite( InterfaceId, (uint8_t*)pString, strlen(pString) );
    }
    else
    {
        status = Serial_AsyncWrite( InterfaceId, (uint8_t*)pString, strlen(pString), NULL, NULL );
    }
#else
    (void)pString;
    (void)allowToBlock;
    (void)InterfaceId;
#endif
    return status;
}

/*! *********************************************************************************
* \brief   Prints an number in hexadecimal format to the serial interface
*
* \param[in] InterfaceId the interface number
* \param[in] hex pointer to the number to be printed
* \param[in] len the number of bytes of the number
* \param[in] flags specify display options: comma, space, new line
*
* \return The status of the operation
*
* \remarks The task will wait until the tx has finished
*
********************************************************************************** */
serialStatus_t Serial_PrintHex( uint8_t InterfaceId,
                                const uint8_t *hex,
                                uint8_t len,
                                uint8_t flags )
{
    serialStatus_t status = gSerial_Success_c;
#if (gSerialManagerMaxInterfaces_c)
    uint8_t i=0;
    uint8_t hexString[6]; /* 2 bytes  - hexadecimal display
    1 byte   - separator ( comma)
    1 byte   - separator ( space)
    2 bytes  - new line (\n\r)  */

    if ( !(flags & gPrtHexBigEndian_c) )
    {
        hex = hex + (len-1);
    }

    while ( len )
    {
        /* start preparing the print of a new byte */
        i=0;
        hexString[i++] = HexToAscii( (*hex)>>4 );
        hexString[i++] = HexToAscii( *hex );

        if ( flags & gPrtHexCommas_c )
        {
            hexString[i++] = ',';
        }
        if ( flags & gPrtHexSpaces_c )
        {
            hexString[i++] = ' ';
        }
        hex = hex + (flags & gPrtHexBigEndian_c ? 1 : -1);
        len--;

        if ( (len == 0) && (flags & gPrtHexNewLine_c) )
        {
            hexString[i++] = '\n';
            hexString[i++] = '\r';
        }

        /* transmit formatted byte */
        status = Serial_SyncWrite( InterfaceId, hexString, (uint8_t)i) ;
        if ( gSerial_Success_c != status )
        {
            break;
        }
    }
#else
    /* Avoid compiler warning */
    (void)hex;
    (void)len;
    (void)InterfaceId;
    (void)flags;
#endif
    return status;
}

/*! *********************************************************************************
* \brief   Prints an unsigned integer to the serial interface
*
* \param[in] InterfaceId the interface number
* \param[in] nr the number to be printed
*
* \return The status of the operation
*
* \remarks The task will wait until the tx has finished
*
********************************************************************************** */
serialStatus_t Serial_PrintDec( uint8_t InterfaceId, uint32_t nr )
{
#if (gSerialManagerMaxInterfaces_c)
    uint8_t decString[12];
    uint8_t i = sizeof(decString)-1;

    if ( nr == 0 )
    {
        decString[i] = '0';
    }
    else
    {
        while ( nr )
        {
            decString[i] = '0' + (uint8_t)(nr % 10);
            nr = nr / 10;
            i--;
        }
        i++;
    }

    /* transmit formatted byte */
    return Serial_SyncWrite( InterfaceId, (uint8_t*)&decString[i], sizeof(decString)-i );
#else
    (void)nr;
    (void)InterfaceId;
    return gSerial_Success_c;
#endif
}


/*! *********************************************************************************
* \brief   Configures the enabled hardware modules of the given interface type as a wakeup source from STOP mode
*
* \param[in] interface type of the modules to configure
*
* \return  gSerial_Success_c if there is at least one module to configure
*          gSerial_InvalidInterface_c otherwise
* \pre
*
* \post
*
* \remarks
*
********************************************************************************** */

serialStatus_t Serial_EnableLowPowerWakeup( serialInterfaceType_t interfaceType )
{
    serialStatus_t status = gSerial_Success_c;
#if gSerialManagerMaxInterfaces_c
    uint32_t i;

    for(i=0; i<gSerialManagerMaxInterfaces_c; i++)
    {
        if( mSerials[i].serialType == interfaceType )
        {
            switch(interfaceType)
            {
#if (gSerialMgrUseUart_c)
#if FSL_FEATURE_SOC_UART_COUNT
            case gSerialMgrUart_c:
                UART_EnableLowPowerWakeup(mSerials[i].serialChannel);
                break;
#endif
#if FSL_FEATURE_SOC_LPUART_COUNT
            case gSerialMgrLpuart_c:
                LPUART_EnableLowPowerWakeup(mSerials[i].serialChannel);
                break;
#endif
#if FSL_FEATURE_SOC_LPSCI_COUNT
            case gSerialMgrLpsci_c:
                LPSCI_EnableLowPowerWakeup(mSerials[i].serialChannel);
                break;
#endif
#if FSL_FEATURE_SOC_USART_COUNT
                case gSerialMgrUsart_c:
                    USART_EnableLowPowerWakeup(mSerials[i].serialChannel);
                    break;
#endif
#endif /* #if (gSerialMgrUseUart_c) */
            default:
                status = gSerial_InvalidInterface_c;
                break;
            } /* switch(...) */
        } /* if(...) */
    }/* for(...) */
#endif /* #if gSerialManagerMaxInterfaces_c */
    return status;
}

/*! *********************************************************************************
* \brief   Configures the enabled hardware modules of the given interface type as modules without wakeup capabilities
*
* \param[in] interface type of the modules to configure
*
* \return  gSerial_Success_c if there is at least one module to configure
*          gSerial_InvalidInterface_c otherwise
* \pre
*
* \post
*
* \remarks
*
********************************************************************************** */
serialStatus_t Serial_DisableLowPowerWakeup( serialInterfaceType_t interfaceType )
{
    serialStatus_t status = gSerial_Success_c;
#if gSerialManagerMaxInterfaces_c
    uint32_t i;

    for(i=0; i<gSerialManagerMaxInterfaces_c; i++)
    {
        if( mSerials[i].serialType == interfaceType )
        {
            switch(interfaceType)
            {
#if (gSerialMgrUseUart_c)
#if FSL_FEATURE_SOC_UART_COUNT
            case gSerialMgrUart_c:
                UART_DisableLowPowerWakeup(mSerials[i].serialChannel);
                break;
#endif
#if FSL_FEATURE_SOC_LPUART_COUNT
            case gSerialMgrLpuart_c:
                LPUART_DisableLowPowerWakeup(mSerials[i].serialChannel);
                break;
#endif
#if FSL_FEATURE_SOC_LPSCI_COUNT
            case gSerialMgrLpsci_c:
                LPSCI_DisableLowPowerWakeup(mSerials[i].serialChannel);
                break;
#endif
#if FSL_FEATURE_SOC_USART_COUNT
            case gSerialMgrUsart_c:
                USART_DisableLowPowerWakeup(mSerials[i].serialChannel);
                break;
#endif
#endif /* #if (gSerialMgrUseUart_c) */
            default:
                status = gSerial_InvalidInterface_c;
                break;
            }
        }
    }
#endif /* #if gSerialManagerMaxInterfaces_c */
    return status;
}

/*! *********************************************************************************
* \brief   Decides whether a enabled hardware module of the given interface type woke up the CPU from STOP mode.
*
* \param[in] interface type of the modules to be evaluated as wakeup source.
*
* \return  TRUE if a module of the given interface type was the wakeup source
*          FALSE otherwise
* \pre
*
* \post
*
* \remarks
*
********************************************************************************** */
bool_t Serial_IsWakeUpSource( serialInterfaceType_t interfaceType)
{
    bool_t status = FALSE;
#if gSerialManagerMaxInterfaces_c
    uint32_t i;

    for(i=0; i<gSerialManagerMaxInterfaces_c; i++)
    {
        if( mSerials[i].serialType == interfaceType )
        {
            switch(interfaceType)
            {
#if (gSerialMgrUseUart_c)
#if FSL_FEATURE_SOC_UART_COUNT
            case gSerialMgrUart_c:
                if( UART_IsWakeupSource(mSerials[i].serialChannel) )
                {
                    status = TRUE;
                }
                break;
#endif
#if FSL_FEATURE_SOC_LPUART_COUNT
            case gSerialMgrLpuart_c:
                if( LPUART_IsWakeupSource(mSerials[i].serialChannel) )
                {
                    status = TRUE;
                }
                break;
#endif
#if FSL_FEATURE_SOC_LPSCI_COUNT
            case gSerialMgrLpsci_c:
                if (LPSCI_IsWakeupSource(mSerials[i].serialChannel))
                {
                    status = TRUE;
                }
                break;
#endif
#if FSL_FEATURE_SOC_USART_COUNT
            case gSerialMgrUsart_c:
                if (USART_IsWakeupSource(mSerials[i].serialChannel))
                {
                    status = TRUE;
                }
                break;
#endif
#endif /* #if (gSerialMgrUseUart_c) */
            default:
                break;
            }
        }
    }/* for(...) */
#else
    (void)interfaceType;
#endif
    return status;
}


/*! *********************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
********************************************************************************* */
#if (gSerialManagerMaxInterfaces_c)
/*! *********************************************************************************
* \brief Transmit a data buffer to the specified interface.
*
* \param[in] InterfaceId the interface number
*
* \return The status of the operation
*
********************************************************************************** */
static serialStatus_t Serial_WriteInternal( uint8_t InterfaceId )
{
    serialStatus_t status = gSerial_Success_c;
    serial_t *pSer = &mSerials[InterfaceId];
    uint16_t idx;

    OSA_InterruptDisable();
    if( pSer->state == 0 )
    {
        pSer->state = 1;
#if gSerialMgr_DisallowMcuSleep_d
        PWR_DisallowDeviceToSleep();
#endif
    }
    else
    {
        /* The interface is busy transmitting!
         * The current data will be transmitted after the previous transmissions end.
         */
        OSA_InterruptEnable();
        return gSerial_Success_c;
    }

    idx = pSer->txCurrent;
    if(pSer->txQueue[idx].dataSize == 0)
    {
#if gSerialMgr_DisallowMcuSleep_d
        PWR_AllowDeviceToSleep();
#endif
        pSer->state = 0;
        OSA_InterruptEnable();
        return gSerial_Success_c;
    }

    switch ( mSerials[InterfaceId].serialType )
    {
#if (gSerialMgrUseUart_c)
#if FSL_FEATURE_SOC_UART_COUNT
    case gSerialMgrUart_c:
        if( UART_SendData( pSer->serialChannel, pSer->txQueue[idx].pData, pSer->txQueue[idx].dataSize ) )
        {
            status = gSerial_InternalError_c;
        }
        break;
#endif
#if FSL_FEATURE_SOC_LPUART_COUNT
    case gSerialMgrLpuart_c:
        if( LPUART_SendData(pSer->serialChannel, pSer->txQueue[idx].pData, pSer->txQueue[idx].dataSize) )
        {
            status = gSerial_InternalError_c;
        }
        break;
#endif
#if FSL_FEATURE_SOC_LPSCI_COUNT
    case gSerialMgrLpsci_c:
        if( LPSCI_SendData(pSer->serialChannel, pSer->txQueue[idx].pData, pSer->txQueue[idx].dataSize ) )
        {
            status = gSerial_InternalError_c;
        }
        break;
#endif
#if FSL_FEATURE_SOC_USART_COUNT
    case gSerialMgrUsart_c:
        if (USART_SendData(pSer->serialChannel, pSer->txQueue[idx].pData, pSer->txQueue[idx].dataSize))
        {
            status = gSerial_InternalError_c;
        }
        break;
#endif
#endif /* #if (gSerialMgrUseUart_c) */

#if gSerialMgrUseUSB_c
    case gSerialMgrUSB_c:
        if( VirtualCom_Write(mDrvData[InterfaceId].pDrvData,
                             pSer->txQueue[idx].pData,
                             pSer->txQueue[idx].dataSize) )
        {
            status = gSerial_InternalError_c;
        }
        break;
#endif
#if gSerialMgrUseUSB_VNIC_c
    case gSerialMgrUSB_VNIC_c:
        if( VirtualNic_Write(mDrvData[InterfaceId].pDrvData,
                             pSer->txQueue[idx].pData,
                             pSer->txQueue[idx].dataSize) )
        {
            status = gSerial_InternalError_c;
        }
        break;
#endif

#if gSerialMgrUseIIC_c
    case gSerialMgrIICMaster_c:
        if( kStatus_Success != I2C_MasterSend( pSer->serialChannel,
                                               pSer->txQueue[idx].pData,
                                               pSer->txQueue[idx].dataSize ) )
        {
            status = gSerial_InternalError_c;
        }
        break;

    case gSerialMgrIICSlave_c:
        /* Notify IIC Master that we have data to send */
        if( kStatus_Success != I2C_SlaveSend(pSer->serialChannel,
                                             pSer->txQueue[idx].pData,
                                             pSer->txQueue[idx].dataSize) )
        {
            status = gSerial_InternalError_c;
            break;
        }
#if gSerialMgrSlaveDapTxLogicOne_c
        GpioSetPinOutput(&mI2cSlaveDapCfg);
#else
        GpioClearPinOutput(&mI2cSlaveDapCfg);
#endif
        break;
#endif

#if gSerialMgrUseSPI_c
    case gSerialMgrSPISlave_c:
        /* Notify SPI Master that we have data to send */
        if( (bool)Spi_AsyncTransferSlave(pSer->serialChannel,
                                   pSer->txQueue[idx].pData,
                                   &pSer->rxBuffer[pSer->rxIn],
                                   pSer->txQueue[idx].dataSize) )
        {
            status = gSerial_InternalError_c;
            break;
        }

#if gSerialMgrSlaveDapTxLogicOne_c
        GpioSetPinOutput(&mSpiSlaveDapCfg);
#else
        GpioClearPinOutput(&mSpiSlaveDapCfg);
#endif
#if gNvStorageIncluded_d
        NvSetCriticalSection();
#endif
        break;

    case gSerialMgrSPIMaster_c:
        if( (bool)Spi_AsyncTransfer(pSer->serialChannel,
                                    pSer->txQueue[idx].pData,
                                    &pSer->rxBuffer[pSer->rxIn],
                                    pSer->txQueue[idx].dataSize) )
        {
            status = gSerial_InternalError_c;
        }
        break;
#endif

#if gSerialMgrUseCustomInterface_c
    case gSerialMgrCustom_c:
#if ENABLE_UART_SERIAL_IF
        OSA_InterruptEnable();
#endif
        /* The Serial_CustomSendData() function must be implemented by the custom protocol. */
        if( Serial_CustomSendData(pSer->txQueue[idx].pData, pSer->txQueue[idx].dataSize) )
        {
            status = gSerial_InternalError_c;
        }
#if ENABLE_UART_SERIAL_IF
        OSA_InterruptDisable();
#endif
        break;
#endif

    default:
        status = gSerial_InternalError_c;
        break;
    }

    OSA_InterruptEnable();

    if( status != gSerial_Success_c )
    {
#if gSerialMgr_DisallowMcuSleep_d
        PWR_AllowDeviceToSleep();
#endif
//        pSer->txQueue[idx].dataSize = 0;
//        pSer->txQueue[idx].txCallback = NULL;
//        mSerial_IncIdx_d(pSer->txCurrent, gSerialMgrTxQueueSize_c)
        pSer->state = 0;
#if USE_SDK_OSA
        (void)OSA_EventSet(mSMTaskEventId, gSMGR_TxNew_c);
#endif
    }

    return status;
}

/*! *********************************************************************************
* \brief Inform the Serial Manager task that new data is available
*
* \param[in] pData The id interface
*
* \return none
*
* \remarks Called from usb task
*
********************************************************************************** */
#if gSerialMgrUseUSB_c
void SerialManager_VirtualComRxNotify(uint8_t* pData, uint16_t dataSize, uint8_t interface)
{

    while(dataSize)
    {
        OSA_InterruptDisable();
        mSerials[interface].rxBuffer[mSerials[interface].rxIn] = *pData++;
        mSerial_IncIdx_d(mSerials[interface].rxIn, gSMRxBufSize_c);
        if(mSerials[interface].rxIn == mSerials[interface].rxOut)
        {
            mSerial_IncIdx_d(mSerials[interface].rxOut, gSMRxBufSize_c);
        }
        OSA_InterruptEnable();
        dataSize--;
    }

    mSerials[interface].events |= gSMGR_Rx_c;
#if USE_SDK_OSA
    (void)OSA_EventSet(mSMTaskEventId, gSMGR_Rx_c);
#endif
}
#endif

/*! *********************************************************************************
* \brief Inform the Serial Manager task that new data is available
*
* \param[in] pData The id interface
*
* \return none
*
* \remarks Called from USB task
*
********************************************************************************** */
#if gSerialMgrUseUSB_VNIC_c
uint16_t SerialManager_VirtualNicRxNotify(uint8_t* pData, uint16_t dataSize, uint8_t interface)
{
  bufIndex_t inIndex;
  uint16_t charReceived = 0;
  inIndex = mSerials[interface].rxIn;
  mSerial_IncIdx_d(inIndex, gSMRxBufSize_c);
  while(dataSize && (inIndex != mSerials[interface].rxOut))
  {
    //OSA_InterruptDisable();
    mSerials[interface].rxBuffer[mSerials[interface].rxIn] = *pData++;
    mSerials[interface].rxIn = inIndex;
    charReceived++;
    mSerial_IncIdx_d(inIndex, gSMRxBufSize_c);
    //OSA_InterruptEnable();
    dataSize--;
  }
  if(charReceived)
  {
    mSerials[interface].events |= gSMGR_Rx_c;
#if USE_SDK_OSA
    (void)OSA_EventSet(mSMTaskEventId, gSMGR_Rx_c);
#endif
  }
  return charReceived;
}
#endif

/*! *********************************************************************************
* \brief Inform the Serial Manager task that new data is available
*
* \param[in] pData The id interface
*
* \return none
*
* \remarks Called from ISR
*
********************************************************************************** */
void SerialManager_RxNotify( uint32_t i )
{
    serial_t *pSer = &mSerials[i];

#if gSerialMgrUseFSCIHdr_c
    uint8_t rxByte = pSer->rxBuffer[pSer->rxIn];
    uint8_t slaveDapRxEnd = 0;
#endif

    mSerial_IncIdx_d(pSer->rxIn, gSMRxBufSize_c);

    /* if rxIn producer index caught up with rxOut it means that the buffer is now full */
    if(pSer->rxIn == pSer->rxOut)
    {
        mSerial_IncIdx_d(pSer->rxOut, gSMRxBufSize_c);
    }

    switch( pSer->serialType )
    {
        /* Uart driver is in continuous Rx. No need to restart reception. */
#if gSerialMgrUseSPI_c
    case gSerialMgrSPISlave_c:
        /* No need to restart RX since SPI is in continuous RX mode */
        break;
    case gSerialMgrSPIMaster_c:
#if gSerialMgrUseFSCIHdr_c
        if( (0 == pSer->rxFsciPkt) && (gFSCI_StartMarker_c == rxByte) )
        {
            pSer->rxFsciPkt = 1;
        }

        if( pSer->rxFsciPkt )
        {
            pSer->rxFsciIn++;

#if gFsciLenHas2Bytes_c
            if( (mSMGR_FSCIHdrLen_c - 1) == pSer->rxFsciIn )
            {
                pSer->rxFsciLen = rxByte + 1; /* CRC */
            }
            else if( mSMGR_FSCIHdrLen_c == pSer->rxFsciIn )
            {
                pSer->rxFsciLen += rxByte << 8;
            }
#else
            if( mSMGR_FSCIHdrLen_c == pSer->rxFsciIn )
            {
                pSer->rxFsciLen = rxByte + 1; /* CRC */
            }
#endif
            if( pSer->rxFsciLen == (pSer->rxFsciIn - mSMGR_FSCIHdrLen_c) )
            {
                pSer->rxFsciPkt = 0;
                pSer->rxFsciIn = 0;
                pSer->rxFsciLen = 0;

#if gSerialMgrSlaveDapTxLogicOne_c
                slaveDapRxEnd = GpioReadPinInput(&mSpiMasterDapCfg);
#else
                slaveDapRxEnd = !GpioReadPinInput(&mSpiMasterDapCfg);
#endif
            }
        }

        /* If more bytes need to be received */
        if( (pSer->rxFsciPkt || slaveDapRxEnd) && !pSer->state )
        {
#if gSMGR_UseOsSemForSynchronization_c
            if( (0 == pSer->txQueue[pSer->txIn].dataSize) && (NULL == pSer->txQueue[pSer->txIn].txCallback) )
#endif
            {
                Serial_SpiSendDummyByte(i);
            }
        }
#else /* gSerialMgrUseFSCIHdr_c */
        /* If the SPI Slave has more data to transmit, restart the transfer */
#if gSerialMgrSlaveDapTxLogicOne_c
        if( (GpioReadPinInput(&mSpiMasterDapCfg)) && (0 == pSer->state) )
#else
        if( (!GpioReadPinInput(&mSpiMasterDapCfg)) && (0 == pSer->state) )
#endif
        {
#if gSMGR_UseOsSemForSynchronization_c
            if( (0 == pSer->txQueue[pSer->txIn].dataSize) && (NULL == pSer->txQueue[pSer->txIn].txCallback) )
#endif
            {
                Serial_SpiSendDummyByte(i);
            }
        }
#endif
        break;
#endif

#if gSerialMgrUseIIC_c
    case gSerialMgrIICSlave_c:
        break;

    case gSerialMgrIICMaster_c:
#if gSerialMgrSlaveDapTxLogicOne_c
        if( GpioReadPinInput(&mI2cMasterDapCfg) )
#else
        if( !GpioReadPinInput(&mI2cMasterDapCfg) )
#endif
        {
            I2C_MasterReceive( pSer->serialChannel, &mSerials[i].rxBuffer[mSerials[i].rxIn], 1 );
        }
        break;
#endif
    default:
        break;
    }

    /* Signal SMGR task if not already done */
    if( !pSer->events )
    {
        pSer->events |= gSMGR_Rx_c;
#if USE_SDK_OSA
        (void)OSA_EventSet(mSMTaskEventId, gSMGR_Rx_c);
#endif
    }
    else
    {
        pSer->events |= gSMGR_Rx_c;
    }
}

/*! *********************************************************************************
* \brief Inform the Serial Manager task that a transmission has finished
*
* \param[in] pData the Id interface
*
* \return none
*
* \remarks Called from ISR
*
********************************************************************************** */
void SerialManager_TxNotify( uint32_t i )
{
    serial_t *pSer = &mSerials[i];

    OSA_InterruptDisable();
#if (gSerialMgrUseSPI_c)
    if( 2 != pSer->state )
#endif
    {
    pSer->txQueue[pSer->txCurrent].dataSize = 0; /* Mark as transmitted */
    mSerial_IncIdx_d(pSer->txCurrent, gSerialMgrTxQueueSize_c)
    }
#if gSerialMgr_DisallowMcuSleep_d
    PWR_AllowDeviceToSleep();
#endif
    pSer->state = 0;
    OSA_InterruptEnable();

    /* Transmit next block if available */
      if( pSer->txQueue[pSer->txCurrent].dataSize)
    {
       if( (pSer->serialType != gSerialMgrIICMaster_c) && (pSer->serialType != gSerialMgrIICSlave_c) )
        {
#if gNvStorageIncluded_d
            if (pSer->serialType == gSerialMgrSPISlave_c)
             {
                NvClearCriticalSection();
            }
#endif
            (void)Serial_WriteInternal(i);
        }
    }
    else
    {
        switch(pSer->serialType)
        {
#if (gSerialMgrUseIIC_c)
        case gSerialMgrIICMaster_c:
#if gSerialMgrSlaveDapTxLogicOne_c
            if( GpioReadPinInput(&mI2cMasterDapCfg) )
#else
            if( !GpioReadPinInput(&mI2cMasterDapCfg) )
#endif
            {
                I2C_MasterReceive( pSer->serialChannel, &mSerials[i].rxBuffer[mSerials[i].rxIn], 1 );
            }
            break;

        case gSerialMgrIICSlave_c:
#if gSerialMgrSlaveDapTxLogicOne_c
            GpioClearPinOutput(&mI2cSlaveDapCfg);
#else
            GpioSetPinOutput(&mI2cSlaveDapCfg);
#endif
            break;
#endif

#if (gSerialMgrUseSPI_c)
        case gSerialMgrSPISlave_c:
#if gSerialMgrSlaveDapTxLogicOne_c
            GpioClearPinOutput(&mSpiSlaveDapCfg);
#else
            GpioSetPinOutput(&mSpiSlaveDapCfg);
#endif

#if gNvStorageIncluded_d
            NvClearCriticalSection();
#endif
            break;
#endif
        default:
            break;
        }
    }

    /* Signal SMGR task if not already done */
    if( (pSer->events & gSMGR_TxDone_c) == 0)
    {
    pSer->events |= gSMGR_TxDone_c;
#if USE_SDK_OSA
    (void)OSA_EventSet(mSMTaskEventId, gSMGR_TxDone_c);
#endif
    }
}


/*! *********************************************************************************
* \brief   This function will mark all finished TX queue entries as empty.
*          If a callback was provided, it will be run.
*
* \param[in] pSer pointer to the serial interface internal structure
*
********************************************************************************** */
static void Serial_TxQueueMaintenance(serial_t *pSer)
{
    uint32_t i;

    OSA_InterruptDisable();
    if( FALSE == pSer->txMntOngoing )
    {
    	pSer->txMntOngoing = TRUE;
        OSA_InterruptEnable();

        while( pSer->txQueue[pSer->txOut].dataSize == 0 )
        {
            i = pSer->txOut;
            if(pSer->txNo == 0)
            {
              break;
            }
            OSA_InterruptDisable();
            pSer->txNo--;
            OSA_InterruptEnable();
            mSerial_IncIdx_d(pSer->txOut, gSerialMgrTxQueueSize_c)

            /* Run Callback */
            if( pSer->txQueue[i].txCallback )
            {
                pSer->txQueue[i].txCallback( pSer->txQueue[i].pTxParam );
                pSer->txQueue[i].txCallback = NULL;
            }

#if gSerialMgr_BlockSenderOnQueueFull_c && gSMGR_UseOsSemForSynchronization_c
            OSA_InterruptDisable();
            if( pSer->txBlockedTasks )
            {
                pSer->txBlockedTasks--;
                OSA_InterruptEnable();
                (void)OSA_SemaphorePost(pSer->txQueueSemId);
            }
            else
            {
              OSA_InterruptEnable();
            }
#endif
            if( pSer->txOut == pSer->txIn )
            {
                break;
            }
/*
#if defined(FWK_SMALL_RAM_CONFIG)
            if( pSer->txQueue[pSer->txOut].dataSize )
                (void)OSA_EventSet(mSMTaskEventId, gSMGR_TxDone_c);
            return;
#endif
*/
        }

        pSer->txMntOngoing = FALSE;
    }
    else
    {
        OSA_InterruptEnable();
    }
}

/*! *********************************************************************************
* \brief   This function will unblock the task who called Serial_SyncWrite().
*
* \param[in] pSer pointer to the serial interface internal structure
*
********************************************************************************** */
#if gSMGR_UseOsSemForSynchronization_c
static void Serial_SyncTxCallback(void *pSer)
{
    (void)OSA_SemaphorePost(((serial_t *)pSer)->txSyncSemId );
}
#endif

/*! *********************************************************************************
* \brief   This function will return the interfaceId for the specified interface
*
* \param[in] type     the interface type
* \param[in] channel  the instance of the interface
*
* \return The mSerials index for the specified interface type and channel
*
********************************************************************************** */
uint32_t Serial_GetInterfaceId(serialInterfaceType_t type, uint32_t channel)
{
    uint32_t i, id = gSerialMgrInvalidIdx_c;

    for(i=0; i<gSerialManagerMaxInterfaces_c; i++)
    {
        if( (mSerials[i].serialType == type) && (mSerials[i].serialChannel == channel) )
        {
            id = i;
            break;
        }
    }

    return id;
}

/*! *********************************************************************************
* \brief   This function will return the first interfaceId for the specified interface type
*
* \param[in] type     the interface type
*
* \return The mSerials index for the specified interface type
*
*
********************************************************************************** */
#if (gSerialMgrUseSPI_c) || (gSerialMgrUseIIC_c)
static uint32_t Serial_GetInterfaceIdFromType(serialInterfaceType_t type)
{
    uint32_t i, id = gSerialMgrInvalidIdx_c;

    for(i=0; i<gSerialManagerMaxInterfaces_c; i++)
    {
        if( mSerials[i].serialType == type )
        {
            id = i;
            break;
        }
    }

    return id;
}
#endif

/*! *********************************************************************************
* \brief   SPI transfer complete ISR callback
*
* \param[in] instance     the instance of the SPI module
*
********************************************************************************** */
#if (gSerialMgrUseSPI_c)
static void SpiCallback(uint32_t flags, spiState_t* pState)
{
    uint32_t    instance = (uint32_t)pState->callbackParam;
    serial_t   *pSer = &mSerials[instance];

    if( flags & gSPI_TxEndFlag_d )
    {
        /* SPI Tx sequence end */
        SerialManager_TxNotify(instance);
    }

    if( flags & (gSPI_RxEndFlag_d | gSPI_ByteRxFlag_d) )
    {
        /* SPI Rx sequence end OR new byte received */
        SerialManager_RxNotify(instance);
    }

    if( flags & gSPI_ByteRxFlag_d )
    {
        /* Update data pointer for next SPI Rx*/
        pState->pRxData = &pSer->rxBuffer[pSer->rxIn];
    }
}
#endif

/*! *********************************************************************************
* \brief   SPI Master data available pin ISR
*
********************************************************************************** */
#if (gSerialMgrUseSPI_c)
static void SpiMasterDapISR(void)
{
    uint32_t i = Serial_GetInterfaceIdFromType(gSerialMgrSPIMaster_c);

    if( i == gSerialMgrInvalidIdx_c )
    {
        panic(0,0,0,0);
    }
    else if( GpioIsPinIntPending(&mSpiMasterDapCfg) )
    {
#if gSerialMgrSlaveDapTxLogicOne_c
        if( GpioReadPinInput(&mSpiMasterDapCfg) )
#else
        if( !GpioReadPinInput(&mSpiMasterDapCfg) )
#endif
        {
            /* Change IRQ logic to detect when SPI Slave has no more data to send */
#if gSerialMgrSlaveDapTxLogicOne_c
            mSpiMasterDapCfg.interruptSelect = pinInt_LogicZero_c;
#else
            mSpiMasterDapCfg.interruptSelect = pinInt_LogicOne_c;
#endif

            /* SPI Master will start a dummy transfer to receive data from SPI Slave */
                Serial_SpiSendDummyByte(i);
        }
        else
        {
            /* Change IRQ logic to detect when SPI Slave has new data to send */
#if gSerialMgrSlaveDapTxLogicOne_c
            mSpiMasterDapCfg.interruptSelect = pinInt_LogicOne_c;
#else
            mSpiMasterDapCfg.interruptSelect = pinInt_LogicZero_c;
#endif
        }

        GpioInputPinInit(&mSpiMasterDapCfg, 1);
        GpioClearPinIntFlag(&mSpiMasterDapCfg);
    }
}
#endif

/*! *********************************************************************************
* \brief   I2C Rx ISR callback.
*
* \param[in] state     pointer to the I2C state structure
*
********************************************************************************** */
#if (gSerialMgrUseIIC_c)
static void Serial_I2cRxCb(i2cState_t* state)
{
    uint32_t i = state->rxCbParam;

    SerialManager_RxNotify(i);
    /* Update rxBuff because rxIn was incremented by the RxNotify function */
    state->pRxData = &mSerials[i].rxBuffer[mSerials[i].rxIn];
}

/*! *********************************************************************************
* \brief   I2C Tx ISR callback.
*
* \param[in] state     pointer to the I2C state structure
*
********************************************************************************** */
static void Serial_I2cTxCb(i2cState_t* state)
{
    SerialManager_TxNotify(state->txCbParam);
}
#endif

/*! *********************************************************************************
* \brief   I2C Master data available pin ISR
*
********************************************************************************** */
#if (gSerialMgrUseIIC_c)
static void I2cMasterDapISR(void)
{
    uint32_t i = Serial_GetInterfaceIdFromType(gSerialMgrIICMaster_c);

    if( i == gSerialMgrInvalidIdx_c )
    {
        panic(0,0,0,0);
    }
    else if( GpioIsPinIntPending(&mI2cMasterDapCfg) )
    {
#if gSerialMgrSlaveDapTxLogicOne_c
        if( GpioReadPinInput(&mI2cMasterDapCfg) )
#else
        if( !GpioReadPinInput(&mI2cMasterDapCfg) )
#endif
        {
#if gSerialMgrSlaveDapTxLogicOne_c
            mI2cMasterDapCfg.interruptSelect = pinInt_LogicZero_c;
#else
            mI2cMasterDapCfg.interruptSelect = pinInt_LogicOne_c;
#endif

            I2C_MasterReceive(mSerials[i].serialChannel, &mSerials[i].rxBuffer[mSerials[i].rxIn], 1 );
        }
        else
        {
#if gSerialMgrSlaveDapTxLogicOne_c
            mI2cMasterDapCfg.interruptSelect = pinInt_LogicOne_c;
#else
            mI2cMasterDapCfg.interruptSelect = pinInt_LogicZero_c;
#endif
        }

        GpioInputPinInit(&mI2cMasterDapCfg, 1);
        GpioClearPinIntFlag(&mI2cMasterDapCfg);
    }
}
#endif /* #if (gSerialMgrUseIIC_c) */

#if (gSerialMgrUseUart_c)
/*! *********************************************************************************
* \brief   UART Rx ISR callback.
*
* \param[in] state     pointer to the UART state structure
*
********************************************************************************** */
static void Serial_UartRxCb(uartState_t* state)
{
    uint32_t i = state->rxCbParam;

    SerialManager_RxNotify(i);
    /* Update rxBuff because rxIn was incremented by the RxNotify function */
//    state->pRxData = &mSerials[i].rxBuffer[mSerials[i].rxIn];
}

/*! *********************************************************************************
* \brief   UART Tx ISR callback.
*
* \param[in] state     pointer to the UART state structure
*
********************************************************************************** */
static void Serial_UartTxCb(uartState_t* state)
{
    SerialManager_TxNotify(state->txCbParam);
}
#endif /* #if (gSerialMgrUseUart_c) */

#if gSerialMgrUseCustomInterface_c
/*! *********************************************************************************
* \brief   This function is used for a custom interface to notify the SerialManager
*          that the data transfer has ended
*
* \param[in] InterfaceId the interface number
*
********************************************************************************** */
void Serial_CustomSendCompleted(uint32_t InterfaceId)
{
    SerialManager_TxNotify(InterfaceId);
}

/*! *********************************************************************************
* \brief   This function is used for a custom interface to notify the SerialManager
*          that the data transfer has ended
*
* \param[in] InterfaceId - The interface number
* \param[in] pRxData     - Pointer to the received bytes
* \param[in] size        - Number of bytes received
*
* \return The number of bytes that have not been stored.
*
********************************************************************************** */
uint32_t Serial_CustomReceiveData(uint8_t InterfaceId, uint8_t *pRxData, uint32_t size)
{
    serial_t *pSer = &mSerials[InterfaceId];

    while(size--)
    {
        OSA_InterruptDisable();
        pSer->rxBuffer[pSer->rxIn] = *pRxData++;
        mSerial_IncIdx_d(pSer->rxIn, gSMRxBufSize_c);
        /* Check for overflow */
        if(pSer->rxIn == pSer->rxOut)
        {
            mSerial_DecIdx_d(pSer->rxIn, gSMRxBufSize_c);
            OSA_InterruptEnable();
            size++;
            break;
        }
        OSA_InterruptEnable();
    }

    /* Signal SMGR task if not already done */
    pSer->events |= gSMGR_Rx_c;
#if USE_SDK_OSA
    (void)OSA_EventSet(mSMTaskEventId, gSMGR_Rx_c);
#endif
    return size;
}
#endif

#if (gSerialMgrUseSPI_c)
static void Serial_SpiSendDummyByte(uint32_t i)
{
  bool_t proceed = FALSE;
    OSA_InterruptDisable();
    if( mSerials[i].state == 0 )
    {
        mSerials[i].state = 2;
        proceed = TRUE;
#if gSerialMgr_DisallowMcuSleep_d
        PWR_DisallowDeviceToSleep();
#endif
    }
    OSA_InterruptEnable();
    if(proceed)
    {
        Spi_SendDummyByte(mSerials[i].serialChannel, &mSerials[i].rxBuffer[mSerials[i].rxIn]);
    }
}
#endif
#endif /* #if (gSerialManagerMaxInterfaces_c) */

void SerialInterface_Reinit(uint8_t i)
{
#if (gSerialManagerMaxInterfaces_c)
    if (i < gSerialManagerMaxInterfaces_c)
    {
        serial_t *pSer = &mSerials[i];
        uint8_t instance = pSer->serialChannel;

        switch ( pSer->serialType )
        {
        case gSerialMgrUart_c:
#if gSerialMgrUseUart_c && FSL_FEATURE_SOC_UART_COUNT
            UART_Initialize(instance, &mDrvData[i].uartState);
#endif
            break;

        case gSerialMgrLpuart_c:
#if gSerialMgrUseUart_c && FSL_FEATURE_SOC_LPUART_COUNT
            LPUART_ReInit(instance);
#endif
            break;

        case gSerialMgrLpsci_c:
#if gSerialMgrUseUart_c && FSL_FEATURE_SOC_LPSCI_COUNT
            LPSCI_Initialize(instance, &mDrvData[i].uartState);
#endif
            break;

        case gSerialMgrIICMaster_c:
        case gSerialMgrIICSlave_c:
#if gSerialMgrUseIIC_c
            I2C_SaveRestoreConfig(instance, FALSE);
#endif
            break;

        case gSerialMgrSPIMaster_c:
        case gSerialMgrSPISlave_c:
#if gSerialMgrUseSPI_c
            Spi_Init(instance, &(mDrvData[i].spiState), SpiCallback, (void*)i );
            Spi_Configure(instance, &gSpiConfig);
#endif
            break;

        case gSerialMgrUsart_c:
#if gSerialMgrUseUart_c && FSL_FEATURE_SOC_USART_COUNT
#if defined (CPU_JN518X)
            if (instance == 0)
                RESET_PeripheralReset(kUSART0_RST_SHIFT_RSTn);
            else if (instance == 1)
                RESET_PeripheralReset(kUSART1_RST_SHIFT_RSTn);
#endif
            USART_Initialize(instance, &mDrvData[i].uartState);
#endif
            break;

        default:
            break;
        }
    }
#endif /* #if (gSerialManagerMaxInterfaces_c) */
}
