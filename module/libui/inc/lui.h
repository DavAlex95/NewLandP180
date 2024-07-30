/***************************************************************************
** All right reserved:  Newland Copyright (c) 2016-2026  
** File name:  lui.h
** File indentifier: 
** Brief:  
** Current Verion:  v1.0
** Auther: linw
** Complete date: 2016-9-26
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#ifndef _LUI_H_
#define _LUI_H_

#include "napi.h"

/** @addtogroup UI
* @{
*/

/** @addtogroup Keyboard
* @{
*/

#define KEY_0				'0'
#define KEY_1				'1'
#define KEY_2				'2'
#define KEY_3				'3'
#define KEY_4				'4'
#define KEY_5				'5'
#define KEY_6				'6'
#define KEY_7				'7'
#define KEY_8				'8'
#define KEY_9				'9'
#define KEY_F1				0x01
#define KEY_F2				0x02
#define KEY_F3				0x03
#define KEY_F4				0x04
#define KEY_F5				0x05
#define KEY_F6				0x06
#define KEY_F7				0x07
#define KEY_F8				0x08
#define KEY_F9				0x09
#define KEY_LEFT			KEY_F1
#define KEY_RIGHT			KEY_F2
#define KEY_UP				KEY_F1
#define KEY_DOWN			KEY_F2
#define KEY_STAR			0x2e
#define KEY_SHARP			0x1c
#define KEY_BACK			0x0a		
#define KEY_ENTER			0x0d		
#define KEY_ESC				0x1b		
#define KEY_MENU			0x03		
#define KEY_ENTERPAPER  	0xb1	
#define KEY_BACKPAPER   	0xb2		
#define KEY_NULL			0xfe		/**<0xfe*/

#define KEY_a				'a'
#define KEY_b				'b'
#define KEY_c				'c'
#define KEY_d				'd'
#define KEY_e				'e'
#define KEY_f				'f'
#define KEY_g				'g'
#define KEY_h				'h'
#define KEY_i				'i'
#define KEY_j				'j'
#define KEY_k				'k'
#define KEY_l				'l'
#define KEY_m				'm'
#define KEY_n				'n'
#define KEY_o				'o'
#define KEY_p				'p'
#define KEY_q				'q'
#define KEY_r				'r'
#define KEY_s				's'
#define KEY_t				't'
#define KEY_u				'u'
#define KEY_v				'v'
#define KEY_w				'w'
#define KEY_x				'x'
#define KEY_y				'y'
#define KEY_z				'z'
#define KEY_A				'A'
#define KEY_B				'B'
#define KEY_C				'C'
#define KEY_D				'D'
#define KEY_E				'E'
#define KEY_F				'F'
#define KEY_G				'G'
#define KEY_H				'H'
#define KEY_I				'I'
#define KEY_J				'J'
#define KEY_K				'K'
#define KEY_L				'L'
#define KEY_M				'M'
#define KEY_N				'N'
#define KEY_O				'O'
#define KEY_P				'P'
#define KEY_Q				'Q'
#define KEY_R				'R'
#define KEY_S				'S'
#define KEY_T				'T'
#define KEY_U				'U'
#define KEY_V				'V'
#define KEY_W				'W'
#define KEY_X				'X'
#define KEY_Y				'Y'
#define KEY_Z				'Z'

#define KEY_DOT				'.'
#define KEY_COMMA			','
#define KEY_SEMICOLON		';'
#define KEY_DIAGONAL		'/'
#define KEY_LEFTPRNTHS		'('
#define KEY_RIGHTPRNTHS		')'

#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLACK     0x0000
#define COLOR_DBLUE     0x000F
#define COLOR_BLUE      0x001F
#define COLOR_SKYBLUE   0x7D5C
#define COLOR_DGREY     0x7BEF
#define COLOR_WHITE     0xFFFF
#define COLOR_YELLOW    0xFFE0
#define COLOR_LGRAY     0xC618
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F        //  magenta
#define COLOR_DGREEN    0x03E0        //dark green
#define COLOR_DCYAN     0x03EF
#define COLOR_MAROON    0x7800
#define COLOR_PURPLE    0x780F
#define COLOR_OLIVE     0x7BE0

