#include <string.h>
#include <stdlib.h>
#include "CanUsb.h"

#define DETECT_PACKET_START             0
#define DETECT_FRAME_INFO               1
#define DETECT_CAN_ID_STD               2
#define DETECT_CAN_ID_EXT               3
#define DETECT_CAN_DATA                 4
#define DETECT_CAN_CONTROLLER_STATUS    5
#define DETECT_EOF_DATA                 6

/**
  * @brief Private handle struct
  */
typedef struct {
    struct {
        uint8_t                 u8DetectState;
        uint8_t                 au8Buffer[20];
        uint8_t                 u8Idx;
        uint8_t                 u8Type;
        uint8_t                 u8DLC;
        uint8_t                 u8CanId0;
        uint8_t                 u8CanId1;
        uint8_t                 u8CanId2;
        uint8_t                 u8CanId3;
        uint8_t                 u8RxErrCnt;
        uint8_t                 u8TxErrCnt;
    } Rx;

    CanRxCallbackType           RxCallback;
    CanStatusCallbackType       StatusCallback;
    SendUartCallbackType        SendUartCallback;
} HandleType;


/**
 * @brief CanUsb_Init - Call this function first to init library
 * @param fRxCallback - Callback when received CAN frame data
 * @param fStatusCallback - Callback when received status data
 * @param fSendUartCallback - Send UART callback
 * @return
 */
CanUsbHandleType    CanUsb_Init(CanRxCallbackType       fRxCallback,
                                CanStatusCallbackType   fStatusCallback,
                                SendUartCallbackType    fSendUartCallback)
{
    HandleType *pxHandle = malloc(sizeof(HandleType));
    if(pxHandle == NULL) {
        return NULL;
    }

    memset(pxHandle, 0, sizeof(HandleType));

    pxHandle->RxCallback = fRxCallback;
    pxHandle->StatusCallback = fStatusCallback;
    pxHandle->SendUartCallback = fSendUartCallback;

    return pxHandle;
}

/**
 * @brief CanUsb_Deinit - Call this to deinit library
 * @param pvHandle
 */
void    CanUsb_Deinit(CanUsbHandleType pvHandle)
{
    free(pvHandle);
}

/**
 * @brief CanUsb_Send - Send CAN frame
 * @param pvHandle
 * @param pxFrame
 * @return
 */
CanUsbReturnType    CanUsb_Send(CanUsbHandleType pvHandle, CanFrameType *pxFrame)
{
    if(pvHandle == NULL || pxFrame == NULL) {
        return CAN_USB_ERR;
    }

    HandleType *pxHandle = (HandleType*) pvHandle;

    uint8_t     au8SendBuf[7+/*pxFrame->u8DLC*/8];
    uint8_t     u8FrameLen;

    if(pxFrame->Type == CAN_TYPE_STD) {
        u8FrameLen = 5 + pxFrame->u8DLC;
        au8SendBuf[0] = 0xAA;           /* Packet start */
        au8SendBuf[1] = (1 << 7) | (1 << 6) | ((pxFrame->Type == CAN_TYPE_STD ? 0 : 1) << 5) | pxFrame->u8DLC;
        au8SendBuf[2] = pxFrame->u32FrameId;
        au8SendBuf[3] = pxFrame->u32FrameId >> 8;
        if(pxFrame->u8DLC) {
            memcpy(&au8SendBuf[4], pxFrame->pu8Data, pxFrame->u8DLC);
        }
        au8SendBuf[4+pxFrame->u8DLC] = 0x55;     /* Packet end */
    }
    else {
        u8FrameLen = 7 + pxFrame->u8DLC;
        au8SendBuf[0] = 0xAA;
        au8SendBuf[1] = (1 << 7) | (1 << 6) | ((pxFrame->Type == CAN_TYPE_STD ? 0 : 1) << 5) | pxFrame->u8DLC;
        au8SendBuf[2] = pxFrame->u32FrameId;
        au8SendBuf[3] = pxFrame->u32FrameId >> 8;
        au8SendBuf[4] = pxFrame->u32FrameId >> 16;
        au8SendBuf[5] = pxFrame->u32FrameId >> 24;
        if(pxFrame->u8DLC) {
            memcpy(&au8SendBuf[6], pxFrame->pu8Data, pxFrame->u8DLC);
        }
        au8SendBuf[6+pxFrame->u8DLC] = 0x55;
    }

    if(pxHandle->SendUartCallback) {
        return pxHandle->SendUartCallback(au8SendBuf, u8FrameLen);
    }

    return CAN_USB_OK;
}

