#ifndef _SOFTALG_H_
#define _SOFTALG_H_
#include "napi.h"
typedef enum  {
	SALG_OK = 0,
	SALG_ERR = -1,
	SALG_ERR_PARA = -2,
} SOFT_ERR_TYPE;

typedef enum {
	SALG_TDS_MODE_ENC = 0, /**<DES encryption*/
	SALG_TDS_MODE_DEC = 1, /**<DES decryption*/
} EM_SALG_TDS_MODE;

/**
* @brief soft alg des (3des or des)
* @param [in] psDataIn
* @param [out] psDataOut
* @param [in] psKey
* @param [in] nKeyLen
* @param [in] emMode
* @li APP_FAIL
* @li APP_SUCC
* @author chenxiulin
* @date 2019-08-05
*/
int Soft_AlgTDes (uchar *psDataIn, uchar *psDataOut,
		uchar *psKey, int nKeyLen, EM_SALG_TDS_MODE emMode);

#endif /* _SOFTALG_H_*/
