//IniMem.cpp
/************************************************************************/
/* 
2009-10-21 修正:
当两个字段中有一个包含另一个的所有字符串时, 读写错误的bug
*/
/************************************************************************/

#include <stdio.h>
#include <TCHAR.H>
#include <WCHAR.H>
#include <vector>
#include "IniMem.h"

TCHAR *stristr(LPCTSTR lpFirst, LPCTSTR lpSrch)
{
	TCHAR                *pString1 = NULL, *pString2 = NULL;
	TCHAR                *pStringTemp1 = NULL, *pStringTemp2 = NULL;
	const TCHAR        *pTemp1 = lpFirst;
	const TCHAR        *pReturn = NULL;
	int                        pos = -1;

	if (!lpFirst || !lpSrch)
	{
		return NULL;
	}

	int len1 = 0, len2 = 0;

	len1 = _tcslen(lpFirst);
	pString1 = new TCHAR[len1 + 1];
	pStringTemp1 = pString1;

	len2 =_tcslen(lpSrch);
	pString2 = new TCHAR[len2 + 1];
	pStringTemp2 = pString2;

	while (*lpFirst)
	{
		if (*lpFirst >= 'a' && *lpFirst <= 'z')
		{
			*pString1 = *lpFirst - 32;
		}
		else
		{
			*pString1 = *lpFirst;
		}
		pString1++;
		lpFirst++;
	}
	*pString1 = 0;
	while (*lpSrch)
	{
		if (*lpSrch >= 'a' && *lpSrch <= 'z')
		{
			*pString2 = *lpSrch - 32;
		}
		else
		{
			*pString2 = *lpSrch;
		}
		pString2++;
		lpSrch++;
	}
	*pString2 = 0;
	pReturn = _tcsstr(pStringTemp1, pStringTemp2);

	if (pReturn)
	{
		pos = pReturn - pStringTemp1;
		pReturn = pTemp1 + pos;
	}

	delete [] pStringTemp1;
	delete [] pStringTemp2;

	return const_cast<TCHAR *>(pReturn);
}



LPTSTR strcpyEx(LPTSTR d, LPTSTR s, LPTSTR *pPointToEnd, int *pLen)
{
	//把s拷贝到d
	//把pPointToEnd指向d字符串结尾处, 即0
	//把字符串的长度写到pLen
	int                len = 0;
	wchar_t        *p = NULL;

	p = d;
	if(s && p)
	{
		while(*s)
		{
			*p = *s;
			p++;
			s++;
			len++;
		}
		*p = L'\0';
		if(pPointToEnd) *pPointToEnd = p;
	}
	else
	{
		if(pPointToEnd) *pPointToEnd = NULL;
	}
	if(pLen) *pLen = len;

	return d;
}

CIniMem::CIniMem()
{
	m_file[0] = 0;
}


CIniMem::~CIniMem()
{
	Release();
}


bool CIniMem::Open( IN LPCTSTR lpFileName, bool bLoadDataIntoMem)
{
	_tcsncpy(m_file, lpFileName, MAX_PATH);

	if (bLoadDataIntoMem)
	{
		Release();

		return IoLoad();
	}

	return true;
}





//------------------------------------------------------------------
bool CIniMem::IoLoad()
{
	HANDLE			hFile = INVALID_HANDLE_VALUE;
	bool			bSuccess=false;
	const int		max=1048576;		//最大1M,否则文件异常
	DWORD			count=0;
	TCHAR			*buf=NULL;	
	int				len=0;
	int				nTotalSize = 0;

	hFile = ::CreateFile(m_file, GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, 0, NULL);

	if(hFile != INVALID_HANDLE_VALUE)
	{
		nTotalSize = GetFileSize(hFile, NULL);
		if(nTotalSize != 0 && nTotalSize <= max)
		{
			len = nTotalSize/sizeof(TCHAR);
			buf = new TCHAR[len + 1];
			if(buf)
			{
				if(::ReadFile(hFile, buf, nTotalSize, &count, NULL))
				{
					buf[len]=0;
					bSuccess = true;
				}
			}
		}
		::CloseHandle(hFile);
	}

	if (!buf)
	{
		return false;
	}

	Split(buf, len);

	delete [] buf;

	return bSuccess;
}

