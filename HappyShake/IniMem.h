//IniMem.h
#pragma once

#include <vector>
//为M8智能设备专用优化版本
#include <Windows.h>

//虽然类里面使用的是TCHAR宏, 实际编译时要选择使用unicode字符集

class CIniMem
{
private:
	std::vector<TCHAR *> m_data;		//以行为单位保存一行数据的指针引用
	TCHAR			m_file[MAX_PATH];
public:
	CIniMem();
	virtual ~CIniMem();
//初始化代码
	bool	Open( IN LPCTSTR lpFileName, bool bLoadDataIntoMem);
//初始化代码	end
private:
	int	FindSection(IN int nLineStart, IN int nLineEnd, IN LPTSTR lpSection);	//返回所在的行索引
	int	FindNextSection(IN int nLineStart);	//选择下一个section
	int FindItem(IN int nLineSectionStart, IN LPTSTR lpItem);		//获取项目的数据所在的行索引
	bool	IoLoad();
	void Split(TCHAR *pData, int len);		//将数据拆分为一行一行的, 并保存到m_data
	TCHAR *GetData(int nLineItem);		//从一行里面提取数据
	bool SetData(int nLineItem, LPTSTR lpData);		//从一行里面提取数据
	void Release();
	void Add(IN LPTSTR lpSection, IN LPTSTR lpItem, IN LPTSTR lpValue);		//添加一个(新增)
	void Insert(IN int nLineSection, IN LPTSTR lpItem, IN LPTSTR lpValue);		//插入一个
public:
//在内存缓冲区中操作
//SET
	bool	SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, IN LPTSTR lpValue );	
	bool	SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, bool bValue );
	bool	SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, int iValue );
	bool	SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, long lValue );
	bool	SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, DWORD dwValue );
	bool	SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, float fValue);
	bool	SetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, double dbValue);
//GET
	bool	GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, IN LPTSTR lpDefault , OUT TCHAR *buffer, int ncbBuffer);
	bool	GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, bool bDefault );
	int		GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, int nDefault );
	long	GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, long nDefault );
	DWORD	GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, DWORD nDefault );		
	float	GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, float fDefault );
	double	GetValue( IN LPTSTR lpSection, IN LPTSTR lpItem, double dbDefault );
public:
	bool Flush();		//写出数据
};