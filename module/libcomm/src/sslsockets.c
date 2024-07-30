#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <memory.h>
#include <pthread.h>
#include <dlfcn.h>
#include <openssl/ossl_typ.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include "sslsockets.h"
#include "lui.h"
#include "ltool.h"

typedef struct _sslhandle {
    int CloseFlag;
    SSL *CurrentSsl;
    SSL_CTX *CurrentCtx;
    int CurrentSocketMode;
    int CurrentConnectState;
    int CurrentSocket;
    int TimeOutFlag;
    int timeout;
} SSLHANDLE;

#define DEBUG 1
#ifdef DEBUG
#define PDEBUG(fmt, args...) PubDebug("SSL: " fmt, ##args)
#else
#define PDEBUG(fmt, args...)
#endif

#define PBE_SHA1_3DES "pbeWithSHA1And3-KeyTripleDES-CBC"
static const char *mon[12]= {
    "Jan","Feb","Mar","Apr","May","Jun",
    "Jul","Aug","Sep","Oct","Nov","Dec"
};
static int MY_ASN1_UTCTIME_print(const ASN1_UTCTIME *tm)
{
    const char *v;
    int gmt=0;
    int i;
    int y=0,M=0,d=0,h=0,m=0,s=0;

    i=tm->length;
    v=(const char *)tm->data;

    if (i < 10) goto err;
    if (v[i-1] == 'Z') gmt=1;
    for (i=0; i<10; i++)
        if ((v[i] > '9') || (v[i] < '0')) goto err;
    y= (v[0]-'0')*10+(v[1]-'0');
    if (y < 50) y+=100;
    M= (v[2]-'0')*10+(v[3]-'0');
    if ((M > 12) || (M < 1)) goto err;
    d= (v[4]-'0')*10+(v[5]-'0');
    h= (v[6]-'0')*10+(v[7]-'0');
    m=  (v[8]-'0')*10+(v[9]-'0');
    if (tm->length >=12 &&
        (v[10] >= '0') && (v[10] <= '9') &&
        (v[11] >= '0') && (v[11] <= '9'))
        s=  (v[10]-'0')*10+(v[11]-'0');

    if (fprintf(stderr,"%s %2d %02d:%02d:%02d %d%s",
                mon[M-1],d,h,m,s,y+1900,(gmt)?" GMT":"") <= 0)
        return(0);
    else
        return(1);
err:
    return(0);
}