/** @}*/ // End of Keyboard

#define MAX_MUNE_LINE_NUM    5

typedef enum
{
	MENU_TITLE_MUST = 0,			/**<Title must exist*/
	MENU_TITLE_AUTO					/**<Title probably exist*/
}EM_MENU_TITLE;

typedef enum
{
	BUTTON_CONFIRM = 0,
	BUTTON_NORMAL,
	BUTTON_HOMEMENU,
}EM_BUTTON_ATTR;

typedef enum
{
	KB_VIRTUAL = 0,
	KB_PHYSICAL = 1,
}EM_KEYBOARD_ATTR;

typedef enum
{
	DISPLAY_MODE_NORMAL=0,			/**<Donot clear line before display*/
	DISPLAY_MODE_CLEARLINE=1,		/**<Clear line before display*/
	DISPLAY_MODE_CENTER=4,			/**<Display at middle*/
	DISPLAY_MODE_TAIL=8,			/**<Display at tail of a line*/
}EM_DISPLAY_MODE;

typedef enum
{
	DISPLAY_ALIGN_BIGFONT	=0,		/**<Align with big font*/
	DISPLAY_ALIGN_SMALLFONT			/**<Align with small font*/
}EM_DISPLAY_ALIGN;

typedef enum 
{
	MENU_MODE_NONE = 0,				/**<Not allow menu exit*/
	MENU_MODE_ESCCONFIRM = 0x01,	/**<Need to confirm when exit*/
	MENU_MODE_ESC = 0x02,			/**<Press cancel to exit*/
	MENU_MODE_AUTOESC=0x03			/**<Auto exit when finishing execution*/
}EM_MENU_MODE;

typedef enum
{
	INPUT_MODE_STRING,				/**<Normal string(alphabet and number)*/
	INPUT_MODE_NUMBER,				/**<Only number*/
	INPUT_MODE_PASSWD,				/**<Shield the string with '*' */
	INPUT_MODE_AMOUNT,				/**<Amount*/
	INPUT_MODE_NUMBERPOINT			/**<Number and point*/
}EM_INPUT_MODE;

typedef enum
{
	INPUT_DATE_MODE_YYMM,			/**< YYMM */ 
	INPUT_DATE_MODE_MMDD,			/**< MMDD */ 
	INPUT_DATE_MODE_YYMMDD,			/**< YYMMDD */ 
	INPUT_DATE_MODE_YYYYMMDD,		/**< YYYYMMDD */ 
	INPUT_DATE_MODE_MMDD_NULL,		/**< MMDD or skip */ 
	INPUT_DATE_MODE_YYMM_NULL,		/**< YYMM or skip */ 
	INPUT_DATE_MODE_YYMMDD_NULL,	/**< YYMMDD or skip */ 
	INPUT_DATE_MODE_YYYYMMDD_NULL	/**< YYYYMMDD or skip */ 
}EM_INPUT_DATE_MODE;				/**<Input date mode*/

typedef enum
{
	INPUT_AMOUNT_MODE_NONE,			/**<The amount allow to be zero*/
	INPUT_AMOUNT_MODE_NOT_NONE		/**<The amount must greater than zero*/
}EM_INPUT_AMOUNT_MODE;

typedef struct
{
	EM_MENU_TITLE emMenu;			/**<EM_MENU_TITLE*/
	char szPageDown[4+1];			/**<Content for pagedown*/
	char szPageUp[4+1];				/**<Content for pageup*/
}STUIFUN_STYLE;


/**
* @brief Initialize UI parameter. It must be called first before using UI library.
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Linw
* @date 2012-07-12
*/
int PubInitUiParam(void);

/**
* @brief Get UI library version
* @param [out] pszVer Version string(16 bytes)
* @return 
* @li void
* @author Linw
* @date 2012-06-26
*/
void PubGetUIVer(char *pszVer);

