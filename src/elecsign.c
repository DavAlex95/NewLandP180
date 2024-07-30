#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libapiinc.h"
#include "elecsign.h"
#include "lui.h"
#include "ldisp.h"

#define SIGNZONE240_HEIGHT 140  // sign screen size 320*140
#define SIGNZONE480_HEIGHT 160  // sign screen size 320*480

char buffer[320*480*2] = {0};

typedef unsigned long       DWORD;
typedef unsigned short      WORD;
typedef long LONG;

typedef struct {
 unsigned short bfType;
 unsigned int    bfSize;
 unsigned short  bfReserved1;
 unsigned short  bfReserved2;
 unsigned int    bfOffBits;
}__attribute__((packed))FileHead;

typedef struct{
 unsigned int   biSize;
 int    biWidth;
 int         biHeight;
 unsigned short biPlanes;
 unsigned short  biBitCount;
 unsigned int    biCompress;
 unsigned int    biSizeImage;
 int         biXPelsPerMeter;
 int         biYPelsPerMeter;
 unsigned int  biClrUsed;
 unsigned int    biClrImportant;
}__attribute__((packed))Infohead;

typedef struct tagRGBQUAD {
char    rgbBlue;
char    rgbGreen;
char    rgbRed;
char    rgbReserved;
} RGBQUAD;


static int unpack_rgb565(unsigned char* pixel, 
    unsigned char* r, unsigned char* g, unsigned char* b)
{
    unsigned short color = *(unsigned short*)pixel;
    *r = ((color >> 11) & 0xff) << 3;
    *g = ((color >> 5) & 0xff)  << 2;
    *b = (color & 0xff )<< 3;

    return 0;
}

static int DrawConfirmButton()
{
	int nMaxLine;
	uint unX, unY, unScrWidth, unScrHeight;
	int nGap = 2, nButWidth;
	int nTxtWidth, nTxtHeight, x, y, nSelectLine, nColumnNum = 0, nSpace;
	int nFontSize, nLineSpacing, nLineHeight;
	int nOff = 0;
	char *pszButton1 = "CANCEL";
	char *pszButton2 = "RESIGN";
	char *pszButton3 = "ENTER";

	PubGetDispForm(&nFontSize, &nLineSpacing, &nLineHeight);
	
	PubGetDispView(&nMaxLine, NULL);
	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth,&unScrHeight);
	if (pszButton1 == NULL) {
		return APP_QUIT;
	}

	if (unScrHeight > 240) {
		nColumnNum = 3;
		nSelectLine = 2;

	} else {
		nColumnNum = 3;
		nSelectLine = 1;
	}

	nButWidth = (unScrWidth - (nColumnNum+1)*nGap) / nColumnNum;
	nSpace = (unScrWidth - (nColumnNum+1)*nGap) % nColumnNum;

	if (pszButton1 != NULL)
	{
		nOff = nGap;
		NAPI_ScrRectangle(nOff, nLineHeight*(nMaxLine-nSelectLine), nButWidth, nLineHeight*nSelectLine, RECT_PATTERNS_SOLID_FILL, COLOR_YELLOW);
		nOff += nButWidth + nGap;
		x = 0;
		NAPI_ScrGetTrueTypeFontSize(pszButton1, strlen(pszButton1), &nTxtWidth, &nTxtHeight);
		y = nLineHeight*(nMaxLine-nSelectLine) + (nLineHeight*nSelectLine-nTxtHeight)/2;
		DispBoldFontText((nButWidth-nTxtWidth)/2, y, pszButton1);
	}

	if (pszButton2 != NULL)
	{
		nButWidth += nSpace;
		NAPI_ScrRectangle(nOff, nLineHeight*(nMaxLine-nSelectLine), nButWidth, nLineHeight*nSelectLine, RECT_PATTERNS_SOLID_FILL, COLOR_GREEN);
		x = nButWidth + nGap;
		NAPI_ScrGetTrueTypeFontSize(pszButton2, strlen(pszButton2), &nTxtWidth, &nTxtHeight);
		y = nLineHeight*(nMaxLine-nSelectLine) + (nLineHeight*nSelectLine-nTxtHeight)/2;
		DispBoldFontText((nButWidth-nTxtWidth)/2+x, y, pszButton2);
		nOff += nButWidth + nGap;
	}

	if (pszButton3 != NULL)
	{
		nButWidth += nSpace;
		NAPI_ScrRectangle(nOff, nLineHeight*(nMaxLine-nSelectLine), nButWidth, nLineHeight*nSelectLine, RECT_PATTERNS_SOLID_FILL, COLOR_RED);
		x += nButWidth + nGap;
		NAPI_ScrGetTrueTypeFontSize(pszButton3, strlen(pszButton3), &nTxtWidth, &nTxtHeight);
		y = nLineHeight*(nMaxLine-nSelectLine) + (nLineHeight*nSelectLine-nTxtHeight)/2;
		DispBoldFontText((nButWidth-nTxtWidth)/2+x, y, pszButton3);
		nOff += nButWidth + nGap;
	}

	PubUpdateWindow();
	return APP_SUCC;
}

