/**************************************************************************
* Copyright (C) 2019 Newland Payment Technology Co., Ltd All Rights Reserved
*
* @file		emvconfig.h
* @brief	Parameter Setting AID,CAPK 
*
* @version  0.1
* @date		18.12.06
**************************************************************************/
#ifndef _EMV_CONFIG_H_
#define _EMV_CONFIG_H_

/**
*	Terminal Config Parameters(recommend)
*/
#define   _EMVPARAM_5F2A_TRANSCCODE			0x5F2A	/* 5F2A(Transaction currency code), n3, 2 bytes */
#define   _EMVPARAM_5F36_TRANSCEXP			0x5F36	/* 5F36(Transaction currency exponent), n1, 1 bytes */
#define   _EMVPARAM_9F1A_TERMCCODE			0x9F1A	/* 9F1A(Terminal country code), n3, 2 bytes */
#define   _EMVPARAM_9F1B_FLOORLIMIT			0x9F1B	/* 9F1B(Floor limit), b, 4 bytes */
#define   _EMVPARAM_9F1C_TERMID				0x9F1C	/* 9F1C((Terminal Identification), an8, 8 bytes */
#define   _EMVPARAM_9F1D_TRMDATA            0x9F1D  /* 9F1D(terminal risk manage data), b,1-8 */
#define   _EMVPARAM_9F1E_IFDSERNUM			0x9F1E	/* 9F1E(IFD Serial number), an8, 8 bytes */
#define   _EMVPARAM_9F33_CAP				0x9F33	/* 9F33(Terminal capability), b,  3 */
#define   _EMVPARAM_9F35_TYPE				0x9F35	/* 9F35(Terminal type), n2, 1 */
#define   _EMVPARAM_9F40_ADDCAP				0x9F40	/* 9F40(Addtional Terminal capability), b,  5 */

#define   _EMVPARAM_9F09_APPVER				0x9F09	/* 9F09(application version), b, 2 bytes */
#define   _EMVPARAM_9F39_POSENTRY			0x9F39	/* 9F39(POS entry mode), n2, 1 bytes */
#define   _EMVPARAM_9F01_ACQID				0x9F01	/* 9F01(Acquirerer identifier), n6-11, 6 bytes */
#define   _EMVPARAM_9F15_MERCACODE			0x9F15	/* 9F15(Merchant category code), n4, 2 bytes */
#define   _EMVPARAM_9F16_MERCHID			0x9F16	/* 9F16(Merchant Identifier), ans15, 15 bytes */
#define   _EMVPARAM_9F3C_TRANSREFCCODE		0x9F3C	/* 9F3C(Reference currency code), n3, 2 bytes */
#define   _EMVPARAM_9F3D_TRANSREFCEXP		0x9F3D	/* 9F3D(Feference currency exponent), n1, 1 bytes */
#define   _EMVPARAM_9F4E_MERCNAME			0x9F4E	/* 9F4E(Merchant Name), ans20, 20 bytes */	
#define   _EMVPARAM_DF44_DEDDOL				0xDF44	/*default ddol, var*/
#define   _EMVPARAM_DF45_DETDOL				0xDF45	/*default tdol, var*/
#define   _EMVPARAM_DF15_THRESHOLDVA		0xDF15	/*thresold value, n4*/
#define   _EMVPARAM_DF16_MAXTARPER	   		0xDF16	/*max target percent,n1*/
#define   _EMVPARAM_DF17_TARGETPER			0xDF17	/*target percent, n1*/
#define   _EMVPARAM_DF24_ICS				0xDF24 	/* ICS (Implementation Comformance Statement), b, 7*/
#define   _EMVPARAM_DF25_STATUS				0xDF25	/* Test type indicator,n1*/
#define   _EMVPARAM_9F7A_ECIND				0x9F7A	/* 9F7A if Terminal  support EC?Or paywave: supprt wave2 ?support  = 1, n1*/
#define   _EMVPARAM_DF22_TRANSCONV			0xDF22	/*transaction reference currency convert, n4,default 0 */
#define   _EMVPARAM_DF26_FALLPOTERY			0xDF26	/* fallback pos entry, n1*/
#define   _EMVPARAM_DF01_APPSELIND			0xDF01	/*application select indicator,n1*/
#define   _EMVPARAM_DF2A_APPID				0xDF2A	/*Application ID */	
#define   _EMVPARAM_DF3B_EXAIDSUPP			0xDF3B	/*Extern aid supprort*/
#define   _EMVPARAM_DF65_TERMINALPRIORITY   0xDF65  //terminal priority
#define   _EMVPARAM_DF11_TACDEFAULT			0xDF11	/*Terminal Action Code-default, n5*/
#define   _EMVPARAM_DF13_TACDENIAL			0xDF13	/*Terminal Action Code-denial , n5*/
#define   _EMVPARAM_DF12_TACONLINE  		0xDF12	/*Terminal Action Code-online , n5*/
#define   _EMVPARAM_9F7B_ECLIMIT			0x9F7B    /* EMV,PBOC use for Ecash terminal limit, Paypass used for READER_CLSS_TRANS_LIMIT_ON_DEVICE_CVM(DF8125),n12  6bytes*/
#define   _EMVPARAM_1F8102_PPSEAIDSEL		0x1F8102  /* CL,Select by AID list,n1*/
#define   _EMVPARAM_DF27_LIMITEXIST			0xDF27	  /* CL,limist exist?(To determine the identity of the following limit exists),b8, 1*/
#define   _EMVPARAM_DF20_CLLIMMIT			0xDF20	  /* CL,contactless terminal transaction limit n12  6bytes */		
#define   _EMVPARAM_DF19_CLOFFLIMIT			0xDF19	  /* CL,contactless terminal offline minimum limit n12  6bytes */	
#define   _EMVPARAM_DF21_CVMLIMT			0xDF21	  /* CL,terminal implement CVM Limit, n12  6bytes */	
#define   _EMVPARAM_DF39_STATUSCHECK		0xDF39	  /* CL,status check*/	
#define   _EMVPARAM_DF3A_ZEROALLOW			0xDF3A	  /* CL,zero amount allow*/	
#define	  _EMVPARAM_DF63_IOOPTION			0xDF63	  /* pure, io_options*/
#define   _EMVPARAM_DF64_TERMINALAIDVALUE   0xDF64    /* pure, terminal aid value*/
#define   _EMVPARAM_1F8118_VerifyTrackData		0x1F8118  /*EMV,validate track 2 equivalent data against the PAN and expiry date*/
//#define   _EMVPARAM_1F8119_ZeroFloorForCashBack	0x1F8119  /*EMV,apply a zero value floor limit for cashback transactions*/
#define   _EMVPARAM_1F811F_CUSTOM_TAG			0x1F811F  /**Custom defined tags*/
#define	  _EMVPARAM_1F8120_CUSTOM_FLOW			0x1F8120  /**Special transaction processing flow options. */
#define	  _EMVPARAM_1F812B_Multiple_Application_Version			0x1F812B  /**Multiple_Application_Version */