/**
* @brief Get Pos module type(string)
* @return 
* @li NULL Fail
* @li POS module name (string) --Success
* @author Linw
* @date 2013-05-27
*/
char *PubGetPosTypeStr(void);

/**
* @brief  Get the maximum line and column 
* @param [out] pnMaxLine maximum lines to display
* @param [out] pnLineCol maximum Cn Char to display(pnLineCol * 2 is the max columns)
* @return
* @li void
*/
void PubGetDispView(int *pnMaxLine, int *pnLineCol);


/**
* @brief Set UI menu style
* @return 
* @li void
* @author Linw
* @date 2012-11-28
*/
void PubSetUiStyle(STUIFUN_STYLE stUiFunStyle);

/**
* @brief Get UI menu style
* @param [out] pstUiFunStyle refer to STUIFUN_STYLE
* @return 
* @li void
* @date 2015-01-09
*/
void PubGetUiStyle(STUIFUN_STYLE *pstUiFunStyle);

/**
* @brief Get color depth in order to identify the type of screen
* @return 
* @li APP_FAIL    Fail
* @li other       1----black-white, 16----color
*/
int PubScrGetColorDepth(void);

/**
* @brief Get format string, support multiple lines
* @param [in] nFont	0 big font,  1 small font
* @param [in] pFormat |L close to left, |C close to centre, |R close to right, such as("L%s|C%s|R%s",szLeft,szCenter,szRight)
* @param [out] pOutStr   Aligned string
* @return 
* @li void
*/
void PubGetStrFormat(int nFont, char * pOutStr, char * pFormat, ...);

/** @addtogroup Display
* @{
*/

/**
* @brief Clear screen
* @return 
* @li void
*/
void PubClearAll(void);

/**
* @brief Clear from the second line to the last line
* @return 
* @li void
*/
void PubClear2To4(void);

/**
* @brief Clear appointed lines(large font)
* @param [in] ucStartLine Begin line, from 1
* @param [in] ucEndLine	  End line, last line can use (0xff)
* @return 
* @li void
*/
void PubClearLine(uchar ucStartLine, uchar ucEndLine);

/**
* @brief  Clear appointed lines(small font)
* @param [in] ucStartLine Begin line, from 1
* @param [in] ucEndLine   End line, last line can use (0xff)
* @return 
* @li void
*/
void PubClearLines(uchar ucStartLine, uchar ucEndLine);

/**
* @brief Display string in appointed location(large font,Support Hanzi)
* @details  When nMode is DISPLAY_MODE_CENTER or DISPLAY_MODE_RIGHT, nLineno & nClumno will be ignored
* @param [in] nMode   Show mode---EM_DISPLAY_MODE 
* @param [in] nLineno line, from 1
* @param [in] nClumno Clumn, from 1
* @param [in] pFormat Display format
* @return 
* @li void
*/
void PubDisplayStr(int nMode, int nLineno, int nClumno, char * pFormat, ...);

/**
* @brief Display string in appointed location(small font, English only)
* @details  When nMode is DISPLAY_MODE_CENTER or DISPLAY_MODE_RIGHT, nLineno & nClumno will be ignored
* @param [in] nAlign  Align mode---EM_DISPLAY_ALIGN(colorful machine ignores nAlign)
* @param [in] nMode   Show mode---EM_DISPLAY_MODE
* @param [in] nLineno Line, from 1
* @param [in] nClumno Clumn, from 1
* @param [in] pFormat Display format
* @return 
* @li void
*/
void PubDisplayStrs(int nAlign, int nMode, int nLineno, int nClumno, char *pFormat, ...);

/**
* @brief Center display string at appointed line(Large font, clear line before displaying)
* @param [in] pStr    Enter string
* @param [in] nLineno Appointed line, from 1
* @return 
* @li void
*/
void PubDisplayGen(int nLineno, char *pStr); 

/**
* @brief Center display string at appointed line(Small font, clear line before displaying)
* @param [in] nAlign  Align mode---EM_DISPLAY_ALIGN(colorful machine ignores nAlign)
* @param [in] pStr    Enter string
* @param [in] nLineno Appointed line, from 1
* @return 
* @li void
*/
void PubDisplayGens(int nAlign, int nLineno, char *pStr);

