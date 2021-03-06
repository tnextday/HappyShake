#include "CHappyShakeApp.h"
#include <ShellNotifyMsg.h>
#include "MyImageContainer.h"
#include <MyStoreLib.h>
#include "IniMem.h"
#include <mzfc\mzfc_driver.h>
#pragma comment(lib, "MyStoreLib.lib")

#define MUTEX_WND_TITTLE		L"摇摇乐 "

bool g_bDemoVersion = true;

static MyImageContainer g_imageContainer;

CIniMem *g_config;

//检测是否已经有另外的实例在运行中:
BOOL checkRunning(TCHAR *wndName)
{
	BOOL bFound = FALSE;
	HANDLE hMutexOne = CreateMutex(NULL, TRUE, wndName);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		bFound = TRUE;
	if(hMutexOne)
	{
		ReleaseMutex(hMutexOne);
	}
	if (bFound == TRUE)
	{ 
		//激活已经在运行的实例的窗口   
		HWND pwndFirst = FindWindow(NULL, wndName);
		if(pwndFirst)
		{
			if(!IsWindowVisible(pwndFirst))
				ShowWindow(pwndFirst, SW_SHOW);
			SetForegroundWindow(pwndFirst);
		}
		return TRUE;
	}
	return FALSE;
}

ImagingHelper* ldImg(wchar_t* name)
{
	return g_imageContainer.LoadImage(name);
}

bool verifyLiscense()
{
#if defined(_DEBUG)
	g_bDemoVersion = false;
	return true;
#endif
	wchar_t pszFileName[MAX_PATH] = {0};
	GetModuleFileName(0, pszFileName, MAX_PATH);            // 当前进程EXE的文件名
	MYSTORE_VERIFY_CONTEXT mystore = {0};
	DWORD dwRet = MyStoreVerify(pszFileName, &mystore);     // 验证此文件是否合法
	switch(dwRet)
	{
	case 0:			// 合法
		{
			// 检验是否超过试用期
			// 			RETAILMSG(1, (L"合法 , LicenseValid:%u, Expired:%u, 试用期终止日期 :%u\n",
			// 				mystore.LicenseValid, mystore.Expired, mystore.ExpireDate ));
			if(mystore.Expired)
			{
				// 超过试用期
				// 认为是试用软件 做限制功能 ...
				g_bDemoVersion = true;
			}
			else
			{
				if((mystore.Reserved[0] & 0x40) == 0x40)
					g_bDemoVersion = true;
				else
					g_bDemoVersion = false;
			}
		}
		break;
	case 1:		//PostQuitMessage(0);
	case 2:		//打开 License 文件失败
	case 3:		//验证失败
		g_bDemoVersion = true;
		break;
	case 4:		//序列号获取失败
		PostQuitMessage(0);
		break;
	}
	return !g_bDemoVersion;
}

CHappyShakeApp::CHappyShakeApp(void)
{
	m_MainWnd = NULL;
	g_config = new CIniMem;
}

CHappyShakeApp::~CHappyShakeApp(void)
{
	if (m_MainWnd)
	{
		delete m_MainWnd;
		m_MainWnd = NULL;
	}
	g_config->Flush();
	delete g_config;
}

BOOL CHappyShakeApp::Init()
{
	if (checkRunning(MUTEX_WND_TITTLE))
	{
		return FALSE;
	}
	
	// Initialize COM
	CoInitializeEx(0, COINIT_MULTITHREADED);

	CMzString strFilePath(MAX_PATH);
	GetWorkPath(strFilePath.C_Str(), strFilePath.GetBufferSize());
	strFilePath += L"\\Data\\";
	g_imageContainer.setDefaultPath(strFilePath);
	//verifyLiscense();
	//MzDisableAntiTearing(true);
	
	// Get work area
	int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	// Creat main window
	m_MainWnd = new CHappyShakeMainWnd;
	m_MainWnd->Create(0, 0, nScreenWidth, nScreenHeight, 0, 0, 0);
	m_MainWnd->SetWindowText(MUTEX_WND_TITTLE);
	m_MainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_MainWnd->SetWindowPos(HWND_TOP, 0, 0, nScreenWidth, nScreenHeight);

	return TRUE;
}

int CHappyShakeApp::Done()
{
	//MzEnableAntiTearing(true);
	// 解除屏幕高亮
	SetScreenAutoOff ();	
	// 显示顶部
	ShowMzTopBar();
	CoUninitialize();
	
	return 0;
}
// Global App Variable
CHappyShakeApp theApp;
