/*
* Newland payment(c) 2011-2015
*
* NDK API
* main module:
* Author:	Product Development Department
* Date:		2012-08-17
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/vfs.h>
#include "napi.h"

#define USER_BIN_PATH	"/app/"
#define FILENAME_MAXLEN 19

int napi_fs_open[1024];
int napi_fs_open_num = 0;
int napi_fs_flag = 0;

int PubFsOpen(const char *pszName,const char *pszMode)
{
	int fd;

	if (pszName == NULL || pszMode==NULL)
	{
		return NAPI_ERR_PARA;
	}
	if (pszMode[0] == 'w')
	{
		fd = open(pszName, O_RDWR|O_CREAT|O_SYNC, 0666);
	}
	else if (pszMode[0] == 'r')
	{
		fd = open(pszName, O_RDONLY);
	}
	else
	{
		return NAPI_ERR_PARA;
	}

	if (fd < 0)
	{
		return NAPI_ERR_OPEN_DEV;
	}

	if (napi_fs_open_num == 1024)
	{
		return NAPI_ERR_OPEN_DEV;
	}
	napi_fs_open[napi_fs_open_num] = fd;
	napi_fs_open_num++;

	return fd;
}

static int ProFsOpened(int handle)
{
	int i;

	if(napi_fs_open_num == 0)
	{
		return 0;
	}
	for (i=0; i<napi_fs_open_num; i++)
	{
		if (napi_fs_open[i] == handle)
		{
			napi_fs_flag = i;
			return 1;
		}
	}
	napi_fs_flag = 0;

	return 0;
}

int PubFsClose(int nHandle)
{
	int ret,i;

	if (!ProFsOpened(nHandle))
	{
		return NAPI_ERR;
	}
	ret = close(nHandle);
	if (ret < 0)
	{
		return NAPI_ERR;
	}
	else
	{
		for (i = napi_fs_flag; i < (napi_fs_open_num-1); i++)
		{
			napi_fs_open[i] = napi_fs_open[i+1];
		}
		napi_fs_open_num--;

		return NAPI_OK;
	}
}

int PubFsRead(int nHandle, char *psBuffer, uint unLength)
{
	int ret;

	if (psBuffer == NULL)
	{
		return NAPI_ERR_PARA;
	}
	if (!ProFsOpened(nHandle))
	{
		return NAPI_ERR_READ;
	}
	ret = read(nHandle, psBuffer, unLength);
	if (ret < 0)
	{
		return NAPI_ERR_READ;
	}

	return ret;
}

int PubFsWrite(int nHandle, const char *psBuffer, uint unLength)
{
	int ret;

	if (psBuffer == NULL)
	{
		return NAPI_ERR_PARA;
	}
	if (!ProFsOpened(nHandle))
	{
		return NAPI_ERR_WRITE;
	}
	ret = write(nHandle, psBuffer, unLength);
	if ( ret < 0)
	{
		return NAPI_ERR_WRITE;
	}
	return ret;
}

int PubFsSeek(int nHandle, ulong ulDistance, uint unPosition)
{
	long ret;

	if (!ProFsOpened(nHandle))
	{
		return NAPI_ERR;
	}
	ret = lseek(nHandle, ulDistance, unPosition);
	if (ret < 0)
	{
		return NAPI_ERR;
	}

	return NAPI_OK;
}

int PubFsDel(const char *pszName)
{
	int ret;

	if (pszName == NULL)
	{
		return NAPI_ERR_PARA;
	}

	ret = remove(pszName);
	if (ret == 0)
	{
		return NAPI_OK;
	}

	return NAPI_ERR;
}

int PubFsFileSize(const char *pszName,uint *punSize)
{
	int ret;
	int fd;
	struct stat buf, *p=&buf;

	if (pszName == NULL || punSize==NULL)
	{
		return NAPI_ERR_PARA;
	}
	fd = open(pszName, O_RDONLY);
	if (fd < 0)
	{
		return NAPI_ERR;
	}
	ret = fstat(fd, p);
	if (ret < 0)
	{
		close(fd);
		return NAPI_ERR;
	}
	*punSize = p->st_size;
	close(fd);

	return NAPI_OK;
}

int PubFsRename(const char *pszsSrcname, const char *pszDstname)
{
	int ret;

	if ((pszsSrcname == NULL) || (pszDstname == NULL))
	{
		return NAPI_ERR_PARA;
	}

	ret = rename(pszsSrcname, pszDstname);
	if (ret < 0)
	{
		return NAPI_ERR;
	}

	return NAPI_OK;
}

int PubFsExist(const char *pszName)
{
	if (pszName==NULL)
	{
		return NAPI_ERR_PARA;
	}
	if (access(pszName, F_OK) == 0)
	{
		return NAPI_OK;
	}

	return NAPI_ERR;
}

int PubFsTruncate(const char *pszPath ,uint unLen)
{
	int ret,i;
	uint size;
	int fd;
	char c=0xff;

	if (pszPath == NULL)
	{
		return NAPI_ERR_PARA;
	}
	ret=PubFsFileSize(pszPath,&size);
	if (ret!=0)
	{
		return NAPI_ERR;
	}
	if (unLen > size)
	{
		fd = open(pszPath, O_RDWR);
		if ( fd < 0 )
		{
			return NAPI_ERR_PATH;
		}
		ret = lseek(fd, 0, SEEK_END);
		if (ret < 0)
		{
			close(fd);
			return NAPI_ERR;
		}
		for(i = 0; i < (unLen-size); i++)
		{
			ret = write(fd, (char *)&c, 1);
			if(ret < 0)
			{
				close(fd);
				return NAPI_ERR_WRITE;
			}
		}
		close(fd);
		return NAPI_OK;
	}
	ret = truncate(pszPath,unLen);
	if (ret < 0)
	{
		return NAPI_ERR;
	}

	return NAPI_OK;
}

int PubFsTell(int nHandle,ulong *pulRet)
{
	long ret;

	if (pulRet==NULL)
	{
		return NAPI_ERR_PARA;
	}
	if (!ProFsOpened(nHandle))
	{
		return NAPI_ERR;
	}
	ret = lseek(nHandle, 0, SEEK_CUR);
	if (ret < 0)
	{
		return NAPI_ERR;
	}
	*pulRet = ret;

	return NAPI_OK;
}


int PubFsGetDiskSpace(uint unWhich, ulong *pulSpace)
{
	struct statfs buf, *p=&buf;
	int ret;

	if (pulSpace == NULL)
	{
		return NAPI_ERR_PARA;
	}
	ret = statfs(USER_BIN_PATH, p);
	if (ret < 0)
	{
		return NAPI_ERR;
	}
	if (unWhich == 1)
	{
		/* when userfs_root is not mounted, f_bavail will be a invalid huge value */
		if (p->f_bavail > 0x80000)
		{
			p->f_bavail = 0;
		}
		*pulSpace=p->f_bsize*p->f_bavail;
	}
	else
	{
		*pulSpace = p->f_bsize * (p->f_blocks - p->f_bavail);
	}

	return NAPI_OK;
}


