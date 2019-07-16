#ifndef _CAN_USB_H_
#define _CAN_USB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint8_t         CanUsbReturnType;
typedef void*           CanUsbHandleType;

#define CAN_USB_OK      0
#define CAN_USB_ERR     1

typedef enum {
    CAN_BAUDRATE_1000k = 0x01,
    CAN_BAUDRATE_800k,
    CAN_BAUDRATE_500k,
    CAN_BAUDRATE_400k,
    CAN_BAUDRATE_250k,
    CAN_BAUDRATE_200k,
    CAN_BAUDRATE_125k,
    CAN_BAUDRATE_100k,
    CAN_BAUDRATE_50k,
    CAN_BAUDRATE_20k,
    CAN_BAUDRATE_10k,
    CAN_BAUDRATE_5k
} CanBaudrateType;

typedef enum {
    CAN_TYPE_STD = 1,
    CAN_TYPE_EXT
} CanIdType;

typedef enum {
    CAN_MODE_NORMAL = 0,
    CAN_MODE_LOOPBACK,
    CAN_MODE_SILENT,
    CAN_MODE_LOOPBACK_SILENT
} CanUsbModeType;

typedef struct {
    CanUsbModeType      Mode;
    CanIdType           Type;
    CanBaudrateType     Baudrate;
    uint32_t            u32FilterId;
    uint32_t            u32MaskId;
} CanConfigType;

typedef struct {
    CanIdType           Type;
    uint32_t            u32FrameId;
    uint8_t             u8DLC;
    uint8_t             *pu8Data;
} CanFrameType;

typedef   void              (*CanRxCallbackType)(CanFrameType *pxFrame);
typedef   void              (*CanStatusCallbackType)(uint8_t u8TxErrCnt, uint8_t u8RxErrCnt);
typedef   CanUsbReturnType  (*SendUartCallbackType)(uint8_t *pu8Data, uint8_t u8Len);

/* APIs */
CanUsbHandleType    CanUsb_Init(CanRxCallbackType       fRxCallback,
                                CanStatusCallbackType   fStatusCallback,
                                SendUartCallbackType    fSendUartCallback);
void                CanUsb_Deinit(CanUsbHandleType pvHandle);
CanUsbReturnType    CanUsb_Send(CanUsbHandleType pvHandle, CanFrameType *pxFrame);
CanUsbReturnType    CanUsb_Configure(CanUsbHandleType pvHandle, CanConfigType *pxConfig);
CanUsbReturnType    CanUsb_EnableReceiveSelected(CanUsbHandleType pvHandle, uint32_t *pu32IdList, uint8_t u8Count);
CanUsbReturnType    CanUsb_CancelReceiveSelected(CanUsbHandleType pvHandle);
void                CanUsb_ReceiveByteUart(CanUsbHandleType pvHandle, uint8_t u8Rx);

#ifdef __cplusplus
}
#endif

#endif
