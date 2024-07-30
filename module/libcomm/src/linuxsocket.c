#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <time.h>
#include <setjmp.h>
#include <signal.h>
#include <arpa/inet.h>
#include "linuxsocket.h"

#define TCP_CLOSE_TIME_WAIT (2)
inline TCP_ERR_CODE __get_error_code(int nValue) {return (TCP_ERR_CODE)(nValue);}
inline TCP_ERR_CODE __get_subvalue_error_code(int nFirst, int nSecond) {return (TCP_ERR_CODE)(nFirst - nSecond);}
static int GetNetworkErrCode()
{
	int nRet = TCP_OK;

	switch (errno)
	{
	case ENOBUFS:
	case ENOMEM:
	case ENFILE:
	case EMFILE:
	case EAGAIN:
		nRet = TCP_ERR_ALLOC;
		break;
	case EINVAL:
	case EBADF:
	case EFAULT:
	case ENOTSOCK:
	case EISCONN:
	case EAFNOSUPPORT:
		nRet = TCP_ERR_PARAM;
		break;
	case ETIMEDOUT:
	case EINTR:
		nRet = TCP_ERR_TIMEOUT;
		break;
	case EADDRINUSE:
		nRet = TCP_ERR_INVADDR;
		break;
	case ECONNREFUSED:
		nRet = TCP_ERR_CONNECT;
		break;
	case EPROTONOSUPPORT:
		nRet = TCP_ERR_PROTOCOL;
		break;
	default:
		nRet = TCP_ERR_NETWORK;
		break;
	}

	return nRet;
}

TCP_ERR_CODE _TcpOpen(uint * punFd)
{
	int nRet;

	if (punFd == NULL)
	{
		return TCP_ERR_PARAM;
	}
	nRet = socket(PF_INET, SOCK_STREAM, 0);
	if (nRet < 0)
	{
		nRet = GetNetworkErrCode();
		return __get_error_code(nRet);
	}
	*punFd = nRet;

	return TCP_OK;
}

TCP_ERR_CODE _TcpClose(uint unFd)
{
	int nRet = TCP_OK;

	if(unFd==0)
	{
		return TCP_ERR;
	}
	nRet = shutdown(unFd, SHUT_RDWR);
	errno =0;
	nRet = close(unFd);
	if (nRet < 0)
	{
		return TCP_ERR;
	}
	else if(errno == EBADF)
	{
		return TCP_ERR;
	}

	return TCP_OK;
}

TCP_ERR_CODE _TcpWait(uint unFd)
{
	int nRet = TCP_OK;
	char buf[1];
	int val = 0;
	time_t startTM,currentTM;

	if (0 == unFd)
	{
		return TCP_ERR;
	}

	val = fcntl(unFd, F_GETFL, 0);
	fcntl(unFd, F_SETFL, val | O_NONBLOCK); /**<nonblock*/

	startTM = time(NULL);
	while(1)
	{
		currentTM=time(NULL); //linsx modify 2013/7/15 16:25:26 for EAGAIN error
		if ((currentTM - startTM) >= TCP_CLOSE_TIME_WAIT)
		{
			fprintf(stderr,"[%s] timeout\n",__func__);
			break;
		}
		nRet = recv(unFd, buf, 1, 0);
		if (nRet==0)
		{
			break;
		}
		else if ((nRet < 0) && (errno == EAGAIN))
		{
			continue;
		}
		else if (nRet<0)
		{
			break;
		}
	}
	close(unFd);

	return TCP_OK; //__get_error_code(nRet);
}

TCP_ERR_CODE NAPI_TcpBind(uint unFd, const char *pszMyIp, ushort usMyPort)
{
	int nRet = TCP_ERR;
	struct sockaddr_in pstMyAddr;

	if (NULL == pszMyIp || 0 == unFd || inet_addr(pszMyIp) == INADDR_NONE)
	{
		return TCP_ERR_PARAM;
	}

	pstMyAddr.sin_family = AF_INET;
	pstMyAddr.sin_port = usMyPort;
	pstMyAddr.sin_addr.s_addr = inet_addr(pszMyIp);

	nRet = bind(unFd, (struct sockaddr *)&pstMyAddr, sizeof(struct sockaddr));
	if (nRet < 0)
	{
		if((nRet = bind(unFd, (struct sockaddr *)&pstMyAddr, sizeof(struct sockaddr))) <0)
		{
			nRet = GetNetworkErrCode();
			return __get_error_code(nRet);
		}
	}
	return TCP_OK;
}

