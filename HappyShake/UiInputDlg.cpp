#include "UiInputDlg.h"
#include <ShellNotifyMsg.h>

MZ_IMPLEMENT_DYNAMIC(CUiInputDlg)

CUiInputDlg::CUiInputDlg(void)
{
}

CUiInputDlg::~CUiInputDlg(void)
{
}

BOOL CUiInputDlg::OnInitDialog()
{
	// First invoke initialization of parent class
	if (!CMzWndEx::OnInitDialog())
	{
		return FALSE;
	}

	SetAnimateType_Show(MZ_ANIMTYPE_SCROLL_BOTTOM_TO_TOP_2);
	// 设置窗口切换动画（结束时的动画）
	SetAnimateType_Hide(MZ_ANIMTYPE_SCROLL_TOP_TO_BOTTOM_1 );

	m_lable.SetPos(10, 10, GetWidth()-20, MZM_HEIGHT_SINGLELINE_EDIT_BG);
	m_lable.SetTextSize(27);
	AddUiWin(&m_lable);
	

	m_Edit.SetMaxChars(16);
	m_Edit.SetPos(10, 80, GetWidth()-20, MZM_HEIGHT_SINGLELINE_EDIT_BG);
	m_Edit.SetSipMode(IM_SIP_MODE_GEL_PY);
	AddUiWin(&m_Edit); 

	m_description.SetPos(10, 154, GetWidth()-20, MZM_HEIGHT_SINGLELINE_EDIT_BG );
	m_description.SetTextColor (RGB(163, 163, 163));
	m_description.SetMargin(3) ;
	m_description.SetTextSize(22);
	m_description.SetDrawTextFormat(DT_LEFT|DT_WORDBREAK|DT_END_ELLIPSIS);
	AddUiWin(&m_description);

	m_Toolbar.SetPos(0,GetHeight()-MZM_HEIGHT_TEXT_TOOLBAR,GetWidth(),MZM_HEIGHT_TOOLBARPRO);
	m_Toolbar.SetButton(TOOLBARPRO_LEFT_TEXTBUTTON, true, true, L"确定");
	m_Toolbar.SetButton(TOOLBARPRO_RIGHT_TEXTBUTTON, true, true, L"取消");
	m_Toolbar.SetID(MZ_IDC_TOOLBAR1);
	m_Toolbar.SetVisible(false);
	AddUiWin(&m_Toolbar);

	return TRUE;
}


CMzString& CUiInputDlg::getInputText()
{
	if (m_Edit.GetText().IsEmpty())
	{
		return m_defaultText;
	}
	return m_Edit.GetText();
}

void CUiInputDlg::setTip( CMzString &tip )
{
	m_Edit.SetTip(tip);
	m_defaultText = tip;
}

void CUiInputDlg::OnMzCommand( WPARAM wParam, LPARAM lParam )
{
	UINT_PTR id = LOWORD(wParam);
	if (id == MZ_IDC_TOOLBAR1) 
	{
		RECT rcWork = MzGetWorkArea();
		SetWindowPos(HWND_TOP, rcWork.left,rcWork.top,RECT_WIDTH(rcWork),RECT_HEIGHT(rcWork) - MZM_HEIGHT_TEXT_TOOLBAR);
		
		int nIndex = lParam;
		if (nIndex == TOOLBARPRO_LEFT_TEXTBUTTON)
		{
			EndModal(ID_OK);
			return;
		}
		if (nIndex== TOOLBARPRO_RIGHT_TEXTBUTTON)
		{
			EndModal(ID_CANCEL);
			return;
		}
	}
}

LRESULT CUiInputDlg::MzDefWndProc( UINT message, WPARAM wParam, LPARAM lParam )
{
// 	if (message >= MZFC_WM_MESSAGE)
// 		printf("MZFC_WM_MESSAGE 0x%08x\n", message - MZFC_WM_MESSAGE);
// 	else
// 		printf("WM_MESSAGE 0x%08x\n", message);
	if (MZ_WM_WND_ACTIVATE == message && wParam != WA_INACTIVE)
	{
		RECT rcWork = MzGetWorkArea();
		SetWindowPos(HWND_TOP, rcWork.left,rcWork.top,RECT_WIDTH(rcWork),RECT_HEIGHT(rcWork));
		m_Toolbar.SetPos(0,GetHeight()-MZM_HEIGHT_TOOLBARPRO,GetWidth(),MZM_HEIGHT_TOOLBARPRO);
		m_Toolbar.SetVisible(true);
		m_Edit.SetFocus (true);
		Invalidate();
	}

	return CMzWndEx::MzDefWndProc(message,wParam,lParam);
}

void CUiInputDlg::setTitle( CMzString& title )
{
	m_lable.SetText(title);
}

void CUiInputDlg::setDescription( CMzString &str )
{
	m_description.SetText(str);
}
int MyInputMessageBox( HWND hWnd, const wchar_t* title, CMzString& Text, const wchar_t* description /*= NULL*/ )
{
	CUiInputDlg inputDlg;
	RECT rcWork = MzGetWorkArea();
	int ret;
	if (description)
	{
		inputDlg.setDescription(CMzString(description));
	}
	inputDlg.setTitle(CMzString(title));
	inputDlg.setTip(Text);
	inputDlg.Create(rcWork.left,rcWork.top,RECT_WIDTH(rcWork),RECT_HEIGHT(rcWork) - MZM_HEIGHT_TOOLBARPRO,
		hWnd, 0, WS_POPUP);
	ret = inputDlg.DoModal();
	Text = inputDlg.getInputText();
	return ret;
}