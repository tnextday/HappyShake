#include "CHappyShakeWnd.h"
#include <ShellNotifyMsg.h>
#include <TouchNotifyApi.h>
#include <UsbNotifyApi.h>
#include <CallNotifyApi.h>
#include <InitGuid.h>
#include <IMzUnknown.h>
#include <IMzUnknown_IID.h>
#include <IPhotoViewer.h>
#include <IPhotoViewer_GUID.h>
#include "MyImageContainer.h"
#include "IniMem.h"


//������ȡ�쳣�¼�����Ϣ�õı���
INT		g_iUsbNotifyMsg = 0;
DWORD	g_dwCallMsg;
DWORD	g_dwSmsMsg;

extern bool g_bDemoVersion;
extern CIniMem *g_config;

ImagingHelper* ldImg(wchar_t* name);


MZ_IMPLEMENT_DYNAMIC(CHappyShakeMainWnd)

//////////////////////////////////////////////////////////////////////////

CHappyShakeMainWnd::CHappyShakeMainWnd(void)
{
	
}

CHappyShakeMainWnd::~CHappyShakeMainWnd(void)
{
	UnRegisterTouchNotify(m_hWnd, 0);
}

BOOL CHappyShakeMainWnd::OnInitDialog()
{
	// First invoke initialization of parent class
	if (!CMzWndEx::OnInitDialog())
	{
		return FALSE;
	}
	
	//�쳣�¼���ȡ��Ϣ
	g_iUsbNotifyMsg = RegisterUsbNotifyMsg();
	g_dwCallMsg = GetCallRegisterMessage();
	//g_dwSmsMsg = GetSmsRegisterMessage();

	// ע������Ӳ������Ϣ WM_MZSH_ALL_KEY_EVENT
	//RegisterShellMessage(m_hWnd, WM_MZSH_ALL_KEY_EVENT);
	// ��ȡ����Ӳ������Ϣֵ
	m_allkeyEvent = GetShellNotifyMsg_AllKeyEvent();

	//SetScreenAlwaysOn(m_hWnd);

	RegisterTouchNotifyEx(m_hWnd, MZ_TOUCH, TCH_NOTIFY_FLAG_GESTURE);

	m_bEdit = false;
	HideMzTopBar();
	m_glWnd.CreateChildWindow(0,0,GetWidth(),GetHeight(), m_hWnd);
	m_glWnd.setParentWnd(this);
	m_glWnd.SetWindowText(MAINWND_TITTLE);
	m_glWnd.Show();
	SetTimer(m_hWnd, TIMER_LOADDEFAULT, 1, NULL);
	return TRUE;
}

 void CHappyShakeMainWnd::OnMzCommand( WPARAM wParam, LPARAM lParam )
 {
 	UINT_PTR id = LOWORD(wParam);
	int nIndex = lParam;
 	switch(id)
 	{
 		case WM_IDC_GLBUTTON:  // click Button control
 		{
			// ������߹̶����ְ�������Ϣ���˳�����
			if (nIndex == 0)
				changeEditer();
			else if (nIndex == 1)
				m_glWnd.addEditBubble();
			else if (nIndex == 2)
				m_glWnd.removeEditBubble();
			else if (nIndex == 3)
			{
				if (g_bDemoVersion)
				{
					MzMessageBoxV2(m_hWnd, L"��ǰ�汾Ϊ���ð�,���Զ���ͼƬ����!\n���๦���빺����ʽ����Ȩ!\nлл����֧��!", MZV2_MB_OK);
					return;
				}
				selectPicture();
			}
			else if (nIndex == 4)
				showLibrary();
		}
		break;
 	}
 }


// int CHappyShakeMainWnd::OnShellHomeKey(UINT message, WPARAM wParam, LPARAM lParam)
// {
//  	 switch(message)
//  	 {
//  	 }
// 	return 0;
// }

LRESULT CHappyShakeMainWnd::MzDefWndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_USER_EDIT_STATE_CHANGE && m_bEdit)
	{
	}
	if (message == MZ_TOUCH)
	{
		if (wParam == TCH_EVENT_ZOOM_IN)
			m_glWnd.zoomCrrtSelect(true);
		else if (wParam == TCH_EVENT_ZOOM_OUT)
			m_glWnd.zoomCrrtSelect(false);
		return 0;
	}

	if(message == g_iUsbNotifyMsg)
	{
		INT iEvenType = (INT)wParam;
		if(USB_MASSSTORAGE_ATTACH == iEvenType)
		{
			//�쳣��Ϣ��U��ģʽ�²�USB���˳�
			PostQuitMessage(0);
		}
	}
	if (message == g_dwCallMsg /*|| message == g_dwSmsMsg*/)
	{
		//�쳣��Ϣ�������磬�˳�
		//gprs�Ҳ�ᷢ�����Ϣ
		if (wParam == PAL_CALLSTAT_BIT_DIALING)
			PostQuitMessage(0);
	}
	if (message == MZ_WM_WND_ACTIVATE && wParam != WA_INACTIVE)
	{
		HideMzTopBar();
	}
	return CMzWndEx::MzDefWndProc(message,wParam,lParam);

}