/** 
*	Part II: 
*	AID Config Parameters
*/
#define   _EMVPARAM_9F06_AID				0x9F06	/*AID, 4F(ICC), 9F06(Terminal), b, 5-16 bytes */
#define   _EMVPARAM_DF37_KERNELID			0xDF37	/*kernel id*/	
#define   _EMVPARAM_DF25_STATUS				0xDF25	/* Test type indicator,n1*/
#define   _EMVPARAM_9F66_TRANSPROP			0x9F66	/*9F66/9F6E(AE) Terminal transaction attribute ,b32  4bytes*/	

//Paypass
#define   _EMVPARAM_9F7B_ECLIMIT			0x9F7B  /*Paypass,READER_CLSS_TRANS_LIMIT_ON_DEVICE_CVM(DF8125), [history reason,tag same as EMV EC-limit], n12  6bytes*/
#define   _EMVPARAM_DF2B_DEUDOL				0xDF2B  /*Paypass,default udol*/    
#define   _EMVPARAM_DF2C_MAGSTRIND			0xDF2C	/*paypass,--unused-- if value=0,terminal will not support SDA*/	
#define   _EMVPARAM_DF2D_MAGAPPVER			0xDF2D	/*paypass mag stripe application version 9F6D*/	
#define   _EMVPARAM_DF2E_DEXCHANGE			0xDF2E	/*Paypass,support data exchang or not*/	
#define   _EMVPARAM_DF2F_KERNELCONF			0xDF2F	/*Paypass,b8  Only EMV mode b7  Only mag-stripe mode b6 On device cardholder verification*/	
#define   _EMVPARAM_DF32_MNUMTORN			0xDF32	/*Paypass, Max Number of Torn Transaction Log Records !=0 support Torn*/	
#define   _EMVPARAM_DF42_MAGSCVM			0xDF42	/*Paypass,Mag-stripe CVM Capability - CVM Required */
#define   _EMVPARAM_DF43_MEXLTTORN			0xDF43	/*Paypass,Max Lifetime of Torn Transaction Log Record  0x012c */
#define   _EMVPARAM_DF46_MOSUPPIND			0xDF46	/*Paypass,Mobile Support Indicator*/	
#define   _EMVPARAM_DF47_MAGSNOCVM			0xDF47	/*Paypass,Mag-stripe CVM Capability - No CVM Required */	
#define   _EMVPARAM_DF48_CAPNOCVM			0xDF48	/*Paypass,CVM Capability- No CVM Required*/	
#define   _EMVPARAM_DF52_PPTLV              0xDF52  /*Paypass,TLV data string*/
#define	  _EMVPARAM_DF54_MAXRRPGP			0xDF54	 /*paypass 3.1 rrp Maximum Relay Resistance Grace Period*/
#define	  _EMVPARAM_DF55_MINRRPGP			0xDF55	 /*paypass 3.1 rrp Minimum Relay Resistance Grace Period*/
#define	  _EMVPARAM_DF56_RRPAT				0xDF56	 /*paypass 3.1 rrp Relay Resistance Accuracy Threshold*/
#define	  _EMVPARAM_DF57_RRPTTMT			0xDF57	 /*paypass 3.1 rrp Relay Resistance Transmission Time Mismatch Threshold*/
#define	  _EMVPARAM_DF58_TETTFRRC			0xDF58	 /*paypass 3.1 rrp Terminal Expected Transmission Time For Relay Resistance C-APDU*/
#define	  _EMVPARAM_DF59_TETTFRRR			0xDF59	 /*paypass 3.1 rrp Terminal Expected Transmission Time For Relay Resistance R-APDU*/
#define _EMVPARAM_1F8101_TRANSTYPECKFLAG	0x1F8101  /*paypass, trans type check flag,use for paypass,n1 */
#define	_EMVPARAM_DF7D_TRANSTYPE			0xDF7D	  /*paypass, trans type, [same as Pure TYPEAAT],n1,*/