int PubFsCreateDirectory(const char *pszName)
{
	char buf[200];
	int ret;

	memset(buf,0x00,sizeof(buf));
	if (pszName == NULL)
	{
		return NAPI_ERR_PARA;
	}
	ret=mkdir(pszName,0755);
	if(ret==0)
	{
		return NAPI_OK;
	}

	return NAPI_ERR;
}

int PubFsRemoveDirectory(const char *pszName)
{
	int ret;

	if (pszName == NULL)
	{
		return NAPI_ERR_PARA;
	}

	ret = remove(pszName);

	if (ret == 0)
	{
		return NAPI_OK;
	}

	return NAPI_ERR;
}

int PubFsFormat(void)
{
	return NAPI_ERR_NOT_SUPPORT;
}

int PubFsDir(const char *pPath,char *psBuf,uint *punNum)
{
	DIR * thedir=NULL;
	struct dirent * ent=NULL;
	int ret = 0;
	char *p;

	if (psBuf == NULL||pPath==NULL||punNum==NULL)
	{
		return NAPI_ERR_PARA;
	}

	thedir = opendir(pPath);
	if (!thedir)
	{
		return NAPI_ERR_PATH;
	}
	p = (char*)psBuf;
	ent = readdir(thedir);
	while (ent)
	{
		if(strcmp(ent->d_name,".")==0||strcmp(ent->d_name,"..")==0)
		{
			ent = readdir(thedir);
			continue;
		}
		else
		{
			strncpy(p, ent->d_name, FILENAME_MAXLEN);
			*(p+FILENAME_MAXLEN) = (char)(ent->d_type==DT_DIR? 1:0);
			p += (FILENAME_MAXLEN+1);
			ret++;
			ent = readdir(thedir);
		}
	}
	if (thedir)
	{
		closedir(thedir);
	}
	//if(punNum !=NULL)
	*punNum = ret;

	return NAPI_OK;
}

