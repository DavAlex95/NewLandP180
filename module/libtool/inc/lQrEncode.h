// QR_Encode.h : CQR_Encode
#ifndef _QR_ENCODE_H_
#define _QR_ENCODE_H_

#define QR_LEVEL_L	0
#define QR_LEVEL_M	1
#define QR_LEVEL_Q	2
#define QR_LEVEL_H	3


#define QR_MODE_NUMERAL		0
#define QR_MODE_ALPHABET	1
#define QR_MODE_8BIT		2
#define QR_MODE_KANJI		3


#define QR_VRESION_S	0 // 1 - 9
#define QR_VRESION_M	1 // 10 - 26
#define QR_VRESION_L	2 // 27 - 40

#define MAX_ALLCODEWORD	 3706
#define MAX_DATACODEWORD 2956
#define MAX_CODEBLOCK	  153
#define MAX_MODULESIZE	  177

#define N_TIMES		4	//打印放大倍数

#define min(a,b)    (((a) < (b)) ? (a) : (b))

typedef struct tagRS_BLOCKINFO
{
	int ncRSBlock;
	int ncAllCodeWord;
	int ncDataCodeWord;

} RS_BLOCKINFO;

typedef struct tagQR_VERSIONINFO
{
	int nVersionNo;	   // 1-40
	int ncAllCodeWord;

	// (0 = L, 1 = M, 2 = Q, 3 = H)
	int ncDataCodeWord[4];

	int ncAlignPoint;
	int nAlignPoint[6];

	RS_BLOCKINFO RS_BlockInfo1[4];
	RS_BLOCKINFO RS_BlockInfo2[4];

} QR_VERSIONINFO;

#endif