//Paywave
#define   _EMVPARAM_DF34_PWCONFIG			0xDF34	/*paywave, config */	
#define   _EMVPARAM_DF3D_DRLSTATUS			0xDF3D	/*Paywave,0x00 Deactivated; 0x01 Activated*/
#define   _EMVPARAM_DF3F_DRLDATA			0xDF3F	/*Paywave,DRL Data len = 8*36*/

//Express
#define _EMVPARAM_DF49_EXTERMCAP			0xDF49	/*expresspay 3.0 9F6D terminal capability*/	
#define _EMVPARAM_DF4A_EXRANDOM				0xDF4A	/*expresspay 3.0 rand scope*/	
#define _EMVPARAM_DF4B_EXTIMEEX				0xDF4B	/*expresspay 3.0 out-time*/	
#define	_EMVPARAM_DF53_EXDRLDATA			0xDF53	/*expresspay 3.1 drl limit*/

//JCB
#define	_EMVPARAM_DF60_COMBINATIONOPT		0xDF60	//JCB Combination options
#define	_EMVPARAM_9F53_TIP					0x9F53	//JCB Terminal Interchange Profile (static)

//Pure
#define	_EMVPARAM_DF62_CONTAPPCAP			0xDF62	//pure Contactless App Cap
#define	_EMVPARAM_BF71_MEMSLOTREADTEM		0xBF71	//pure memory slot read tempklate
#define	_EMVPARAM_BF70_MEMSLOTUPDATETEM		0xBF70	//pure memory slot update tempklate
#define	_EMVPARAM_9F76_TERMTRANSDATA		0x9F76	//pure terminal transactioin data
#define	_EMVPARAM_DF66_MTOL					0xDF66	//pure mtol
#define	_EMVPARAM_DF79_ATDTOL				0xDF79	//pure atdtol
#define	_EMVPARAM_DF7A_POSTIMEOUTTRANS		0xDF7A	//pure postimeout transaction
#define	_EMVPARAM_DF7B_AUTORUN				0xDF7B	//pure auto run
#define	_EMVPARAM_DF7C_POSTIMEOUTLONG		0xDF7C	//pure POSTIMEOUTLONG	
#define	_EMVPARAM_DF7D_TRANSTYPE			0xDF7D	//pure TYPEAAT [same as Paypass TransType]
#define	_EMVPARAM_DF7E_ATOL					0xDF7E	//pure ATOL

//Interac
#define	_EMVPARAM_9F59_TTI					0x9F59	//interac Terminal Transaction Information (TTI)
#define	_EMVPARAM_9F5E_TOS					0x9F5E	//interac Terminal Option Status (TOS)
#define	_EMVPARAM_9F58_MTI					0x9F58	//interac Merchant Type Indicator
#define	_EMVPARAM_9F5D_TerConRecLimit		0x9F5D	//interac Terminal Contactless Receipt Required Limit
#define	_EMVPARAM_9F5A_TTT					0x9F5A	//interac TTT
#define	_EMVPARAM_DF4C_INTERAC_RELimit		0xDF4C	//Retry Limit

