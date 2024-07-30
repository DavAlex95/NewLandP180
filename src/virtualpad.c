#include "libapiinc.h"
#include "ldisp.h"

#define BUTTON_GAP		0x02
#define	DISP_BLACK 0x0000
#define DARK_BLUE 0x000F
#define SKY_BLUE 0x7D5C
#define LIGHT_GREY 0xC618//0x7BEF

#define VIRTUAL_TRACE(fmt,args...) PubDebug("[%s][%s][%d]>>>"""fmt,__FILE__, __FUNCTION__, __LINE__,##args)


#define REC_WIDTH	(320 - 4*BUTTON_GAP) / 4 // 78
#define REC_HEIGHT	(240 - 4*BUTTON_GAP) / 4 // 58

#define REC_LETTERWIDTH	    (320 - 7*BUTTON_GAP) / 6 // 78
#define REC_LETTERHEIGHT	50

const char gKeySeq[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
static EM_KB_MODE gcKbMode = KB_MODE_DIGITAL;
static char gcSupportKbSwitch = NO;

button_info buttons_Letter_init[] = {
	{"",	 {0x00,	 {{1*BUTTON_GAP+0*REC_LETTERWIDTH, 186+1*BUTTON_GAP}, {1*(REC_LETTERWIDTH+BUTTON_GAP), 186 + BUTTON_GAP+REC_LETTERHEIGHT}}}},
	{"",	 {0x00,	 {{2*BUTTON_GAP+1*REC_LETTERWIDTH, 186+1*BUTTON_GAP}, {2*(REC_LETTERWIDTH+BUTTON_GAP), 186 + BUTTON_GAP+REC_LETTERHEIGHT}}}},
	{"",	 {0x00,	 {{3*BUTTON_GAP+2*REC_LETTERWIDTH, 186+1*BUTTON_GAP}, {3*(REC_LETTERWIDTH+BUTTON_GAP), 186 + BUTTON_GAP+REC_LETTERHEIGHT}}}},
	{"",	 {0x00,	 {{4*BUTTON_GAP+3*REC_LETTERWIDTH, 186+1*BUTTON_GAP}, {4*(REC_LETTERWIDTH+BUTTON_GAP), 186 + BUTTON_GAP+REC_LETTERHEIGHT}}}},
	{"",	 {0x00,	 {{5*BUTTON_GAP+4*REC_LETTERWIDTH, 186+1*BUTTON_GAP}, {5*(REC_LETTERWIDTH+BUTTON_GAP), 186 + BUTTON_GAP+REC_LETTERHEIGHT}}}},
	{"",	 {0x00,	 {{6*BUTTON_GAP+5*REC_LETTERWIDTH, 186+1*BUTTON_GAP}, {6*(REC_LETTERWIDTH+BUTTON_GAP), 186 + BUTTON_GAP+REC_LETTERHEIGHT}}}},

	{"QZ",     {KEY_1,     {{1*BUTTON_GAP+0*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}},//(2, 242) (80, 300) (0x02,0x00,0xF2,0x00,0x50,0x00,0x2C,0x01,)
	{"ABC",     {KEY_2,     {{2*BUTTON_GAP+1*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}},//(82,242) (160,300) (0x52,0x00,0xF2,0x01,0xA0,0x00,0x2C,0x01,)
	{"DEF",     {KEY_3,     {{3*BUTTON_GAP+2*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}},//(162,242)(240,300) (0xA2,0x00,0xF2,0x01,0xF0,0x00,0x2C,0x01,)
	{"ESC",   	{KEY_ESC,  {{4*BUTTON_GAP+3*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}}, //(242,242)(320,300) (0xF2,0x00,0xF2,0x00,0x40,0x01,0x2C,0x01,)

	{"GHI",     {KEY_4,    {{1*BUTTON_GAP+0*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},//(2, 302) (80, 360)  (0x02,0x00,0x2E,0x01,0x50,0x00,0x68,0x01,)
	{"JKL",     {KEY_5,    {{2*BUTTON_GAP+1*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},//(82,302) (160, 360) (0x52,0x00,0x2E,0x01,0xA0,0x00,0x68,0x01,)
	{"MNO",     {KEY_6,    {{3*BUTTON_GAP+2*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},//(162,302) (240, 360)(0xA2,0x00,0x2E,0x01,0xF0,0x00,0x68,0x01,)
	{"Clear", {KEY_BACK, {{4*BUTTON_GAP+3*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},//(242,302) (320, 360)(0xF2,0x00,0x2E,0x01,0x40,0x01,0x68,0x01)

	{"PRS",     {KEY_7,    {{1*BUTTON_GAP+0*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},//(2, 362) (80, 420)  (0x02,0x00,0x6A,0x01,0x50,0x00,0xA4,0x01,)
	{"TUV",     {KEY_8,    {{2*BUTTON_GAP+1*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},//(82, 362) (160, 420) (0x52,0x00,0x6A,0x01,0xA0,0x00,0xA4,0x01,)
	{"WXY",     {KEY_9,    {{3*BUTTON_GAP+2*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},//(162, 362) (240, 420)(0xA2,0x00,0x6A,0x01,0xF0,0x00,0xA4,0x01,)
	{"Enter", {KEY_ENTER,{{4*BUTTON_GAP+3*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},//(242, 362) (320, 420)(0xF2,0x00,0x6A,0x01,0x40,0x01,0xA4,0x01)

	{"*",     {KEY_STAR, {{1*BUTTON_GAP+0*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},//(2, 422) (80, 480)  (0x02,0x00,0xA6,0x01,0x50,0x00,0xE0,0x01,)
	{"0",     {KEY_0,    {{2*BUTTON_GAP+1*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},//(82, 422) (160, 480) (0x52,0x00,0xA6,0x01,0xA0,0x00,0xE0,0x01,)
	{"#",     {KEY_SHARP,{{3*BUTTON_GAP+2*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},//(162, 422) (240, 480)(0xA2,0x00,0xA6,0x01,0xF0,0x00,0xE0,0x01,)
	{"123",   {0xFF,     {{4*BUTTON_GAP+3*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},//(242, 422) (320, 480) (0xF2,0x00,0xA6,0x01,0x40,0x01,0xE0,0x01)
};

button_info buttons_init[] = {
	{"1",     {KEY_1,     {{1*BUTTON_GAP+0*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}},
	{"2",     {KEY_2,     {{2*BUTTON_GAP+1*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}},
	{"3",     {KEY_3,     {{3*BUTTON_GAP+2*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}},
	{"ESC",   {KEY_ESC,  {{4*BUTTON_GAP+3*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}},
	{"4",     {KEY_4,    {{1*BUTTON_GAP+0*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},
	{"5",     {KEY_5,    {{2*BUTTON_GAP+1*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},
	{"6",     {KEY_6,    {{3*BUTTON_GAP+2*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},
	{"Clear", {KEY_BACK, {{4*BUTTON_GAP+3*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},

	{"7",     {KEY_7,    {{1*BUTTON_GAP+0*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},
	{"8",     {KEY_8,    {{2*BUTTON_GAP+1*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},
	{"9",     {KEY_9,    {{3*BUTTON_GAP+2*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},
	{"Enter", {KEY_ENTER,{{4*BUTTON_GAP+3*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},

	{"*",     {KEY_STAR, {{1*BUTTON_GAP+0*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},
	{"0",     {KEY_0,    {{2*BUTTON_GAP+1*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},
	{"#",     {KEY_SHARP,{{3*BUTTON_GAP+2*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},
	{"",   {0x00,     {{4*BUTTON_GAP+3*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},
};

// digital key
button_info buttons[] = {
	{"1",     {KEY_1,     {{1*BUTTON_GAP+0*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}},//(2, 242) (80, 300) (0x02,0x00,0xF2,0x00,0x50,0x00,0x2C,0x01,)
	{"2",     {KEY_2,     {{2*BUTTON_GAP+1*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}},//(82,242) (160,300) (0x52,0x00,0xF2,0x01,0xA0,0x00,0x2C,0x01,)
	{"3",     {KEY_3,     {{3*BUTTON_GAP+2*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}},//(162,242)(240,300) (0xA2,0x00,0xF2,0x01,0xF0,0x00,0x2C,0x01,)
	{"ESC",   {KEY_ESC,  {{4*BUTTON_GAP+3*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}}, //(242,242)(320,300) (0xF2,0x00,0xF2,0x00,0x40,0x01,0x2C,0x01,)

	{"4",     {KEY_4,    {{1*BUTTON_GAP+0*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},//(2, 302) (80, 360)  (0x02,0x00,0x2E,0x01,0x50,0x00,0x68,0x01,)
	{"5",     {KEY_5,    {{2*BUTTON_GAP+1*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},//(82,302) (160, 360) (0x52,0x00,0x2E,0x01,0xA0,0x00,0x68,0x01,)
	{"6",     {KEY_6,    {{3*BUTTON_GAP+2*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},//(162,302) (240, 360)(0xA2,0x00,0x2E,0x01,0xF0,0x00,0x68,0x01,)
	{"Clear", {KEY_BACK, {{4*BUTTON_GAP+3*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},//(242,302) (320, 360)(0xF2,0x00,0x2E,0x01,0x40,0x01,0x68,0x01)

	{"7",     {KEY_7,    {{1*BUTTON_GAP+0*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},//(2, 362) (80, 420)  (0x02,0x00,0x6A,0x01,0x50,0x00,0xA4,0x01,)
	{"8",     {KEY_8,    {{2*BUTTON_GAP+1*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},//(82, 362) (160, 420) (0x52,0x00,0x6A,0x01,0xA0,0x00,0xA4,0x01,)
	{"9",     {KEY_9,    {{3*BUTTON_GAP+2*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},//(162, 362) (240, 420)(0xA2,0x00,0x6A,0x01,0xF0,0x00,0xA4,0x01,)
	{"Enter", {KEY_ENTER,{{4*BUTTON_GAP+3*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},//(242, 362) (320, 420)(0xF2,0x00,0x6A,0x01,0x40,0x01,0xA4,0x01)

	{"*",     {KEY_STAR, {{1*BUTTON_GAP+0*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},//(2, 422) (80, 480)  (0x02,0x00,0xA6,0x01,0x50,0x00,0xE0,0x01,)
	{"0",     {KEY_0,    {{2*BUTTON_GAP+1*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},//(82, 422) (160, 480) (0x52,0x00,0xA6,0x01,0xA0,0x00,0xE0,0x01,)
	{"#",     {KEY_SHARP,{{3*BUTTON_GAP+2*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},//(162, 422) (240, 480)(0xA2,0x00,0xA6,0x01,0xF0,0x00,0xE0,0x01,)
	{"",   {0x00,     {{4*BUTTON_GAP+3*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},//(242, 422) (320, 480) (0xF2,0x00,0xA6,0x01,0x40,0x01,0xE0,0x01)
};

// letter key
button_info buttons_Letter[] = {
	{"QZ",     {KEY_1,     {{1*BUTTON_GAP+0*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}},//(2, 242) (80, 300) (0x02,0x00,0xF2,0x00,0x50,0x00,0x2C,0x01,)
	{"ABC",     {KEY_2,     {{2*BUTTON_GAP+1*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}},//(82,242) (160,300) (0x52,0x00,0xF2,0x01,0xA0,0x00,0x2C,0x01,)
	{"DEF",     {KEY_3,     {{3*BUTTON_GAP+2*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}},//(162,242)(240,300) (0xA2,0x00,0xF2,0x01,0xF0,0x00,0x2C,0x01,)
	{"ESC",   	{KEY_ESC,  {{4*BUTTON_GAP+3*REC_WIDTH, 240+1*BUTTON_GAP+0*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 1*(REC_HEIGHT+BUTTON_GAP)}}}}, //(242,242)(320,300) (0xF2,0x00,0xF2,0x00,0x40,0x01,0x2C,0x01,)

	{"GHI",     {KEY_4,    {{1*BUTTON_GAP+0*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},//(2, 302) (80, 360)  (0x02,0x00,0x2E,0x01,0x50,0x00,0x68,0x01,)
	{"JKL",     {KEY_5,    {{2*BUTTON_GAP+1*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},//(82,302) (160, 360) (0x52,0x00,0x2E,0x01,0xA0,0x00,0x68,0x01,)
	{"MNO",     {KEY_6,    {{3*BUTTON_GAP+2*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},//(162,302) (240, 360)(0xA2,0x00,0x2E,0x01,0xF0,0x00,0x68,0x01,)
	{"Clear", {KEY_BACK, {{4*BUTTON_GAP+3*REC_WIDTH, 240+2*BUTTON_GAP+1*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 2*(REC_HEIGHT+BUTTON_GAP)}}}},//(242,302) (320, 360)(0xF2,0x00,0x2E,0x01,0x40,0x01,0x68,0x01)

	{"PRS",     {KEY_7,    {{1*BUTTON_GAP+0*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},//(2, 362) (80, 420)  (0x02,0x00,0x6A,0x01,0x50,0x00,0xA4,0x01,)
	{"TUV",     {KEY_8,    {{2*BUTTON_GAP+1*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},//(82, 362) (160, 420) (0x52,0x00,0x6A,0x01,0xA0,0x00,0xA4,0x01,)
	{"WXY",     {KEY_9,    {{3*BUTTON_GAP+2*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},//(162, 362) (240, 420)(0xA2,0x00,0x6A,0x01,0xF0,0x00,0xA4,0x01,)
	{"Enter", {KEY_ENTER,{{4*BUTTON_GAP+3*REC_WIDTH, 240+3*BUTTON_GAP+2*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 3*(REC_HEIGHT+BUTTON_GAP)}}}},//(242, 362) (320, 420)(0xF2,0x00,0x6A,0x01,0x40,0x01,0xA4,0x01)

	{"*",     {KEY_STAR, {{1*BUTTON_GAP+0*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {1*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},//(2, 422) (80, 480)  (0x02,0x00,0xA6,0x01,0x50,0x00,0xE0,0x01,)
	{"0",     {KEY_0,    {{2*BUTTON_GAP+1*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {2*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},//(82, 422) (160, 480) (0x52,0x00,0xA6,0x01,0xA0,0x00,0xE0,0x01,)
	{"#",     {KEY_SHARP,{{3*BUTTON_GAP+2*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {3*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},//(162, 422) (240, 480)(0xA2,0x00,0xA6,0x01,0xF0,0x00,0xE0,0x01,)
	{"123",   {0xFF,     {{4*BUTTON_GAP+3*REC_WIDTH, 240+4*BUTTON_GAP+3*REC_HEIGHT}, {4*(REC_WIDTH+BUTTON_GAP), 240 + 4*(REC_HEIGHT+BUTTON_GAP)}}}},//(242, 422) (320, 480) (0xF2,0x00,0xA6,0x01,0x40,0x01,0xE0,0x01)
};

// letter key
button_info buttons_Letter_select[] = {
	{"",     {0x00,     {{1*BUTTON_GAP+0*REC_LETTERWIDTH, 186+1*BUTTON_GAP}, {1*(REC_LETTERWIDTH+BUTTON_GAP), 186 + BUTTON_GAP+REC_LETTERHEIGHT}}}},
	{"",     {0x00,     {{2*BUTTON_GAP+1*REC_LETTERWIDTH, 186+1*BUTTON_GAP}, {2*(REC_LETTERWIDTH+BUTTON_GAP), 186 + BUTTON_GAP+REC_LETTERHEIGHT}}}},
	{"",     {0x00,     {{3*BUTTON_GAP+2*REC_LETTERWIDTH, 186+1*BUTTON_GAP}, {3*(REC_LETTERWIDTH+BUTTON_GAP), 186 + BUTTON_GAP+REC_LETTERHEIGHT}}}},
	{"",     {0x00,     {{4*BUTTON_GAP+3*REC_LETTERWIDTH, 186+1*BUTTON_GAP}, {4*(REC_LETTERWIDTH+BUTTON_GAP), 186 + BUTTON_GAP+REC_LETTERHEIGHT}}}},
	{"",     {0x00,     {{5*BUTTON_GAP+4*REC_LETTERWIDTH, 186+1*BUTTON_GAP}, {5*(REC_LETTERWIDTH+BUTTON_GAP), 186 + BUTTON_GAP+REC_LETTERHEIGHT}}}},
	{"",     {0x00,     {{6*BUTTON_GAP+5*REC_LETTERWIDTH, 186+1*BUTTON_GAP}, {6*(REC_LETTERWIDTH+BUTTON_GAP), 186 + BUTTON_GAP+REC_LETTERHEIGHT}}}},
};

/*
n line   m column
width = (320 - (m + 1)*gap ) / m
height = (240 - (n + 1)*gap ) / n

top:
n line: (ngap + (n-1)width, ngap + (n-1)height)

bottom:
n column: (ngap + nwidth, ngap + nheight)

txt:
txtsize =  NAPI_ScrGetTrueTypeFontSize()
(x, y) = (width - txtsize)/2
*/
void draw_release(const button_info *button)
{
	uint16_t x, y;
	unsigned int disp_x, disp_y, disp_width, disp_height;
	int nTxtWidth, nTxtHeight;

	NAPI_ScrGetViewPort(&disp_x, &disp_y, &disp_width, &disp_height);
	x = button->keyinfo.btn.l_top.x;
	y = button->keyinfo.btn.l_top.y-disp_y;

	NAPI_ScrGetTrueTypeFontSize((char *)button->keyname, strlen((char *)button->keyname), &nTxtWidth, &nTxtHeight);

	//VIRTUAL_TRACE("btn.l_top.y %d", button->keyinfo.btn.l_top.y);
	if (button->keyinfo.btn.l_top.y >= 240+1*BUTTON_GAP) {
		NAPI_ScrRectangle(x, y, REC_WIDTH, REC_HEIGHT, RECT_PATTERNS_SOLID_FILL, SKY_BLUE);
		DispBoldFontText(x+(REC_WIDTH-nTxtWidth)/2, y+(REC_HEIGHT-nTxtHeight)/2, (char *)button->keyname);
	} else {
		NAPI_ScrRectangle(x, y, REC_LETTERWIDTH, REC_LETTERHEIGHT, RECT_PATTERNS_SOLID_FILL, SKY_BLUE);
		DispBoldFontText(x+(REC_LETTERWIDTH-nTxtWidth)/2, y+(REC_LETTERHEIGHT-nTxtHeight)/2, (char *)button->keyname);
	}
}

void draw_press(const button_info *button)
{
	uint16_t x, y;
	unsigned int disp_x, disp_y, disp_width, disp_height;
	int nTxtWidth, nTxtHeight;

	NAPI_ScrGetViewPort(&disp_x, &disp_y, &disp_width, &disp_height);
	x = button->keyinfo.btn.l_top.x;
	y = button->keyinfo.btn.l_top.y-disp_y;

	NAPI_ScrGetTrueTypeFontSize((char *)button->keyname, strlen((char *)button->keyname), &nTxtWidth, &nTxtHeight);

	if (button->keyinfo.btn.l_top.y >= 240+1*BUTTON_GAP) {
		NAPI_ScrRectangle(x, y, REC_WIDTH, REC_HEIGHT, RECT_PATTERNS_SOLID_FILL, LIGHT_GREY);
		DispBoldFontText(x+(REC_WIDTH-nTxtWidth)/2, y+(REC_HEIGHT-nTxtHeight)/2, (char *)button->keyname);
	} else {
		NAPI_ScrRectangle(x, y, REC_LETTERWIDTH, REC_LETTERHEIGHT, RECT_PATTERNS_SOLID_FILL, LIGHT_GREY);
		DispBoldFontText(x+(REC_LETTERWIDTH-nTxtWidth)/2, y+(REC_LETTERHEIGHT-nTxtHeight)/2, (char *)button->keyname);
	}
}

int Virtual_KbInit()
{
	button_operation ops;
	int nBtn_num;

	VIRTUAL_TRACE("init %s", gcKbMode == KB_MODE_DIGITAL ? "Digit keyboard" : "Letter keyboard");
	if (gcKbMode == KB_MODE_DIGITAL) {
		nBtn_num = sizeof(buttons_init) / sizeof(button_info);
		ops.draw_press = draw_press;
		ops.draw_release = draw_release;
		if (NAPI_KbVppTpInit(buttons_init, nBtn_num, &ops) != NAPI_OK)
		{
			VIRTUAL_TRACE("Failed to init virtual keypad");
			return APP_FAIL;
		}
		return APP_SUCC;
	}

	nBtn_num = sizeof(buttons_Letter_init) / sizeof(button_info);
	ops.draw_press = draw_press;
	ops.draw_release = draw_release;
	if (NAPI_KbVppTpInit(buttons_Letter_init, nBtn_num, &ops) != NAPI_OK)
	{
		VIRTUAL_TRACE("Failed to init virtual keypad");
		return APP_FAIL;
	}

	return APP_SUCC;
}

/**
* @brief Virtual_KbCreate: create key board
* @param [in] pszKeySeq :NULL : use default digital seq：1,2,3,4,5,6,7,8,9,0 --gKeySeq
* @param [in] nEditMask :EM_INPUT_MODE :string /digital/password
* @return 
* @li 
*/
int Virtual_KbCreate(char *pszKeySeq, int nEditMask)
{
	int index;
	int btn_num;
	int j;
	unsigned int disp_x, disp_y, disp_width, disp_height;

	NAPI_ScrGetViewPort(&disp_x, &disp_y, &disp_width, &disp_height);
	
	VIRTUAL_TRACE("creat %s", gcKbMode == KB_MODE_DIGITAL ? "Digit keyboard" : "Letter keyboard");
	if (nEditMask == INPUT_MODE_STRING) {
		gcSupportKbSwitch = YES;
		btn_num = sizeof(buttons) / sizeof(button_info);
		strcpy((char *)buttons[btn_num-1].keyname, "abc");
		buttons[btn_num-1].keyinfo.key = 0xFF;

		btn_num = sizeof(buttons_init) / sizeof(button_info);
		strcpy((char *)buttons_init[btn_num-1].keyname, "abc");
		buttons_init[btn_num-1].keyinfo.key = 0xFF;
	} else {
		gcKbMode = KB_MODE_DIGITAL;
		gcSupportKbSwitch = NO;
		btn_num = sizeof(buttons) / sizeof(button_info);
		strcpy((char *)buttons[btn_num-1].keyname, "");
		buttons[btn_num-1].keyinfo.key = 0x00;

		btn_num = sizeof(buttons_init) / sizeof(button_info);
		strcpy((char *)buttons_init[btn_num-1].keyname, "");
		buttons_init[btn_num-1].keyinfo.key = 0x00;
	}

	if (gcKbMode == KB_MODE_DIGITAL)
	{
		btn_num = sizeof(buttons) / sizeof(button_info);
		NAPI_ScrClrLine(buttons[0].keyinfo.btn.l_top.y-disp_y, buttons[btn_num-1].keyinfo.btn.r_bottom.y-disp_y);
		NAPI_ScrClrLine(buttons_Letter_select[0].keyinfo.btn.l_top.y-disp_y, buttons_Letter_select[0].keyinfo.btn.r_bottom.y-disp_y);

		for (index = 0, j = 0; index < btn_num; index++)
		{
			if (buttons[index].keyinfo.key >= '0' && buttons[index].keyinfo.key <= '9')
			{
				if (pszKeySeq == NULL)
				{
					sprintf((char *)buttons[index].keyname, "%d", gKeySeq[j]);
				}
				else
				{
					sprintf((char *)buttons[index].keyname, "%c", pszKeySeq[j]);
				}
				j++;
			}
			draw_release(&buttons[index]);
		}
	}
	else
	{
		btn_num = sizeof(buttons_Letter) / sizeof(button_info);
		NAPI_ScrClrLine(buttons_Letter_select[0].keyinfo.btn.l_top.y-disp_y, buttons_Letter_select[0].keyinfo.btn.r_bottom.y-disp_y);
		for (index = 0; index < btn_num; index++)
		{
			draw_release(&(buttons_Letter)[index]);
		}
	}

	NAPI_ScrRefresh();
	Virtual_KbInit();

	return APP_SUCC;
}

int ShowLetterInfo(int nKey)
{
	int index, btn_num, i;
	unsigned int disp_x, disp_y, disp_width, disp_height;

	switch (nKey)
	{
	case KEY_1:
		buttons_Letter_init[0].keyname[0] = 'q';
		buttons_Letter_init[0].keyinfo.key = KEY_q;
		buttons_Letter_init[1].keyname[0] = 'z';
		buttons_Letter_init[1].keyinfo.key = KEY_z;
		buttons_Letter_init[2].keyname[0] = 'Q';
		buttons_Letter_init[2].keyinfo.key = KEY_Q;
		buttons_Letter_init[3].keyname[0] = 'Z';
		buttons_Letter_init[3].keyinfo.key = KEY_Z;
		buttons_Letter_init[4].keyname[0] = 0x00;
		buttons_Letter_init[4].keyinfo.key = 0x00;
		buttons_Letter_init[5].keyname[0] = 0x00;
		buttons_Letter_init[5].keyinfo.key = 0x00;
		break;
	case KEY_2:
		for (i = 0; i < 3; i++) {
			buttons_Letter_init[i].keyname[0] = 'a'+i;
			buttons_Letter_init[i].keyinfo.key = KEY_a+i;
			buttons_Letter_init[i+3].keyname[0] = 'A'+i;
			buttons_Letter_init[i+3].keyinfo.key = KEY_A+i;
		}
		break;
	case KEY_3:
		for (i = 0; i < 3; i++) {
			buttons_Letter_init[i].keyname[0] = 'd'+i;
			buttons_Letter_init[i].keyinfo.key = KEY_d+i;
			buttons_Letter_init[i+3].keyname[0] = 'D'+i;
			buttons_Letter_init[i+3].keyinfo.key = KEY_D+i;
		}
		break;
	case KEY_4:
		for (i = 0; i < 3; i++) {
			buttons_Letter_init[i].keyname[0] = 'g'+i;
			buttons_Letter_init[i].keyinfo.key = KEY_g+i;
			buttons_Letter_init[i+3].keyname[0] = 'G'+i;
			buttons_Letter_init[i+3].keyinfo.key = KEY_G+i;
		}
		break;
	case KEY_5:
		for (i = 0; i < 3; i++) {
			buttons_Letter_init[i].keyname[0] = 'j'+i;
			buttons_Letter_init[i].keyinfo.key = KEY_j+i;
			buttons_Letter_init[i+3].keyname[0] = 'J'+i;
			buttons_Letter_init[i+3].keyinfo.key = KEY_J+i;
		}
		break;
	case KEY_6:
		for (i = 0; i < 3; i++) {
			buttons_Letter_init[i].keyname[0] = 'm'+i;
			buttons_Letter_init[i].keyinfo.key = KEY_m+i;
			buttons_Letter_init[i+3].keyname[0] = 'M'+i;
			buttons_Letter_init[i+3].keyinfo.key = KEY_M+i;
		}
		break;
	case KEY_7:
		buttons_Letter_init[0].keyname[0] = 'p';
		buttons_Letter_init[0].keyinfo.key = KEY_p;
		buttons_Letter_init[1].keyname[0] = 'r';
		buttons_Letter_init[1].keyinfo.key = KEY_r;
		buttons_Letter_init[2].keyname[0] = 's';
		buttons_Letter_init[2].keyinfo.key = KEY_s;
		buttons_Letter_init[3].keyname[0] = 'P';
		buttons_Letter_init[3].keyinfo.key = KEY_P;
		buttons_Letter_init[4].keyname[0] = 'R';
		buttons_Letter_init[4].keyinfo.key = KEY_R;
		buttons_Letter_init[5].keyname[0] = 'S';
		buttons_Letter_init[5].keyinfo.key = KEY_S;
		break;
	case KEY_8:
		for (i = 0; i < 3; i++) {
			buttons_Letter_init[i].keyname[0] = 't'+i;
			buttons_Letter_init[i].keyinfo.key = KEY_t+i;
			buttons_Letter_init[i+3].keyname[0] = 'T'+i;
			buttons_Letter_init[i+3].keyinfo.key = KEY_T+i;
		}
		break;
	case KEY_9:
		for (i = 0; i < 3; i++) {
			buttons_Letter_init[i].keyname[0] = 'w'+i;
			buttons_Letter_init[i].keyinfo.key = KEY_w+i;
			buttons_Letter_init[i+3].keyname[0] = 'W'+i;
			buttons_Letter_init[i+3].keyinfo.key = KEY_W+i;
		}
		break;
	case KEY_0:
		buttons_Letter_init[0].keyname[0] = '.';
		buttons_Letter_init[0].keyinfo.key = KEY_DOT;
		buttons_Letter_init[1].keyname[0] = ',';
		buttons_Letter_init[1].keyinfo.key = KEY_COMMA;
		buttons_Letter_init[2].keyname[0] = ';';
		buttons_Letter_init[2].keyinfo.key = KEY_SEMICOLON;
		buttons_Letter_init[3].keyname[0] = '/';
		buttons_Letter_init[3].keyinfo.key = KEY_DIAGONAL;
		buttons_Letter_init[4].keyname[0] = '(';
		buttons_Letter_init[4].keyinfo.key = KEY_LEFTPRNTHS;
		buttons_Letter_init[5].keyname[0] = ')';
		buttons_Letter_init[5].keyinfo.key = KEY_RIGHTPRNTHS;
		break;
	default:
		break;
	}
	
	NAPI_ScrGetViewPort(&disp_x, &disp_y, &disp_width, &disp_height);
	NAPI_ScrClrLine(buttons_Letter_init[0].keyinfo.btn.l_top.y-disp_y, buttons_Letter_init[0].keyinfo.btn.r_bottom.y-disp_y);
	Virtual_KbInit();
	btn_num = sizeof(buttons_Letter_select) / sizeof(button_info);
	for (index = 0; index < btn_num; index++)
	{
		draw_release(&(buttons_Letter_init)[index]);
	}
	NAPI_ScrRefresh();

	return APP_SUCC;
}

int Virtual_KbGetCode(int nTimeout)
{
	int nKeyCode, nRet;

	while (1)
	{
		nRet = NAPI_KbVppTpGetCode(nTimeout, &nKeyCode);
		if (nRet != NAPI_OK)
		{
			if (nRet == NAPI_ERR_TIMEOUT) {
				return APP_TIMEOUT;
			}
			VIRTUAL_TRACE("NAPI_KbVppTpGetCode fail(%d)", nRet);
			return nRet;
		}
		VIRTUAL_TRACE("nKeyCode = %x %c", nKeyCode, nKeyCode);

		if (nKeyCode == 0x00) { // Invalid key
			continue;
		}

		if (nKeyCode == 0xFF) // swich digital and letter
		{
			gcKbMode = !gcKbMode;
			if (gcSupportKbSwitch == YES) {
				Virtual_KbCreate(NULL, INPUT_MODE_STRING);
			} else {
				Virtual_KbCreate(NULL, INPUT_MODE_NUMBER);
			}
			continue;
		}
		VIRTUAL_TRACE("gcKbMode = %d", gcKbMode);

		// digital key
		if (gcKbMode == KB_MODE_DIGITAL)
		{
			return nKeyCode;
		}

		// letter info
		if (nKeyCode >= '0' && nKeyCode <= '9')
		{
			ShowLetterInfo(nKeyCode);
		}
		else
		{
			return nKeyCode;
		}
	}

	return APP_SUCC;
}

int Virtual_KbHit()
{
	int nKeyCode;

	if (NAPI_KbVppTpHit(&nKeyCode) != NAPI_OK) {
		VIRTUAL_TRACE("hit fail(%d)", NAPI_KbVppTpHit(&nKeyCode));
		return APP_QUIT;
	}

	return nKeyCode;
}

