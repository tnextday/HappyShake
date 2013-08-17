#pragma once

#include <mzfc_inc.h>

#define MZ_IDC_TOOLBAR1  102

class CUiInputDlg : public CMzWndEx
{
MZ_DECLARE_DYNAMIC(CUiInputDlg);
public:
	CUiInputDlg(void);
	~CUiInputDlg(void);
	UiSingleLineEdit m_Edit;
	UiToolBarPro m_Toolbar;
	UiStatic	m_lable;
	UiStatic	m_description;

	void setTitle(CMzString& title);
	void setTip(CMzString &tip);
	void setDescription(CMzString &str);
	CMzString& getInputText();

	CMzString m_defaultText;

protected:
	// Initialize main form
	virtual BOOL OnInitDialog();
	LRESULT MzDefWndProc(UINT message, WPARAM wParam, LPARAM lParam);

	// Over write MZFC's OnMzCommand function to dispose command notify
	virtual void OnMzCommand(WPARAM wParam, LPARAM lParam);

};

int MyInputMessageBox(HWND hWnd, const wchar_t* title, CMzString& Text, const wchar_t* description = NULL);
