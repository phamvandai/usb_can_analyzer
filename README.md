# USB CAN Analyzer library

This is C/C++ library for PC to work with USB CAN Analyzer (example from [Seeed studio](https://www.seeedstudio.com/USB-CAN-Analyzer-p-2888.html))

## API
	CanUsbHandleType    CanUsb_Init(CanRxCallbackType fRxCallback, CanStatusCallbackType   fStatusCallback, SendUartCallbackType    fSendUartCallback);

	void                CanUsb_Deinit(CanUsbHandleType pvHandle);

	CanUsbReturnType    CanUsb_Send(CanUsbHandleType pvHandle, CanFrameType *pxFrame);

	CanUsbReturnType    CanUsb_Configure(CanUsbHandleType pvHandle, CanConfigType *pxConfig);

	CanUsbReturnType    CanUsb_EnableReceiveSelected(CanUsbHandleType pvHandle, uint32_t *pu32IdList, uint8_t u8Count);

	CanUsbReturnType    CanUsb_CancelReceiveSelected(CanUsbHandleType pvHandle);

	void                CanUsb_ReceiveByteUart(CanUsbHandleType pvHandle, uint8_t u8Rx);

Usage example with Qt can be found [here](QtExample).