TCP_ERR_CODE _TcpConnect(uint unFd, const char *pszRemoteIp, ushort usRemotePort, uint unTimeout)
{
	int nRet = TCP_ERR;
	long arg;
	fd_set myset;
	socklen_t lon;
	int valopt=0;
	struct timeval tv;
	struct sockaddr_in addr;
	struct sockaddr_in pstRemoteAddr;
	int iTempRet;

	if (NULL == pszRemoteIp)
	{
		return TCP_ERR_PARAM;
	}

	pstRemoteAddr.sin_family = AF_INET;
	pstRemoteAddr.sin_port = usRemotePort;
	pstRemoteAddr.sin_addr.s_addr = inet_addr(pszRemoteIp);
	memcpy(&addr, &pstRemoteAddr, sizeof(struct sockaddr_in));
	addr.sin_port = htons(addr.sin_port);

	/**<unblocking */
	if ((arg = fcntl(unFd, F_GETFL, NULL)) < 0)
	{
		return TCP_ERR;
	}
	arg |= O_NONBLOCK;
	if (fcntl(unFd, F_SETFL, arg) < 0)
	{
		return TCP_ERR;
	}

	tv.tv_sec = unTimeout;
	tv.tv_usec = 0;

	nRet = connect(unFd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
	if (nRet < 0)
	{
		if (errno == EINPROGRESS)
		{
			do {
				FD_ZERO(&myset);
				FD_SET(unFd, &myset);
				nRet = select(unFd+1, NULL, &myset, NULL, &tv);
				if (nRet < 0)
				{
					if (EINTR == errno)
					{
						continue;
					}
					iTempRet=errno;
					perror(" _TcpConnect ERROR:");
					shutdown(unFd, SHUT_RDWR);
					return __get_subvalue_error_code(TCP_ERR_LINUX_ERRNO_BASE, iTempRet);
				}
				else if (nRet > 0)
				{
					/**<Socket selected for write*/
					lon = sizeof(int);
					if (getsockopt(unFd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0)
					{
						iTempRet=errno;
						perror("_TcpConnect ERROR:");
						shutdown(unFd, SHUT_RDWR);
						return __get_subvalue_error_code(TCP_ERR_LINUX_ERRNO_BASE, iTempRet);
					}
					if (valopt)
					{
						iTempRet=valopt;
						perror("_TcpConnect ERROR:");
						shutdown(unFd, SHUT_RDWR);
						return __get_subvalue_error_code(TCP_ERR_LINUX_ERRNO_BASE, iTempRet);
					}
					break;
				}
				else
				{
					shutdown(unFd, SHUT_RDWR);
					return TCP_ERR_TIMEOUT;
				}
			} while (1);
		}
		else
		{
			iTempRet = errno;
			perror("_TcpConnect ERROR:");
			shutdown(unFd, SHUT_RDWR);
			return __get_subvalue_error_code(TCP_ERR_LINUX_ERRNO_BASE, iTempRet);
		}
	}
	if ((arg = fcntl(unFd, F_GETFL, NULL)) < 0)
	{
		return TCP_ERR;
	}
	arg &= (~O_NONBLOCK);
	if( fcntl(unFd, F_SETFL, arg) < 0)
	{
		return TCP_ERR;
	}

	return TCP_OK;
}

TCP_ERR_CODE _TcpListen(uint unFd, int nBacklog)
{
	int nRet = TCP_ERR
;

	nRet = listen(unFd, nBacklog);
	if (nRet < 0)
	{
		nRet = GetNetworkErrCode();
		return __get_error_code(nRet);
	}

	return TCP_OK;
}

TCP_ERR_CODE _TcpAccept(uint unFd, const char *pszPeerIp, ushort usPeerPort, uint *punNewFd)
{
	int nRet = TCP_ERR;
	int nLen = sizeof(struct sockaddr);
	struct sockaddr_in pstPeerAddr;

	if ((NULL == pszPeerIp) || (NULL == punNewFd))
	{
		return TCP_ERR_PARAM;
	}

	pstPeerAddr.sin_family = AF_INET;
	pstPeerAddr.sin_port = usPeerPort;
	pstPeerAddr.sin_addr.s_addr = inet_addr(pszPeerIp);

	nRet = accept(unFd, (struct sockaddr *)&pstPeerAddr, (socklen_t *)&nLen);
	if (nRet < 0)
	{
		return TCP_ERR;
	}

	*punNewFd = nRet;

	return TCP_OK;
}

TCP_ERR_CODE _TcpWrite(uint unFd, const void *pInbuf, uint unLen, uint unTimeout, uint *punWriteLen)
{
	int n = -1;
	sigset_t newmask, oldmask;
	struct timeval timeout;

	if (NULL == pInbuf)
	{
		return TCP_ERR_PARAM;
	}
	timeout.tv_sec = unTimeout;
	timeout.tv_usec= 0;
	signal(SIGPIPE, SIG_IGN);
	setsockopt(unFd, SOL_SOCKET,SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
	/**
	* 无线信号、电源状态刷新需要使用SIGALRM信号(widget/notifier.c)，
	* 此处可能被该信号中断，因此需要暂时屏蔽
	* wireless and battery need SIGALRM. It may be interrupted by this signal,
	* so it needs to be shielded temporarily.
	*/
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGALRM);
	sigprocmask(SIG_BLOCK, &newmask, &oldmask);

	n = send(unFd, pInbuf, unLen, 0);
	if ((n < 0) || (n < unLen))
	{
		sigprocmask(SIG_UNBLOCK, &newmask, NULL);
		return TCP_ERR_SEND;
	}

	sigprocmask(SIG_UNBLOCK, &newmask, NULL);
	signal(SIGPIPE, SIG_DFL);

	if(punWriteLen != NULL)
	{
		*punWriteLen = n;
	}

	return TCP_OK;
}

TCP_ERR_CODE _TcpRead(uint unFd, void *pOutbuf, uint unLen, uint unTimeout, uint *punReadlen)
{
	int nRet = TCP_ERR;
	fd_set readfds;
	struct timeval tv;
	int n = -1;
	int read_bytes = 0;
	sigset_t newmask, oldmask;

	if ((NULL == pOutbuf) || (NULL == punReadlen))
	{
		return TCP_ERR_PARAM;
	}
	tv.tv_sec = unTimeout;
	tv.tv_usec= 0;

	while (read_bytes < unLen)
	{
		FD_ZERO(&readfds);
		FD_SET(unFd, &readfds);
		nRet = select(unFd+1, &readfds, NULL, NULL, &tv);
		if (nRet < 0)
		{
			if (EINTR == errno)
			{
				continue;
			}
			return TCP_ERR;
		}
		else if (0 == nRet)
		{
			if (0 == read_bytes)
			{
				return TCP_ERR_TIMEOUT;
			}
			*punReadlen = read_bytes;
			return TCP_OK;
		}
		else
		{
			/**
			*无线信号、电源状态刷新需要使用SIGALRM信号(widget/notifier.c)，
			*此处可能被该信号中断，因此需要暂时屏蔽
			* wireless and battery need SIGALRM. It may be interrupted by this signal,
			* so it needs to be shielded temporarily.
			*/
			sigemptyset(&newmask);
			sigaddset(&newmask, SIGALRM);
			sigprocmask(SIG_BLOCK, &newmask, &oldmask);
			n = recv(unFd, (char *)pOutbuf+read_bytes, unLen-read_bytes, 0);
			sigprocmask(SIG_UNBLOCK, &newmask, NULL);
			if (n <= 0)
			{
				if(0==read_bytes)
				{
					return TCP_ERR_RECV;
				}
				else
				{
					*punReadlen = read_bytes;
					return TCP_OK;
				}
			}

			read_bytes += n;
		}
	}
	*punReadlen = read_bytes;

	return TCP_OK;
}

TCP_ERR_CODE _TcpReset(uint unFd)
{
	int nRet = TCP_ERR;
	struct linger ling = {1, 0};

	setsockopt(unFd, SOL_SOCKET, SO_LINGER, (void*)&ling, sizeof(ling));

	if (unFd==0)
	{
		return TCP_ERR;
	}
	nRet = shutdown(unFd, SHUT_RDWR);
	nRet = close(unFd);
	if (nRet < 0)
	{
		return TCP_ERR;
	}
	else if (errno == EBADF)
	{
		return TCP_ERR;
	}
	return TCP_OK;
}

TCP_ERR_CODE _UdpOpen(uint *punFd)
{
	int nRet = TCP_ERR;

	if (NULL == punFd)
	{
		return TCP_ERR_PARAM;
	}
	nRet = socket(PF_INET, SOCK_DGRAM, 0);
	if (nRet < 0)
	{
		nRet = GetNetworkErrCode();
		return __get_error_code(nRet);
	}
	*punFd = nRet;

	return TCP_OK;
}