//Rupay
#define _EMVPARAM_1F8103_ADDTERCAP			0x1F8103  //rupay Additional Terminal Capabilities Extension DF3A
#define _EMVPARAM_1F8104_SERVICEID			0x1F8104  //rupay service id DF16.
#define _EMVPARAM_1F8105_SERVICEQUA			0x1F8105  //rupay Service Qualifier
#define _EMVPARAM_1F8106_SERVICEDATA		0x1F8106  //rupay Service Data DF45
#define _EMVPARAM_1F8107_SERVICEPRMISS		0x1F8107  //rupay PRMiss DF47
#define _EMVPARAM_1F8108_SERVICEPRMACQKEY	0x1F8108  //rupay PRMacq Key value

//Extend
#define _EMVPARAM_1F8113_DataRecord_EMV		0x1F8113  //Data Record Tags (EMV)
#define _EMVPARAM_1F8114_DataRecord_MSD		0x1F8114  //Data Record Tags (MSD / Magstripe / Non-EMV)
#define _EMVPARAM_1F8115_DiscretionaryData_EMV		0x1F8115  //Discretionary Data Tags (EMV)
#define _EMVPARAM_1F8116_DiscretionaryData_MSD		0x1F8116  //Discretionary Data Tags (MSD / Magstripe / Non-EMV)
#define _EMVPARAM_1F8117_Optional_DataRecord_MSD	0x1F8117  //Optional Data Record Tags (MSD / Magstripe / Non-EMV)

//MIR
#define _EMVPARAM_1F811A_NOCVMLIMIT						0x1F811A  //Terminal No Cvm Limit DF52
#define _EMVPARAM_1F811B_CONTACTLESSLIMIT_NOCDCVM		0x1F811B  //Terminal Contactless Limit (Non CD-CVM) DF53
#define _EMVPARAM_1F811C_CONTACTLESSLIMIT_CDCVM		0x1F811C  //Terminal Contactless Limit (CD-CVM) DF54
#define _EMVPARAM_1F811D_TERTPM_CAP						0x1F811D  //Terminal TPM Capabilities DF55
#define _EMVPARAM_1F811E_DE_TAGLIST						0x1F811E  //Data Exchange Tag List

//Multibanco
#define _EMVPARAM_DF8118_CVMCAP_ABOVEEQUALLIMIT		0xDF8118  //CVM Capabilities (above or equal to CVM Limit)
#define _EMVPARAM_DF8119_CVMCAP_BELOWLIMIT				0xDF8119  //CVM Capabilities (below to CVM Limit)



/* x is: STPARAM.ucLimitExist
        Parameters used to determine whether the definition of the corresponding terminal limit.
        1 indicates  exist
        0 does not exist
*/
#define EC_LIMIT_EXIST(x)							( (x) & 0x01)       /*electronic cash limit exist or not*/
#define CL_LIMIT_EXIST(x)							( (x) & 0x02)       /*contactless terminal transaction limits exist or not*/
#define CL_OFFLINE_LIMIT_EXIST(x)					( (x) & 0x04)       /* contactless terminal Minimum offline amount exist or not*/
#define CVM_LIMIT_EXIST(x)							( (x) & 0x08)       /*Limit of  the process CVM exist  or not*/
#define CL_OFFLINE_LIMIT_SUPPROT_BUT_NOPRESENT(x)	( (x) & 0x10 )		/* contactless terminal Minimum offline amount support but no present*/
#define TERM_OFFLINE_LIMIT_PRESENT(x)				( (x) & 0x20 )
#define CL_ZERO_OPTION1_2(x)						( (x) & 0x20 )      /*Zero option1(1) or option2(0)*/
#define CL_STATUS_CHECK(x)							( (x) & 0x40 )      /*Support status check*/
#define CL_ZERO_ALLOW_CHECK(x)						( (x) & 0x80 )      /*Zero amount allow check*/


/*STPARAM.ucKernelConfig */
/* b8  Only EMV mode transactions supported
    b7  Only mag-stripe mode transactions supported
    b6  On device cardholder verification supported
    b5-1  RFU */
#define KERNEL_CONFIG_ONLY_EMV_MODE(x)				( (x) & 0x80 )
#define KERNEL_CONFIG_ONLY_MAGSTRIPE_MODE(x)		( (x) & 0x40 )
#define KERNEL_CONFIG_ON_DEVICE_CV(x)				( (x) & 0x20 )
#define KERNEL_CONFIG_RELAY_RESISTANCE_PROTOCOL(x)	( (x) & 0x10 )
#define KERNEL_CONFIG_ON_DF61(x)					( (x) & 0x08 )

/* AID STCANDIDATE list structure, used for STCANDIDATE list selection
* use for Contactless
*/