/**
* @brief Inverse diplay string (large font)
* @param [in] pstr    Eenter string
* @param [in] nLineno Appointed line
* @return 
* @li void
*/
void PubDisplayInv(int nLineno, char *pstr);

/**
* @brief Inverse diplay string(small font)
* @param [in] nAlign   Align mode---EM_DISPLAY_ALIGN(colorful machine ignores nAlign)
* @param [in] nLineno  Appointed line
* @param [in] pstr     Enter string
* @return 
* @li void
*/
void PubDisplayInvs(int nAlign, int nLineno, char *pstr); 

/**
* @brief Display title
* @param [in] pszTitle
* @return 
* @author Liug
* @date 2012-7-23
*/
void PubDisplayTitle(char *pszTitle);

/**
* @brief Display string at the end of lines
* @param [in] nFont	Font
* @param [in] nMode	Show mode---EM_DISPLAY_MODE 
* @param [in] nIsInv NO-normal  YES-inverse
* @param [in] pFormat Display content
* @return 
*/
void PubDisplayTail(int nFont, int nMode, int nIsInv, char *pFormat, ...);

/**
* @brief Display string in appointed location. Support multiple lines
* @param [in] nFont		0 Big font, 1 small font
* @param [in] nLineno	Line from 1-8
* @param [in] nClumno	Clumno form 1-16
* @param [in] pFormat	Content
* @return 
* @li void
*/
void PubDispMultLines(int nFont, int nLineno, int nClumno, char *pFormat, ...);

/**
* @brief Display logo
* @param [in] pszPath Logo path
* @param [in] nX      Abscissa of picture (top left corner)
* @param [in] nY	  Ordinate of picture (top left corner)
* @return 
* @li void
*/
void PubDisplayLogo(char *pszPath, int nX, int nY);

/**
* @brief Display string in appointed line(big font)
* @param [in] nMode	Show mode---EM_DISPLAY_MODE
* @param [in] nLineno Line from 1
* @param [in] pFormat Content
* @return 
* @li void
*/
void PubDisplayStrInline(int nMode, int nLineno, char *pFormat, ...);

/**
* @brief Display string in appointed line(small font)
* @param [in] nAlign Align mode---EM_DISPLAY_ALIGN(colorful machine ignores nAlign)
* @param [in] nMode	show mode---EM_DISPLAY_MODE
* @param [in] nLineno Line from 1
* @param [in] pFormat Content
* @return 
* @li void
*/
void PubDisplayStrInlines(int nAlign, int nMode, int nLineno, char *pFormat, ...);

/**
* @brief Update & display screen
* @return void
* @author Liug
* @date 2012-7-20
*/
void PubUpdateWindow(void);

/**
* @brief Show pub api error info(no err, no display)
* @param [in] pszTitle Title
* @return 
* @li APP_SUCC  
* @li APP_FAIL  
*/
int PubDispErr(const char *pszTitle);

/**
 *@brief    Display data in HEX format
* @param [in] pcData		Display data 
* @param [in] nLen			Length
 *@return
 *@li       APP_SUCC    Success
 *@li       APP_FAIL 	Fail
*/
int PubDispData(const char *pcData, int nLen);

/** @}*/ // End of Display

/** @addtogroup Input
* @{
*/

/**
* @brief Get the first key value from buffer and return immediately
* @return 
* @li the key value
*/
int PubKbHit(void);

/**
* @brief Get key value before time is out. Timeout(>0) can be set, set 0 for blocking unless any key is pressed.
* @param [in] nTimeout	
* @return 
* @li key value or 0 when time is out
*/
int PubGetKeyCode(int nTimeout);

