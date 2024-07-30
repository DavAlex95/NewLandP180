/**
* @file lfileSystem.h
* @brief filesystem operation
* @version
*	v2.0 chenxiulin  2019-08-06
*	v1.0 Product Development Department 2012-08-17
*/
#ifndef _LFILESYSTEM_H_
#define _LFILESYSTEM_H_

/**
 *@brief 		Open file.
 *@param[in]   pszName File name
 *@param[in]   pszMode "r" (Read only) or "w"(Write only).
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsOpen(const char *pszName, const char *pszMode);

/**
 *@brief 		Close file.
 *@param[in]    nHandle File handle
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsClose(int nHandle);

/**
 *@brief 		Read file.
 *@param[in]    nHandle File handle
 *@param[in]    unLength	Length of bytes to be read
 *@param[out]   psBuffer	Buffer to save data
 *@return
  On success, it returns \ref read len; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsRead(int nHandle, char *psBuffer, uint unLength);

/**
 *@brief 		Write file.
 *@param[in]    nHandle File handle
 *@param[in]    psBuffer	Data buffer with data to be written
 *@param[in]    unLength	Length of bytes to be written
 *@return
  On success, it returns \ref write Len; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsWrite(int nHandle, const char *psBuffer, uint unLength);

/**
 *@brief 		Set read/write position
 *@param[in]    nHandle File handle
 *@param[in]    ulDistance New position.
 *@param[in]    unPosition SEEK_SET: From start of file.\n
			SEEK_CUR: From current position.\n
			SEEK_END: From end of file.\n
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsSeek(int nHandle, ulong ulDistance, uint unPosition);

/**
 *@brief 		Delete file
 *@param[in]   pszName File name
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsDel(const char *pszName);

/**
 *@brief 		Get file length
 *@param[in]    pszName File name
 *@param[out]   punSize File length
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsFileSize(const char *pszName, uint *punSize);

/**
 *@brief 		Rename file
 *@param[in]    pszSrcName Old file name
 *@param[in]    pszDstName New file name
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsRename(const char *pszSrcName, const char *pszDstName);

/**
 *@brief 		Check if file exists
 *@param[in]    pszName File name
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/

int PubFsExist(const char *pszName);

/**
 *@brief 		Truncate file
 *@details   It will change file length to unLen. If previous file length is larger than unLen, it will be truncated. If previous file length is smaller than unLen, it will be padded with 0xff.
 *@param[in]    pszPath File path
 *@param[in]    unLen New file length
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsTruncate(const char *pszPath, uint unLen);

/**
 *@brief 	  	Get read/write position
 *@param[in]    nHandle File handle
 *@param[out]   pulRet Current read/write postion
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsTell(int nHandle, ulong *pulRet);

/**
 *@brief 	  	Get used space
 *@param[in]    	unWhich 0: Used; 1: Free
 *@param[out]     pulSpace Amount of space used or free
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsGetDiskSpace(uint unWhich, ulong *pulSpace);

/**
 *@brief 		Create directory
 *@param[in]    pszName Directory name
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsCreateDirectory(const char *pszName);

/**
 *@brief 		Delete directory.
 *@param[in]    pszName Directory name
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsRemoveDirectory(const char *pszName);

/**
 *@brief 		Format file system (Not valid for Phoenix OS)
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsFormat(void);

/**
 *@brief 		List file in directory
 *@details	The size of psBuf must be large enough to avoid overflow, 20 bytes to store one file name. The first 19 bytes is file name, the last byte indicates directory or file (0: Normal file; 1: Directory)
 *@param[in]       psPath Directory to read
 *@param[out]      psBuf Buffer to store file name
 *@param[out]      punNum Total number of files under this directory
 *@return
  On success, it returns \ref NAPI_OK "NAPI_OK"; on error, it returns \ref EM_NAPI_ERR "EM_NAPI_ERR".
*/
int PubFsDir(const char *psPath, char *psBuf, uint *punNum);

#endif /* _LFILESYSTEM_H_ */