/**
 * @brief CanUsb_Configure - Call this to configure USB CAN analyzer
 * @param pvHandle
 * @param pxConfig
 * @return
 */
CanUsbReturnType CanUsb_Configure(CanUsbHandleType pvHandle, CanConfigType *pxConfig)
{
    if(pvHandle == NULL || pxConfig == NULL) {
        return CAN_USB_ERR;
    }

    HandleType *pxHandle = (HandleType*) pvHandle;

    uint8_t au8SendBuf[20];

    au8SendBuf[0] = 0xAA;
    au8SendBuf[1] = 0x55;
    au8SendBuf[2] = 0x12;

    au8SendBuf[3] = pxConfig->Baudrate;
    au8SendBuf[4] = pxConfig->Type;

    au8SendBuf[5] = pxConfig->u32FilterId;
    au8SendBuf[6] = pxConfig->u32FilterId >> 8;
    au8SendBuf[7] = pxConfig->u32FilterId >> 16;
    au8SendBuf[8] = pxConfig->u32FilterId >> 24;

    au8SendBuf[9] = pxConfig->u32MaskId;
    au8SendBuf[10] = pxConfig->u32MaskId >> 8;
    au8SendBuf[11] = pxConfig->u32MaskId >> 16;
    au8SendBuf[12] = pxConfig->u32MaskId >> 24;

    au8SendBuf[13] = pxConfig->Mode;
    au8SendBuf[14] = 0x01;
    au8SendBuf[15] = 0;
    au8SendBuf[16] = 0;
    au8SendBuf[17] = 0;
    au8SendBuf[18] = 0;

    uint8_t u8Sum = 0;
    for(int idx = 2; idx < 19; idx++) {
        u8Sum += au8SendBuf[idx];
    }

    au8SendBuf[19] = u8Sum;

    if(pxHandle->SendUartCallback) {
        return pxHandle->SendUartCallback(au8SendBuf, 20);
    }

    return CAN_USB_OK;
}

/**
 * @brief CanUsb_EnableReceiveSelected - Received only CAN IDs specified in pu32IdList
 * @param pvHandle
 * @param pu32IdList - List of enabled CAN Ids
 * @param u8Count - Count of enabled CAN Ids
 * @return
 */
CanUsbReturnType CanUsb_EnableReceiveSelected(CanUsbHandleType pvHandle, uint32_t *pu32IdList, uint8_t u8Count)
{
    if(pu32IdList == NULL || u8Count == 0 || pvHandle == NULL) {
        return CAN_USB_ERR;
    }

    HandleType *pxHandle = (HandleType*) pvHandle;

    uint8_t *au8SendBuf = malloc(5+4*u8Count);

    au8SendBuf[0] = 0xAA;
    au8SendBuf[1] = 0x55;
    au8SendBuf[2] = 0x10;
    au8SendBuf[3] = u8Count;

    for(int idx = 0; idx < u8Count; idx++) {
        au8SendBuf[4+idx*4] = pu32IdList[idx];
        au8SendBuf[4+idx*4 + 1] = pu32IdList[idx] >> 8;
        au8SendBuf[4+idx*4 + 2] = pu32IdList[idx] >> 16;
        au8SendBuf[4+idx*4 + 3] = pu32IdList[idx] >> 24;
    }

    uint8_t u8Sum = 0;

    for(int idx = 2; idx < (4+4*u8Count); idx++) {
        u8Sum += au8SendBuf[idx];
    }

    au8SendBuf[4+4*u8Count] = u8Sum;

    int iRet = CAN_USB_OK;

    if(pxHandle->SendUartCallback) {
        iRet =  pxHandle->SendUartCallback(au8SendBuf, 5+4*u8Count);
    }

    free(au8SendBuf);

    return iRet;
}