/**********************************ICS************************************************/
/*
 * AS   : Application Selection
 * Macro:
   AS_Support_PSE                 : Support PSE selection method
   AS_Support_CardHolder_Confirm  : Support Cardholder confirmation
   AS_Support_Prefferd_Order      : Have a preferred order of displaying applications
   AS_Support_Partial_AID         : Does the terminal perform partial AID selection
   AS_Support_Multi_Language      : Does the terminal have multi language support
   AS_Support_Common_Charset      : Does the terminal support Common Character Set as
                                    defined in "Annex B table 20 Book 4"

 * EMV 4.1 ICS Version 3.9 Level2
 */
#define AS_Support_PSE                     (0x0080)
#define AS_Support_CardHolder_Confirm      (0x0040)
#define AS_Support_Preferred_Order         (0x0020)
#define AS_Support_Partial_AID             (0x0010)
#define AS_Support_Multi_Language          (0x0008)
#define AS_Support_Common_Charset          (0x0004)

/*
 * DA   : Data Authentication
 * IPKC : Issuer Public Key Certificate
 * CAPK : Certification Authority Public Key
 * Macro:
     DA_Support_IPKC_Revoc_Check      : During DA, does the terminal check the revocation of IPKC
     DA_Support_Default_DDOL          : Does the terminal contain a default DDOL
     DA_Support_CAPKLoad_Fail_Action  : Is operation action required when loading CAPK fails
     DA_Support_CAPK_Checksum         : Is CAPK verified with CAPK checksum

 * EMV 4.1 ICS Version 3.9 Level2
 */
#define DA_Support_IPKC_Revoc_Check        (0x0180)
#define DA_Support_Default_DDOL            (0x0140)
#define DA_Support_CAPKLoad_Fail_Action    (0x0120)
#define DA_Support_CAPK_Checksum           (0x0110)


/*
 * CV   : Cardholder Verification
 * CVM   : Cardholder Verification Methods
 * Macro:
 	 CV_Support_Bypass_PIN            : Terminal supports bypass PIN entry
	 CV_Support_Subsequent_Bypass_PIN : Terminal supports subsequent bypass PIN entry
 	 CV_Support_PIN_Try_Counter       : Terminal supports Get Data for PIN Try Counter
 	 CV_Support_Fail_CVM              : Terminal supports Fail CVM
 	 CV_Support_Amounts_before_CVM    : Are amounts known before CVM processing

 * EMV 4.3 ICS Version 4.3c Level2
 */
#define CV_Support_Bypass_PIN              (0x0280)
#define CV_Support_PIN_Try_Counter         (0x0240)
#define CV_Support_Fail_CVM                (0x0220)
#define CV_Support_Amounts_before_CVM      (0x0210)
#define CV_Support_Subsequent_Bypass_PIN   (0x0208)

/*
 * TRM  : Terminal Risk Management
 * Macro:
   TRM_Support_FloorLimit     : Floor Limit Checking,
                                Mandatory for terminal with offline capability
   TRM_Support_RandomSelect   : Random Transaction Selections,
                                Mandatory for offline terminal with online capability,
                                except when cardholder controlled
   TRM_Support_VelocityCheck  : Velocity checking,
                                Mandatory for for terminal with offline capability
   TRM_Support_TransLog       : Support transaction log
   TRM_Support_ExceptionFile  : Support exception file
   TRM_Support_AIPBased       : Performance of TRM based on AIP setting
   TRM_Use_EMV_LogPolicy      : EMV has a different log policy with PBOC2, marked here

 * EMV 4.1 ICS Version 3.9 Level2
 */
#define TRM_Support_FloorLimit            (0x0380)
#define TRM_Support_RandomSelect          (0x0340)
#define TRM_Support_VelocityCheck         (0x0320)
#define TRM_Support_TransLog              (0x0310)
#define TRM_Support_ExceptionFile         (0x0308)
#define TRM_Support_AIPBased              (0x0304)
#define TRM_Use_EMV_LogPolicy             (0x0302)

