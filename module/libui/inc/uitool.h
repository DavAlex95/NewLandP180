#ifndef _TOOL_H_
#define _TOOL_H_


typedef enum 
{
	FONTMODE_NORMAIL = 0 ,	
	FONTMODE_AMOUNT ,			
	FONTMODE_TITLE,				
	FONTMODE_BACKGROUNT			
}EM_FONTMODE;


#define NORMAL_TEXT		0
#define REVERSE_TEXT	1
#define BIGFONT			0
#define SMALLFONT		1
#define NOFONT			-1
#define FALSE 0
#define TRUE (!FALSE)


#define  POS_WHITE_CHAR 0x20
#define  GUI_AMOUNT_MAXLEN 13

int DispBigAscStr(int x, int y, const char *pszStr, int nNum);
void DispBigASC(int x, int y, char c);
int UI_ChkIsUpDownMenu();

#endif