void CIniMem::Split(TCHAR *pData, int len)
{
	//以行为单位将数据保存到m_data
	TCHAR	*pLine = NULL, *pLineTemp = NULL;
	int		nLine;		//一行的字符数量
	BYTE	*pUnicodeFlag = NULL;
	TCHAR	*pStart=NULL, *pEnd=NULL;

	pUnicodeFlag = (BYTE *)pData;
	if (pUnicodeFlag[0] == 0xFF && pUnicodeFlag[1] == 0xFE)
	{
		pStart = pData+1;
	}
	else
	{
		//不支持非unicode编码
		return;
	}

	while (*pStart)
	{
		pEnd = pStart;		//设置指针重合
		nLine = 0;
		//找到行结束位置
		while (*pEnd && *pEnd != 13 && *pEnd != 10)
		{
			pEnd++;
			nLine++;
		}
		//判断结束位置的数据
		if (*pEnd == 13 || *pEnd == 10)
		{
			*pEnd = 0;		//设置字符串结束
		}
		pLine = new TCHAR[nLine + 1];
		pLineTemp = pLine;
		//拷贝一行数据
		while (*pStart)
		{
			*pLineTemp = *pStart;
			pLineTemp++;
			pStart++;
		}
		pStart++;
		//设置字符串结束标志
		pLine[nLine] = 0;
#ifdef _DEBUG
		//OutputDebugString(pLine);
		//OutputDebugString(L"\n");
#endif
		//加入到向量
		m_data.push_back(pLine);
		if (pStart > (pData + len))
		{
			break;
		}
		while (*pStart == 13 || *pStart == 10 || *pStart == 9) pStart++;
	}	
}


TCHAR *CIniMem::GetData(int nLineItem)
{
	TCHAR	*pData = NULL;

	pData = m_data[nLineItem];

	while (*pData != '=' && *pData)
	{
		pData++;
	}
	if (*pData == '=')
	{
		pData++;
	}

	return pData;
}

bool CIniMem::SetData(int nLineItem, LPTSTR lpData)
{
	bool	bRet = false;
	TCHAR	*pData = NULL, *pDataTemp = NULL;
	TCHAR	*pDataOld = NULL, *pDataOldTemp = NULL;
	int		count;
	int		len;

	count = m_data.size();
	if (nLineItem<0 || nLineItem>=count || !lpData)
	{
		return false;
	}

	pDataOld = m_data[nLineItem];
	pDataOldTemp = pDataOld;
	//找到'='
	len = 0;
	while (*pDataOldTemp != '=')
	{
		pDataOldTemp++;
		len++;
	}
	len = len + _tcslen(lpData) + 1;		//+1 '='
	pData = new TCHAR[len + 1];
	pDataTemp = pData;
	//拷贝item
	pDataOldTemp = pDataOld;
	while (*pDataOldTemp != '=')
	{
		*pDataTemp = *pDataOldTemp;
		pDataTemp++;
		pDataOldTemp++;
	}
	*pDataTemp = '=';
	pDataTemp++;
	//拷贝数据
	while (*lpData)
	{
		*pDataTemp = *lpData;
		pDataTemp++;
		lpData++;
	}
	*pDataTemp = 0;
	delete [] pDataOld;
	//保存数据

	m_data[nLineItem] = pData;

	return true;
}

void CIniMem::Release()
{
	int		count;
	TCHAR	*data = NULL;

	count = (int)m_data.size();
	for (int i=0; i<m_data.size(); i++)
	{
		data = m_data[i];
		delete [] data;
	}
	m_data.clear();
}


void CIniMem::Add(IN LPTSTR lpSection, IN LPTSTR lpItem, IN LPTSTR lpValue)
{
	TCHAR	*pNewSection = NULL, *pNewItem = NULL;
	int		len = 0;
	int		len2 = 0;

	len = _tcslen(lpSection) + 2;		//[]
	pNewSection = new TCHAR[len+1];
	pNewSection[0] = '[';
	_tcscpy(pNewSection+1, lpSection);
	pNewSection[len-1] = ']';
	pNewSection[len] = 0;

	len = _tcslen(lpItem);
	len2 = _tcslen(lpValue);
	
	pNewItem = new TCHAR[len + len2 + 1 + 1];		// '='
	_tcscpy(pNewItem, lpItem);
	*(pNewItem + len) = '=';
	_tcscpy(pNewItem + len + 1, lpValue);

	m_data.push_back(pNewSection);
	m_data.push_back(pNewItem);
}

