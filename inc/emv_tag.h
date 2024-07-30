/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  emv_tag.h
** File indentifier: 
** Brief: EMV TAG DEFINE 
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-20
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#ifndef _TAG_H_
#define _TAG_H_


/*
* ICC Data Flag
*/
#define   _EMV_TAG_9F26_IC_AC 				0x9F26		/**<  0 - (Application Cryptogram)*/
#define   _EMV_TAG_9F42_IC_APPCURCODE 		0x9F42		/**<  1 - (Application Currency Code)*/
#define   _EMV_TAG_9F44_IC_APPCUREXP		0x9F44		/**<  2 - (Application Currency Exponent)*/
#define   _EMV_TAG_9F05_IC_APPDISCDATA		0x9F05		/**<  3 - (Application Discretionary Data)*/
#define   _EMV_TAG_5F25_IC_APPEFFECTDATE	0x5F25		/**<  4 - (Application Effective Date)*/
#define   _EMV_TAG_5F24_IC_APPEXPIREDATE	0x5F24		/**<  5 - (Application Expiration Date)*/
#define   _EMV_TAG_94_IC_AFL				0x94		/**<  6 - (Application File Locator)	*/
#define   _EMV_TAG_4F_IC_AID				0x4F		/**<  7 - (Application Identifier) */
#define   _EMV_TAG_82_IC_AIP				0x82		/**<  8 - (Application Interchange Profile) */
#define   _EMV_TAG_50_IC_APPLABEL			0x50		/**<  9 - (Application Label) */
#define   _EMV_TAG_9F12_IC_APNAME 			0x9F12		/**< 10 - (Application Preferred Name) */
#define   _EMV_TAG_5A_IC_PAN				0x5A			/**< 11 - (Application Primary Account Number) */
#define   _EMV_TAG_5F34_IC_PANSN			0x5F34		/**< 12 - (Application Primary Account Number Sequence Number) */
#define   _EMV_TAG_87_IC_APID				0x87			/**< 13 - (Application Priority Indicator)*/
#define   _EMV_TAG_9F3B_IC_APCUR			0x9F3B		/**< 14 - (Application Preference Currency)*/
#define   _EMV_TAG_9F43_IC_APCUREXP			0x9F43		/**< 15 - (Application Preferece Currency Exponent)*/
#define   _EMV_TAG_61_IC_APPTEMP			0x61			/**< 16 - (Application Template)*/
#define   _EMV_TAG_9F36_IC_ATC				0x9F36		/**< 17 - (Application Transaction Counter)*/
#define   _EMV_TAG_9F07_IC_AUC				0x9F07		/**< 18 - (Application Usage Control)*/
#define   _EMV_TAG_9F08_IC_APPVERNO			0x9F08		/**< 19 - (Application Version Number)*/
#define   _EMV_TAG_8C_IC_CDOL1				0x8C			/**< 20 - (Card Risk Management Data Object List 1)*/
#define   _EMV_TAG_8D_IC_CDOL2				0x8D			/**< 21 -  (Card Risk Management Data Object List 2)*/
#define   _EMV_TAG_5F20_IC_HOLDERNAME 		0x5F20		/**< 22 -  (Cardholder Name)*/
#define   _EMV_TAG_9F0B_IC_HOLDERNAMEEX 	0x9F0B		/**< 23 -  (Cardholder Name Extended)*/
#define   _EMV_TAG_8E_IC_CVMLIST			0x8E			/**< 24 -  (Cardholder Verification Method List)*/
#define   _EMV_TAG_8F_IC_CAPKINDEX			0x8F			/**< 25 -  (Certification Authority Public Key Index)*/
#define   _EMV_TAG_9F27_IC_CID				0x9F27		/**< 26 -  (Cryptogram Infomation Data)*/
#define   _EMV_TAG_9F45_IC_DTAUTHCODE 		0x9F45		/**< 27 -  (Data Authentication Code)*/
#define   _EMV_TAG_84_IC_DFNAME 			0x84			/**< 28 -  (Dedicated File Name)*/
#define   _EMV_TAG_9D_IC_DDFNAME			0x9D			/**< 29 -  (Directory Definition File)*/
#define   _EMV_TAG_73_IC_DIRDISCTEMP		0x73			/**< 30 -  (Directory Discretionary Template)*/
#define   _EMV_TAG_9F49_IC_DDOL				0x9F49		/**< 31 -  (Dynamic Data Authentication Data Object List)*/
#define   _EMV_TAG_BF0C_IC_FCIDISCDATA		0xBF0C		/**< 32 -  (File Control Information Issuer Discretionary Data)*/
#define   _EMV_TAG_A5_IC_FCIPROPTEMP		0xA5			/**< 33 -  (File Control Information Proprietary Template)*/
#define   _EMV_TAG_6F_IC_FCITEMP			0x6F			/**< 34 -  (File Control Information Template)*/
#define   _EMV_TAG_9F4C_IC_ICCDYNNUM		0x9F4C		/**< 35 -  (ICC Dynamic Number)*/
#define   _EMV_TAG_9F2D_IC_PECERT 			0x9F2D		/**< 36 -  (ICC PIN Encipherment Public Key Certificate)*/
#define   _EMV_TAG_9F2E_IC_PEEXP			0x9F2E		/**< 37 -  (ICC PIN Encipherment Public Key Exponent)*/
#define   _EMV_TAG_9F2F_IC_PERMD			0x9F2F		/**< 38 -  (ICC PIN Encipherment Public Key Remainder)*/
#define   _EMV_TAG_9F46_IC_ICCPKCERT		0x9F46		/**< 39 -  (ICC Public Key Certificate)*/
#define   _EMV_TAG_9F47_IC_ICCPKEXP			0x9F47		/**< 40 -  (ICC Public Key Exponent)*/
#define   _EMV_TAG_9F48_IC_ICCPKRMD			0x9F48		/**< 41 -  (ICC Public Key Remainder)*/
#define   _EMV_TAG_9F0D_IC_IAC_DEFAULT		0x9F0D		/**< 42 -  (Issuer Action Code-Default)*/
#define   _EMV_TAG_9F0E_IC_IAC_DENIAL 		0x9F0E		/**< 43 -  (Issuer Action Code-Denial)*/
#define   _EMV_TAG_9F0F_IC_IAC_ONLINE 		0x9F0F		/**< 44 -  (Issuer Action Code-Online)*/
#define   _EMV_TAG_9F10_IC_ISSAPPDATA 		0x9F10		/**< 45 -  (Issuer Application Data)*/
#define   _EMV_TAG_9F11_IC_ISSCTINDEX 		0x9F11		/**< 46 -  (Issuer Code Table Index)*/
#define   _EMV_TAG_5F28_IC_ISSCOUNTRYCODE 0x5F28		/**< 47 -  (Issuer Country Code)*/
#define   _EMV_TAG_90_IC_ISSPKCERT			0x90			/**< 48 -  (Issuer Public Key Certificate)*/
#define   _EMV_TAG_9F32_IC_ISSPKEXP			0x9F32		/**< 49 -  (Issuer Public Key Exponent)*/
#define   _EMV_TAG_92_IC_ISSPKRMD			0x92			/**< 50 -  (Issuer Public Key Remainder)*/
#define   _EMV_TAG_5F50_IC_ISSURL 			0x5F50		/**< 51 -  (Issuer URL)*/
#define   _EMV_TAG_5F2D_IC_LANGPREF		0x5F2D		/**< 52 -  (Language Preference)*/
#define   _EMV_TAG_9F13_IC_LASTATC			0x9F13		/**< 53 -  (Last Online Application Transaction Counter Register)*/
#define   _EMV_TAG_9F14_IC_LCOL				0x9F14		/**< 54 -  (Lower Consecutive Offline Limit)*/
#define   _EMV_TAG_9F17_IC_PINTRYCNTR 		0x9F17		/**< 55 -  (Personal Identification Number Try Counter)*/
#define   _EMV_TAG_9F38_IC_PDOL				0x9F38		/**< 56 -  (Processing Options Data Object List)*/
#define   _EMV_TAG_80_IC_RMTF1				0x80			/**< 57 -  (Response Message Template Format 1)*/
#define   _EMV_TAG_77_IC_RMTF2				0x77			/**< 58 -  (Response Message Template Format 2)*/
#define   _EMV_TAG_5F30_IC_SERVICECODE		0x5F30		/**< 59 -  (Service Code)*/
#define   _EMV_TAG_88_IC_SFI					0x88			/**< 60 -  (Short File Indicator)*/
#define   _EMV_TAG_9F4B_IC_SIGNDYNAPPDT	0x9F4B		/**< 61 -  (Signed Dynamic Application Data)*/
#define   _EMV_TAG_93_IC_SIGNSTAAPPDT		0x93			/**< 62 -  (Signed Static Application Data)*/
#define   _EMV_TAG_9F4A_IC_SDATAGLIST 		0x9F4A		/**< 63 -  (Static Data Authentication Tag List)*/
#define   _EMV_TAG_9F1F_IC_TRACK1DATA 		0x9F1F		/**< 64 -  (Track 1 Discretionary Data)*/
#define   _EMV_TAG_9F20_IC_TRACK2DATA 		0x9F20		/**< 65 -  (Track 2 Discretionary Data)*/
#define   _EMV_TAG_57_IC_TRACK2EQUDATA	0x57			/**< 66 -  (Track 2 Equivalent Data)*/
#define   _EMV_TAG_97_IC_TDOL				0x97			/**< 67 -  (Transaction Certificate Data Object List)*/
#define   _EMV_TAG_9F23_IC_UCOL				0x9F23		/**< 68 -  (Upper Consecutive Offline Limit)	*/
#define 	_EMV_TAG_DF31_IC_IISSCRIRES		0xDF31		/**< 69 -  (Issuer Script Results)	*/
#define   _EMV_TAG_9F63_IC_PRODUCTID        0x9F63      /**< 70-  (Card Product Idatification)*/

