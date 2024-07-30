/***************************************************************************
** All right reserved:  Newland Copyright (c) 2016-2026  
** File name:  lprint.h
** File indentifier: 
** Brief:  Print lib
** Current Verion:  v1.0
** Auther: liug
** Complete date: 2016-9-27
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#ifndef _LPRINT_H_
#define	_LPRINT_H_

#include "napi.h"

/** @addtogroup Printer
* @{
*/

#define MAX_PRINT_TAG_NAME	32
#define PRINT_DATA_MAXLEN	128
#define MAX_PRINT_MODULE_NUM  40
#define MAX_IMAGE_BLOCK   2000   

#define PRINT_PAPER_WIDTH	384

#define PubPrePrinter(p,args...) PubPriPrintStr(0,p,##args)

typedef enum {
        PRN_MODE_ALL_DOUBLE = 0, /**<Enlarge horizontally and vertically*/
        PRN_MODE_WIDTH_DOUBLE,   /**<Enlarge horizontally*/
        PRN_MODE_HEIGHT_DOUBLE,  /**<Enlarge vertically*/
        PRN_MODE_NORMAL          /**<No enlargement*/
} EM_PRN_MODE;

/**
*   Error code
*/
typedef enum
{
	PRN_OK,                         /**<Operate Success*/                                   
	PRN_ERR = -4001,				/**<Fail*/ 
	PRN_ERR_FILE_LOST = -4002,      /**<File lost*/
	PRN_ERR_FILE_OPEN = -4003,		/**<Open file error*/
	PRN_ERR_FILE_READ = -4004,		/**<Read file error*/
	PRN_ERR_FILE_WRITE = -4005,		/**<Write file error*/
	PRN_ERR_FORMAT = -4006,         /**<Print Model format error*/
	PRN_ERR_MODULEINFO = -4007,		/**<Saved Print Model info error*/
	PRN_ERR_INITPNT = -4008,        /**<Fail to initialize printer*/
	PRN_ERR_IMG_OPER = -4009,       /**<Fail to process picture data*/
	PRN_ERR_IMG_DATA = -4010,       /**<No picture data found*/
	PRN_ERR_FONT_ERR = -4011,       /**<Font setting error*/
}EM_PRINT_ERR;

enum _PRINTTYPE_DEF{
	_PRINTTYPE_TP=0, 	/**Thermal printer*/
	_PRINTTYPE_IP, 	 	/**Stylus printer*/ 
};


typedef enum
{
	PRINT_ALIGN_NORMAL=0,			/**<Donot clear line before display*/
	PRINT_ALIGN_CENTER=4,			/**<Display at middle*/
	PRINT_ALIGN_TAIL=8,			/**<Display at right*/
	
}EM_PRINTF_ALIGN;

/**
* @brief Get type of printer
* @return 
* @li Printer type refer to '_PRINTTYPE_DEF'
*/
int PubGetPrinter(void);

/**
* @brief Check current status of printer
* @return 
* @li The value of printer status
* @author Liug
* @date 2012-9-19
*/
int PubGetPrintStatus(void);
/**
* @brief Print str
* @param
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenxiulin
* @date 2019-8-28
*/
int PubPrnStr(const char *pszStr);
/**
* @brief Formats output string to print buffer
* @param [in] pszContent Pointer of string that needs to be printed
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-6-5
*/
//int PubPrePrinter(const char *pszContent, ...);

/**
* @brief Line feed
* @param [in] nLine  Number of line 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-6-5
*/
int PubPaperPeed(int nLine);

/**
* @brief Commit datas to printer
* @param [in] (*SendPrintData)() ---Function pointer
* @param [in] pvParam            ---The parameter of SendPrintData
* @param [in] unPrintType        ---Print type 0:print single time, 1:Print by send
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-9-20
*/
int PubPrintCommit(int (*SendPrintData)(void *), void *pvParam, uint unPrintType);

/**
* @brief Print bitmap
* @param [in] unXSize The width of picture
* @param [in] unYSize The height of picture
* @param [in] unXPos The position of top left corner of picture
* @param [in] psPrintBuf The buffer of picture
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-9-19
*/
int PubPrintImage(uint unXSize, uint unYSize, uint unXPos, const char *psPrintBuf);

/**
* @brief Print picture
* @param [in] unXPos The position of top left corner of picture
* @param [in] pszPicPath: Path of picture
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-9-19
*/
int PubPrintPicture(uint unXPos, const char *pszPicPath);

/**
* @brief Get print version
* @param pszOutVer The version of print library
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int PubGetPrintLibVer(char *pszOutVer);

/**
* @brief Low-voltage detect (Printer cannot work when voltage is low)
* @return 
* @li APP_SUCC Low-voltage
* @li APP_FAIL 
*/
int PubLowPowerCheck(void);

/**
* @brief Formats output string to print buffer
* @param [in] 	nMode	   Align Mode, see EM_PRINTF_ALIGN
				pszContent Pointer of string that needs to be printed
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author jiangxc
* @date 2018-5-15
*/
int PubPriPrintStr(int nMode, const char *pszContent, ...);
/**
* @brief Set print font
* @param [in] nFontSize       font size
* @param [in] emMode          Print Mode, refer to 'EM_PRN_MODE' in lprint.h
* @param [in] unSigOrDou      Single or double (for stylus printer)
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2018-05-15
*/

int PubSetPrtFont(int nFontSize, EM_PRN_MODE  emMode,  uint  unSigOrDou);

/**
* @brief Print a tail line
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author jiangxc
* @date 2018-5-15
*/
int PubPrintTail();


/**
* @brief Print double str on the both ends of one line
* @param [in] pszContent1
			  pszContent2
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author jiangxc
* @date 2018-5-15
*/
int PubPrtDoubleStrs(const char *pszContent1, const char *pszContent2);

/**
* @brief Whether the device supports printing
* @return 
* @li YES
* @li NO
* @author lingdz
*/
YESORNO PubIsSupportPrint();

/**
* @brief Set print font file
* @return 
* @li FilePathName TTF font file name with absolute path
* @author lingdz
*/
int PubSetPrnTTFFontFile(const char *pszFilePathName);

/**
* @brief Get print font file
* @return 
* @li FilePathName TTF font file name with absolute path
* @author lingdz
*/
int PubGetPrnTTFFontFile(char *pszFilePathName);

/** @}*/ // End of print

#endif
/* End of lprint.h */

