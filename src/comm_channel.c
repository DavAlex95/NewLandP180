/**
 * @file comm_channel.c
 * @author Guillermo Garcia Maynez (ggarcia@necsweb.com)
 * @brief Abstraccion del canal de comunicacion utilizable por PIN pad
 * @date 2021-04-15
 *
 * @copyright Copyright (c) 2021 New England Computer Solutions
 *
 */

#include "comm_channel.h"

#include <stdbool.h>
#include <stdlib.h>

struct Channel {
  bool portOpen;
  bool checkBufferSet;
  bool flushSet;
  bool writeSet;
  bool readSet;
  int (*checkBuffer)(unsigned int *outLen);
  int (*flush)();
  int (*write)(const unsigned char *content, unsigned int len);
  int (*read)(unsigned char *outBuff,
              unsigned int *outLen,
              unsigned int timeout);
};

struct Channel btChannel = {false, false, false, false, false,
                            NULL,  NULL,  NULL,  NULL};
struct Channel usbChannel = {false, false, false, false, false,
                             NULL,  NULL,  NULL,  NULL};

// Setup

int SetWriteFor(enum CommChannel channel,
                int (*write)(const unsigned char *content, unsigned int len)) {
  switch (channel) {
    case kBluetooth:
      btChannel.write = write;
      btChannel.writeSet = true;
      break;

    case kUSB:
      usbChannel.write = write;
      usbChannel.writeSet = true;
      break;

    default:
      return kChannelUnknown;
      break;
  }
  return 0;
}

int SetReadFor(enum CommChannel channel,
               int (*read)(unsigned char *outBuff,
                           unsigned int *outLen,
                           unsigned int timeout)) {
  switch (channel) {
    case kBluetooth:
      btChannel.read = read;
      btChannel.readSet = true;
      break;

    case kUSB:
      usbChannel.read = read;
      usbChannel.readSet = true;
      break;

    default:
      return kChannelUnknown;
      break;
  }
  return 0;
}

int SetBuffCheckFor(enum CommChannel channel,
                    int (*checkBuffer)(unsigned int *outLen)) {
  switch (channel) {
    case kBluetooth:
      btChannel.checkBuffer = checkBuffer;
      btChannel.checkBufferSet = true;
      break;

    case kUSB:
      usbChannel.checkBuffer = checkBuffer;
      usbChannel.checkBufferSet = true;
      break;

    default:
      return kChannelUnknown;
      break;
  }
  return 0;
}

int SetBuffFlushFor(enum CommChannel channel, int (*flush)()) {
  switch (channel) {
    case kBluetooth:
      btChannel.flush = flush;
      btChannel.flushSet = true;
      break;

    case kUSB:
      usbChannel.flush = flush;
      usbChannel.flushSet = true;
      break;

    default:
      return kChannelUnknown;
      break;
  }
  return 0;
}

// Action

int WriteToChannel(enum CommChannel channel,
                   const unsigned char *content,
                   unsigned int len) {
  struct Channel selectedChannel;
  switch (channel) {
    case kBluetooth:
      selectedChannel = btChannel;
      break;

    case kUSB:
      selectedChannel = usbChannel;
      break;

    default:
      return kChannelUnknown;
      break;
  }

  if (selectedChannel.writeSet == true) {
    return selectedChannel.write(content, len);
  } else {
    return kFuncNotSet;
  }
}

int ReadFromChannel(enum CommChannel channel,
                    unsigned char *outBuff,
                    unsigned int *outLen,
                    unsigned int timeout) {
  struct Channel selectedChannel;
  switch (channel) {
    case kBluetooth:
      selectedChannel = btChannel;
      break;

    case kUSB:
      selectedChannel = usbChannel;
      break;

    default:
      return kChannelUnknown;
      break;
  }

  if (selectedChannel.readSet == true) {
    return selectedChannel.read(outBuff, outLen, timeout);
  } else {
    return kFuncNotSet;
  }
}

int CheckBuffFromChannel(enum CommChannel channel, unsigned int *outLen) {
  struct Channel selectedChannel;
  switch (channel) {
    case kBluetooth:
      selectedChannel = btChannel;
      break;

    case kUSB:
      selectedChannel = usbChannel;
      break;

    default:
      return kChannelUnknown;
      break;
  }

  if (selectedChannel.checkBufferSet == true) {
    return selectedChannel.checkBuffer(outLen);
  } else {
    return kFuncNotSet;
  }
}

int FlushChannel(enum CommChannel channel) {
  struct Channel selectedChannel;
  switch (channel) {
    case kBluetooth:
      selectedChannel = btChannel;
      break;

    case kUSB:
      selectedChannel = usbChannel;
      break;

    default:
      return kChannelUnknown;
      break;
  }

  if (selectedChannel.flushSet == true) {
    return selectedChannel.flush();
  } else {
    return kFuncNotSet;
  }
}

void SetOpenForChannel(enum CommChannel channel, bool open) {
  struct Channel *selectedChannel;
  switch (channel) {
    case kBluetooth:
      selectedChannel = &btChannel;
      break;

    case kUSB:
      selectedChannel = &usbChannel;
      break;

    default:
      //return kChannelUnknown;
      return;
      break;
  }

  selectedChannel->portOpen = open;
}

bool IsChannelOpen(enum CommChannel channel) {
  struct Channel selectedChannel;
  switch (channel) {
    case kBluetooth:
      selectedChannel = btChannel;
      break;

    case kUSB:
      selectedChannel = usbChannel;
      break;

    default:
      return kChannelUnknown;
      break;
  }

  return selectedChannel.portOpen;
}
