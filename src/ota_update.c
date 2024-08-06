#include "ota_update.h"

#include "libapiinc.h"

#include "apiinc.h"
#include "appinc.h"
#include "pinpad_commands.h"

#include <log.h>

static char ch_OtaStatus = OTA_STATUS_IDLE;
static char ch_OtaType = 0x00;

void SYS_SetOTAStatus(char type) { ch_OtaStatus = type; }
char SYS_GetOTAStatus() { return ch_OtaStatus; }

void OTA_SetOTAType(char type) { ch_OtaType = type; }
char OTA_GetOTAType() { return ch_OtaType; }

int INSTRUCTION_OTA(char *pMessage, int nMessageLength) {
  return 0;
}

/*
int INSTRUCTION_OTA(char *pMessage, int nMessageLength) {
  int nLength = 0, offset = 0, extoffset = 0;
  int nReqType = 0;
  int nDataLength = 0;
  int nFd = 0;
  int nRet = 0;
  unsigned int unFileOffset = 0, unFileSize = 0;
  int nLimitResult = 0;
  char *pdata = NULL;
  char szReturnCode[2] = {0};
  char szFileName[64] = {0};
  char szDispInfo[32] = {0};
  static char szFileBlock[MAX_OTA_FILE_BUFFER_LENGTH] = {0};
  static int nCurrentFileBlockOffset = 0;
  static int nCurrentSavedFileOffset = 0;

  memcpy(szReturnCode, INSTRUCTION_OK, 2);
  memcpy(szFileName, "/appfs/", strlen("/appfs/"));
  strcat(szFileName, OTA_APP_FILE);

  nLength = MPOS_VARIABLE_READ(pMessage);
  offset = MPOS_VARIABLE_OFFSET;
  extoffset = MPOS_EXT_OFFSET + nLength;

  nReqType = pMessage[offset];
  offset++;
  PubBcdToInt(pMessage + offset, &nDataLength);
  offset += 2;
  pdata = pMessage + offset;
  offset += nDataLength;
  TRACE("type=%d", nReqType);
  TRACE("length=%d", nDataLength);
  TRACE("szFileName=%s", szFileName);

  SYS_SetOTAStatus(OTA_STATUS_ACTIVE);
  if (nLength != 3 + nDataLength + MPOS_VARIABLE_MINLEN) {
    PubC4ToInt(&unFileOffset, pMessage + offset);
    offset += 4;
    PubC4ToInt(&unFileSize, pMessage + offset);
    offset += 4;
    TRACE("unFileOffset=%d", unFileOffset);
    TRACE("unFileSize=%d", unFileSize);

    if (nReqType == 0x02) {
#if 0
			if( false == SpaceEnoughForUpdate(unFileSize))
			{
				UI_ClearMainScreen();
				UI_DisplayString(LINE_1, DISPLAY_MODE_CENTER, "UPDATE FAILED");
				UI_DisplayString(LINE_2, DISPLAY_MODE_CENTER, "DISK SPACE IS NOT ENOUGH");				
				SYS_delay(3000);
				memcpy(szReturnCode, Response_Code_General_error, 2);
				goto on_err_ack;
			}
#endif
      PubClearAll();
      memset(szDispInfo, 0x00, sizeof(szDispInfo));
      sprintf(szDispInfo, "UPDATING...,%02d%%",
              (unFileOffset * 100) / unFileSize);
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, szDispInfo);
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "DON'T SHUT DOWN");
      OTA_SetOTAType(EM_NEED_TO_UPDATE_APP);
    }
  } else if (nReqType == 0x02) {
    PubClearAll();
    PubDisplayStr(DISPLAY_MODE_CENTER, 1, 1, "UPDATING...");
    nRet = FILE_size(szFileName, &nLength);
    TRACE("nLength=%d", nLength);
    if ((nRet == 0) && (nLength >= 0)) {
      memset(szDispInfo, 0x00, sizeof(szDispInfo));
      PubDisplayStr(DISPLAY_MODE_CENTER, 1, 1, "%d KB", nLength / 1024);
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "DON'T SHUT DOWN");
    } else {
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "DON'T SHUT DOWN");
    }
    OTA_SetOTAType(EM_NEED_TO_UPDATE_APP);
  }

  if ((nReqType == 0x00) || (nReqType == 0x04) || (nReqType > 0x05)) {
    memcpy(szReturnCode, Response_Code_ERR_PARM, 2);

    if (nReqType == 0x00) {
      memcpy(szReturnCode, Response_Code_Good, 2);
    }

    goto on_err_ack;
  }

  if (nReqType == 0x01) {
    int ulRemainStorage = 0;
    nCurrentFileBlockOffset = 0;
    nCurrentSavedFileOffset = 0;
    memset(szFileBlock, 0, sizeof(szFileBlock));
    SYS_SetOTAStatus(OTA_STATUS_ACTIVE);
    FILE_delete(szFileName);
    if (0 == NAPI_FsGetDiskSpace(1, &ulRemainStorage)) {
      if (0 > ulRemainStorage - unFileSize) {
        //insuffcient storage
        memcpy(szReturnCode, Response_Code_Insufficient_Storage, 2);
        TRACE("insuffcient storage:remain[%l] to storage %s[%l]",
                  ulRemainStorage, szFileName, unFileSize);
        goto on_err_ack;
      }
    } else {
      //gain remain storage failed
      memcpy(szReturnCode, INSTRUCTION_ERR_OTHER, 2);
      TRACE("Gain remain storage failed");
      goto on_err_ack;
    }
  } else {
    if (SYS_GetOTAStatus() != OTA_STATUS_ACTIVE) {
      memcpy(szReturnCode, Response_Code_ERR_SEQUENCE, 2);
      goto on_err_ack;
    }
  }

  // need to save file
  if ((nCurrentFileBlockOffset + nDataLength > MAX_OTA_FILE_BUFFER_LENGTH) ||
      ((unFileOffset + nDataLength) == unFileSize)) {
    if (nDataLength != 0) {
      nFd = PubFsOpen(szFileName, "w");
      // nFd = FILE_open(szFileName, "w");
      if (nFd < 0) {
        memcpy(szReturnCode, Response_Code_ERR_PARM, 2);
        goto on_err_ack;
      }

      if (unFileOffset == 0) {
        PubFsSeek(nFd, 0, SEEK_END);
      } else {
        PubFsSeek(nFd, nCurrentSavedFileOffset, SEEK_END);
        // FILE_seek(nFd, nCurrentSavedFileOffset, SEEK_SET);
      }
      if (nCurrentFileBlockOffset > 0) {
        nLength = FILE_write(nFd, szFileBlock, nCurrentFileBlockOffset);
        TRACE("FILE_write = %d", nLength);
        if (nLength != nCurrentFileBlockOffset) {
          SYS_SetOTAStatus(OTA_STATUS_IDLE);
          PubFsClose(nFd);
          // FILE_close(nFd);
          PubFsDel(szFileName);
          // FILE_delete(szFileName);
          memcpy(szReturnCode, Response_Code_General_error, 2);
          goto on_err_ack;
        }
        nCurrentSavedFileOffset += nCurrentFileBlockOffset;
      }

      if ((unFileOffset + nDataLength) == unFileSize) {
        nLength = FILE_write(nFd, pdata, nDataLength);
        nCurrentSavedFileOffset += nLength;
      }
      PubFsClose(nFd);
      nCurrentFileBlockOffset = 0;
      memset(szFileBlock, 0, sizeof(szFileBlock));
    }
  }

  if ((unFileOffset + nDataLength) < unFileSize) {
    memcpy(szFileBlock + nCurrentFileBlockOffset, pdata, nDataLength);
    nCurrentFileBlockOffset += nDataLength;
  } else if ((unFileOffset + nDataLength) > unFileSize) {
    PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "UPDATE FAILED");
    PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "TOTAL LENGTH ERROR!");
    memcpy(szReturnCode, Response_Code_ERR_SIGN, 2);
    goto on_err_ack;
  }

  TRACE("nCurrentFileBlockOffset=%d", nCurrentFileBlockOffset);
  TRACE("nCurrentSavedFileOffset=%d", nCurrentSavedFileOffset);

  offset = 0;
  pMessage[extoffset] = nReqType;
  offset++;
  memcpy(pMessage + extoffset + offset, szReturnCode, 2);
  offset += 2;
  NAPI_AlgSHA1((unsigned char *)pdata, nDataLength,
               (unsigned char *)pMessage + extoffset + offset);
  offset += 20;
  FRAME_send(pMessage + extoffset, offset, INSTRUCTION_OK);
  // LOG_HEX(":", pMessage + extoffset, offset);

  if (nReqType == 0x03) {
    TRACE("START INSTALL");
    PubClearAll();
    PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "START UPDATING");
    SYS_SetOTAStatus(OTA_STATUS_IDLE);

    if ((OTA_GetOTAType() & EM_NEED_TO_UPDATE_APP) == EM_NEED_TO_UPDATE_APP)
      PubFsRename(OTA_APP_FILE, OTA_APP_INSTALL_FILE);

    if ((OTA_GetOTAType() & EM_NEED_TO_UPDATE_MASTER) ==
        EM_NEED_TO_UPDATE_MASTER) {
      PubFsRename(OTA_MASTER_FILE, OTA_MASTER_INSTALL_FILE);
      PubFsRename(OTA_DEVCFG_FILE, OTA_DEVCFG_INSTALL_FILE);
      nLimitResult = CheckPatchUpdateLimit();
      if (APP_SUCC != nLimitResult) {
        OTA_ShowPatchErrMessage(nLimitResult);
        memcpy(szReturnCode, Response_Code_ERR_SIGN, 2);
        goto on_err_ack;
      }
    }

    if ((OTA_GetOTAType() & EM_NEED_TO_UPDATE_MASTER) ==
        EM_NEED_TO_UPDATE_MASTER) {
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1,
                    "AUTOMATIC RESTART AFTER UPDATE");
      nRet = OTA_UpdateMasterFromFileSystem();
      TRACE("OTA_UpdateMasterFromFileSystem = %d", nRet);
    } else if ((OTA_GetOTAType() & EM_NEED_TO_UPDATE_APP) ==
               EM_NEED_TO_UPDATE_APP) {
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1,
                    "AUTOMATIC RESTART AFTER UPDATE");
      PubFsRename(OTA_APP_INSTALL_FILE, OTA_APP_FILE);
      nRet = NAPI_AppInstall(OTA_APP_FILE);
      TRACE("NAPI_AppInstall = %d", nRet);
    } else
      nRet = -1;

    if (nRet == NAPI_ERR_APP_NLD_HEAD_LEN) {
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "UPDATE FAILED");
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "HEADER ERROR!");
      memcpy(szReturnCode, Response_Code_ERR_SIGN, 2);
      goto on_err_ack;
    } else if (nRet == NAPI_ERR_APP_SIGN_DECRYPT) {
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "UPDATE FAILED");
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "CERTIFICATION ERROR!");
      memcpy(szReturnCode, Response_Code_ERR_SIGN, 2);
      goto on_err_ack;
    } else if (nRet == NAPI_ERR_APP_SIGN_CHECK) {
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "UPDATE FAILED");
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "SHA256 ERROR!");
      memcpy(szReturnCode, Response_Code_ERR_SIGN, 2);
      goto on_err_ack;
    } else if (nRet <= NAPI_ERR) {
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "UPDATE FAILED");
      memcpy(szReturnCode, Response_Code_General_error, 2);
      goto on_err_ack;
    }

    PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "UPDATED SUCCESSFULLY");
    SYS_delay(3000);
    OTA_SetOTAType(EM_NOTHING_TO_UPDATE);
  }

  SYS_UpdateDisplayCount();
  return 0;

on_err_ack:
  PubFsClose(nFd);
  offset = 0;
  pMessage[extoffset] = nReqType;
  offset++;
  memcpy(pMessage + extoffset + offset, szReturnCode, 2);
  offset += 2;
  NAPI_AlgSHA1((unsigned char *)pdata, nDataLength,
               (unsigned char *)pMessage + extoffset + offset);
  offset += 20;
  FRAME_send(pMessage + extoffset, offset, INSTRUCTION_OK);
  SYS_UpdateDisplayCount();
  OTA_DeleteAllDownloadFiles();
  SYS_SetOTAStatus(OTA_STATUS_IDLE);
  return 0;
}
*/