void initSignbuf()
{
	memset(buffer, 0xFF, sizeof(buffer)); // white
}

void SaveSignBuff(int x1, int x2, int y1, int y2, ushort color)
{
    int dx, dy, x, y, e;

    (x1 > x2) ? (dx = x1 - x2) : (dx = x2 - x1);
    (y1 > y2) ? (dy = y1 - y2) : (dy = y2 - y1);
    if (((dx > dy) && (x1 > x2)) || ((dx <= dy) && (y1 > y2)))
    {
        x = x2; y = y2;
        x2 = x1; y2 = y1;
        x1 = x; y1 = y;
    }
    x = x1; y = y1;
    if (dx > dy)
    {
        e = dy - dx / 2;
        for (; x1 <= x2; ++x1, e += dy)
        {
            ((unsigned short *)buffer)[x1 + (y1) *320] = color;

            if (e > 0) { e -= dx; (y > y2) ? --y1 : ++y1; }
        }
    }
    else
    {
        e = dx - dy / 2;
        for (; y1 <= y2; ++y1, e += dx)
        {
            ((unsigned short *)buffer)[x1 + (y1) *320] = color;
            if (e > 0) { e -= dy; (x > x2) ? --x1 : ++x1; }
        }
    }
}

static void draw_width_line(int StartX, int StartY, int EndX, int EndY)
{
	// 2 * 2
	SaveSignBuff(StartX, 	EndX, 	  StartY,	  EndY, 	0);
	SaveSignBuff(StartX+1, 	EndX+1,	  StartY, 	  EndY, 	0);
	SaveSignBuff(StartX+1, 	EndX+1,	  StartY+1,   EndY+1, 	0);
	SaveSignBuff(StartX, 	EndX,	  StartY+1,   EndY+1, 	0);

	NAPI_ScrLine(StartX,	StartY, 	EndX  ,   EndY, 	0);
	NAPI_ScrLine(StartX+1, 	StartY, 	EndX+1,	  EndY, 	0);
	NAPI_ScrLine(StartX+1, 	StartY+1, 	EndX+1,	  EndY+1, 	0);
	NAPI_ScrLine(StartX, 	StartY+1, 	EndX,	  EndY+1, 	0);

	// 3 * 3
#if 0
	SaveSignBuff(StartX+2,	EndX+2,   StartY,	  EndY, 	0);
	SaveSignBuff(StartX+2,	EndX+2,   StartY+1,   EndY+1,	0);
	SaveSignBuff(StartX+2,	EndX+2,   StartY+2,   EndY+2,	0);
	SaveSignBuff(StartX,	EndX,	  StartY+2,   EndY+2,	0);
	SaveSignBuff(StartX+1,	EndX+1,   StartY+2,   EndY+2,	0);

	NAPI_ScrLine(StartX+2, 	StartY, 	EndX+2,	  EndY, 	0);
	NAPI_ScrLine(StartX+2, 	StartY+1, 	EndX+2,	  EndY+1, 	0);
	NAPI_ScrLine(StartX+2, 	StartY+2, 	EndX+2,	  EndY+2, 	0);
	NAPI_ScrLine(StartX, 	StartY+2, 	EndX,	  EndY+2, 	0);
	NAPI_ScrLine(StartX+1, 	StartY+2, 	EndX+1,	  EndY+2, 	0);
#endif
}