void CHappyShakeMainWnd::OnLButtonDblClk( UINT fwKeys, int xPos, int yPos )
{
	//showFullScreen(!m_bFullScreen);
}


void CHappyShakeMainWnd::showLibrary()
{
	m_glWnd.setActive(false);
	ShowMzTopBar();
	if (m_libraryWnd.doShow(&m_glWnd) == ID_OK && m_glWnd.getActiveBubbleCount())
		changeEditer();
	m_glWnd.setActive(true);
}


void CHappyShakeMainWnd::changeEditer()
{
	m_bEdit = !m_bEdit;
	m_glWnd.showEdit(m_bEdit);
	if (!m_bEdit)
	{
		int nIndex = m_libraryWnd.getCrrtPreset();
		g_config->SetValue(L"Last", L"Index", nIndex);
		if (nIndex < 0)
		{
			TCHAR lpItem[6];
			TCHAR lpValue[50];
			for (int i = 0; i < 5; i++)
			{
				int x,y,r;
				m_glWnd.getBubbleSet(i, x, y, r);
				_stprintf(lpItem, L"Sets%d", i);
				_stprintf(lpValue, L"%d,%d,%d", x, y, r);
				g_config->SetValue(L"Last", lpItem,lpValue);
			}
		}
		else
		{
			m_libraryWnd.saveCrrtPreset();
		}
	}

}


void CHappyShakeMainWnd::selectPicture()
{
	m_glWnd.setActive(false);
	ShowMzTopBar();
	if (m_glWnd.selectImageFile())
	{
		m_libraryWnd.setUseUserPic();
	}
	m_glWnd.setActive(true);
}	

bool verifyLiscense();

void CHappyShakeMainWnd::LoadDefault()
{
	RECT rect;
	rect.left = 218;
	rect.top = 338;
	rect.bottom = 382;
	rect.right = 262;
	MzBeginWaitDlg (m_hWnd, &rect, true);

	verifyLiscense();
	RECT rcWork = MzGetWorkArea();
	m_libraryWnd.Create(rcWork.left,rcWork.top,RECT_WIDTH(rcWork),RECT_HEIGHT(rcWork),
		m_hWnd, 0, WS_POPUP);
	changeEditer();
	CMzString strFilePath(_MAX_PATH);
	GetWorkPath(strFilePath.C_Str(), strFilePath.GetBufferSize());
	strFilePath += L"\\config.ini";
	g_config->Open(strFilePath.C_Str(), true);
	int nIndex = g_config->GetValue(L"Last", L"Index", -1);
	if (nIndex >= 0)
		m_libraryWnd.doLoadPreset(nIndex, &m_glWnd);
	else
	{
		CMzString strFilePath(_MAX_PATH);
		GetWorkPath(strFilePath.C_Str(), strFilePath.GetBufferSize());
		strFilePath += L"\\Data\\Last.jpg";
		if (m_glWnd.setImage(strFilePath))
		{
			TCHAR lpItem[6];
			TCHAR lpValue[50];
			m_glWnd.removeAllBubbles();
			for (int i = 0; i < 5; i++)
			{
				_stprintf(lpItem, L"Sets%d", i);
				int x,y,r;
				if (g_config->GetValue(L"Last", lpItem, L"", lpValue, 50))
				{
					_stscanf(lpValue,L"%d,%d,%d", &x, &y, &r);
					if (r >= MIN_BUBBLE_R && r <= MAX_BUBBLE_R)
						m_glWnd.addEditBubble(x, y, r);
				}
			}
		}
		else
			m_libraryWnd.doLoadPreset(0, &m_glWnd);	
	}
	
	changeEditer();
	MzEndWaitDlg();
}

void CHappyShakeMainWnd::OnTimer( UINT_PTR nIDEvent )
{
	if (nIDEvent == TIMER_LOADDEFAULT)
	{
		KillTimer(m_hWnd, TIMER_LOADDEFAULT);
		LoadDefault();
		return;
	}
}

