#ifndef _COMM_CHANNEL_H_
#define _COMM_CHANNEL_H_

#include <stdbool.h>

enum CommChannel { kBluetooth, kUSB = 8 };
enum CommErrors { kChannelUnknown = -1, kFuncNotSet = -2 };

/// Setup

/**
 * @brief Configura la funcion de escritura para el canal indicado.
 *
 * @param channel
 * @param write
 * @return int
 */
int SetWriteFor(enum CommChannel channel,
                int (*write)(const unsigned char *content, unsigned int len));

/**
 * @brief Configura la funcion de lectura para el canal indicado.
 *
 * @param channel
 * @param read
 * @return int
 */
int SetReadFor(enum CommChannel channel,
               int (*read)(unsigned char *outBuff,
                           unsigned int *outLen,
                           unsigned int timeout));

/**
 * @brief Configura la funcion para checar el buffer de entrada del canal
 * indicado.
 *
 * Se utiliza para saber si hay datos nuevos por leer.
 *
 * @param channel
 * @param checkBuffer
 * @return int
 */
int SetBuffCheckFor(enum CommChannel channel,
                    int (*checkBuffer)(unsigned int *outLen));

/**
 * @brief  Configura la funcion para limpiar el buffer de entrada del canal
 * indicado.
 *
 * @param channel
 * @param flush
 * @return int
 */
int SetBuffFlushFor(enum CommChannel channel, int (*flush)());

/// Actions

/**
 * @brief Escribe al canal seleccionado.
 *
 * @param content
 * @param len
 *
 * @return int
 */
int WriteToChannel(enum CommChannel channel,
                   const unsigned char *content,
                   unsigned int len);

/**
 * @brief Lee del canal seleccionado.
 *
 * @param outBuff
 * @param outLen
 * @param timeout
 * @return int
 */
int ReadFromChannel(enum CommChannel channel,
                    unsigned char *outBuff,
                    unsigned int *outLen,
                    unsigned int timeout);

/**
 * @brief Verifica si hay datos nuevos en el buffer del canal seleccionado.
 *
 * @param outLen
 * @return int
 */
int CheckBuffFromChannel(enum CommChannel channel, unsigned int *outLen);

/**
 * @brief Limpia el buffer del canal seleccionado.
 *
 * @return int
 */
int FlushChannel(enum CommChannel channel);

/**
 * @brief Establece si un puerto está abierto o cerrado.
 *
 */
void SetOpenForChannel(enum CommChannel channel, bool open);

/**
 * @brief Verifica si el canal está abierto.
 *
 */
bool IsChannelOpen(enum CommChannel channel);

#endif