int INSTRUCTION_WriteFile(char *pMessage, int nMessageLength) {
  return 0;
}
/*
int INSTRUCTION_WriteFile(char *pMessage, int nMessageLength) {
  int fd = 0, offset = 0;
  int nRet = 0, nFileNameLen = 0, nWriteLen = 0;
  char szReturnCode[2] = {0};
  char pOutput[2] = {0};
  char szFileName[64] = {0};
  int nFileOffset = 0;
  static char szFileBlock[MAX_OTA_FILE_BUFFER_LENGTH] = {0};
  static int nCurrentFileBlockOffset = 0;
  static int nCurrentSavedFileOffset = 0;
  int nTotalFileSize = 0;

  offset = MPOS_VARIABLE_OFFSET;
  memcpy(szReturnCode, INSTRUCTION_OK, 2);
  memcpy(pOutput, INSTRUCTION_OK, 2);

  // LL file name length
  PubBcdToInt(pMessage + offset, &nFileNameLen);
  if (nFileNameLen > 12) {
    memcpy(pOutput, "05", 2);
    goto on_ack;
  }
  offset += 2;
  // file name
  memcpy(szFileName, (char *)pMessage + offset, nFileNameLen);
  offset += nFileNameLen;
  // 4BYTE offset
  PubC4ToInt(&nFileOffset, pMessage + offset);
  offset += 4;
  // LL WRITELEN file buffer length
  PubBcdToInt(pMessage + offset, &nWriteLen);  //æ–‡ä»¶å†…å®¹LL+CONTENT
  offset += 2;
  // file size
  PubC4ToInt(&nTotalFileSize, pMessage + offset + nWriteLen);

  if (nFileOffset == 0) {
    nCurrentFileBlockOffset = 0;
    nCurrentSavedFileOffset = 0;
    memset(szFileBlock, 0, sizeof(szFileBlock));
  }

  TRACE("FILE NAME: = %s", szFileName);
  TRACE("nFileOffset: = %d", nFileOffset);
  TRACE("nWriteLen: = %d", nWriteLen);
  TRACE("nTotalFileSize: = %d", nTotalFileSize);
  TRACE("nCurrentFileBlockOffset: = %d", nCurrentFileBlockOffset);
  TRACE("nCurrentSavedFileOffset: = %d", nCurrentSavedFileOffset);
  // LOG_HEX("write data: = ", pMessage + offset, nWriteLen);
  OTA_CheckUpdateDisplay(szFileName);

  // need to save file
  if ((nCurrentFileBlockOffset + nWriteLen > MAX_OTA_FILE_BUFFER_LENGTH) ||
      ((nFileOffset + nWriteLen) == nTotalFileSize)) {
    if (nWriteLen != 0) {
      TRACE("go to save file block to file system");

      if (nFileOffset == 0) {
        PubFsDel(szFileName);
      }

      fd = PubFsOpen(szFileName, "w");
      if (fd < 0) {
        memcpy(pOutput, "02", 2);
        PubFsClose(fd);
        goto on_ack;
      }

      if (PubFsSeek(fd, nCurrentSavedFileOffset, SEEK_SET) != NAPI_OK) {
        memcpy(pOutput, "03", 2);
        PubFsClose(fd);
        goto on_ack;
      }
      if (nCurrentFileBlockOffset > 0) {
        nRet = FILE_write(fd, szFileBlock, nCurrentFileBlockOffset);
        if (nRet != nCurrentFileBlockOffset) memcpy(pOutput, "04", 2);

        nCurrentSavedFileOffset += nCurrentFileBlockOffset;
      }
      if ((nFileOffset + nWriteLen) == nTotalFileSize) {
        nRet = FILE_write(fd, pMessage + offset, nWriteLen);
        if (nRet != nWriteLen) memcpy(pOutput, "04", 2);
        nCurrentSavedFileOffset += nWriteLen;
      }
      nCurrentFileBlockOffset = 0;
      memset(szFileBlock, 0, sizeof(szFileBlock));
      PubFsClose(fd);
      TRACE("nCurrentSavedFileOffset(%d)", nCurrentSavedFileOffset);
      TRACE("nCurrentFileBlockOffset(%d)", nCurrentFileBlockOffset);
    }
  }

  if ((nFileOffset + nWriteLen) < nTotalFileSize) {
    memcpy(szFileBlock + nCurrentFileBlockOffset, pMessage + offset, nWriteLen);
    nCurrentFileBlockOffset += nWriteLen;
  } else if ((nFileOffset + nWriteLen) > nTotalFileSize) {
    memcpy(pOutput, "03", 2);
  }
on_ack:
  if (memcmp(pOutput, INSTRUCTION_OK, 2) != 0) {
    nCurrentFileBlockOffset = 0;
    nCurrentSavedFileOffset = 0;
    memset(szFileBlock, 0, sizeof(szFileBlock));
    PubFsDel(szFileName);
  }
  FRAME_send(pOutput, 2, szReturnCode);
  return 0;
}
*/

