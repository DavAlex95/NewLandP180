#ifndef _VIRTUALPAD_H_
#define _VIRTUALPAD_H_

typedef enum {
	KB_MODE_DIGITAL = 0,
	KB_MODE_LETTER = 1,
} EM_KB_MODE;

extern int Virtual_KbInit();
extern int Virtual_KbHit();
extern int Virtual_KbCreate(char *pszKeySeq, int nEditMask);
extern int Virtual_KbGetCode(int nTimeOut);

#endif