#define   _EMV_TAG_9F6B_MCC_IC_TRACK2DATA        0x9F6B      /**< Track2 data from MCC Card*/

/*
* Terminal Data Flag
*/
#define   _EMV_TAG_9F01_TM_ACQID			0x9F01	  /**< 0 - (Acquirer Identifier)*/
#define   _EMV_TAG_9F40_TM_CAP_AD 		  	0x9F40	  /**< 1 - (Additional Terminal Capability)*/
#define   _EMV_TAG_81_TM_AUTHAMNTB	   	0x81		  /**< 2 - (Amount,Authorised<Binary>)*/
#define   _EMV_TAG_9F02_TM_AUTHAMNTN	   	0x9F02	  /**< 3 - (Amount,Authorised<Binary>)*/
#define   _EMV_TAG_9F04_TM_OTHERAMNTB 	0x9F04	  /**< 4 - (Amount,Other<Binary>)*/
#define   _EMV_TAG_9F03_TM_OTHERAMNTN 	0x9F03	  /**< 5 - (Amount,Other<Numeric>)*/
#define   _EMV_TAG_9F3A_TM_REFCURAMNT 	0x9F3A	  /**< 6 - (Amount,Reference Currency)*/
#define   _EMV_TAG_9F06_TM_AID			  	0x9F06	  /**< 7 - (Terminal Application Identifier)*/
#define   _EMV_TAG_9F09_TM_APPVERNO		0x9F09	  /**< 8 - (Terminal Application Version Number)*/
#define   _EMV_TAG_89_TM_AUTHCODE		   	0x89		  /**< 9 - (Authorization Code)*/
#define   _EMV_TAG_8A_TM_ARC			   	0x8A		  /**< 10 - (Authorisation Response Code)*/
#define   _EMV_TAG_9F34_TM_CVMRESULT	   	0x9F34	  /**< 11 - (Cardholder Verification Method Results)*/
#define   _EMV_TAG_9F22_TM_CAPKINDEX		0x9F22	  /**< 12 - (Terminal Certification Authority Public Key Index)*/
#define   _EMV_TAG_9F1E_TM_IFDSN			0x9F1E	  /**< 13 - (Interface Device Serial Number)*/
#define   _EMV_TAG_91_TM_ISSAUTHDT	   		0x91		  /**< 14 - (Issuer Authentication Data)*/
#define   _EMV_TAG_86_TM_ISSSCRCMD		   	0x86		  /**< 15 - (Issuer Script Command)*/
#define   _EMV_TAG_9F18_TM_ISSSCRID		   	0x9F18	  /**< 16 - (Issuer Script Identifier)*/
#define   _EMV_TAG_9F15_TM_MCHCATCODE 	0x9F15	  /**< 17 - (Merchant Category Code)*/
#define   _EMV_TAG_9F16_TM_MCHID			0x9F16	  /**< 18 - (Merchant Identifier)*/
#define   _EMV_TAG_9F39_TM_POSENTMODE 	0x9F39	  /**< 19 - (Point-of-Service Entry Mode)*/
#define   _EMV_TAG_9F33_TM_CAP			   	0x9F33	  /**< 20 - (Terminal Capabilities)*/
#define   _EMV_TAG_9F1A_TM_CNTRYCODE	   	0x9F1A	  /**< 21 - (Terminal Country Code)*/
#define   _EMV_TAG_9F1B_TM_FLOORLMT		0x9F1B	  /**< 22 - (Terminal Floor Limit)*/
#define   _EMV_TAG_9F1C_TM_TERMID 		   	0x9F1C	  /**< 23 - (Terminal Identification)*/
#define   _EMV_TAG_9F1D_TM_RMDATA 		   	0x9F1D	  /**< 24 - (Terminal Risk Management Data)*/
#define   _EMV_TAG_9F35_TM_TERMTYPE		0x9F35	  /**< 25 - (Terminal Type)*/
#define   _EMV_TAG_95_TM_TVR			   	0x95		  /**< 26 - (Terminal Verification Result)*/
#define   _EMV_TAG_98_TM_TCHASH 		   	0x98		  /**< 27 - (Transaction Certificate Hash Value)*/
#define   _EMV_TAG_5F2A_TM_CURCODE		   	0x5F2A	  /**< 28 - (Transaction Currency Code)*/
#define   _EMV_TAG_5F36_TM_CUREXP 		   	0x5F36	  /**< 29 - (Transaction Currency Exponent)*/
#define   _EMV_TAG_9A_TM_TRANSDATE	   		0x9A		  /**< 30 - (Transaction Date)*/
#define   _EMV_TAG_99_TM_PINDATA		   	0x99		  /**< 31 - (Transaction Personal Identification Number Data)*/
#define   _EMV_TAG_9F3C_TM_REFCURCODE 	   	0x9F3C	  /**< 32 - (Transaction Reference Currency Code)*/
#define   _EMV_TAG_9F3D_TM_REFCUREXP		0x9F3D	  /**< 33 - (Transaction Reference Currency Exponent)*/
#define   _EMV_TAG_9F41_TM_TRSEQCNTR	   	0x9F41	  /**< 34 - (Transcation Sequence Counter)*/
#define   _EMV_TAG_9B_TM_TSI			   	0x9B		  /**< 35 - (Transaction Status Information)*/
#define   _EMV_TAG_9F21_TM_TRANSTIME	   	0x9F21	  /**< 36 - (Transaction Time)*/
#define   _EMV_TAG_9C_TM_TRANSTYPE	   		0x9C		  /**< 37 - (Transaction Type)*/
#define   _EMV_TAG_9F37_TM_UNPNUM 		   	0x9F37	  /**< 38 -  (Unpredictable Number) */


//EDC Addition
#define _EMV_TAG_72_ISSUE_SCRIPT		0x72	  /**< ISSUE Script Template*/
#define _EMV_TAG_9F28_ARQC		0x9F28	  /**<Authorization requst cryptogram ARQC*/
#define _EMV_TAG_9F29_TC		0x9F29	  /**<Transaction Certificate TC*/
#define _EMV_TAG_9F53_MCC		0x9F53	  /**<Transaction Category Code/Merchant Category Code*/


#endif