void CIniMem::Insert(IN int nLineSection, IN LPTSTR lpItem, IN LPTSTR lpValue)
{
	TCHAR	*pNewItem = NULL;
	int		len = 0, len2;
	std::vector<TCHAR *>::iterator	iter;

	len = _tcslen(lpItem);
	len2 = _tcslen(lpValue);

	pNewItem = new TCHAR[len + len2 + 1 + 1];		// '='
	_tcscpy(pNewItem, lpItem);
	*(pNewItem + len) = '=';
	_tcscpy(pNewItem + len + 1, lpValue);

	iter = m_data.begin();
	iter = iter + nLineSection + 1;

	m_data.insert(iter, pNewItem);
}

//--------------------------set--------------------------

bool CIniMem::SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, IN LPTSTR lpValue )
{
	int		nLineSection, nLineItem;
	
	
	nLineSection = FindSection(0, -1, lpSection);
	if (-1 == nLineSection)
	{
		Add(lpSection, lpItem, lpValue);
		return true;
	}
	nLineItem = FindItem(nLineSection, lpItem);
	if (-1 == nLineItem)
	{
		Insert(nLineSection, lpItem, lpValue);
		return true;
	}

	return SetData(nLineItem, lpValue);
}


bool CIniMem::SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, bool bValue )
{
	TCHAR szValue[64];

	_sntprintf(szValue, 64, TEXT( "%d" ), bValue ? 1 : 0 );
	return SetValue( lpSection, lpItem, szValue );
}



bool CIniMem::SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, int iValue )
{
	TCHAR szValue[64];

	_sntprintf(szValue, 64, TEXT( "%d" ), iValue );
	return SetValue( lpSection, lpItem, szValue );
}


bool CIniMem::SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, long lValue )
{
	TCHAR szValue[64];

	_sntprintf(szValue, 64, TEXT( "%ld" ), lValue );

	return SetValue( lpSection, lpItem, szValue );
}


bool CIniMem::SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, DWORD dwValue )
{
	TCHAR szValue[64];

	_sntprintf(szValue, 64, TEXT( "%u" ), dwValue );

	return SetValue( lpSection, lpItem, szValue );
}


bool CIniMem::SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, float fValue )
{
	return SetValue( lpSection, lpItem, (double)fValue );
}


bool CIniMem::SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, double dbValue )
{
	TCHAR szValue[64];

	_sntprintf(szValue, 64, TEXT( "%f" ), dbValue );
	return SetValue( lpSection, lpItem, szValue );
}



//在内存缓冲区中读取数据
bool CIniMem::GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, IN LPTSTR lpDefault , 
					   OUT TCHAR *buffer, int ncbBuffer)
{
	TCHAR		*pStart = NULL;
	TCHAR		*pData = NULL;
	bool		bContinue=false;
	int			nLineSection, nLineItem;

	nLineSection = FindSection(0, -1, lpSection);
	if (-1 == nLineSection) goto END_GET_VALUE;

	nLineItem = FindItem(nLineSection, lpItem);
	if (-1 == nLineItem) goto END_GET_VALUE;

	pData = GetData(nLineItem);	

END_GET_VALUE:	

	if (!pData)
	{
		pData = lpDefault;
	}

	if (pData)
	{
		_tcsncpy(buffer, pData, ncbBuffer);
	}
	else
	{
		buffer[0] = 0;
	}
	return (pData != lpDefault);
}


bool CIniMem::GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, bool bDefault )
{
	bool	bReturn = bDefault;
	int		nRet;

	//CMzString strReturn;
	//
	//GetValue( lpSection, lpItem, TEXT("") ,strReturn);

	//if ( !strReturn.IsEmpty() )
	//{
	//	nRet = _ttoi( strReturn );
	//	bReturn = nRet != 0;
	//}
	TCHAR		buffer[32];

	buffer[0] = 0;
	GetValue( lpSection, lpItem, TEXT(""), buffer, 32);
	if (buffer[0])
	{
		nRet = _ttoi(buffer);
		bReturn = nRet != 0;
	}


	return bReturn;
}


