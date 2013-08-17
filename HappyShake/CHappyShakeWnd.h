#pragma once

// Include MZFC header file
#include <mzfc_inc.h>
#include "glWnd.h"
#include "LibraryWnd.h"

#define MAINWND_TITTLE		L"“°“°¿÷"

// Button control's ID
#define MZ_TOUCH	WM_USER + 2

#define TIMER_LOADDEFAULT  5


// Application main form class derived from CMzWndEx
class CHappyShakeMainWnd: public CMzWndEx
{
  MZ_DECLARE_DYNAMIC(CHappyShakeMainWnd);
public:
	CHappyShakeMainWnd(void);
	~CHappyShakeMainWnd(void);

	CglWnd				m_glWnd;
	UINT				m_allkeyEvent;
	CLibraryWnd			m_libraryWnd;
	void OnLButtonDblClk(UINT fwKeys, int xPos, int yPos);
	void showLibrary();
	void changeEditer();

	void selectPicture();

	void LoadDefault();

private:
	bool	m_bEdit;

protected:
  // Initialize main form
  virtual BOOL OnInitDialog();
  LRESULT MzDefWndProc(UINT message, WPARAM wParam, LPARAM lParam);

  // Over write MZFC's OnMzCommand function to dispose command notify
  virtual void OnMzCommand(WPARAM wParam, LPARAM lParam);

//     void OnCommond(WORD wNotifyCode, WORD wID, HWND hwndctl);
//   
  
//   
//     void CHappyShakeMainWnd::OnLButtonDown(UINT fwKeys, int xPos, int yPos);
//   
//     void CHappyShakeMainWnd::OnLButtonUp(UINT fwKeys, int xPos, int yPos);
//   
//     void CHappyShakeMainWnd::OnMouseMove(UINT fwKeys, int xPos, int yPos);
//   
//     void CHappyShakeMainWnd::OnPaint(HDC hdc, LPPAINTSTRUCT ps);
//   
//     void CHappyShakeMainWnd::OnSettingChange(DWORD wFlag, LPCTSTR pszSectionName);
//   
//     int CHappyShakeMainWnd::OnShellHomeKey(UINT message, WPARAM wParam, LPARAM lParam);
//   
//     void CHappyShakeMainWnd::OnSize(int nWidth, int nHeight);
//   
     void OnTimer(UINT_PTR nIDEvent);
  };