/*
 * TAA  : Terminal Action Analysis
 * (x)  : the var of struct STCONFIG
 * TAC  : Terminal Action Codes
 * DAC  : Default Action Codes
 * Macro:
   TAA_Support_TAC                  : Does the terminal support Terminal Action Codes
   TAA_Support_DAC_before_1GenAC    : Does the terminal process DAC prior to first GenAC
   TAA_Support_DAC_after_1GenAC     : Does the terminal process DAC after first GenAC
   TAA_Support_Skip_DAC_OnlineFail  : Does the terminal skip DAC processing and automatically
                                      request an AAC when unable to go online
   TAA_Support_DAC_OnlineFail       : Does the terminal process DAC as normal
                                      when unable to go online
   TAA_Support_CDAFail_Detected     : Device capable of detecting CDA Failure before TAA
   TAA_Support_CDA_Always_in_ARQC   : CDA always requested in a first Gen AC, ARQC request
   TAA_Support_CDA_Never_in_ARQC    : CDA never requested in a first Gen AC, ARQC request
   TAA_Support_CDA_Alawys_in_2TC    : CDA always requested in a second Gen AC when successful
                                      host response is received, with TC request
   TAA_Support_CDA_Never_in_2TC     : CDA never requested in a second Gen AC when successful
                                      host response is received, with TC request
 * EMV 4.1 ICS Version 3.9 Level2
 */
#define TAA_Support_TAC                    (0x0480)
#define TAA_Support_DAC_before_1GenAC      (0x0440)
#define TAA_Support_DAC_after_1GenAC       (0x0420)
#define TAA_Support_Skip_DAC_OnlineFail    (0x0410)
#define TAA_Support_DAC_OnlineFail         (0x0408)
#define TAA_Support_CDAFail_Detected       (0x0404)
#define TAA_Support_CDA_Always_in_ARQC     (0x0402)
#define TAA_Support_CDA_Alawys_in_2TC      (0x0401)

/*
 * CP  : Completion Process
 * (x)  : the var of struct STCONFIG
 * Macro:
   CP_Support_Force_Online         : Transaction forced Online capability
   CP_Support_Force_Accept         : Transaction forced Acceptance capability
   CP_Support_Advices              : Does the terminal support advices
   CP_Support_Issuer_VoiceRef      : Does the terminal support Issuer Initiated Voice Referrals
   CP_Support_Batch_Data_Capture   : Does the terminal support Batch Data Capture
   CP_Support_Online_Data_capture  : Does the terminal support Online Data Capture
   CP_Support_Default_TDOL         : Does the terminal support a default TDOL

 * EMV 4.1 ICS Version 3.9 Level2
 */
#define CP_Support_Force_Online            (0x0580)
#define CP_Support_Force_Accept            (0x0540)
#define CP_Support_Advices                 (0x0520)
#define CP_Support_Issuer_VoiceRef         (0x0510)
#define CP_Support_Batch_Data_Capture      (0x0508)
#define CP_Support_Online_Data_capture     (0x0504)
#define CP_Support_Default_TDOL            (0x0502)

/*
 * MISC : Miscellaneous
 * (x)  : the var of struct STCONFIG
 * Macro:
   MISC_Support_Account_Select         : Does the terminal support account type selection
   MISC_Support_ISDL_Greater_than_128  : Is Issuer Script Device Limit greater than 128 bytes
   MISC_Support_Internal_Date_Mana     : Does the terminal support internal date management

 * EMV 4.1 ICS Version 3.9 Level2
 */
#define MISC_Support_Account_Select        (0x0680)
#define MISC_Support_ISDL_Greater_than_128 (0x0640)
#define MISC_Support_Internal_Date_Mana    (0x0620)
#define MISC_PP_Support_Default_UDOL       (0x0602)    /* default UDOL (paypass only)*/
#define MISC_MISC_PP_Support_MagAppVer     (0x0601)    /* mag stripe application version (paypass only) */


/* Used to determine the ICS support functions,
        1 support
        0 not supported
        nr      means the ICS supports these functions above macro definitions: XXX_Support_XXX
        usIcs    ICS pointer is passed */
#define ics_opt_get(nr, usIcs) \
        ((usIcs)[((unsigned int)(nr)) >> 8] & (((unsigned int)(nr)) & 0x00FF))

/*Settings for ICS-related functions*/
#define ics_opt_set(nr, usIcs) \
        ((usIcs)[((unsigned int)(nr)) >> 8] |= (((unsigned int)(nr)) & 0x00FF))

/*ICS supports these functions cancel*/
#define ics_opt_unset(nr, usIcs) \
        ((usIcs)[((unsigned int)(nr)) >> 8] &= ~(((unsigned int)(nr)) & 0x00FF))

/*
   RESV_Terminal_Present_DefaultTAC
   RESV_Terminal_Present_DenialTAC
   RESV_Terminal_Present_OnlineTAC     : to indicate whether the terminal provide these TACs
   define for external parameters tool TAC-related assignments
 */
#define RESV_Terminal_Present_DefaultTAC	(0x0008)
#define RESV_Terminal_Present_DenialTAC		(0x0004)
#define RESV_Terminal_Present_OnlineTAC		(0x0002)