/**
* @brief Input message in appointed location 
* @param [in] nClumno   Column(from 1)
* @param [in] nLineno   Line(from 1)
* @param [in] nMinLen   Minimum length
* @param [in] nMaxLen   Maximum length
* @param [in] nTimeOut  Timeout
* @param [in] nEditMask Input mode---EM_INPUT_MODE
* @param [out] pszOut   Output
* @param [out] pnOutLen Output length
* @return 
* @li APP_TIMEOUT 	Timeout
* @li APP_FAIL    	Abnormal
* @li APP_QUIT    	User cancel
* @li APP_SUCC    	Success
* @li KEY_F1/KEY_F2 Only when 'PubInputxy' is called in a callback function which is parameter of 'PubUpDownMenus'.
* <pre>	
*	e.g. 
*	int func(void)
*	{
*		//In this circumstance, PubInputxy will return KEY_F1 or KEY_F2, 
*		//if KEY_F1 or KEY_F2 is pressed, in order to achieve function of scrolling up and down.
*		PubInputxy	
*	}
*	int (*lSetFuns[])(void) = 
*	{
*		func,
*		...,
*		NULL
*	};
*	PubUpDownMenus(lSetFuns);
* </pre>
* 
*/
int PubInputxy(int nClumno, int nLineno, char *pszOut, int *pnOutLen, int nMinLen, int nMaxLen, int nTimeOut, int nEditMask);

/**
* @brief Inout dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Prompt content
* @param [in] nMinLen    Minimum length
* @param [in] nMaxLen    Maximum length
* @param [in] nTimeOut   Timeout
* @param [in] nEditMask  Input mode---EM_INPUT_MODE
* @param [out] pszOut    Output string
* @param [out] pnOutLen  Output length
* @return 
* @li APP_TIMEOUT Timeout
* @li APP_FAIL    	Abnormal
* @li APP_QUIT    	user cancel
* @li APP_SUCC    	Success
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
*/
int PubInputDlg(const char *pszTitle, const char *pszContent, char *pszOut, int *pnOutLen, \
				int nMinLen, int nMaxLen, int nTimeOut, int nEditMask);

/**
* @brief Extension input dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Prompt content
* @param [in] psInfo      If no need, set NULL
* @param [in] nClumno   Column(from 1)
* @param [in] nLineno   Line(from 1)
* @param [in] nMinLen    Minimum length
* @param [in] nMaxLen    Maximum length
* @param [in] nTimeOut   Timeout
* @param [in] nEditMask  Input mode---EM_INPUT_MODE
* @param [out] pszOut    Output
* @param [out] pnOutLen  Output length
* @return 
* @li APP_TIMEOUT Timeout
* @li APP_FAIL    Abnormal
* @li APP_QUIT    User cancel
* @li APP_SUCC    Success
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
*/
int PubExtInputDlg(const char *pszTitle, const char *pszContent, char *psInfo, \
                int nClumno, int nLineno, char *pszOut, int *pnOutLen,int nMinLen, \
                int nMaxLen, int nTimeOut, int nEditMask);

/**
* @brief Input Amount dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Content
* @param [in] nNoneFlag  EM_INPUT_AMOUNT_MODE
* @param [in] nTimeOut   Timeout
* @param [out] pszAmount Amount
* @param [out] pnInputLen Output length
* @return 
* @li APP_TIMEOUT Timeout
* @li APP_FAIL    	Abnormal
* @li APP_QUIT    	Cancel
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
*/
int PubInputAmount(const char *pszTitle, const char *pszContent, char *pszAmount, int *pnInputLen,\
				   int nNoneFlag, int nTimeOut);

/**
* @brief Input date dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Content
* @param [in] nFormat    Date format---EM_INPUT_DATE_MODE
* @param [in] nTimeOut   Timeout
* @param [out] pszDate   Date
* @return 
* @li APP_FAIL    	Abnormal
* @li APP_TIMEOUT 	Timeout
* @li APP_QUIT    	Cancel
* @li APP_SUCC    	Success
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
*/
int PubInputDate(const char *pszTitle, const char *pszContent, char *pszDate, int nFormat, int nTimeOut);

