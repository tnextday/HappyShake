#pragma once
#include <mzfc_inc.h>
#include "UiMyList.h"
#include "glWnd.h"


#define MZ_IDC_TESTTOOLBAR				101
#define MZ_IDC_LIST						102


#define MZ_IDC_GRIDMENU_IMPORT			1
#define MZ_IDC_GRIDMENU_EXPORT			2
#define MZ_IDC_GRIDMENU_SELECT			3
#define MZ_IDC_GRIDMENU_SAVE			4
#define MZ_IDC_GRIDMENU_DELETE			5
#define MZ_IDC_GRIDMENU_RENAME			6

bool selectFile(HWND hWnd, CMzString &StrFile, TCHAR *filter, TCHAR*defDir = L"");
void showWaiting(HWND hWnd);
void hideWaiting();


struct FilePreset {
	Preset		preset;
	int			imageSize;
};

class CLibraryWnd :
	public CMzWndEx
{
MZ_DECLARE_DYNAMIC(CLibraryWnd);
public:
	CLibraryWnd(void);
	~CLibraryWnd(void);
	UiMyList		m_list;
	UiToolBarPro	m_ToolBarPro;
	MzGridMenu		m_GridMenu;
	ImageContainer	m_imgContainer;

	int doShow(CglWnd* glWnd);

	bool saveAll();
	bool savePreset(int nIndex);
	bool reloadAll();
	bool saveCrrtPreset();

	void doSave();
	void doRename();
	void doDelete();
	bool doLoadPreset(int nIndex, CglWnd* glWnd = NULL);

	int importPresets();
	int exportPresets();

	CMzString getDefaultPresetName(wchar_t* defaultPrefix);
	Preset* getPresetByName(wchar_t* strName);
	void setUseUserPic();
	int	getCrrtPreset(){return m_crrtPreset;}
	
private:
	CglWnd*			m_glWnd;
	int				m_crrtPreset;
	int				m_nMaxPresets;
	

protected:
	// Initialize main form
	virtual BOOL OnInitDialog();
	LRESULT MzDefWndProc(UINT message, WPARAM wParam, LPARAM lParam);

	// Over write MZFC's OnMzCommand function to dispose command notify
	virtual void OnMzCommand(WPARAM wParam, LPARAM lParam);
	void OnLButtonDown(UINT fwKeys, int xPos, int yPos);
	
	
};
