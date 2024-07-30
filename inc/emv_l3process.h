#ifndef _EMVL3PROCESS_H_
#define _EMVL3PROCESS_H_

#define CONFIG_PATH    ""

#define SERVICECODE_ATM_103			"103"

extern int PerformTransaction(char *, STSYSTEM *, int *);
extern int CompleteTransaction(char* , int , STSYSTEM* , STREVERSAL* , int );
extern int GetServiceCodeFromTk2(const char *, char *);

#endif

