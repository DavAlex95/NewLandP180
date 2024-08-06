#ifndef _EMVL3PROCESS_H_
#define _EMVL3PROCESS_H_

#define CONFIG_PATH    ""

#define SERVICECODE_ATM_103			"103"

enum TRANSMODE {
  TXN_FULLEMV = 0x01,
  TXN_FULLEMV_AFTERC50 = 0x02,
  TXN_READPAN = 0x03,
};

extern int PerformTransaction(char *, STSYSTEM *, int *, char);
extern int CompleteTransaction(char* , int , STSYSTEM* , STREVERSAL* , int );
extern int GetServiceCodeFromTk2(const char *, char *);

#endif

