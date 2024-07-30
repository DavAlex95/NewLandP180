/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  packet.c
** File indentifier: 
** Brief:  Packing module
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-21
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

#define BITMAP_64 64
#define BITMAP_128 128

/**
* Define ISO Format
*/
const static struct STISO_FIELD_DEF gstIso8583Def[] =
{
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  '\0', PubFPBMsgIDpack,  PubFPBMsgIDUnpack},  /* 0   "MESSAGE TYPE INDICATOR" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT,  '\0', PubFPBBitmapPack, PubFPBBitmapUnPack}, /* 1   "BIT MAP" */
	{ 19, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 2   "PAN - PRIMARY ACCOUNT NUMBER" */
	{  6, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 3   "PROCESSING CODE" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPBCharPack,   PubFPBCharUnPack},   /* 4   "AMOUNT, TRANSACTION" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 5   "AMOUNT, SETTLEMENT" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 6   "AMOUNT, CARDHOLDER BILLING" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 7   "TRANSMISSION DATE AND TIME" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 8   "AMOUNT, CARDHOLDER BILLING FEE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 9   "CONVERSION RATE, SETTLEMENT" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 10  "CONVERSION RATE, CARDHOLDER BILLING" */
	{  6, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 11  "SYSTEM TRACE AUDIT NUMBER" */
	{  6, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 12  "TIME, LOCAL TRANSACTION" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 13  "DATE, LOCAL TRANSACTION" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 14  "DATE, EXPIRATION" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 15  "DATE, SETTLEMENT" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 16  "DATE, CONVERSION" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 17  "DATE, CAPTURE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 18  "MERCHANTS TYPE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 19  "ACQUIRING INSTITUTION COUNTRY CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 20  "PAN EXTENDED COUNTRY CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 21  "FORWARDING INSTITUTION COUNTRY CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 22  "POINT OF SERVICE ENTRY MODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 23  "CARD SEQUENCE NUMBER" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 24  "NETWORK INTERNATIONAL IDENTIFIEER" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPBCharPack,   PubFPBCharUnPack},   /* 25  "POINT OF SERVICE CONDITION CODE" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 26  "POINT OF SERVICE PIN CAPTURE CODE" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 27  "AUTHORIZATION IDENTIFICATION RESP LEN" */
	{ 28, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 28  "AMOUNT, TRANSACTION FEE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBAmountPack, PubFPBAmountUnPack}, /* 29  "AMOUNT, SETTLEMENT FEE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBAmountPack, PubFPBAmountUnPack}, /* 30  "AMOUNT, TRANSACTION PROCESSING FEE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBAmountPack, PubFPBAmountUnPack}, /* 31  "AMOUNT, SETTLEMENT PROCESSING FEE" */
	{ 11, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 32  "ACQUIRING INSTITUTION IDENT CODE" */
	{ 11, LENGTH_TYPE_LLVAR, ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 33  "FORWARDING INSTITUTION IDENT CODE" */
	{ 28, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 34  "PAN EXTENDED" */
	{ 37, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPBCharPack,   PubFPBCharUnPack},   /* 35  "TRACK 2 DATA" */
	{104, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPBCharPack,   PubFPBCharUnPack},   /* 36  "TRACK 3 DATA" */
	{  12, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 37  "RETRIEVAL REFERENCE NUMBER" */
	{  6, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 38  "AUTHORIZATION IDENTIFICATION RESPONSE" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 39  "RESPONSE CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 40  "SERVICE RESTRICTION CODE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 41  "CARD ACCEPTOR TERMINAL IDENTIFICACION" */
	{  15, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 42  "CARD ACCEPTOR IDENTIFICATION CODE"  */
	{ 40, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 43  "CARD ACCEPTOR NAME/LOCATION" */
	{25, LENGTH_TYPE_LLVAR,ALIGN_TYPE_RIGHT,  ' ',  PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 44  "ADITIONAL RESPONSE DATA" */
	{ 76, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 45  "TRACK 1 DATA" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 46  "ADITIONAL DATA - ISO" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 47  "ADITIONAL DATA - NATIONAL" */
	{4, LENGTH_TYPE_FIX,ALIGN_TYPE_RIGHT,  ' ',  PubFPBCharPack,   PubFPBCharUnPack},   	/* 48  "ADITIONAL DATA - PRIVATE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 49  "CURRENCY CODE, TRANSACTION" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT,  ' ',  PubFPBCharPack,   PubFPBCharUnPack},   /* 50  "CURRENCY CODE, SETTLEMENT" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 51  "CURRENCY CODE, CARDHOLDER BILLING"    */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack, PubFPBBinaryUnPack}, /* 52  "PIN DATA"    */
	{ 16, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 53  "SECURITY RELATED CONTROL INFORMATION" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_RIGHT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 54  "ADDITIONAL AMOUNTS" */
	{255, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_RIGHT,  '\0',  PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 55  "RESERVED ISO" */
	{ 999, LENGTH_TYPE_LLLVAR, ALIGN_TYPE_LEFT, '\0', PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 56  "RESERVED ISO" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 57  "RESERVED NATIONAL" */
	{255, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 58  "RESERVED NATIONAL" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack, PubFPBBinaryUnPack}, /* 59  "RESERVED NATIONAL" */
	{13, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack, PubFPBBinaryUnPack}, /* 60  "RESERVED PRIVATE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 61  "RESERVED PRIVATE" */
	{512, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_RIGHT,  '\0', PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 62  "RESERVED PRIVATE" */
	{163, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_RIGHT,  '\0', PubFPBBinaryPack,   PubFPBBinaryUnPack},   /* 63  "RESERVED PRIVATE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack, PubFPBBinaryUnPack}, /* 64  "MESSAGE AUTHENTICATION CODE FIELD" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  '\0', PubFPBBitmapPack, PubFPBBitmapUnPack}, /* 65  "BITMAP, EXTENDED" */
	{  1, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 66  "SETTLEMENT CODE" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 67  "EXTENDED PAYMENT CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 68  "RECEIVING INSTITUTION COUNTRY CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 69  "SETTLEMENT INSTITUTION COUNTRY CODE" */
	{  3, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 70  "NETWORK MANAGEMENT INFORMATION CODE" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 71  "MESSAGE NUMBER" */
	{  4, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 72  "MESSAGE NUMBER LAST" */
	{  6, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 73  "DATE ACTION" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 74  "CREDITS NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 75  "CREDITS REVERSAL NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 76  "DEBITS NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 77  "DEBITS REVERSAL NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 78  "TRANSFER NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 79  "TRANSFER REVERSAL NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 80  "INQUIRIES NUMBER" */
	{ 10, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 81  "AUTHORIZATION NUMBER" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 82  "CREDITS, PROCESSING FEE AMOUNT" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 83  "CREDITS, TRANSACTION FEE AMOUNT" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 84  "DEBITS, PROCESSING FEE AMOUNT" */
	{ 12, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 85  "DEBITS, TRANSACTION FEE AMOUNT" */
	{ 16, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 86  "CREDITS, AMOUNT" */
	{ 16, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 87  "CREDITS, REVERSAL AMOUNT" */
	{ 16, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 88  "DEBITS, AMOUNT" */
	{ 16, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 89  "DEBITS, REVERSAL AMOUNT" */
	{ 42, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 90  "ORIGINAL DATA ELEMENTS" */
	{  1, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 91  "FILE UPDATE CODE" */
	{  2, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 92  "FILE SECURITY CODE" */
	{  5, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 93  "RESPONSE INDICATOR" */
	{  7, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 94  "SERVICE INDICATOR" */
	{ 42, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 95  "REPLACEMENT AMOUNTS" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack, PubFPBBinaryUnPack}, /* 96  "MESSAGE SECURITY CODE" */
	{ 16, LENGTH_TYPE_FIX,   ALIGN_TYPE_RIGHT, '\0', PubFPBAmountPack, PubFPBAmountUnPack}, /* 97  "AMOUNT, NET SETTLEMENT" */
	{ 25, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 98  "PAYEE" */
	{ 11, LENGTH_TYPE_LLVAR, ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 99  "SETTLEMENT INSTITUTION IDENT CODE" */
	{ 11, LENGTH_TYPE_LLVAR, ALIGN_TYPE_RIGHT, '\0', PubFPBCharPack,   PubFPBCharUnPack},   /* 100 "RECEIVING INSTITUTION IDENT CODE" */
	{ 17, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 101 "FILE NAME" */
	{ 28, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 102 "ACCOUNT IDENTIFICATION 1" */
	{ 28, LENGTH_TYPE_LLVAR, ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 103 "ACCOUNT IDENTIFICATION 2" */
	{100, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 104 "TRANSACTION DESCRIPTION" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 105 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 106 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 107 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 108 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 109 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 110 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 111 "RESERVED ISO USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 112 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 113 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 114 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 115 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 116 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 117 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 118 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 119 "RESERVED NATIONAL USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 120 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 121 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 122 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 123 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 124 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 125 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 126 "RESERVED PRIVATE USE" */
	{999, LENGTH_TYPE_LLLVAR,ALIGN_TYPE_LEFT,  ' ',  PubFPACharPack,   PubFPACharUnPack},   /* 127 "RESERVED PRIVATE USE" */
	{  8, LENGTH_TYPE_FIX,   ALIGN_TYPE_LEFT,  '\0', PubFPBBinaryPack, PubFPBBinaryUnPack}, 	/* 128 "MAC 2" */
};


/**
* define global variable
*/
static const char gszCode[][64]={       
	{"00Approved"},
	{"01Refer to card issuer."},
	{"02Refer to card issuer.s special conditions"},
	{"03Invalid Merchant."},
	{"04Pick Up Card"},
	{"05Do Not Honour"},
	{"06Error"},
	{"07Pickup card, special condition"},
	{"08Verify id and sign"},
	{"09Accepted"},
	{"10Approved for partial amount"},
	{"11Approved (VIP)"},
	{"12Invalid Transaction"},
	{"13Invalid Amount"},
	{"14Invalid card number"},
	{"15No such Issuer"},
	{"16Approved, update tk 3"},
	{"17Customer cancellation"},
	{"18Customer dispute"},
	{"19Re-enter transaction"},
	{"20Invalid response"},
	{"21No transactions"},
	{"22Suspected malfunction"},
	{"23Unaccepted trans fee"},
	{"24File Upd not Suported"},
	{"25Unable to locate record on file"},
	{"26Dup rec,old rec rplcd"},
	{"27Field edit error"},
	{"28File Locked out"},
	{"29File update error"},
	{"30Format error"},
	{"31Bank Not Supported by Switch"},
	{"32Completed partially"},
	{"33Expired card"},
	{"34Suspected fraud"},
	{"35Contact acquirer"},
	{"36Restricted card"},
	{"37Call acq. Security"},
	{"38PIN tries Exceeded"},
	{"39No credit account"},
	{"40Func. Not Supported"},
	{"41Lost card"},
	{"42No universal account"},
	{"43Stolen card"},
	{"44No investment account"},
	{"45ISO error #45"},
	{"46ISO error #46"},
	{"47ISO error #47"},
	{"48ISO error #48"},
	{"49ISO error #49"},
	{"50ISO error #50"},
	{"51Not sufficient funds"},
	{"52No cheque acc"},
	{"53No savings acc"},
	{"54Expired card"},
	{"55Incorrect PIN"},
	{"56No card record"},
	{"57Txn not Permtd-card"},
	{"58Transaction Not Permitted in Terminal"},
	{"59Suspected fraud"},
	{"60Contact acquirer"},
	{"61Amount limit exceeded"},
	{"62Restricted card"},
	{"63Security violation"},
	{"64Org amount incorrect"},
	{"65Frequency limit exceeded"},
	{"66Call acq's security"},
	{"67Hard capture"},
	{"68Resp Recvd too Late"},
	{"69ISO error #69"},
	{"70ISO error #70"},
	{"71ISO error #71"},
	{"72ISO error #72"},
	{"73ISO error #73 "},
	{"74Transaction declined due to PIN bypass not allowed"},
	{"75PIN tries exceeded"},
	{"76Invalid product codes"},
	{"77Reconcile error"},
	{"78Trace not found"},
	{"79Declined CVV2"},
	{"80Batch number not found"},
	{"81Exceed redp limit"},
	{"82No closed soc slots"},
	{"83No susp. Soc slots"},
	{"84Insufficient pts"},
	{"85Batch not found"},
	{"86Private error #86"},
	{"87Private error #87"},
	{"88Have cm call amex"},
	{"89Bad terminal Id."},
	{"90Cutoff in Process"},
	{"91Issuer or switch inoperative"},
	{"92Transaction can't be routed"},
	{"93Txn cant be Completed"},
	{"94Duplicate transmission"},
	{"95Batch upload started"},
	{"96System malfuntion"},
	{"97Resvd. For Nat. Use"},
	{"98Resvd. For Nat. Use"},
	{"99Terminal data dld"},	
	{"--NO RESPONSE CODE"},
};

static STISODataStru gstISOData;	/**< Iso data*/
static struct STISO_FIELD_LENGTH_CALC gstSetFieldLengthCalc = {PubFLBLengthSet, PubFLBLengthGet};

#define DISP_PACKET_ERR(pszTitle, nFieldNum)\
	char szMsg[64+1];\
	sprintf(szMsg, "[%d] Field Err", nFieldNum);\
	PubMsgDlg(pszTitle, szMsg, 3, 10);\

static YESORNO IsNeedToSendDE2(STSYSTEM *pstSystem);
static YESORNO IsNeedToSendDE35(STSYSTEM *pstSystem);

/**
* @brief Initialize ISO8583 format
* @param void
* @return 
* @li APP_SUCC
* @li APP_FAIL
*/
int InitISO8583(void)
{
	int nRet = 0;	
	nRet = PubImport8583(gstIso8583Def );
	nRet += PubSetFieldLengthCalc(&gstSetFieldLengthCalc );

	ASSERT_FAIL(nRet);
	return APP_SUCC;
}


/**
* @brief Check MAC
* @param in const char *psBuffer 
* @param in int nDataLen
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int CheckMac(const char *psBuffer,  int nDataLen)
{
	int nRet, nGroupNo;
	char sCalcMacBuf[8+1]={0};
	
	GetVarMainKeyNo(&nGroupNo);
	PubSetCurrentMainKeyIndex(nGroupNo);

	nRet = PubCalcMac(MAC_TYPE_X919, psBuffer, nDataLen - 8, sCalcMacBuf);
	if (nRet != APP_SUCC)
	{
		TRACE("nRet = %d", nRet);
		return nRet;
	}
	if (memcmp(sCalcMacBuf, psBuffer+nDataLen - 8, 8) == 0 )
	{
		return APP_SUCC;
	}
	else
	{
		TRACE_HEX(sCalcMacBuf,8,"sCalcMacBuf:");
		TRACE_HEX((char *)(psBuffer+nDataLen-8),8,"realmac:");
		return APP_FAIL;
	}
}

/**
* @brief Add MAC in the data tail
* @param in char * psData
* @param  int pnDataLen 
* @param  out pnDataLen 
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int AddMac(char * psData, int *pnDataLen, char cKeyType)
{
	int nRet, nGroupNo = 0;

	GetVarMainKeyNo(&nGroupNo);
	PubSetCurrentMainKeyIndex(nGroupNo);

	nRet = PubCalcMac(MAC_TYPE_X919, psData, *pnDataLen, psData+(*pnDataLen));
	if (nRet != APP_SUCC)
	{
		TRACE("nRet = %d", nRet);
		return nRet;
	}

	*pnDataLen += 8;
	return APP_SUCC;
}


/**
* @brief Clear ISO global buffer
* @param void
* @li APP_SUCC
* @li APP_FAIL
*/
int ClrPack()
{
	return PubDelAllBit(&gstISOData);
}

/**
* @brief Set Field data to ISO global buffer
* @param in const int nFieldNum
* @param in const char *psFieldBuffer 
* @param in const int nFieldLen 
* @li APP_SUCC
* @li APP_FAIL
*/
int SetField(const int nFieldNum , const char *psFieldBuffer, const int nFieldLen)
{
	int nRet = 0;

	nRet = PubSetBit (&gstISOData, nFieldNum, (char *)psFieldBuffer, nFieldLen);
	if (nRet != APP_SUCC)
	{
		DISP_PACKET_ERR(tr("SET FIELD"), nFieldNum);
	}
	return nRet;
}


/**
* @brief Get Field data from ISO global buffer
* @param in const int nFieldNum
* @param out const char *psFieldBuffer 
* @param out const int *pnFieldLen
* @li APP_SUCC
* @li APP_FAIL
*/
int GetField(const int nFieldNum, char *psFieldBuffer, int *pnFieldLen)
{
	int nRet = 0;

	nRet = PubGetBit (&gstISOData, nFieldNum, psFieldBuffer, pnFieldLen);
	if (nRet != APP_SUCC)
	{
		DISP_PACKET_ERR(tr("GET FIELD"), nFieldNum);
	}
	return nRet;
}


/**
* @brief Pack ISO global buffer to str
* @param out const char *psPackBuffer 
* @param out const int *pnPackLen 
* @li APP_SUCC
* @li APP_FAIL
*/
int Pack(char *psPackBuffer,  int *pnPackLen )
{
	return PubIsoToStr(psPackBuffer, &gstISOData, pnPackLen );
}	


/**
* @brief Unpack to ISO global buffer
* @param in const char *psPackBuffer
* @param in const int nPackLen 
* @li APP_SUCC
* @li APP_FAIL
*/
int Unpack(const char *psPackBuffer, const int nPackLen)
{
	int nRes = 0, nRet = 0;

	PubDelAllBit(&gstISOData);
	nRet = PubStrToIso((char *)psPackBuffer, nPackLen, &gstISOData, &nRes);
	if (nRet != APP_SUCC)
	{
		PubClearAll();
		PubDisplayGen(1, tr("UNPACK ERR"));
		PubDisplayStrInline(0, 2, tr("[%d]Field Err"),nRes);
		PubUpdateWindow();
		PubWaitConfirm(3);
		return nRet;
	}
	return nRet;
}


/**
* @brief Display Error message
* @param in const char *szRespCode 
* @return */
void DispResp(const char *pszRespCode)
{
	int i;
	char szContent[100];

	for (i = 0; ; i++)
	{
		if ((!memcmp(gszCode[i], "--", 2)) ||(!memcmp(gszCode[i], pszRespCode, 2)))
		{
			memset(szContent, 0, sizeof(szContent));
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "%2.2s-%s", pszRespCode, gszCode[i]+2);
			PubMsgDlg(tr("TRANS FAIL"), szContent, 3, 30);
			return;
		}
	}
}

/**
* @brief Check Respond isSuccess
* @param [in] IsChkReversal   
* @param [in] pszRespCode   
* @return
* @li APP_FAIL 
* @li APP_SUCC 
*/
int ChkRespIsSucc(YESORNO IsChkReversal,const char *pszRespCode)
{
	if (memcmp(pszRespCode, "00", 2) != 0)
	{
		if (IsChkReversal  == YES)
		{
			SetVarIsReversal(NO);
		}
		DispResp(pszRespCode);
		return APP_FAIL;
	}
	return APP_SUCC;
}


/**
* @brief Do Pack and Communicaiton 
* @detail [Pack],[SEND],[RECV][UNPACK]
* @param [in] cOperFlag  see [ENM_DEALPACKCOMM]
* @param [out] pstSystem 
* @param [out] pstReversal  
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author lg
* @date 2013-3-14
*/
int DealPackAndComm(char* pszTitle, EM_OPERATEFLAG cOperFlag,STSYSTEM *pstSystem,STREVERSAL *pstReversal, int nInputPinNum)
{
	int nRet = 0, nPackLen = 0, nFieldLen = 0;
	char sPackBuf[MAX_PACK_SIZE] = {0};
	char szContent[200]= {0};
	char szMsgID[4+1] = {0};
	int nMainKeyNo = 0;

	nFieldLen = 4;
	GetField(0, szMsgID, &nFieldLen);
	/**
	*Pack
	*/
	ASSERT_HANGUP_FAIL(Pack(sPackBuf, &nPackLen));
	if (pstSystem->cMustChkMAC == 0x01)
	{
		nPackLen -= 8;
		ASSERT_HANGUP_FAIL(AddMac(sPackBuf,&nPackLen, KEY_TYPE_MAC));
	}

	if ((cOperFlag & CFG_REVERSAL) == CFG_REVERSAL)
	{
		TxnSystemToReveral(pstSystem, pstReversal);
		if (pstSystem->cTransAttr == ATTR_CONTACTLESS || pstSystem->cTransAttr == ATTR_CONTACT) 
		{
			EmvPackField55(TRANS_REVERSAL, pstReversal->szFieldAdd1, &pstReversal->nFieldAdd1Len);
		}
		SetReversalData(pstReversal);
		SetVarIsReversal(YES);
	}

	IncVarTraceNo();
	nRet = CommSend(sPackBuf, nPackLen);
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		return nRet;
	}
	nRet = CommRecv(sPackBuf, &nPackLen);
	if (nRet != APP_SUCC)
	{
		CommHangUp();
		return nRet;
	}

	CommHangUpSocket();

	ASSERT_HANGUP_FAIL(Unpack(sPackBuf, nPackLen));
	ASSERT_HANGUP_FAIL(ChkRespMsgID(szMsgID, sPackBuf));
	ASSERT_HANGUP_FAIL(ChkRespon(pstSystem, sPackBuf + 2));
	if (pstSystem->cMustChkMAC == 0x01)
	{
		nRet = CheckMac(sPackBuf, nPackLen);
		ASSERT(nRet);
		if (nRet != APP_SUCC)
		{
			CommHangUp();
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, tr("\n|CMAC FROM HOST IS ERROR"));
			PubMsgDlg(pszTitle, szContent, 3, 10);
			return APP_FAIL;
		}
	}	

	if(GetVarKeySystemType() == KS_DUKPT)	//DUKPT
	{	
		GetVarMainKeyNo(&nMainKeyNo);
		PubSetCurrentMainKeyIndex(nMainKeyNo);
		PubDukptIncreaseKSN();
	}

	if((memcmp("55", pstSystem->szResponse,2) == 0) && (nInputPinNum < 3))
	{
		if ((cOperFlag & CFG_REVERSAL) == CFG_REVERSAL)
		{
			SetVarIsReversal(NO);
		}
		return APP_REPIN;
	}
	
	PubClear2To4();
	if ((cOperFlag & CFG_NOCHECKRESP) == CFG_NOCHECKRESP)
	{
		return APP_SUCC;
	}
	
	if ((cOperFlag & CFG_REVERSAL) == CFG_REVERSAL)
	{
		nRet = ChkRespIsSucc(YES,pstSystem->szResponse);
	}
	else
	{
		nRet = ChkRespIsSucc(NO,pstSystem->szResponse);
	}
	if(nRet != APP_SUCC)
	{
		CommHangUp();
		return APP_FAIL;
	}

	return APP_SUCC;
}

/**
* @brief Trace Complete
* @detail [Save record],[Cleat reversal],[Print][Send TC]
* @param [in] pszTitle  
* @param [in] cOperFlag  
* @param [in] pstSystem 
* @param [in] pstTransRecord 
* @param [in] pszInvno 
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author lg
* @date 2014-9-14
*/
int TradeComplete(char* pszTitle, const STSYSTEM *pstSystem, STTRANSRECORD *pstTransRecord, const char *pszInvno)
{
	int nRet;
	char sTrace[3+1];
	char szTrace[6+1] = {0};
	STTRANSRECORD stTransRecordOld;
	STREVERSAL stReversal;	
	
	nRet = AppendTransRecord(pstTransRecord);
	if (nRet != APP_SUCC)
	{
	    GetReversalData(&stReversal);
		strcpy(stReversal.szResponse, "96");
		SetReversalData(&stReversal);
		CommHangUp();
		return APP_FAIL;
	}

	SetVarIsReversal(NO);
	memset(&stReversal, 0, sizeof(STREVERSAL));
	SetReversalData(&stReversal);

	if(pszInvno != NULL && strlen(pszInvno) > 0)
	{
		/**
		* update old record
		*/
		memset(&stTransRecordOld, 0, sizeof(STTRANSRECORD));
		PubAscToHex((uchar *)pszInvno, 6, 0, (uchar *)sTrace);	
		ASSERT_FAIL(FindRecordWithTagid(TAG_RECORD_TRACE, sTrace, &stTransRecordOld));
		if (pstSystem->cTransType == TRANS_VOID || pstSystem->cTransType == TRANS_VOID_AUTHSALE)
		{
			stTransRecordOld.cStatus = CANCELED;
		}
		else if (pstSystem->cTransType == TRANS_AUTHCOMP)
		{
			stTransRecordOld.cStatus = SALECOMPLETED;
		}
		else if (pstSystem->cTransType == TRANS_ADJUST)
		{
			PubAscToHex((uchar *)pstSystem->szTipAmount, 12, 0, (uchar *)stTransRecordOld.sTipAmount);
			PubAscToHex((uchar *)pstSystem->szAmount, 12, 0, (uchar *)stTransRecordOld.sAmount);
		}

		UpdateTransRecord(&stTransRecordOld);
	}

	ChangeSettle(pstSystem);

	PubHexToAsc((uchar *)pstTransRecord->sTrace, 6, 0, (uchar *)szTrace);

	if (GetElecSignFlag() == YES) {
		ElecSign(szTrace);
	}

	PubClearAll();
	PubDisplayTitle(pszTitle);
	DISP_TRANS_SUCC;
	if (pstTransRecord->cPrintFlag == YES && PubIsSupportPrint() == YES)
	{
		DISP_PRINTING_NOW;
	}

	IncVarInvoiceNo();
	PrintRecord(pstTransRecord, FIRSTPRINT);

	EmvSendTC();
	TxnSendOffline(0);
	CommHangUp();

	return APP_SUCC;
}

int PackGeneral(STSYSTEM *pstSystem, STTRANSRECORD *pstTransRecord, STTRANSCFG stTransCfg)
{
	int i, nField;
	char sKSN[10+1] = {0};
	int nMainKeyNo = 0;

	if(GetVarKeySystemType() == KS_DUKPT)	//DUKPT
	{
		GetVarMainKeyNo(&nMainKeyNo);
		PubSetCurrentMainKeyIndex(nMainKeyNo);
		PubGetDukptKSN(sKSN);
		TRACE_HEX(sKSN, 10, "sKSN: ");
	}
	/**
	* Begin to pack
	*/
	ClrPack();
	ASSERT_HANGUP_FAIL(SetField(0, pstSystem->szMsgID, 4));

	for(i = 0; i < BITMAP_64; i++) {
		nField = stTransCfg.szField[i];
		if (nField < 1 || nField > BITMAP_64) {
			continue;
		}
		switch(nField) {
		case 2:
			if (YES == IsNeedToSendDE2(pstSystem)) {
				ASSERT_HANGUP_FAIL(SetField(2, pstSystem->szPan, strlen(pstSystem->szPan)));
			}
			break;
		case 3:
			ASSERT_HANGUP_FAIL(SetField(3, pstSystem->szProcCode, 6));
			break;
		case 4:
			if (strlen(pstSystem->szAmount) == 0) {
				memset(pstSystem->szAmount, '0', 12);
			}
			ASSERT_HANGUP_FAIL(SetField(4, pstSystem->szAmount, 12));
			break;
		case 11:
			ASSERT_HANGUP_FAIL(SetField(11, pstSystem->szTrace, 6));
			break;
		case 12:
			ASSERT_FAIL(SetField(12, pstSystem->szTime, 6));
			break;
		case 13:
			ASSERT_FAIL(SetField(13, pstSystem->szDate, 4));
			break;
		case 14:
			if (memcmp(pstSystem->szInputMode, INPUTMODE_KEYIN, 2) == 0) {
				if (ChkDigitStr(pstSystem->szExpDate, 4) == APP_SUCC) {
					ASSERT_HANGUP_FAIL(SetField(14, pstSystem->szExpDate, 4));
				}
			}
			break;
		case 22:
			ASSERT_HANGUP_FAIL(SetField(22, pstSystem->szInputMode, 3));
			break;
		case 23:
			if (memcmp(pstSystem->szCardSerialNo, "\x00\x00\x00", 3) != 0) {
				ASSERT_FAIL(SetField(23, pstSystem->szCardSerialNo, 3));
			}
			break;
		case 24:
			ASSERT_HANGUP_FAIL(SetField(24, pstSystem->szNii, 3));
			break;
		case 25:
			ASSERT_HANGUP_FAIL(SetField(25, pstSystem->szServerCode, 2));
			break;
		case 35:
			if (YES == IsNeedToSendDE35(pstSystem)) {
				if (strlen(pstSystem->szTrack2) > MIN_TK2 && strlen(pstSystem->szTrack2) <= MAX_TK2) {
					ASSERT_HANGUP_FAIL(SetField(35, pstSystem->szTrack2, strlen(pstSystem->szTrack2)));
				}
			}
			break;
		case 37:
			if(strlen(pstSystem->szOldRefnum) > 0) {
				ASSERT_FAIL(SetField(37, pstSystem->szOldRefnum, strlen(pstSystem->szOldRefnum)));
			}
			break;
		case 38:
			if(strlen(pstSystem->szOldAuthCode) > 0) {
				ASSERT_FAIL(SetField(38, pstSystem->szOldAuthCode, 6));
			}
			break;
		case 39:
			if(pstSystem->cTransType == TRANS_AUTHCOMP) {
				ASSERT_FAIL(SetField(39, "00", 2));
			}
			break;
		case 41:
			ASSERT_HANGUP_FAIL(SetField(41, pstSystem->szPosID, strlen(pstSystem->szPosID)));
			break;
		case 42:
			ASSERT_HANGUP_FAIL(SetField(42, pstSystem->szShopID, strlen(pstSystem->szShopID)));
			break;
		case 48:
			if (pstSystem->cTransAttr == ATTR_MANUAL && strlen(pstSystem->szCVV2) > 0) {
				ASSERT_HANGUP_FAIL(SetField(48, pstSystem->szCVV2, strlen(pstSystem->szCVV2)));
			}
			break;
		case 49:
			ASSERT_HANGUP_FAIL(SetField(49, CURRENCY_CODE, 3));
			break;
		case 52:
			if ('1' == pstSystem->szInputMode[2]) {
				ASSERT_HANGUP_FAIL(SetField(52, pstSystem->szPin, 8));
			}
			break;
		case 54:
			if (memcmp(pstSystem->szCashbackAmount, "000000000000", 12) > 0)
			{
				ASSERT_FAIL(SetField(54, pstSystem->szCashbackAmount, 12));
			}
		case 55:
			if(pstSystem->nAddFieldLen > 0) {
				ASSERT_FAIL(SetField(55, pstSystem->psAddField, pstSystem->nAddFieldLen));
			}
			break;
		case 57:
			if(GetVarKeySystemType() == KS_DUKPT) {
				ASSERT_FAIL(SetField(57, sKSN, 10));
			}
			break;
		case 61:
			ASSERT_HANGUP_FAIL(SetField(61, pstSystem->szOldDate, 4));
			break;
		case 62:
			ASSERT_HANGUP_FAIL(SetField(62, pstSystem->szInvoice, strlen(pstSystem->szInvoice)));
			break;
		case 64:
			ASSERT_HANGUP_FAIL(SetField(64, "\x00\x00\x00\x00\x00\x00\x00\x00", 8));
			pstSystem->cMustChkMAC = 0x01;
			break;
		default:
			break;
		}
	}

	return APP_SUCC;
}

static YESORNO IsNeedToSendDE2(STSYSTEM *pstSystem)
{
	if (memcmp(pstSystem->szMsgID, "0400", 4) == 0
		|| (memcmp(pstSystem->szMsgID, "0320", 4) == 0)
			|| (memcmp(pstSystem->szMsgID, "0220", 4) == 0))
	{
		return YES;
	}
	if (pstSystem->cTransAttr == ATTR_MANUAL)
	{
		return YES;		
	}
	return NO;
}


static YESORNO IsNeedToSendDE35(STSYSTEM *pstSystem)
{
	if (YES != IsNeedToSendDE2(pstSystem)
		&& (pstSystem->cTransAttr != ATTR_MANUAL))
	{
		return YES;
	}

	return NO;
}