int OTA_UpdateMasterFromFileSystem() {
  return 0;
}
/*
int OTA_UpdateMasterFromFileSystem() {
  int nRet;
  int nFD = 0, nOffset;
  char szFileBlockBuffer[MAX_OTA_FILE_BUFFER_LENGTH] = {0};
  int nFileTotalSize = 0;
  int nReadBackLength;
  int i = 0;
  int nNeedUpdate = 0;
  char g_pszOTAMasterFileList[1][2]={0};

  if (APP_SUCC != OTA_CheckMasterDevLimit()) {
    OTA_DeleteMasterFiles();
    TRACE("cfg and master don't exists at the same time...");
  }

  for (i = 0;
       i < sizeof(g_pszOTAMasterFileList) / sizeof(g_pszOTAMasterFileList[0]);
       i++) {
    char *p1;
    nReadBackLength = 0;
    memset(szFileBlockBuffer, 0, sizeof(szFileBlockBuffer));

    nRet = FILE_exist(g_pszOTAMasterFileList[i][2]);
    TRACE("FILE_exist=%d", nRet);

    if (nRet == NAPI_OK) {
      TRACE("NEED UPDATE MASTER");
      //nFD = PubFsOpen(g_pszOTAMasterFileList[i][2], "w");
      p1 = &(g_pszOTAMasterFileList[i][2]);
      nFD = PubFsOpen(p1, "w");
      if (nFD < 0) goto on_return;

      FILE_size(g_pszOTAMasterFileList[i][2], &nFileTotalSize);
      TRACE("nFileTotalSize=%d", nFileTotalSize);
      nOffset = 0;
      PubClearAll();
      PubDisplayStr(DISPLAY_MODE_CENTER, 2, 1, "UPDATING...");
      PubDisplayStr(DISPLAY_MODE_CENTER, 3, 1, "DON'T SHUT DOWN");
      NAPI_AppFlashErase(g_pszOTAMasterFileList[i][0]);

      while (1) {
        if (PubFsSeek(nFD, nOffset, SEEK_SET) != NAPI_OK) goto on_return;

        nReadBackLength =
            FILE_read(nFD, szFileBlockBuffer, MAX_OTA_FILE_BUFFER_LENGTH);
        TRACE("FILE_read=%d", nReadBackLength);
        if ((nReadBackLength != MAX_OTA_FILE_BUFFER_LENGTH) &&
            (nReadBackLength != nFileTotalSize - nOffset))
          goto on_return;

        nRet = NAPI_AppFlashWrite(g_pszOTAMasterFileList[i][0], nOffset,
                                  (unsigned char *)szFileBlockBuffer,
                                  nReadBackLength);
        TRACE("NAPI_AppFlashWrite=%d", nRet);
        if (nRet != NAPI_OK) goto on_return;

        TRACE("nOffset=%d", nOffset);
        if (nReadBackLength == nFileTotalSize - nOffset) {
          PubFsClose(nFD);
          nNeedUpdate = 1;
          break;
        }

        nOffset += MAX_OTA_FILE_BUFFER_LENGTH;
        memset(szFileBlockBuffer, 0, sizeof(szFileBlockBuffer));
      }
    }

    if ((i == 1) && (nNeedUpdate == 1)) {
      PubFsDel(OTA_MASTER_FILE);
      PubFsDel(OTA_DEVCFG_FILE);
      PubFsDel(OTA_MASTER_INSTALL_FILE);
      PubFsDel(OTA_DEVCFG_INSTALL_FILE);
      nRet = NAPI_AppRun((const char *)"updater");
      break;
    }
  }

on_return:
  PubFsClose(nFD);
  PubFsDel(OTA_MASTER_FILE);
  PubFsDel(OTA_DEVCFG_FILE);
  return nRet;
}
*/