#define _tac_status_get(nr, ucStatus)		ics_opt_get(nr, ucStatus)
#define _tac_status_set(nr, ucStatus)		ics_opt_set(nr, ucStatus)
#define _tac_status_unset(nr, ucStatus)		ics_opt_unset(nr, ucStatus)

/*=========================================================================
 * TT : Terminal Type(STPARAM.ucType)
 */
#define TT_Unattended(x)					((*(x) & 0x0F) > 3)	/*unattend terminal*/
#define TT_Attended(x)						((*(x) & 0x0F) < 4)	/*attend terminal*/


/*=========================================================================*/

/*
 * TC : Terminal Capabilities(STPARAM.usCap)        Terminal Capabilities
 */
#define TC_Manual_Key_Entry          0x0080        /*Manual key entry */
#define TC_Magnetic_Stripe           0x0040        /*Magnetic stripe */
#define TC_IC_With_Contacts          0x0020        /*IC with contacts */

#define TC_Plaintext_PIN             0x0180        /*Plaintext PIN for ICC verification */
#define TC_Enciphered_PIN_Online     0x0140        /*Enciphered PIN for online verification */
#define TC_Signature_Paper           0x0120        /*Signature (paper) */
#define TC_Enciphered_PIN_Offline    0x0110        /*Enciphered PIN for offline verification */
#define TC_No_CVM_Required           0x0108        /*No CVM Required */
#define TC_Cardholder_Cert           0x0101        /*Cardholder Cert*/

#define TC_SDA                       0x0280        /*SDA*/
#define TC_DDA                       0x0240        /*DDA*/
#define TC_Card_Capture              0x0220        /*Card capture */
#define TC_CDA                       0x0208        /*CDA*/

/*    Determine the functions supported by the terminal Capabilities
    1    support
    0    not support
    nr:    above define function
    cap:    Terminal Capabilities point*/
#define terminal_cap(nr, cap)          ics_opt_get(nr, cap)

/* Set the terminal capabilities to support related functions */
#define terminal_cap_set(nr, cap)      ics_opt_set(nr, cap)

/* Set the terminal capabilities to not support related functions */
#define terminal_cap_unset(nr, cap)    ics_opt_unset(nr, cap)

/* Whether the terminal supports offline PIN (plaintext + ciphertext) */
#define terminal_offline_pin(cap)      (*(cap + 1) & 0x90)

/*
 * ATC : Additional Terminal Capabilities(STPARAM.usAddCap)
 */
#define ATC_Cash                           0x0080    /*cash*/
#define ATC_Goods                          0x0040    /*goods*/
#define ATC_Services                       0x0020    /*sevice*/
#define ATC_Cashback                       0x0010    /*cashback*/
#define ATC_Inquiry                        0x0008    /*inquiry*/
#define ATC_Transfer                       0x0004    /*transfer*/
#define ATC_Payment                        0x0002    /*payment*/
#define ATC_Administrative                 0x0001    /*administrative*/
#define ATC_Cash_Deposit                   0x0180    /*cash deposit*/

#define ATC_Numeric_Keys                   0x0280    /*Numeric key*/
#define ATC_Alphabetic_Special_Keys        0x0240    /*Alphabetic and special characters keys */
#define ATC_Command_Keys                   0x0220    /*Command keys*/
#define ATC_Function_Keys                  0x0210    /*Function key*/

#define ATC_Print_Attendant                0x0380    /*Print,attendant*/
#define ATC_Print_Cardholder               0x0340    /*Print,cardholder*/
#define ATC_Display_Attendant              0x0320    /*Display,attendant*/
#define ATC_Display_Cardholder             0x0310    /*Display,cardholder*/

#define ATC_Code_Table_10                  0x0302    /*Code table 10*/
#define ATC_Code_Table_9                   0x0301    /*Code table 9*/
#define ATC_Code_Table_8                   0x0480    /*Code table 8*/
#define ATC_Code_Table_7                   0x0440    /*Code table 7*/
#define ATC_Code_Table_6                   0x0420    /*Code table 6*/
#define ATC_Code_Table_5                   0x0410    /*Code table 5*/
#define ATC_Code_Table_4                   0x0408    /*Code table 4*/
#define ATC_Code_Table_3                   0x0404    /*Code table 3*/
#define ATC_Code_Table_2                   0x0402    /*Code table 2*/
#define ATC_Code_Table_1                   0x0401    /*Code table 1*/

/*    Determine Additional Terminal Capabilities
    1    support
    0    not support
    nr:    above define Additional Terminal Capabilities
    addcap:    Additional Terminal Capabilities point*/
#define add_terminal_cap(nr, addcap)        ics_opt_get(nr, addcap)

/* Set the Additional Terminal Capabilities to support related functions */
#define add_terminal_cap_set(nr, addcap)    ics_opt_set(nr, addcap)