int verify_callback(int ok, X509_STORE_CTX *ctx)
{
    char buf[256];
   // X509 *err_cert;
    int err,pubkeylen;//depth,
   // err_cert=X509_STORE_CTX_get_current_cert(ctx);
   X509_STORE_CTX_get_current_cert(ctx);
    err=X509_STORE_CTX_get_error(ctx);
   // depth=X509_STORE_CTX_get_error_depth(ctx);
	X509_STORE_CTX_get_error_depth(ctx);

    pubkeylen=EVP_PKEY_bits(X509_get_pubkey(ctx->current_cert));
    if(pubkeylen<1024)
	{
        PDEBUG("%s Certificate with RSA key size(%d) of less than 1024 bits, line:(%d)\n", __func__,pubkeylen,__LINE__);
        ok=0;
        return(ok);
    }
    if (!ok)
	{
        PDEBUG("%s verify error:num=%d:%s,line:(%d)\n", __func__,err,X509_verify_cert_error_string(err),__LINE__);
        switch(err)
		{
            case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
            case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
            case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
            case X509_V_ERR_CERT_SIGNATURE_FAILURE:
            case X509_V_ERR_CERT_NOT_YET_VALID:
            case X509_V_ERR_CERT_HAS_EXPIRED:
            case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
            case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
            case X509_V_ERR_OUT_OF_MEM:
            case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
            case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
            case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
            case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
            case X509_V_ERR_INVALID_CA:
            case X509_V_ERR_PATH_LENGTH_EXCEEDED:
            case X509_V_ERR_INVALID_PURPOSE:
            case X509_V_ERR_CERT_UNTRUSTED:
            case X509_V_ERR_CERT_REJECTED:
            case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
            case X509_V_ERR_AKID_SKID_MISMATCH:
            case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
            case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
                break;
            default:
                ok=1;
                X509_STORE_CTX_set_error(ctx,X509_V_OK);
                break;
        }
    }
    switch (ctx->error)
	{
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
            X509_NAME_oneline(X509_get_issuer_name(ctx->current_cert),buf,256);
            PDEBUG("%s issuer= %s,line:(%d)\n", __func__,buf,__LINE__);
            break;
        case X509_V_ERR_CERT_NOT_YET_VALID:
        case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
            PDEBUG("%s notBefore=", __func__);
            MY_ASN1_UTCTIME_print(X509_get_notBefore(ctx->current_cert));
            fprintf(stderr,"\n");
            break;
        case X509_V_ERR_CERT_HAS_EXPIRED:
        case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
            PDEBUG("%s notAfter=", __func__);
            MY_ASN1_UTCTIME_print(X509_get_notAfter(ctx->current_cert));
            fprintf(stderr,"\n");
            break;
    }
    PDEBUG("%s verify return:%d\n", __func__,ok);
    return(ok);
}
static int my_pem_passwd_cb(char *buf, int size, int rwflag, void *userdata)
{
	(void)rwflag;
    strncpy(buf, (char *)(userdata), size);
    buf[size - 1] = '\0';
    return(strlen(buf));
}
static int connection_set_cipher_list(SSL_CTX *ctx,int *cipher)
{
    char buf[1024];

    memset(buf,0,sizeof(buf));

    if (ctx==NULL||cipher == NULL)
        return -1;

	strcpy(buf,":AES256-SHA:NULL-MD5:NULL-SHA:EXP-RC4-MD5:RC4-MD5:RC4-SHA:EXP-RC2-CBC-MD5:IDEA-CBC-SHA:EXP-DES-CBC-SHA:DES-CBC-SHA:DES-CBC3-SHA:!DH-DES-CBC3-SHA:!EXP-DH-DES-CBC-SHA:!DH-DES-CBC-SHA:!DH-DSS-DES-CBC3-SHA:!EXP-DH-RSA-DES-CBC-SHA:!DH-RSA-DES-CBC-SHA:EXP-EDH-DSS-DES-CBC-SHA:EDH-DSS-CBC-SHA:EDH-DSS-DES-CBC3-SHA:EXP-EDH-RSA-DES-CBC-SHA:EDH-RSA-DES-CBC-SHA:EDH-RSA-DES-CBC3-SHA:EXP-ADH-RC4-MD5:ADH-RC4-MD5:EXP-ADH-DES-CBC-SHA:!FORTEZZA-NULL-SHA:!FORTEZZA-SHA:!FORTEZZA-RC4-SHA:ADH-DES-CBC-SHA:ADH-DES-CBC3-SHA");
    PDEBUG("OpenSSL: Support cipher suites: [%s]\n",buf + 1);
    if(SSL_CTX_set_cipher_list(ctx, buf + 1) != 1)
	{
		PDEBUG("%s Cipher suite configuration failed(%d)\n", __func__,__LINE__);
		ERR_print_errors_fp(stderr);
        return -1;
    }

    return 0;
}