/**
* @brief Extension input date dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Content
* @param [in] pInfo      If no need, set NULL
* @param [in] nClumno    Column(from 1)
* @param [in] nLineno    Line(from 1)
* @param [in] nFormat    Date format---EM_INPUT_DATE_MODE
* @param [in] nTimeOut   Timeout
* @param [out] pszDate   Date
* @return 
* @li APP_FAIL    	Abnormat
* @li APP_TIMEOUT T	imeout
* @li APP_QUIT    	Cancel
* @li APP_SUCC    	Success
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
*/
int PubExtInputDate(const char *pszTitle, const char *pszContent, const char *pInfo, \
				int nClumno, int nLineno, char *pszDate, int nFormat, int nTimeOut);
					
/**
* @brief Input IP dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Menu hint
* @param [in] pnOutLen   Input length
* @param [out] pszOut    Entered Ip
* @param [out] pnOutLen  Output the length of pszOut
* @return
* @li APP_QUIT    	Cancel
* @li APP_SUCC    	Success
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
*/
int PubInputIp(const char *pszTitle, const char *pszContent, char *pszOut, int *pnOutLen);

/**
* @brief Enter PIN (plaintext)
* @param [in] pszTitle    ---Title (no use)
* @param [in] pszAmount   ---Entered Amount or NULL
* @param [in] pszContent  ---Contents or NULL
* @param [in] nMinLen     ---Minimum length
* @param [in] nMaxLen     ---Maximum length
* @param [out] pszOut     ---PIN (plaintext)
* @return 
* @li APP_SUCC		Success
* @li APP_FAIL		Fail 
* @li KEY_F1/KEY_F2 Refer to 'PubInputxy'
* @author Liug
* @date 2012-7-23
*/
int PubInputPin(const char *pszTitle, const char *pszAmount, const char *pszContent, int nMinLen, int nMaxLen, char *pszOut);

/**
* @brief Display contents & picture
* @param [in] unWidth		Width
* @param [in] unHeight		Height
* @param [in] pszBitmap  	Bitmap picture data
* @param [in] pszStr     	If select item list, seperate by '|', else directly show
* @param [in] nTimeout      Timeout
* @param [out] pnItemNum    Return item
* @return 
* @li void
*/
void PubDrawBitmapList(const uint unWidth, const uint unHeight, const char *pszBitmap, const char *pszStr, int nTimeout, int *pnItemNum);

/**
* @brief Display contents & picture
* @param [in] unWidth		Width
* @param [in] unHeight		Height
* @param [in] pszPic   		File path
* @param [in] pszStr     	If select item list, seperate by '|', else directly show
* @param [in] nTimeout		Timeout
* @param [out] pnItemNum     Return item
* @return 
* @li void
*/
void PubDispPicList(const uint unWidth, const uint unHeight, const char *pszPic, const char *pszStr, int nTimeout, int *pnItemNum);

/**
* @brief Multi menu-pages management.
* @details Supporting moving to previous and next menu page in order among the menu page functions which are listed in the 'plSetFuns'.
* <pre>		
*	e.g.
*	int func(void)
*	{
*		//When KEY_F1 or KEY_F2 is pressed, 'func' should return KEY_F1 or KEY_F2, 
*		//in order to achieve function of scrolling up and down.
*	}
*	int func2(void){//Similar to func}
*	int func3(void){//Similar to func}
*
*	int (*lSetFuns[])(void) = 
*	{
*		func,
*		func2,
*		func3,
*		NULL
*	};
*	PubUpDownMenus(lSetFuns);
* </pre>
* @param [in] plSetFuns  Function list (usually used in page settings)
* @return
* @li       APP_SUCC  Success
* @li       APP_QUIT  Cancel
* @li       APP_FAIL  Fail
*/
int PubUpDownMenus(int (*plSetFuns[])(void));

/** @}*/ // End of UI

/** @addtogroup MessageBox
* @{
*/

/**
* @brief Prompt dialog box
* @param [in] pszTitle   Title
* @param [in] pszContent Prompt content
* @param [in] nBeep      Beep times
* @param [in] nTimeOut   Timeout
* @return 
* @li APP_TIMEOUT Timeout
* @li APP_SUCC    Success
*/
int PubMsgDlg(const char *pszTitle, const char *pszContent, int nBeep, int nTimeOut);