int Bmp24toOne(char *pszSrc, char *pszDst, unsigned int width)
{
	int dst_width, j, k, offset;
	int pxl = 0;
//	int nRow_width = ((width >> 3) + 3) & ~3;;

//	printf("nRow_width = %d", nRow_width);
	dst_width = width >> 3;
	offset = 0;
	for(j = 0; j < dst_width; j++)
	{   
		pxl = 0;
		for(k = 7; k >= 0;k--)
		{
			if(*pszSrc > 128)
			{
				pxl = pxl | (0x01 << k);
			}
			pszSrc += 3;
			offset += 1;
			if(offset >= width)
				break;
		}
		pszDst[j] = pxl;
	}

	return 0;
}

int SignData2Bmp(const char *filename, unsigned int width, unsigned int height, int nOffset, unsigned char *data)
{
	FileHead bmp_head;
	Infohead bmp_info;
	RGBQUAD quad[2];
	char szBit1Buff[512] = {0};
	int biBitCount = 1;
	int size = width * height * biBitCount/8;
	int i, j, offset;
	unsigned char* line = NULL;
	FILE *fp;

	//initialize bmp head.
	bmp_head.bfType = 0x4d42;
	bmp_head.bfSize = size + sizeof(FileHead) + sizeof(Infohead) + 2 * sizeof(RGBQUAD);
	bmp_head.bfReserved1 = bmp_head.bfReserved2 = 0;
	bmp_head.bfOffBits = bmp_head.bfSize - size;

	//initialize bmp info.
	bmp_info.biSize = 40;
	bmp_info.biWidth = width;
	bmp_info.biHeight = height;
	bmp_info.biPlanes = 1;
	bmp_info.biBitCount = biBitCount;
	bmp_info.biCompress = 3;
	bmp_info.biSizeImage = size;
	bmp_info.biXPelsPerMeter = 0;
	bmp_info.biYPelsPerMeter = 0;
	bmp_info.biClrUsed = 0;
	bmp_info.biClrImportant = 0;

	//copy the data
	if(!(fp=fopen(filename,"wb")))
	{
		return -1;
	}

	fwrite(&bmp_head, 1, sizeof(FileHead),fp);
	fwrite(&bmp_info, 1, sizeof(Infohead),fp);
	quad[0].rgbBlue = 0;
	quad[0].rgbGreen = 0;
	quad[0].rgbRed = 0;
	quad[0].rgbReserved = 0;
	fwrite(&quad[0], 1, sizeof(RGBQUAD),fp);

	quad[1].rgbBlue = 255;
	quad[1].rgbGreen = 255;
	quad[1].rgbRed = 255;
	quad[1].rgbReserved = 255;
	fwrite(&quad[1], 1, sizeof(RGBQUAD),fp);

	char *image_buffer = malloc(3 * width);
	if (image_buffer == NULL) {
		printf("malloc fail");
		return -1;
	}
    for (i = (height - 1); i >= 0; --i)
    {
    	memset(szBit1Buff, 0, sizeof(szBit1Buff));
        line = data + (i + nOffset) * width * 2;
        for(j = 0; j < width; j++, line += 2)
        {
            offset = j * 3;
            unpack_rgb565(line, (uchar *)image_buffer+offset, (uchar *)image_buffer+offset+1, (uchar *)image_buffer+offset+2);
		}
		Bmp24toOne(image_buffer, szBit1Buff, width);
        fwrite(szBit1Buff, 1, width/8, fp);
    }
	free(image_buffer);
	image_buffer = NULL;
	fclose(fp);

	return 0;
}