/* Set the Additional Terminal Capabilities not to support related functions */
#define add_terminal_cap_unset(nr, addcap)  ics_opt_unset(nr, addcap)

/*
  *Terminal transaction attribute 9F66 (STPARAM.usTransProp)
      ==== Paypass only temporary support Mag Stripe (equivalent domestic MSD) and M / CHIP (domestic QPBOC equivalent)====
  */
#define EMV_PROP_MSD            0x0080              /*1:support CLSS (MSD) */
#define EMV_PROP_VSDCCLSS       0x0040              /*1:clss VSDC supported 0:clss VSDC not supported*/
#define EMV_PROP_PBOCCLSS       EMV_PROP_VSDCCLSS   /*1:support CLSS PBOC(VCPS not support)And on the same value*/
#define EMV_PROP_QVSDC          0x0020              /*1:support QVSDC*/
#define EMV_PROP_QPBOC          EMV_PROP_QVSDC      /*1:support qPBOC*/
#define EMV_PROP_VSDC           0x0010              /*1:support contact VSDC*/
#define EMV_PROP_PBOC           EMV_PROP_VSDC       /*1:support PBOC*/

#define EMV_PROP_OFFLINE_ONLY   0x0008              /*1:reader offline only;0:reader with online ability*/
#define EMV_PROP_ONLINEPIN      0x0004              /*1:support online PIN*/
#define EMV_PROP_SIGNATURE      0x0002              /*1:support signature*/
#define EMV_PROP_ODAONLINE      0x0001              /*1:support online ODA(VCPS not support)*/
/*Readers compliant to this specification must set TTQ byte 1 bit 1 to 0b. */
/*byte1 bit1 usReserve */

#define EMV_PROP_ONLINEAC       0x0180    /*1:Request online*/
#define EMV_PROP_CVM            0x0140    /*1:Request CVM*/
#define EMV_PROP_OFFLINEPIN     0x0120    /*1:Support Contact offline PIN(VCPS Contact chip)*/

#define EMV_PROP_IUP            0x0280    /*1:Support Issuer Update Processing*/
#define EMV_PROP_CDCVM          0x0240    /*1:Support Consumer Device CVM ( mobile )
VCPS should support*/
/*other bits usReserve */
#define EMV_PROP_01VERSUPPORT   0x0380

/**
 * Function to get the Terminal transaction Property
 *  1           support
 *  0           unsupport
 * nr:          A variety of functions defined above.
 * transprop:   Incoming Terminal transaction Property pointer
 */
#define trans_prop(nr, usTransProp)        ics_opt_get(nr, usTransProp)

/**< Set Terminal transaction Property support function */
#define trans_prop_set(nr, usTransProp)    ics_opt_set(nr, usTransProp)

/**< Unset Terminal transaction Property support function */
#define trans_prop_unset(nr, usTransProp)  ics_opt_unset(nr, usTransProp)



/*use for paywave drl*/
typedef struct
{
	unsigned char ucDrlAppIDLen;
	unsigned char usDrlAppId[16]; /*9F5A application ID 1-4 */
	unsigned char ucClLimitExist; /* limist exist?
                                    bit 1    RFU
                                    bit 2    =1    contactless limit exist
                                    bit 3    =1    contactless offline limit exist
                                    bit 4    =1    cvm limit  exist
                                    bit 5    RFU
                                    bit 6    =1    zero option1 or option2
                                    bit 7    =1    support status check
                                    bit 8    =1    zero amount allow check */
	unsigned char usClTransLimit[6];          /* clss transaction limit n12  6bytes */
	unsigned char usClOfflineLimit[6];        /* contactless offline limit n12  6bytes */
	unsigned char usClCvmLimit[6];            /* cvm limit   n12  6bytes */
} STDRLPARAM;

/*use for expresspay drl*/
typedef struct
{
	unsigned char ucDrlExist;
	unsigned char ucDrlId;
	unsigned char ucClLimitExist; /* limist exist?
                                    bit 1    =1
                                    bit 2    =1    contactless limit exist
                                    bit 3    =1    contactless offline limit exist
                                    bit 4    =1    cvm limit  exist
                                    RFU
                                    bit 7    =1    support status check
                                    bit 8    =1    zero amount allow*/                            /*this drl limit is present or not*/
	unsigned char usClTransLimit[6];          /* clss transaction limit n12  6bytes */
	unsigned char usClOfflineLimit[6];        /* contactless offline limit n12  6bytes */
	unsigned char usClCvmLimit[6];            /* cvm limit   n12  6bytes */
	unsigned char usResv[3];
} STEXDRLPARAM;


#endif /* ifndef _EMV_CONFIG_H_.2018-8-17 14:26:04 */

