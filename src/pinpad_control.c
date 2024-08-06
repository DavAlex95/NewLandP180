#include "pinpad_control.h"

#include "log.h"

#include "comm_channel.h"

enum CommChannel pinpadChannel = kBluetooth;

void PinpadIdle() {
  unsigned int portBuffLen = 0;
  unsigned char portBuff[2048] = {0};
  unsigned int portReadTimeout = 100;
  int ret = 0, byteNo = 0;

  BluetoothCheckPairingStatus();

  ret = CheckBuffFromChannel(pinpadChannel, &portBuffLen);
  if (ret == 0) {
    if (portBuffLen > 0) {
      log_debug("El puerto #%d tiene %u datos nuevos para ser leidos",
                pinpadChannel, portBuffLen);

      ReadFromChannel(pinpadChannel, portBuff, &portBuffLen, portReadTimeout);
      log_debug("Leidos %u bytes del puerto #%d", portBuffLen, pinpadChannel);
      for (byteNo = 0; byteNo < portBuffLen; byteNo++) {
        log_debug("Byte leido #%d: %02X", byteNo, portBuff[byteNo]);
      }
    }
  } else {
    log_error("Error al checar buffer del puerto #%d. Codigo de error: %d",
              pinpadChannel, ret);
  }
}