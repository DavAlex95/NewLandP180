#include "log.h"

#include "comm_channel.h"
#include "napi_serialport.h"

static int USBSerialWrite(const unsigned char *content, unsigned int len) {
  return NAPI_PortWrite(USB_SERIAL, content, len);
}

static int USBSerialRead(unsigned char *outBuff,
                         unsigned int *outLen,
                         unsigned int timeout) {
  return NAPI_PortRead(USB_SERIAL, outBuff, outLen, timeout);
}

static int USBSerialBuffCheck(unsigned int *outLen) {
  return NAPI_PortReadLen(USB_SERIAL, outLen);
}

static int USBSerialFlush() { return NAPI_PortFlush(USB_SERIAL); }

int USBSerialSetup() {
  int ret;
  PORT_SETTINGS usbPortSettings;
  SetBuffCheckFor(kUSB, &USBSerialBuffCheck);
  SetBuffFlushFor(kUSB, &USBSerialFlush);
  SetWriteFor(kUSB, &USBSerialWrite);
  SetReadFor(kUSB, &USBSerialRead);

  usbPortSettings.BaudRate = BAUD19200;
  usbPortSettings.DataBits = DATA_8;
  usbPortSettings.Parity = PAR_NONE;
  usbPortSettings.StopBits = STOP_1;
  //usbPortSettings.FlowControl = FLOW_OFF;
  //usbPortSettings.Timeout_Millisec = 1000;

  ret = NAPI_PortOpen(USB_SERIAL, usbPortSettings);
  usleep(1000);
  NAPI_PortFlush(USB_SERIAL);

  if (ret == 0) {
    log_error("Se inicializo el puerto USB correctamente");
    SetOpenForChannel(kUSB, true);
  } else {
    log_error("No pudo inicializarse el puerto USB");
    SetOpenForChannel(kUSB, false);
  }
  return ret;
}

int USBSerialTeardown() {
  int ret;
  NAPI_PortFlush(USB_SERIAL);
  ret = NAPI_PortClose(USB_SERIAL);
  return ret;
}