/**
* @brief Confirm dialog box(need confirmation)
* @param [in] pszTitle   Title
* @param [in] pszContent Content
* @param [in] nBeep      Beep times
* @param [in] nTimeOut   Timeout
* @return 
* @li APP_TIMEOUT Timeout
* @li APP_QUIT    Cancel
* @li APP_SUCC    Press ENTER
*/
int PubConfirmDlg(const char *pszTitle, const char *pszContent, int nBeep, int nTimeOut);

/**
* @brief Scroll pages & display dialox box
* @param [in] pszTitle     ---Title
* @param [in] pszContent   ---Content
* @param [in] nIsResKey    ---Flag of special usage, 'YES' or 'No'
* <pre>	
*	If nFlag is set 'NO' and full screen is not enough to display 'pszContent, 
*		'APP_FUNCQUIT' will be returned as KEY_ENTER is pressed, then apps will call
*		'PubUpDownMsgDlg' again to see the details.
*	If nFlag is set 'YES', it will always return 'APP_SUCC' when KEY_ENTER is pressed.
*	e.g.
*	//Below full screen is not enough to display 'szContent'
*	nRet = PubUpDownMsgDlg("Title", szContent, NO, 60, &nKey);
*	if(nRet == APP_FUNCQUIT && nKey == KEY_ENTER)
*	{
*		nRet = PubUpDownMsgDlg("Title", szContent, YES, 60, &nKey);
*	}
* </pre>
* @param [in] nTimeOut     ---Timeout
* @param [out] nResKey	   ---Key value
* @return 
* @li APP_TIMEOUT  	Timeout
* @li APP_SUCC		Success
* @li APP_QUIT  	Quit
* @li APP_FUNCQUIT 	See above
* @li APP_FAIL  	Fail 
* @author Liug
* @date 2012-8-2
*/
int PubUpDownMsgDlg(const char *pszTitle, const char *pszContent, int nIsResKey, int nTimeOut, int *nResKey);

/**
* @brief Show confirmation dialog box(F1,F2 to turn pages)
* @param [in] pszTitle	 Title
* @param [in] pszContent Content
* @param [in] nBeep 	 Beep times
* @param [in] nTimeOut	 Timeout
* @return 
* @li TIMEOUT 	  Timeout
* @li APP_QUIT	  User cancel
* @li APP_SUCC	  Confrim
*/
int PubUpDownConfirmDlg(const char *pszTitle,const char *pszContent, int nBeep, int nTimeOut);

/**
 *@brief   set font size for display and line space
 * @param [in]  nFontSize 		
 				nLineSpacing	
 *@return
 *@li       APP_SUCC    		Success
 *@li       APP_FAIL 			Failure
  @author  Ashin
  @data    2018-5-14
*/
int PubSetDispForm(int nFontSize, int nLineSpacing);

/**
 *@brief   get font size for display 、line space and line height
 * @param [out] pnFontSize 		
 				pnLineSpacing	
 				pnLineHeight
 *@return
 *@li       APP_SUCC    		Success
 *@li       APP_FAIL 			Failure
  @author  Ashin
  @data    2018-5-14
*/
int PubGetDispForm(int *pnFontSize, int *pnLineSpacing, int *pnLineHeight);

/**
 *@brief  Prompt a menu with 2 options
 * @param [in]  pszTitle 		
 				pszStr	
 				SelMenu		if it is NULL, use the default options:Yes and No
 				pcSelect
 * @param [out]	pcSelect
 				
 *@return
 *@li       APP_SUCC    		Success
 *@li       APP_FAIL 			Failure
  @author  Ashin
  @data    2018-5-14 
*/
int PubSelectYesOrNo(char *pszTitle, char *pszStr, char* SelMenu[], char *pcSelect);