int CIniMem::GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, int nDefault )
{
	int nReturn = nDefault;

	//CMzString strReturn = GetValue( lpSection, lpItem, TEXT("") );

	//if ( !strReturn.IsEmpty() )
	//{
	//	nReturn = _ttoi( strReturn );
	//}

	TCHAR		buffer[32];

	buffer[0] = 0;
	GetValue( lpSection, lpItem, TEXT(""), buffer, 32);
	if (buffer[0])
	{
		nReturn = _ttoi(buffer);
	}

	return nReturn;
}


long CIniMem::GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, long nDefault )
{
	long nReturn = nDefault;
	TCHAR		buffer[32];

	buffer[0] = 0;
	GetValue( lpSection, lpItem, TEXT(""), buffer, 32);
	if (buffer[0])
	{
		nReturn = _ttoi(buffer);
	}


	return nReturn;
}


DWORD CIniMem::GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, DWORD nDefault )
{
	return (DWORD)GetValue( lpSection, lpItem, (long)nDefault);
}


float CIniMem::GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, float fDefault )
{
	return (float)GetValue( lpSection, lpItem, (double)fDefault);
}


double CIniMem::GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, double dbDefault )
{
	double dbReturn = dbDefault;
	TCHAR		buffer[32];

	buffer[0] = 0;
	GetValue( lpSection, lpItem, TEXT(""), buffer, 32);

	if (buffer[0])
	{
#if defined(UNICODE)
		TCHAR		*pStop;
		dbReturn = wcstod( buffer , &pStop);
#else
		dbReturn = atof( buffer);
#endif
	}

	return dbReturn;
}



int CIniMem::FindSection(IN int nLineStart, IN int nLineEnd, IN LPTSTR lpSection)
{
	TCHAR		*pStart;
	//TCHAR		*pSectionFormat = NULL;
	TCHAR		pSectionFormat[1024];
	int			len;
	TCHAR		*pLine = NULL;
	int			nLine;


	len = _tcslen(lpSection);
	//pSectionFormat = new TCHAR[len + 3];
	

	pSectionFormat[0] = '[';
	_tcscpy(pSectionFormat+1, lpSection);
	pSectionFormat[len + 1] = ']';
	pSectionFormat[len + 2] = 0;

	if (-1 == nLineEnd)
	{
		nLineEnd = (int)m_data.size();
	}

	for (nLine = nLineStart; nLine<nLineEnd; nLine++)
	{
		pLine = m_data[nLine];
		pStart = pLine;
		//检查该一行是否是合法性的section段
		//跳过空格和回车
		while(*pStart != 0 && (*pStart == 13 || *pStart == 10 || *pStart == 9))
		{
			pStart++;
		}
		//此时 *pStart 的值必须是 '['才有可能是合法性的section段
		if(*pStart == '[')
		{
			if (stristr(pLine, pSectionFormat))
			{
				//找到了
				break;
			}
		}
		else
		{
			//不合法的section 段
			//continue
		}
	}
	nLine = nLine >= nLineEnd ? -1 : nLine;

	//delete [] pSectionFormat;

	return nLine;
}


int	CIniMem::FindNextSection(IN int nLineStart)
{
	//查找pData开始的下一个section
	TCHAR		*pStart=NULL,*pEnd=NULL, *pData = NULL;
	bool		bFind=false;
	int			count, nLine;


	count = (int)m_data.size();

	if (nLineStart<0 || nLineStart>=count)
	{
		return -1;
	}

	for (nLine=nLineStart; nLine<count; nLine++)
	{
		pData = m_data[nLine];
		pStart = _tcschr(pData, '[');
		pEnd = pStart;
		while(pEnd && !bFind)
		{
			if(pEnd)
			{
				while(*pEnd && *pEnd != 13 && *pEnd != 10 && !bFind)
				{
					if(*pEnd == ']')
					{
						//bFind = true;//bug
						//合法的段']'后面必须是换行, 或结束符
						if(*(pEnd + 1) == 13 || *(pEnd + 1) == 10 || *(pEnd + 1) == 0)
						{
							bFind = true;
						}
						else
						{
							//继续搜索
						}
					}
					pEnd++;	
				}
				if(!bFind)
				{
					pEnd =  _tcschr(pEnd, '[');
					pStart = pEnd;
				}
			}
		}
		if (bFind)
		{
			break;
		}
	}

	nLine = nLine >= count ? -1 : nLine;

	return nLine;
}



