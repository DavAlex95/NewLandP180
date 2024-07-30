
#ifndef _TLV_H_
#define _TLV_H_

/*============================= TLV Structure ===============================*/
typedef struct tlv
{
	/* internal state */
	unsigned char * pusTagPtr;       /* pointer of  'tag' field in the TLV String*/
	unsigned char * pusLenPtr;       /* pointer of  'len' field in the TLV String */
	unsigned int    unOffLen;           /*  length from first pointer  to itself in its parent's string */

	/* parsed information */
	int             nParent;         /* recored the parent position in the tlv_t structure*/
	int             nChildNum;       /* the num of its children (no children = -1)*/

	/**< tag len value*/
	unsigned int    unTagName;       /* record tagname */
	unsigned int    unValueLen;      /* record the length of value */
	unsigned char * pusValue;        /* pointer of value in the TLV String */
} tlv_t, STTLV, STTLVLIST;

/*deflag defined*/
#define SINGLE_TLVOBJ       0x01     /* it is a single constructed object */
#define STRING_TLVOBJ       0x02     /* it is a tlv object string not coded in a constructed object */
#define DECODE_LEVEL1       0x10     /* just decode the object in level one */


#define is_constructed(byte)        ((byte) & 0x20)
#define is_primitive(byte)          !is_constructed(byte)
#define has_subsequent(byte)        (((byte) & 0x1F) == 0x1F)
#define another_byte_follow(byte)   ((byte) & 0x80)
#define lenlen_exceed_one(byte)     ((byte) & 0x80)


extern int TlvParse(unsigned char *pszTlvBuf, const int nTlvBufLen, tlv_t *pstTlvObj, int nTlvArrayNum);
extern int TlvAdd(const uint unTag, const int nInLen, const char *pszInValue, char *pszOutTlvBuf, int *pnOutTlvBufLen);
extern int TlvAddEx(const uchar *usTag, const int nInLen, const char *pszInValue, char *pszOutTlvBuf, int *pnOutTlvBufLen);
extern int TlvUpdate(const uint unTag, const int nInLen, const char *pszInValue, char *pszOutTlvBuf, int *pnOutTlvBufLen);
extern int TlvDelete(const uint unTag, char *pszOutTlvBuf, int *pnOutTlvBufLen);

#endif
