#include <string.h>
#include "softalg.h"
#include "des.h"
#include "ltool.h"

int Soft_AlgTDes (uchar *psDataIn, uchar *psDataOut,
        uchar *psKey, int nKeyLen, EM_SALG_TDS_MODE emMode)
{
    if ((psDataIn == NULL) || (psDataOut == NULL) ||
        (psKey == NULL)) {
        return SALG_ERR_PARA;
    }
    if ((nKeyLen != 8) && (nKeyLen != 16 ) &&
            (nKeyLen != 24)) {
        return SALG_ERR_PARA;
    }
    if ((emMode != SALG_TDS_MODE_ENC) &&
        (emMode != SALG_TDS_MODE_DEC)) {
        return SALG_ERR_PARA;
    }

    if (nKeyLen == 8) {
        deskey(psKey, emMode);
        des(psDataIn, psDataOut);
        return SALG_OK;
    }
    else if (nKeyLen == 16) {
        des3key2(psKey, emMode);
    }
    else if (nKeyLen == 24) {
        des3key3(psKey, emMode);
    }
    des3(psDataIn, psDataOut);

    return SALG_OK;
}

