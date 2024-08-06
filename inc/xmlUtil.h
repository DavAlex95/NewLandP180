/***************************************************************************
 * @Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved
 * @FilePath \inc\xmlUtil.h
 * @brief NONE
 * @version v1.0
 * @author linld
 * @date 2019-9-2
 * @reversion history 
 * @1.0 Initial Version
 ***************************************************************************/

#ifndef _XML_H_
#define _XML_H_

#define XML_EMV_CONFIG     "newland.xml"
#define XML_EMV_CONFIG_BK  "EmvCfg.BK"
#define XML_APP_CONFIG "AppCfg.xml"
#define XML_STRING  "String.xml"

// #define XML_EMV_CONFIGNFIG  "Newland_L3_configuration.xml"


extern int Xml_LoadEMVConfig(void);
extern int Xml_LoadAPPConfig(void);
extern int Xml_LoadString(void);

#endif