int ElecSign(char *pszTrace)
{
	ST_PADDATA stPaddata;
	int nDistanceX = 0xFFFF, nDistanceY = 0xFFFF;
	int AutoFlag;
	int nStartHeight;
	int nHeight;
	int nY;
	char szFileName[32] = {0};
	int nLineHeight;
	uint unX, unY, unScrWidth, unScrHeight;
	int nButWidth, nButtonHeight, nGap = 2, nLine;
	int nKey;

	initSignbuf();
	NAPI_ScrGetViewPort(&unX, &unY, &unScrWidth, &unScrHeight);
	PubGetDispForm(NULL, NULL, &nLineHeight);

	if (unScrHeight > 240) {
		nLine = 4;
		nStartHeight = 160;
		nHeight = SIGNZONE480_HEIGHT;
		nButtonHeight = nLineHeight * 2;
	} else {
		nLine = 1;
		nStartHeight = nLineHeight+2;
		nHeight = SIGNZONE240_HEIGHT;
		nButtonHeight = nLineHeight * 1;
	}

	nButWidth = (unScrWidth - (2+1)*nGap) / 3;
	PubClearAll();

	// drawing
	PubDisplayStrInline(DISPLAY_MODE_CLEARLINE|DISPLAY_MODE_CENTER, nLine, "please sign below");
	NAPI_ScrRectangle(0, nStartHeight-1, 320, 0, RECT_PATTERNS_SOLID_FILL, COLOR_BLACK);
	NAPI_ScrRectangle(0, nStartHeight+nHeight+1, 320, 0, RECT_PATTERNS_SOLID_FILL, COLOR_BLACK);
	DrawConfirmButton();
	PubUpdateWindow();

	NAPI_ScrAutoUpdate(1,&AutoFlag);
	memset(&stPaddata, 0, sizeof(ST_PADDATA));
	stPaddata.unPadX = 0xFFFF;
	stPaddata.unPadY = 0xFFFF;
	NAPI_KbFlush();
	while (1) {
		nKey = 0x00;
		nDistanceX = stPaddata.unPadX;
		nDistanceY = stPaddata.unPadY;
		if(NAPI_KbGetPad(&stPaddata, 60*1000) == NAPI_ERR_TIMEOUT)
		{
			NAPI_ScrAutoUpdate(0, &AutoFlag);
			return APP_TIMEOUT;
		}

		if (stPaddata.emPadState == PADSTATE_KEY) {
			nKey = stPaddata.unKeycode;
		} else {
			nY = stPaddata.unPadY;
			if (nY <=0) {
				continue;
			}
			if (stPaddata.unPadY > nStartHeight && stPaddata.unPadY < nStartHeight + nHeight) { // sign area
				if (nDistanceX == 0xFFFF && nDistanceY == 0xFFFF) {
					nDistanceX = stPaddata.unPadX;
					nDistanceY = stPaddata.unPadY;
				}
				
				draw_width_line(nDistanceX, nDistanceY, stPaddata.unPadX, stPaddata.unPadY);
				if (stPaddata.emPadState == PADSTATE_UP) {
					stPaddata.unPadX = 0xFFFF;
					stPaddata.unPadY = 0xFFFF;
				}
				continue;
			}
			if (stPaddata.unPadY >= unScrHeight-nButtonHeight) { // button area
				if(stPaddata.unPadX < nButWidth + nGap) {
					nKey = KEY_ESC;
				} else if(stPaddata.unPadX < 2*nButWidth + nGap) {
					nKey = KEY_STAR;
				} else {
					nKey = KEY_ENTER;
				}
			}
		}

		if(nKey == KEY_STAR) {
			initSignbuf();
			NAPI_ScrClrLine(nStartHeight, nStartHeight+nHeight);
			PubUpdateWindow();
			stPaddata.unPadX = 0xFFFF;
			stPaddata.unPadY = 0xFFFF;
			continue;
 		} else if(nKey == KEY_ESC) {
			break;
		} else if(nKey == KEY_ENTER) {
			sprintf(szFileName, "%s/%s.bmp", ELECSIGN_PATH, pszTrace);
			SignData2Bmp(szFileName, 320, nHeight, nStartHeight, (uchar *)buffer);
			break;
		}
		stPaddata.unPadX = 0xFFFF;
		stPaddata.unPadY = 0xFFFF;
		PubDebug("unSWidth = %d %d %d %d\n", stPaddata.unPadX, stPaddata.unPadY, stPaddata.emPadState, nKey);
	}
	NAPI_ScrAutoUpdate(0, &AutoFlag);

	return APP_SUCC;
}