int _LoadClientCertificate_(SSL_HANDLE handle, const char *filename, int format)
{
    SSL_CTX *tmpctx = NULL;

	if(handle==NULL||filename==NULL)
	{
        PDEBUG("%s handle and certfile file is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }
    if(((SSLHANDLE *)handle)->CloseFlag==1)
	{
        PDEBUG("%s Current handle already closed(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_ALREADCLOSE;
    }
    tmpctx=((SSLHANDLE *)handle)->CurrentCtx;
    if(format==SSL_FILE_PEM||format==SSL_FILE_DER)
	{
        if(format==SSL_FILE_PEM)
		{
            if(SSL_CTX_use_certificate_file(tmpctx,filename,SSL_FILETYPE_PEM)!=1)
			{
				PDEBUG("%s Load client certificate file fail(%d)\n", __func__,__LINE__);
				ERR_print_errors_fp(stderr);
				return NAPI_ERR;
            }
			else
				return NAPI_OK;
        }
        if(format==SSL_FILE_DER)
		{
            if(SSL_CTX_use_certificate_file(tmpctx,filename,SSL_FILETYPE_ASN1)!=1)
			{
				PDEBUG("%s Load client certificate file fail(%d)\n", __func__,__LINE__);
				ERR_print_errors_fp(stderr);
				return NAPI_ERR;
            }
			else
				return NAPI_OK;
        }
    }
	else
	{
		PDEBUG("%s format is not support(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_MODEUNSUPPORTED;
	}
	return NAPI_OK;
}
int _LoadClientPrivateKey_(SSL_HANDLE handle, const char *filename, int format,char *password)
{
    SSL_CTX *tmpctx = NULL;
    if(handle==NULL||filename==NULL)
	{
        PDEBUG("%s ssl and keyfile is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }
    if(((SSLHANDLE *)handle)->CloseFlag==1)
	{
        PDEBUG("%s Current handle already closed(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_ALREADCLOSE;
    }
    tmpctx=((SSLHANDLE *)handle)->CurrentCtx;

    if(password==NULL)
        goto DONE;
    else
	{
        SSL_CTX_set_default_passwd_cb(tmpctx, my_pem_passwd_cb);
      	SSL_CTX_set_default_passwd_cb_userdata(tmpctx, password);
        goto DONE;
    }
DONE:
    if(format==SSL_FILE_PEM||format==SSL_FILE_DER)
	{
        if(format==SSL_FILE_PEM)
		{
            if(SSL_CTX_use_PrivateKey_file(tmpctx,filename,SSL_FILETYPE_PEM)!=1)
			{
                PDEBUG("%s Load client key file fail(%d),password:%s\n", __func__,__LINE__,password);
                ERR_print_errors_fp(stderr);
				return NAPI_ERR;
            }
			else
				return NAPI_OK;
        }
		else
		{
            if(SSL_CTX_use_PrivateKey_file(tmpctx,filename,SSL_FILETYPE_ASN1)!=1)
			{
                PDEBUG("%s Load client key file fail(%d)\n", __func__,__LINE__);
                ERR_print_errors_fp(stderr);
				return NAPI_ERR;
            }
			else
				return NAPI_OK;
        }
    }
	else
	{
		PDEBUG("%s format is not support(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_MODEUNSUPPORTED;
	}
	return NAPI_OK;
}
static int load_ca_der_or_pem(SSL_CTX *ctx, const char *ca_cert,int type)
{
    X509_LOOKUP *lookup=NULL;
    int ret = 0;
    lookup = X509_STORE_add_lookup(ctx->cert_store, X509_LOOKUP_file());
    if (lookup == NULL)
	{
        PDEBUG("%s Failed add lookup for X509 store(%d)\n", __func__,__LINE__);
		ERR_print_errors_fp(stderr);
        return -1;
    }
    if(type==SSL_FILE_DER)
	{
        if (!X509_LOOKUP_ctrl(lookup,X509_L_FILE_LOAD,ca_cert,X509_FILETYPE_ASN1,NULL)) {
            unsigned long err = ERR_peek_error();
            PDEBUG("%s Load CA in DER format\n", __func__);
            if (ERR_GET_LIB(err) == ERR_LIB_X509 &&ERR_GET_REASON(err) == X509_R_CERT_ALREADY_IN_HASH_TABLE)
			{
                PDEBUG("%s ignoring cert already in hash table error(%d)\n", __func__,__LINE__);
            }
			else
                ret = -1;
			ERR_print_errors_fp(stderr);
        }
    }
	else
	{
        if (!X509_LOOKUP_ctrl(lookup,X509_L_FILE_LOAD,ca_cert,X509_FILETYPE_PEM,NULL))
		{
            unsigned long err = ERR_peek_error();
            PDEBUG("%s Load CA in PEM format\n", __func__);
            if (ERR_GET_LIB(err) == ERR_LIB_X509 &&ERR_GET_REASON(err) == X509_R_CERT_ALREADY_IN_HASH_TABLE)
			{
                PDEBUG("%s ignoring cert already in hash table error(%d)\n", __func__,__LINE__);
            }
			else
                ret = -1;
			ERR_print_errors_fp(stderr);
        }
    }
    return ret;
}

int _LoadServerCertificate_(SSL_HANDLE handle, const char *filename, int format)
{
    int ret=0;
    if(handle==NULL||filename==NULL)
	{
        PDEBUG("%s handle or server cafile is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }
    if(((SSLHANDLE *)handle)->CloseFlag==1)
	{
        PDEBUG("%s Current handle already closed(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_ALREADCLOSE;
    }
    if(format==SSL_FILE_PEM||format==SSL_FILE_DER)
	{
        ret=load_ca_der_or_pem(((SSLHANDLE *)handle)->CurrentCtx,filename,format);
        if(ret==-1)
			return NAPI_ERR;
        else
			return NAPI_OK;
    }
	else
		return NAPI_ERR_SSL_MODEUNSUPPORTED;
	return NAPI_OK;
}

SSL_HANDLE _OpenSSLSocket_(int type,int auth_opt,int* cipher)
{
	//SSL
    SSL_library_init();

    //SSL
    SSL_load_error_strings();

    SSLHANDLE *myhandle=(SSLHANDLE *)malloc(sizeof(SSLHANDLE));

    SSL_METHOD *meth=NULL;

    SSL_CTX *ctx=NULL;

	memset(myhandle, 0, sizeof(SSLHANDLE));
    myhandle->CloseFlag=1;	//

    switch(type)
	{
        case HANDSHAKE_SSLv2:
            meth=(SSL_METHOD *)SSLv2_client_method();
            break;
        case HANDSHAKE_SSLv3:
            meth=(SSL_METHOD *)SSLv3_client_method();
            break;
        case HANDSHAKE_SSLv23:
            meth=(SSL_METHOD *)SSLv23_client_method();
            break;
        case HANDSHAKE_TLSv1:
            meth=(SSL_METHOD *)TLSv1_client_method();
            break;
		case HANDSHAKE_TLSv1_1:
            meth=(SSL_METHOD *)TLSv1_1_client_method();
            break;
		case HANDSHAKE_TLSv1_2:
            meth=(SSL_METHOD *)TLSv1_2_client_method();
            break;
        default:
            PDEBUG("%s type is nonsupport(%d)\n", __func__,__LINE__);
            return NULL;
    }
    //methSSL_CTX
    ctx = SSL_CTX_new(meth);
    if (!ctx)
	{
        PDEBUG("%s SSL_CTX_new failed(%d)\n", __func__,__LINE__);
		ERR_print_errors_fp(stderr);
        return NULL;
    }
    SSL_CTX_set_verify_depth(ctx,4);
	SSL_CTX_ctrl(ctx, SSL_CTRL_OPTIONS, SSL_OP_ALL, NULL);

    switch(auth_opt)
	{
        case SSL_AUTH_NONE:/*:,
    :(),,*/
            SSL_CTX_set_verify(ctx,SSL_VERIFY_NONE,NULL);
            PDEBUG("MODE is SSL_VERIFY_NONE\n");
            break;
        case SSL_AUTH_CLIENT:/*:()
    :*/
            SSL_CTX_set_verify(ctx,SSL_VERIFY_PEER,verify_callback);
            PDEBUG("MODE is SSL_VERIFY_PEER\n");
            break;
        default:
            PDEBUG("%s MODE is NONSUPPORT(%d)\n", __func__,__LINE__);
            return NULL;
    }
    if(SSL_CTX_set_default_verify_paths(ctx)==0)
    {
    	PDEBUG("%s SSL_CTX_set_default_verify_paths failed(%d)\n", __func__,__LINE__);
		ERR_print_errors_fp(stderr);
    }
    if(connection_set_cipher_list(ctx,cipher)!=0)
	{
        PDEBUG("%s Cipher suite configuration failed(%d)\n", __func__,__LINE__);
        return NULL;
    }
    myhandle->CurrentSsl=NULL;
    myhandle->CurrentCtx=ctx;
    myhandle->CloseFlag=0;
	myhandle->CurrentSocketMode=NAPI_SUSPEND;
    myhandle->TimeOutFlag=0;
    myhandle->CurrentConnectState=SSL_IS_DISCONNECTED;

    return myhandle;
}

int _CloseSSLSocket_(SSL_HANDLE handle)
{
//    int flag=0;
    SSL *tmpssl = NULL;
    SSL_CTX *tmpctx = NULL;
    if(handle==NULL)
	{
        PDEBUG("%s ssl is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }
	else
    {
        tmpssl=((SSLHANDLE *)handle)->CurrentSsl;
        tmpctx=((SSLHANDLE *)handle)->CurrentCtx;
        if(((SSLHANDLE *)handle)->CloseFlag==1)
		{
            PDEBUG("%s Current handle already close(%d)\n", __func__,__LINE__);
			return NAPI_ERR_SSL_ALREADCLOSE;
        }
	}
    if(tmpssl!=NULL)
	{
        close(SSL_get_fd(((SSLHANDLE *)handle)->CurrentSsl));
        SSL_clear(((SSLHANDLE *)handle)->CurrentSsl);
        SSL_free(((SSLHANDLE *)handle)->CurrentSsl);
    }
    if(tmpctx!=NULL)
        SSL_CTX_free(((SSLHANDLE *)handle)->CurrentCtx);
    ERR_free_strings();
    ((SSLHANDLE *)handle)->CurrentCtx=NULL;
    ((SSLHANDLE *)handle)->CurrentSsl=NULL;
    ((SSLHANDLE *)handle)->CloseFlag=1;
	((SSLHANDLE *)handle)->CurrentSocketMode=NAPI_SUSPEND;
	free((SSLHANDLE *)handle);
	PDEBUG("%s succ(%d)\n", __func__,__LINE__);
	return NAPI_OK;
}
int _SSLDisconnect_(SSL_HANDLE handle)
{
    int ret,flag=0;
    int state=0;
    SSL *tmpssl=NULL;
    if(handle==NULL)
	{
        PDEBUG("%s ssl is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }
	else
    {
    	if(((SSLHANDLE *)handle)->CloseFlag==1)
		{
            PDEBUG("%s Current handle is closed(%d)\n", __func__,__LINE__);
			return  NAPI_ERR_SSL_ALREADCLOSE;
        }
        tmpssl=((SSLHANDLE *)handle)->CurrentSsl;
        ((SSLHANDLE *)handle)->TimeOutFlag=0;
        ret = _GetSSLConnectStatus_(handle,&state);
		if(ret != NAPI_OK || state==SSL_IS_DISCONNECTED)
		{
			PDEBUG("%s Current handle is unconnected(%d)\n", __func__,__LINE__);
			return NAPI_OK;
        }
		else
        {
        	//sd=SSL_get_fd(tmpssl);
        	SSL_get_fd(tmpssl);
            SSL_set_shutdown(tmpssl,SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);
            ret=SSL_shutdown(tmpssl);
            if(!ret)
			{
                ret=SSL_shutdown(tmpssl);
                switch(ret)
				{
                    case 1:
                        flag=1;
                        break;
                    case 0:
                    case -1:
                    default:
                        flag=0;
						PDEBUG("%sSSL_shutdown failed(%d)\n", __func__,__LINE__);
                        break;
                }
            }
			else
                flag=1;
        }

    }

    if(flag==1)
	{
        PDEBUG("%s succ(%d)\n", __func__,__LINE__);
		return NAPI_OK;
    }
	else
    {
        PDEBUG("%s fail(%d)\n", __func__,__LINE__);
		ERR_print_errors_fp(stderr);
		return NAPI_ERR;
    }

}

int _SSLConnect_(SSL_HANDLE handle, ST_SOCKET_ADDR *pServer, int timeout)
{
    int ret,sd,res,code=0,arg,val=-1;
	socklen_t len;
    int server_addr_len;
    struct sockaddr_in serveraddr;
    struct timeval time_out, starttv,endtv;
    int ms;
    fd_set s;
    SSL *tmpssl = NULL;
	char szTempStr[200] = {0};

    if(handle==NULL||pServer==NULL||timeout<0)
	{
        PDEBUG("%s ssl is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }
    if(((SSLHANDLE *)handle)->CloseFlag==1)
	{
        PDEBUG("%s Can not connect a closed handle(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_ALREADCLOSE;
    }
    tmpssl=SSL_new(((SSLHANDLE *)handle)->CurrentCtx);
    if(tmpssl==NULL)
	{
        PDEBUG("%s SSL_new failed(%d)\n", __func__,__LINE__);
		ERR_print_errors_fp(stderr);
		return NAPI_ERR_SSL_ALLOC;
    }

    sd= socket(AF_INET, SOCK_STREAM, 0);
    if(sd<0)
	{
        perror("socket");
		return NAPI_ERR_SSL_ALLOC;
    }

    if(SSL_set_fd(tmpssl,sd)==0)
	{
        PDEBUG("%s SSL_set_fd failed(%d)\n", __func__,__LINE__);
		ERR_print_errors_fp(stderr);
		return NAPI_ERR_SSL_ALLOC;
    }


    //
    int opt=1;
    res=setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,(const void *)&opt,sizeof(opt));
    if(res<0)
	{
        PDEBUG("%s setsockopt failed(%d)\n", __func__,__LINE__);
        perror("setsockopt");
		return NAPI_ERR;
    }

    /**<*/
    if( (arg = fcntl(sd, F_GETFL, NULL)) < 0)
	{
        PDEBUG("%s fcntl failed(%d)\n", __func__,__LINE__);
		return NAPI_ERR;
    }
    //arg &= ~O_NONBLOCK;
    arg |= O_NDELAY;
    if( fcntl(sd, F_SETFL, arg) < 0)
	{
        PDEBUG("%s fcntl failed(%d)\n", __func__,__LINE__);
		return NAPI_ERR;
    }
    //
    memset(&serveraddr,'\0',sizeof(serveraddr));
    if(pServer->unAddrType==SSL_ADDR_IPV4)
        serveraddr.sin_family=AF_INET;
    else
        serveraddr.sin_family=AF_INET6;

    serveraddr.sin_port=htons(pServer->usPort);
    if(inet_aton((char *)(pServer->psAddr),(struct in_addr *)&serveraddr.sin_addr.s_addr)==0)
	{
        PDEBUG("%s inet_aton error(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_INVADDR;
    }

    server_addr_len=sizeof(serveraddr);
    ret = connect(sd,(struct sockaddr*)&serveraddr, server_addr_len);
    if(ret<0)
	{
		if(errno == EINPROGRESS)
		{
	        do
			{
	            len=sizeof(int);
	            FD_ZERO(&s);
	            FD_SET(sd,&s);
	            time_out.tv_sec=timeout/1000;
	            time_out.tv_usec=(timeout%1000)*1000;
	            res=select(sd+1, NULL, &s, NULL, &time_out);
				if(res>0)
				{
                    if (getsockopt(sd, SOL_SOCKET, SO_ERROR, (void*)(&val), &len) < 0)
					{
                        shutdown(sd, SHUT_RDWR);
                        goto ERR;
                    }
					if (val)
					{
                        shutdown(sd, SHUT_RDWR);
                        goto ERR;
                    }
                    goto CONNECT;
				}
	            else if(res==-1)
				{
	                if (EINTR == errno)
	                    continue;
	                goto ERR;
	            }
				else
				{
					PDEBUG("%connect timeout,%d\n",__func__,__LINE__);
	                goto TIMEOUTERR;
				}
	        }while(1);
		}
		else
		{
            shutdown(sd, SHUT_RDWR);
            goto ERR;
        }
    }
CONNECT:
	gettimeofday(&starttv,NULL);
    while(1)
	{
        code=SSL_connect(tmpssl);
        if(code<=0)
		{
            switch(SSL_get_error(tmpssl,code))
			{
                case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
                    usleep(100);
                    break;
				case SSL_ERROR_SYSCALL:
					ERR_error_string_n(SSL_ERROR_SYSCALL,szTempStr,sizeof szTempStr);
					PDEBUG("%sSSL_ERROR_SYSCALL:%s,%d\n",__func__,szTempStr,__LINE__);
					goto ERR;
                default:
					PDEBUG("%sSSL_connect failed:%d,%d\n",__func__,SSL_get_error(tmpssl,code),__LINE__);
                    goto ERR;
            }
        }
		else
        {
            ((SSLHANDLE *)handle)->CurrentSsl=tmpssl;
            goto SUCC;
        }
        gettimeofday(&endtv,NULL);
        ms=(endtv.tv_sec * 1000 + endtv.tv_usec/1000)-(starttv.tv_sec * 1000 + starttv.tv_usec/1000);
        if(ms>timeout)
            goto TIMEOUTERR;
    }
SUCC:
	((SSLHANDLE *)handle)->CurrentConnectState=SSL_IS_CONNECTED;
    PDEBUG("%s succ,%d\n",__func__,__LINE__);
	return NAPI_OK;
ERR:
	((SSLHANDLE *)handle)->CurrentConnectState=SSL_IS_DISCONNECTED;
    PDEBUG("%s err,%d\n",__func__,__LINE__);
	ERR_print_errors_fp(stderr);
    perror("connect");
	return NAPI_ERR;
TIMEOUTERR:
	((SSLHANDLE *)handle)->CurrentConnectState=SSL_IS_DISCONNECTED;
    PDEBUG("%s timeout,%d\n",__func__,__LINE__);
	ERR_print_errors_fp(stderr);
    perror("connect");
	return NAPI_ERR_SSL_TIMEOUT;
}

int _GetSSLBlockingMode_(SSL_HANDLE handle)
{
    if(handle==NULL)
	{
        PDEBUG("%s ssl is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }
    if(((SSLHANDLE *)handle)->CloseFlag==1)
	{
        PDEBUG("%s Current handle already closed(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_ALREADCLOSE;
    }

    return ((SSLHANDLE *)handle)->CurrentSocketMode;
}
int _SetSSLBlockingMode_(SSL_HANDLE handle,int mode)
{
    if(handle==NULL)
	{
        PDEBUG("%s ssl is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }
    if(((SSLHANDLE *)handle)->CloseFlag==1)
	{
        PDEBUG("%s Current handle already closed(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_ALREADCLOSE;
    }
	if(mode==NAPI_NOWAIT)
	{
		((SSLHANDLE *)handle)->CurrentSocketMode=NAPI_NOWAIT;
    }
	else if(mode==NAPI_SUSPEND)
    {
		((SSLHANDLE *)handle)->CurrentSocketMode=NAPI_SUSPEND;
        ((SSLHANDLE *)handle)->TimeOutFlag=0;
    }
	else
	{
        PDEBUG("%s mode is unsupported(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_MODEUNSUPPORTED;
    }
	return NAPI_OK;
}
int _SSLSend_(SSL_HANDLE handle, const char *pBuffer, size_t SizeOfBuffer, size_t *sent_data)
{
    int state=0;
    if (handle == NULL)
	{
        PDEBUG("%s SSL is failed(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }
    if ((pBuffer==NULL)||(sent_data==NULL))
	{
        PDEBUG("%s pBuffer,sent_data is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }
    _GetSSLConnectStatus_(handle,&state);
    if(state==SSL_IS_CONNECTED)
	{
		if (((*sent_data)=SSL_write(((SSLHANDLE *)handle)->CurrentSsl, pBuffer, SizeOfBuffer)) <= 0)
		{
			PDEBUG("%s SSL_write failed(%d)\n", __func__,__LINE__);
			ERR_print_errors_fp(stderr);
			return NAPI_ERR_SSL_SEND;
        }
    }
	else
		return NAPI_ERR;
	return NAPI_OK;
}

extern void SetCommError(int nCommErr, int nNapiErr);
int _SSLReceive_(SSL_HANDLE handle, void *pBuffer, const size_t SizeOfBuffer, size_t *recv_data)
{
    int state=0;
    int len = 0,totallen = 0;
    SSL *tmpssl = NULL;

    if (handle == NULL)
	{
        PDEBUG("%s ssl is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }

    if ((pBuffer==NULL)||(recv_data==NULL))
	{
        PDEBUG("%s pBuffer,recv_data is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }

    tmpssl=((SSLHANDLE *)handle)->CurrentSsl;
    _GetSSLConnectStatus_(handle,&state);
    if(state==SSL_IS_CONNECTED)
	{
        while(1)
		{
			if (PubKbHit() == KEY_ESC)
			{
				SetCommError(APP_QUIT, 0);
				return APP_QUIT;
			}
			len=SSL_read(tmpssl, (char*)pBuffer+totallen, SizeOfBuffer-totallen);
            if(len<=0)
			{
                switch(SSL_get_error(tmpssl, len))
				{
                    case SSL_ERROR_WANT_READ:
                        usleep(10);
                        break;
                    default:
						PDEBUG("%s SSL_read failed(%d)\n", __func__,__LINE__);
						ERR_print_errors_fp(stderr);
                        goto ERR;
                }
            }
			else
			{
                totallen+=len;
				if(totallen<SizeOfBuffer&&(((SSLHANDLE *)handle)->CurrentSocketMode==NAPI_SUSPEND))
                    continue;
                else
                    goto SUCC;
            }
        }
    }
	else
	{
        *recv_data=0;
		return NAPI_ERR;
    }
SUCC:
    *recv_data=totallen;
	return NAPI_OK;
ERR:
    *recv_data=totallen;
	return NAPI_ERR;
}

int _SSLBind_(SSL_HANDLE handle, ST_SOCKET_ADDR *Address)
{
    int res,sd;
    int server_addr_len;
    struct sockaddr_in serveraddr;
    if(handle==NULL||Address==NULL)
	{
        PDEBUG("%s ssl is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }
    if(((SSLHANDLE *)handle)->CloseFlag==1)
	{
        PDEBUG("%s Can not bind a closed handle(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_ALREADCLOSE;
    }
    sd= socket(AF_INET, SOCK_STREAM, 0);
    if(sd<0)
	{
        perror("socket");
		return NAPI_ERR_SSL_ALLOC;
    }

    //
    int opt=1;
    res=setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,(const void *)&opt,sizeof(opt));
    if(res<0)
	{
        PDEBUG("%s setsockopt fail(%d)\n", __func__,__LINE__);
		return NAPI_ERR;
    }

    //
    memset(&serveraddr,'\0',sizeof(serveraddr));
    if(Address->unAddrType==SSL_ADDR_IPV4)
        serveraddr.sin_family=AF_INET;
    else
        serveraddr.sin_family=AF_INET6;
    serveraddr.sin_port=htons(Address->usPort);
    serveraddr.sin_addr.s_addr=INADDR_ANY;
    server_addr_len=sizeof(serveraddr);
    if(bind(sd,(struct sockaddr*)&serveraddr, server_addr_len)==-1)
	{
        PDEBUG("%s bind fail(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_INVADDR;
    }
    ((SSLHANDLE *)handle)->CurrentSocket=sd;
	return NAPI_OK;
}

int _GetSSLConnectStatus_(SSL_HANDLE handle, int *state)
{
    int flag=0;
    if(handle==NULL||state==NULL)
	{
        PDEBUG("%s ssl or state is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }
    if(((SSLHANDLE *)handle)->CloseFlag==1)
	{
        PDEBUG("%s Current handle already closed(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_ALREADCLOSE;
    }
    if(((SSLHANDLE *)handle)->TimeOutFlag==1)
		return NAPI_ERR_SSL_TIMEOUT;

	if(((SSLHANDLE *)handle)->CurrentConnectState==SSL_IS_DISCONNECTED)
		return NAPI_ERR_SSL_CONNECT;

    SSL *tempSSl=((SSLHANDLE *)handle)->CurrentSsl;
    if(((tempSSl->state)&SSL_ST_BEFORE)==SSL_ST_BEFORE)
	{
        (*state)=SSL_IS_DISCONNECTED;
        flag=1;
    }
	else if(((tempSSl->state)&SSL_ST_OK)==SSL_ST_OK)
    {
        (*state)=SSL_IS_CONNECTED;
        flag=1;
    }
	else
    {
        (*state)=SSL_CONNECTION_IN_PROGRESS;
        flag=1;
    }
    if(flag==1)
		return NAPI_OK;
    else
		return NAPI_ERR;
}

int _SSLDataAvailable_(SSL_HANDLE handle, uint timeout)
{
    int state=0;
	if(handle==NULL)
	{
        PDEBUG("%s ssl is NULL(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_PARAM;
    }
    ((SSLHANDLE *)handle)->TimeOutFlag=0;
    _GetSSLConnectStatus_(handle,&state);
    if(((SSLHANDLE *)handle)->CloseFlag==1)
	{
        PDEBUG("%s Current handle already closed(%d)\n", __func__,__LINE__);
		return NAPI_ERR_SSL_ALREADCLOSE;
    }
    if(state!=SSL_IS_CONNECTED)
		return NAPI_ERR_SSL_CONNECT;
    int fd=SSL_get_fd(((SSLHANDLE *)handle)->CurrentSsl);
    int maxfd,nread;
    maxfd=fd;
    fd_set s;
    FD_ZERO(&s);
    FD_SET(fd,&s);
    struct  timeval tv;
    tv.tv_sec=timeout;
    tv.tv_usec=0;
    while(1)
	{
        nread=select(maxfd+1,&s,NULL,NULL,&tv);
        if(nread==-1)
		{
            if(errno==EINTR)
                continue;
            else
				return NAPI_ERR_SSL_TIMEOUT;
        }
		else if(nread==0)
        {
            //PDEBUG("time out\n");
			return NAPI_ERR_SSL_TIMEOUT;
        } else
			return NAPI_OK;
    }
}
void *_SSLGetPeerCerificate_(SSL_HANDLE handle)
{
	static X509 stX509;
	X509 *pX509 = NULL;

    if(handle==NULL)
        return NULL;

	pX509 = SSL_get_peer_certificate(((SSLHANDLE *)handle)->CurrentSsl);
	if(pX509 != NULL)
	{
		memset(&stX509, 0, sizeof(stX509));
		memcpy(&stX509, pX509, sizeof(stX509));
		X509_free(pX509);
		return (void *)&stX509;
	}
	else
	{
		PDEBUG("%s Get PeerCerificate failed(%d)\n", __func__,__LINE__);
		ERR_print_errors_fp(stderr);
		return NULL;
	}
}