/************************************************************************************
*      函数名 : CIniMem::FindItem
*        描述 : 获取项目的数据所在的行索引
2009-10-21修复匹配bug
解决方法:
在lpItem后面加上 '=', 进行串匹配
*  返回值类型 : int 
*        参数 : IN int nLineSectionStart
*        参数 : IN LPTSTR lpItem
************************************************************************************/
int CIniMem::FindItem(IN int nLineSectionStart, IN LPTSTR lpItem)
{
	TCHAR		*pStart = NULL;
	TCHAR		*pLine = NULL, *pLineTemp = NULL;
	int			nLine;
	int			nLineSectionEnd = -1;
	TCHAR		*pItem = NULL;

	nLineSectionEnd = FindNextSection(nLineSectionStart + 1);	//寻找本节的最后一行
	if (-1 == nLineSectionEnd)		//本节是最后一节?
	{
		nLineSectionEnd = (int)m_data.size();
	}
//////////////////////////////////////////////////////////////////////////
//修复字符串匹配bug
	nLine = _tcslen(lpItem);		//nLine临时保存item的长度
	pItem = new TCHAR[nLine + 2];
	_tcscpy(pItem, lpItem);
	pItem[nLine] = '=';
	pItem[nLine+1] = 0;
//////////////////////////////////////////////////////////////////////////
	for (nLine = nLineSectionStart+1; nLine<nLineSectionEnd; nLine++)
	{
		pLine = m_data[nLine];
		pLineTemp = pLine;
		//跳过空格和回车
		while(*pLineTemp != 0 && 
			(*pLineTemp == 13 || *pLineTemp == 10 || *pLineTemp == 9 || *pLineTemp == ' '))
		{
			pLineTemp++;
		}
		//pStart = stristr(pLineTemp, lpItem);	//bug
		pStart = stristr(pLineTemp, pItem);		//fixed

		//此时 pStart == pLineTemp 才是合法性的Item段
		if (pStart == pLineTemp)
		{
			break;
		}
	}
	nLine = nLine >= nLineSectionEnd ? -1 : nLine;

	delete [] pItem;

	return nLine;
}


bool CIniMem::Flush()
{
	TCHAR	*buffer = NULL;
	TCHAR	*pBuffer = NULL;
	TCHAR	*pEnd = NULL;
	TCHAR	*pDataLine = NULL;
	BYTE	*pUnicodeFlag = NULL;
	int		count;
	int		nLine;
	int		size;		//写到文件的字符数量
	HANDLE	hFile = INVALID_HANDLE_VALUE;

	hFile = ::CreateFile(m_file, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	count = m_data.size();

	if (count == 0)
	{
		::CloseHandle(hFile);
		return true;
	}

	size = 0;
	for (nLine=0; nLine<count; nLine++)
	{
		pDataLine = m_data[nLine];
		size += _tcslen(pDataLine);
	}
	
	//换行和回车count*2 , unicode 标志0xff 0xfe(1),
	//如果使用的ansi字符集, 这里需要调整(通常都是使用unicode字符集, 所以简单起见, 不考虑ansi)
	size = size + count*2 + 1;
	buffer = new TCHAR[size + 1];
	pUnicodeFlag = (BYTE *)buffer;

	pUnicodeFlag[0] = 0xff;
	pUnicodeFlag[1] = 0xfe;
	pBuffer = buffer + 1;
	
	for (nLine=0; nLine<count; nLine++)
	{
		pDataLine = m_data[nLine];
		strcpyEx(pBuffer, pDataLine, &pEnd, NULL);
		pEnd[0] = 13;
		pEnd[1] = 10;
		pBuffer = pEnd + 2;
	}
	*pEnd = 0;

	DWORD	dwWritten;

	//字符换算到字节
	size = sizeof(TCHAR) * size;
	//最后一行不需要换行符少写出2个字符
	BOOL bRet = ::WriteFile(hFile, buffer, size - 2*sizeof(TCHAR), &dwWritten, NULL);
	
	::CloseHandle(hFile);

	delete [] buffer;

	return (bRet == TRUE);
}