/**
 * @brief CanUsb_CancelReceiveSelected - Enable received all CAN ids
 * @param pvHandle
 * @return
 */
CanUsbReturnType CanUsb_CancelReceiveSelected(CanUsbHandleType pvHandle)
{
    if(pvHandle == NULL) {
        return CAN_USB_ERR;
    }

    HandleType *pxHandle = (HandleType*) pvHandle;

    uint8_t au8SendBuf[5];

    au8SendBuf[0] = 0xAA;
    au8SendBuf[1] = 0x55;
    au8SendBuf[2] = 0x10;
    au8SendBuf[3] = 0x00;
    au8SendBuf[4] = 0x10;

    if(pxHandle->SendUartCallback) {
        return pxHandle->SendUartCallback(au8SendBuf, 5);
    }

    return CAN_USB_OK;
}

/**
 * @brief CanUsb_ReceiveByteUart - Call this when received byte from CAN USB analyzer COM
 * @param pvHandle
 * @param u8Rx
 */
void CanUsb_ReceiveByteUart(void *pvHandle, uint8_t u8Rx)
{
    if(pvHandle == NULL) {
        return;
    }

    HandleType *pxHandle = (HandleType*) pvHandle;

    switch(pxHandle->Rx.u8DetectState) {
    case DETECT_PACKET_START:
        if(u8Rx == 0xAA) {
            pxHandle->Rx.au8Buffer[pxHandle->Rx.u8Idx++] = u8Rx;
            pxHandle->Rx.u8DetectState = DETECT_FRAME_INFO;
        }
        break;

    case DETECT_FRAME_INFO:
        pxHandle->Rx.au8Buffer[pxHandle->Rx.u8Idx++] = u8Rx;
        if(u8Rx == 0x55) {
            /* CAN Controller status frame */
            pxHandle->Rx.u8DetectState = DETECT_CAN_CONTROLLER_STATUS;
        }
        else if((u8Rx & 0xC0) == 0xC0){
            /* CAN data frame */
            if(u8Rx & 0x20) {
                /* Ext ID */
                pxHandle->Rx.u8Type = CAN_TYPE_EXT;
                pxHandle->Rx.u8DetectState = DETECT_CAN_ID_EXT;
            }
            else {
                /* Std ID */
                pxHandle->Rx.u8Type = CAN_TYPE_STD;
                pxHandle->Rx.u8DetectState = DETECT_CAN_ID_STD;
            }

            pxHandle->Rx.u8DLC = u8Rx & 0x0F;
        }
        else {
            /* Invalid */
        }
        break;

    case DETECT_CAN_ID_STD:
        pxHandle->Rx.au8Buffer[pxHandle->Rx.u8Idx++] = u8Rx;
        if(pxHandle->Rx.u8Idx == 3) {
            pxHandle->Rx.u8CanId0 = u8Rx;
        }
        else if(pxHandle->Rx.u8Idx == 4) {
            pxHandle->Rx.u8CanId1 = u8Rx;
            pxHandle->Rx.u8DetectState = DETECT_CAN_DATA;
        }
        break;

    case DETECT_CAN_ID_EXT:
        pxHandle->Rx.au8Buffer[pxHandle->Rx.u8Idx++] = u8Rx;
        if(pxHandle->Rx.u8Idx == 3) {
            pxHandle->Rx.u8CanId0 = u8Rx;
        }
        else if(pxHandle->Rx.u8Idx == 4) {
            pxHandle->Rx.u8CanId1 = u8Rx;
        }
        else if(pxHandle->Rx.u8Idx == 5) {
            pxHandle->Rx.u8CanId2 = u8Rx;
        }
        else if(pxHandle->Rx.u8Idx == 6) {
            pxHandle->Rx.u8CanId3 = u8Rx;
            pxHandle->Rx.u8DetectState = DETECT_CAN_DATA;
        }
        break;

    case DETECT_CAN_DATA:
        pxHandle->Rx.au8Buffer[pxHandle->Rx.u8Idx++] = u8Rx;
        if((4+pxHandle->Rx.u8DLC) == pxHandle->Rx.u8Idx && pxHandle->Rx.u8Type == CAN_TYPE_STD) {
            pxHandle->Rx.u8DetectState = DETECT_EOF_DATA;
        }

        if((6+pxHandle->Rx.u8DLC) == pxHandle->Rx.u8Idx && pxHandle->Rx.u8Type == CAN_TYPE_EXT) {
            pxHandle->Rx.u8DetectState = DETECT_EOF_DATA;
        }
        break;

    case DETECT_EOF_DATA:
        pxHandle->Rx.au8Buffer[pxHandle->Rx.u8Idx] = u8Rx;
        if(u8Rx == 0x55) {
            /* Received frame */
            if(pxHandle->RxCallback) {
                CanFrameType xInfo;
                xInfo.Type = pxHandle->Rx.u8Type;
                xInfo.u32FrameId = pxHandle->Rx.u8CanId0 | (pxHandle->Rx.u8CanId1 << 8) |
                                   (pxHandle->Rx.u8CanId2 << 16) | (pxHandle->Rx.u8CanId3 << 24);
                xInfo.u8DLC = pxHandle->Rx.u8DLC;
                if(pxHandle->Rx.u8Type == CAN_TYPE_STD) {
                    xInfo.pu8Data = &pxHandle->Rx.au8Buffer[4];
                }
                else {
                    xInfo.pu8Data = &pxHandle->Rx.au8Buffer[6];
                }
                pxHandle->RxCallback(&xInfo);
            }
        }

        /* Reset */
        pxHandle->Rx.u8Idx = 0;
        pxHandle->Rx.u8DetectState = DETECT_PACKET_START;
        pxHandle->Rx.u8CanId0 = 0;
        pxHandle->Rx.u8CanId1 = 0;
        pxHandle->Rx.u8CanId2 = 0;
        pxHandle->Rx.u8CanId3 = 0;
        pxHandle->Rx.u8DLC = 0;
        break;

    case DETECT_CAN_CONTROLLER_STATUS:
        pxHandle->Rx.au8Buffer[pxHandle->Rx.u8Idx++] = u8Rx;

        if(pxHandle->Rx.u8Idx == 3) {
            if(u8Rx != 0x04) {
                pxHandle->Rx.u8Idx = 0;
                pxHandle->Rx.u8DetectState = DETECT_PACKET_START;
            }
        }
        else if(pxHandle->Rx.u8Idx == 4) {
            pxHandle->Rx.u8RxErrCnt = u8Rx;
        }
        else if(pxHandle->Rx.u8Idx == 5) {
            pxHandle->Rx.u8TxErrCnt = u8Rx;
        }
        else if(pxHandle->Rx.u8Idx == 20) {
            uint8_t u8Sum = 0;
            for(int idx = 2; idx < 19; idx++) {
                u8Sum += pxHandle->Rx.au8Buffer[idx];
            }

            if(u8Rx == u8Sum) {
                /* Callback */
                if(pxHandle->StatusCallback) {
                    pxHandle->StatusCallback(pxHandle->Rx.u8TxErrCnt, pxHandle->Rx.u8RxErrCnt);
                }
            }

            pxHandle->Rx.u8Idx = 0;
            pxHandle->Rx.u8DetectState = DETECT_PACKET_START;
            pxHandle->Rx.u8RxErrCnt = 0;
            pxHandle->Rx.u8TxErrCnt = 0;
        }

        break;

    default:
        break;
    }
}
