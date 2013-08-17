//IniMem.h
#pragma once

#include <vector>
//ΪM8�����豸ר���Ż��汾
#include <Windows.h>

//��Ȼ������ʹ�õ���TCHAR��, ʵ�ʱ���ʱҪѡ��ʹ��unicode�ַ���

class CIniMem
{
private:
	std::vector<TCHAR *> m_data;		//����Ϊ��λ����һ�����ݵ�ָ������
	TCHAR			m_file[MAX_PATH];
public:
	CIniMem();
	virtual ~CIniMem();
//��ʼ������
	bool	Open( IN LPCTSTR lpFileName, bool bLoadDataIntoMem);
//��ʼ������	end
private:
	int	FindSection(IN int nLineStart, IN int nLineEnd, IN LPTSTR lpSection);	//�������ڵ�������
	int	FindNextSection(IN int nLineStart);	//ѡ����һ��section
	int FindItem(IN int nLineSectionStart, IN LPTSTR lpItem);		//��ȡ��Ŀ���������ڵ�������
	bool	IoLoad();
	void Split(TCHAR *pData, int len);		//�����ݲ��Ϊһ��һ�е�, �����浽m_data
	TCHAR *GetData(int nLineItem);		//��һ��������ȡ����
	bool SetData(int nLineItem, LPTSTR lpData);		//��һ��������ȡ����
	void Release();
	void Add(IN LPTSTR lpSection, IN LPTSTR lpItem, IN LPTSTR lpValue);		//���һ��(����)
	void Insert(IN int nLineSection, IN LPTSTR lpItem, IN LPTSTR lpValue);		//����һ��
public:
//���ڴ滺�����в���
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
	bool Flush();		//д������
};