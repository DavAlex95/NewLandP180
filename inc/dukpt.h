/**
 * @file tk.h
 * @author Guillermo Garcia Maynez R. (ggarcia@necsweb.com)
 * @brief Cifrado y operaciones referentes a DUKPT
 * @date 2021-05-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef _DUKPT_H_
#define _DUKPT_H_

/**
 * @brief Verifica la existencia de la llave para DUKPT.
 *
 * @return int 1 si existe
 */
int ipek_exists();

/**
 * @brief Borra la llave para DUKPT.
 *
 * @return int 1 si se borro exitosamente
 */

int ipek_delete();

/**
 * @brief Carga una llave IPEK
 *
 * @warning Asegurese de borrar de forma segura el contenido de la memoria de la
 * llave IPEK despues de llamar a este método.
 *
 * @param ipek
 * @param ksn
 * @return int
 */
int load_ipek(const unsigned char ipek[16], const unsigned char ksn[10]);

/**
 * @brief Cifra con DUKPT
 *
 * @param dataIn Datos a cifrar
 * @param dataLen Longitud de datos a cifrar
 * @param dataOut Bufer de salida del mismo tamaño que la longitud de datos
 * @param ksnOut KSN usado en el cifrado con DUKPT
 *
 * @return int 0
 */
int dukpt_encrypt(const unsigned char *dataIn,
                  unsigned int dataLen,
                  unsigned char *dataOut,
                  unsigned char *ksnOut);

int increase_ksn();

int assemble_sensitiveDataBlock(char chTk1[],
                            char chTk2[],
                            char chCv2[],
                            char chPan[],
                            char chExpDate[],
                            unsigned char currentKsn[],
                            unsigned char chB1[],
                            unsigned char chB2[]);

#endif