/**
 *@brief  Show a menu with items
 * @param [in]  pszTitle 		
 				ppMenuItems	
 				nMenuItemNum
 				pnSelectItem	
 				pnStartItem	
 				nTimeout
 * @param [out]	pnSelectItem
 				pnStartItem
 *@return
 *@li       APP_SUCC    		Success
 *@li       APP_FAIL 			Failure
  @author  Ashin
  @data    2018-5-14 
*/
int PubShowMenuItems(char *pszTitle, char **ppMenuItems, int nMenuItemNum, int *pnSelectItem, int *pnStartItem, int nTimeout);

int PubShowMenuItems_Ext(char *pszTitle, char **ppMenuItems, int nMenuItemNum, int *pnSelectItem, int *pnStartItem, int nTimeout);

/**
* @fn PubStrCopy
* @brief  Smart copy funtion. Used to avoid half char of non-English string.
* @param in  scr: 	Source string
* @param in  AscLen:Length of source string
* @param out Dst:	Buffer to store result
* @return
* @li  Length of result
*/
int PubStrCopy(char *psDst, const char *psSrc, int nLen);

/**
* @brief show buttons
* @param [in] nButton_attr   button attr (normal/confirm...)
* @param [in] pszButton1/2/3/4 first/second/third.. button name
* @return 
* @li APP_SUCC    Success
*/
int PubShowButton(EM_BUTTON_ATTR nButton_attr, char *pszButton1, char *pszButton2, char *pszButton3, char *pszButton4);

/**
* @brief Prompt dialog box
* @param [in] nButton_attr button attr
* @param [in] pszButton1/pszButton2/pszButton3/pszButton3  button key name
* @param [in] nTimeOut   Timeout : second
* @return 
* @ the button num = 1, return key_enter
* @ the button num = 2, return key_esc key_enter
* @ the button num = 3, return key_F1 key_esc key_F2
* @ the button num = 4, return key_up/key_esc/key_enter/key_down
* @li APP_TIMEOUT 0: Block; Non-zero: Timeout in milliseconds
* @li APP_SUCC    Success
*/
int PubShowGetKbPad(int nTimeout, EM_BUTTON_ATTR nButton_attr, char *pszButton1, char *pszButton2, char *pszButton3, char *pszButton4);

/**
* @brief Prompt dialog box
* @param [in] nButton_attr button attr
* @param [in] pszButton1/pszButton2/pszButton3/pszButton3  button key name
* @param [in] nTimeOut   Timeout : milliseconds
* @return 
* @ the button num = 1, return key_enter
* @ the button num = 2, return key_esc key_enter
* @ the button num = 3, return key_F1 key_esc key_F2
* @ the button num = 4, return key_up/key_esc/key_enter/key_down
* @li APP_TIMEOUT 0: Block; Non-zero: Timeout in milliseconds
* @li APP_SUCC    Success
*/
int PubShowGetKbPad_Ms(int nMilliseconds, EM_BUTTON_ATTR nButton_attr, char *pszButton1, char *pszButton2, char *pszButton3, char *pszButton4);

/**
* @brief not key board, jsut Get touch value before time is out. Timeout(>0) can be set, set 0 for blocking unless any key is pressed.
* @param [in] nTimeout	
* @return 
* @li 0 - any touch or APP_TIMEOUT when time is out
*/
int PubWaitGetKbPad(int nTimeout);

/** 
* @brief Get touch value before time is out. Timeout(>0) can be set, set 0 for blocking unless any key is pressed.
* @param [in] nTimeout	
* @return KEY_ENTER or KEY_ESC
* @li 
*/
int PubWaitConfirm(int nTimeout);

/** 
* @brief Set keyboard attr
* @param [in] Virtual and physical keyboards	
* @return 
* @li 
*/
void PubSetKbAttr(EM_KEYBOARD_ATTR emKbAttr);

/** 
* @brief Get keyboard attr
* @param 
* @return Virtual and physical keyboards	
* @li 
*/
EM_KEYBOARD_ATTR PubGetKbAttr();

/** @}*/ // End of MessageBox

/** @}*/ // End of UI
#endif

