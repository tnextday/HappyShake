#include "LibraryWnd.h"
#include <IMzUnknown.h>
#include <IMzUnknown_IID.h>
#include <IPhotoViewer.h>
#include <IPhotoViewer_GUID.h>
#include <ShellNotifyMsg.h>
#include <IFileBrowser.h>
#include <IFileBrowser_GUID.h>
#include "UiInputDlg.h"
#include "IniMem.h"

#define MAX_EXPORT_PRESETS	20
#define PRESET_PATH		L"\\presets\\"

#define APP_VERSION		L"v 3.1"

extern bool g_bDemoVersion;
extern CIniMem *g_config;

ImagingHelper* ldImg(wchar_t* name);

MZ_IMPLEMENT_DYNAMIC(CLibraryWnd)

CLibraryWnd::CLibraryWnd(void)
{
	m_glWnd = NULL;
	m_crrtPreset = -1;
}

CLibraryWnd::~CLibraryWnd(void)
{
	m_list.RemoveAll();
}

BOOL CLibraryWnd::OnInitDialog()
{
	// First invoke initialization of parent class
	if (!CMzWndEx::OnInitDialog())
	{
		return FALSE;
	}
	m_nMaxPresets = g_bDemoVersion ? 8 : 100;
	RegisterShellMessage(m_hWnd, WM_MZSH_ENTRY_LOCKPHONE);

	m_list.SetPos(0,0,GetWidth(),GetHeight()-MZM_HEIGHT_TEXT_TOOLBAR);
	m_list.SetID(MZ_IDC_LIST);
	m_list.EnableScrollBarV(true);
	//m_list.EnableNotifyMessage(true);
	AddUiWin(&m_list);

	// 初始化 UiToolBarPro 控件
	m_ToolBarPro.SetID(MZ_IDC_TESTTOOLBAR);
	m_ToolBarPro.SetPos(0, GetHeight() - MZM_HEIGHT_TOOLBARPRO, GetWidth(), MZM_HEIGHT_TOOLBARPRO);
	m_ToolBarPro.SetButton(TOOLBARPRO_LEFT_TEXTBUTTON, true, true, L"返回");
	m_ToolBarPro.SetButton(TOOLBARPRO_RIGHT_TEXTBUTTON, true, true, L"关于");

	m_ToolBarPro.SetMiddleButton(true, true, L"更多选项", ldImg(L"3_1"), ldImg(L"3_2"), ldImg(L"3_1"));
	AddUiWin(&m_ToolBarPro);

	// 初始化 MzGridMenu 控件
	m_GridMenu.AppendMenuItem(MZ_IDC_GRIDMENU_IMPORT, L"导入", ldImg(L"5_1"), ldImg(L"5_2"));
	m_GridMenu.AppendMenuItem(MZ_IDC_GRIDMENU_EXPORT, L"导出", ldImg(L"6_1"), ldImg(L"6_2"));
	m_GridMenu.AppendMenuItem(MZ_IDC_GRIDMENU_SELECT, L"全选", ldImg(L"7_1"), ldImg(L"7_2"));
	m_GridMenu.AppendMenuItem(MZ_IDC_GRIDMENU_SAVE, L"保存", ldImg(L"8_1"), ldImg(L"8_2"));
	m_GridMenu.AppendMenuItem(MZ_IDC_GRIDMENU_DELETE, L"删除", ldImg(L"2_1"), ldImg(L"2_2"));
	m_GridMenu.AppendMenuItem(MZ_IDC_GRIDMENU_RENAME, L"重命名", ldImg(L"9_1"), ldImg(L"9_2"));

	reloadAll();
	return true;
}

void CLibraryWnd::OnMzCommand( WPARAM wParam, LPARAM lParam )
{
	UINT_PTR id = LOWORD(wParam);
	switch (id)
	{
	case MZ_IDC_TESTTOOLBAR:
		{
			int index = lParam;

			if (index == TOOLBARPRO_MIDDLE_TEXTBUTTON && !m_GridMenu.IsContinue())
			{
				int nSelect = m_list.getSelectedItemCount();
				m_GridMenu.EnableMenuItem(MZ_IDC_GRIDMENU_EXPORT, nSelect > 0);
				m_GridMenu.EnableMenuItem(MZ_IDC_GRIDMENU_DELETE, nSelect > 0);
				m_GridMenu.EnableMenuItem(MZ_IDC_GRIDMENU_RENAME, nSelect == 1);
				m_GridMenu.EnableMenuItem(MZ_IDC_GRIDMENU_SAVE, !m_glWnd->getImageFileName().IsEmpty());
				
				m_GridMenu.TrackGridMenuDialog(m_hWnd, MZM_HEIGHT_TOOLBARPRO);
			}
			else
			{
				m_GridMenu.EndGridMenu();
			}

			if (index == TOOLBARPRO_LEFT_TEXTBUTTON)
			{
				EndModal(ID_CANCEL);
				return;
				//返回
			}
			else if (index == TOOLBARPRO_RIGHT_TEXTBUTTON)
			{

				CMzString tip = g_bDemoVersion ? L"当前版本试用版,不支持导入,不能自定义图片!\n更多功能请购买正式版授权,谢谢支持!" : L"当前版本为正式版，谢谢您的支持！";
				CMzString msg = L"摇摇乐 ";
				msg += APP_VERSION;
				msg += L"\nBy tNextDay";
				MzMessageBoxWithTip(
					m_hWnd, 
					msg,
					L"关于摇摇乐",
					tip.C_Str());
				return;
			}

			break;
		}
		// 处理 GridMenu 的命令消息
	case MZ_IDC_GRIDMENU_IMPORT:
		{
			if (g_bDemoVersion)
			{
				MzMessageBoxV2(m_hWnd, L"当前版本为试用版,不能导入预设!\n更多功能请购买正式版授权!\n谢谢您的支持!", MZV2_MB_OK);
				return;
			}
			int nImport = importPresets();
			if (nImport > 0)
			{
				CMzString str(20);
				swprintf(str.C_Str(), L"成功导入 %d 个预设!", nImport);
				MzMessageBoxV2(m_hWnd, str.C_Str(), MZV2_MB_OK);
			}
			break;
		}
	case MZ_IDC_GRIDMENU_EXPORT:
		{
			int nExport = exportPresets();
			if (nExport > 0)
			{
				CMzString str(20);
				swprintf(str.C_Str(), L"成功导出 %d 个预设!", nExport);
				MzMessageBoxV2(m_hWnd, str.C_Str(), MZV2_MB_OK);
			}
			break;
		}
	case MZ_IDC_GRIDMENU_SELECT:
		{
			if (m_list.getSelectedItemCount() < m_list.GetItemCount())
				m_list.selectAll();
			else
				m_list.unselectAll();
			m_list.Invalidate();
			break;
		}
	case MZ_IDC_GRIDMENU_SAVE:
		{
			doSave();
			//MzMessageBoxV2(m_hWnd, L"You pressed the Search button!", MZV2_MB_OK);
			break;
		}
	case MZ_IDC_GRIDMENU_DELETE:
		{
			if (g_bDemoVersion)
			{
				MzMessageBoxV2(m_hWnd, L"当前版本为试用版,不能自定义图片!\n删掉了还玩啥呢?!", MZV2_MB_OK);
				return;
			}
			doDelete();
			break;
		}
	case MZ_IDC_GRIDMENU_RENAME:
		{
			doRename();
			break;
		}
	}

}

void CLibraryWnd::OnLButtonDown( UINT fwKeys, int xPos, int yPos )
{
	if (yPos < (GetHeight() - MZM_HEIGHT_TOOLBARPRO))
	{
		if (m_GridMenu.IsContinue())
		{
			m_GridMenu.EndGridMenu();
		}
	}
}


int CLibraryWnd::doShow( CglWnd* glWnd )
{
	if (glWnd == NULL) return ID_CANCEL;
	m_glWnd = glWnd;
	SetAnimateType_Show(MZ_ANIMTYPE_SCROLL_BOTTOM_TO_TOP_2);
	// 设置窗口切换动画（结束时的动画）
	SetAnimateType_Hide(MZ_ANIMTYPE_SCROLL_TOP_TO_BOTTOM_1 );
	m_list.unselectAll();
	if (m_crrtPreset >= 0)
	{
		m_list.setItemSelect(m_crrtPreset, true);
		m_list.MoveTopPos(m_crrtPreset);
	}
	m_list.Invalidate();
	return DoModal();
}

bool CLibraryWnd::saveAll()
{
	CMzString strFilePath(_MAX_PATH);
	char mark[4] = {'S', 'P', 'K', 0};
	int nCount = m_list.GetItemCount ();
	bool ret = false;
	nCount = nCount > 100 ? 100 : nCount;
	GetWorkPath(strFilePath.C_Str(), strFilePath.GetBufferSize());
	strFilePath += L"\\presets\\save.sets";
	if (nCount <= 0)
	{
		DeleteFile(strFilePath.C_Str());
		return true;
	}
	FILE* fp = _wfopen(strFilePath.C_Str(),L"wb");
	if (!fp) return false;
	mark[3] = (byte)nCount;
	if(fwrite(mark, sizeof(mark), 1, fp) <= 0) goto _finish;
	int nWrite = 0;
	for (int i = 0; i < nCount; i++)
	{
		Preset* ps = m_list.getPreset(i);
		if (!ps || fwrite(ps, sizeof(Preset), 1, fp) <= 0)
		{
			break;
		}
		nWrite++ ;
	}
	ret = nWrite > 0;
_finish:
	fclose(fp);
	return ret;
}

bool CLibraryWnd::savePreset( int nIndex )
{
	CMzString strFilePath(_MAX_PATH);
	char mark[4] = {0};
	int nCount = 0;
	bool ret = false;
	Preset* ps = m_list.getPreset(nIndex);
	if (!ps) return false; 
	GetWorkPath(strFilePath.C_Str(), strFilePath.GetBufferSize());
	strFilePath += L"\\presets\\save.sets";
	FILE* fp = _wfopen(strFilePath.C_Str(),L"r+b");
	if (!fp) return false;
	fread(mark, sizeof(mark), 1, fp);
	if (mark[0] != 'S' || mark[1] != 'P' || mark[2] != 'K') goto _finish;
	nCount = mark[3];
	if (nCount == 0 || nCount > m_nMaxPresets || nIndex < 0 || nIndex >= nCount) goto _finish;
	if (fseek(fp, sizeof(mark) + nIndex*sizeof(Preset), SEEK_SET) != 0) goto _finish;
	ret = fwrite(ps, sizeof(Preset), 1, fp) > 0;
_finish:
	fclose(fp);
	return ret;
}

bool CLibraryWnd::reloadAll()
{
	CMzString strPresetsDir(MAX_PATH);
	CMzString strFilePath(_MAX_PATH);
	char mark[4] = {0};
	int nCount = 0;
	bool ret = false;
	GetWorkPath(strPresetsDir.C_Str(), strPresetsDir.GetBufferSize());
	strPresetsDir += PRESET_PATH;
	strFilePath = strPresetsDir + L"save.sets";
	FILE* fp = _wfopen(strFilePath.C_Str(),L"rb");
	if (!fp) return false;
	fread(mark, sizeof(mark), 1, fp);
	if (mark[0] != 'S' || mark[1] != 'P' || mark[2] != 'K') goto _finish;
	nCount = mark[3];
	if (nCount == 0) goto _finish;
	if (nCount > m_nMaxPresets) nCount = m_nMaxPresets;
	m_list.RemoveAll();
	int nWrite = 0;
	for (int i = 0; i < nCount; i++)
	{
		Preset ps;
		if (fread(&ps, sizeof(ps), 1, fp) == 1)
		{
			strFilePath = strPresetsDir + ps.name + L".jpg";
			if (::GetFileAttributes(strFilePath.C_Str()) != 0xFFFFFFFF)
			{
				m_list.addItem(ps);
				nWrite++;
			}
		}
	}
	ret = nWrite > 0;
_finish:
	fclose(fp);
	m_list.Invalidate();
	return ret;
}

int CLibraryWnd::importPresets()
{
	MzPopupProgress progress;
	CMzString strPresetsDir(MAX_PATH);
	CMzString strFilePath(MAX_PATH);
	CMzString strProgressbarTitle(30);
	FilePreset* fpresets = NULL;
	char* buf = NULL;
	FILE* sfp = NULL;
	FILE* fp = NULL;
	char mark[4] = {0};
	int nCount = 0;
	int nImprot = 0;
	bool bSelect = false;

	bSelect = selectFile(m_hWnd, strFilePath, L"*.fpk;", L"\\Disk");
	if (!bSelect) return nImprot;
	fp = _wfopen(strFilePath.C_Str(), L"rb");
	if (!fp) return nImprot;

	// 初始化 MzPopupProgress 控件
	progress.SetRange(0, 1);
	progress.SetCurrentValue(0);
	progress.SetTitleText(L"正在导入...");
	progress.StartProgress(m_hWnd,FALSE);

	fread(mark, sizeof(mark), 1, fp);
	if (mark[0] != 'F' || mark[1] != 'P' || mark[2] != 'K') goto _finish;
	nCount = mark[3];
	if (nCount <= 0) goto _finish;
	nCount = nCount > MAX_EXPORT_PRESETS ? MAX_EXPORT_PRESETS : nCount;

	progress.SetRange(0, nCount);

	fpresets = new FilePreset[nCount];
	if (fread(fpresets, sizeof(FilePreset), nCount, fp) != nCount ) goto _finish;
	
	GetWorkPath(strFilePath.C_Str(), strFilePath.GetBufferSize());
	strPresetsDir = strFilePath + PRESET_PATH;
	for (int i = 0; i < nCount; i++)
	{
		if (fpresets[i].imageSize <= 0) continue;

		progress.SetCurrentValue(i);
		strProgressbarTitle = L"正在导入: ";
		strProgressbarTitle += fpresets[i].preset.name;
		progress.SetTitleText(strProgressbarTitle.C_Str());
		progress.UpdateProgress(FALSE);

		buf = new char[fpresets[i].imageSize];
		if (fread(buf, fpresets[i].imageSize , 1, fp) > 0)
		{
			strFilePath = strPresetsDir + fpresets[i].preset.name + L".jpg"; 
			sfp = _wfopen(strFilePath.C_Str(), L"wb");
			if (!sfp)
			{
				continue;
			}
			if (fwrite(buf, fpresets[i].imageSize, 1, sfp) <= 0)
			{
				fclose(sfp);
				continue;
			}
			fclose(sfp);
			Preset* ps = getPresetByName(fpresets[i].preset.name);
			if (ps)
				*ps = fpresets[i].preset;
			else
				m_list.addItem(fpresets[i].preset);
			nImprot++;
		}
		delete [] buf;
		
	}
	progress.SetCurrentValue(nCount);
	strProgressbarTitle = L"一点点收尾工作...";
	progress.SetTitleText(strProgressbarTitle.C_Str());
	progress.UpdateProgress(FALSE);
	if (nImprot > 0 )
	{
		saveAll();
	}
	m_list.MoveTopPos(m_list.GetItemCount() - nImprot );
_finish:
	fclose(fp);
	if (fpresets) delete [] fpresets;
	progress.KillProgress();
	return nImprot;
}

int CLibraryWnd::exportPresets()
{
	MzPopupProgress progress;

	CMzString strPresetsDir(MAX_PATH);
	CMzString strFilePath(MAX_PATH);
	CMzString strExpFilePath(MAX_PATH);
	CMzString strProgressbarTitle(30);
	FilePreset* fpresets = NULL;
	FilePreset* fpreset = NULL;
	char* buf = NULL;
	FILE* fImg = NULL;
	FILE* fp = NULL;
	char mark[4] = {'F', 'P', 'K', 0};
	int nCount;
	int nExprot = 0;
	nCount = m_list.getSelectedItemCount();
	if (nCount <= 0) return nExprot;
	nCount = nCount > MAX_EXPORT_PRESETS ? MAX_EXPORT_PRESETS : nCount;
	mark[3] = (byte) nCount;
	strFilePath = L"我喜欢的摇摇乐";
	int ret = MyInputMessageBox( m_hWnd, L"请输入导出文件名", strFilePath, L"导出文件会保存在\\Disk目录下,扩展名为fpk");
	
	if (ret != ID_OK) return 0;
	swprintf(strExpFilePath.C_Str(), L"\\Disk\\%s.fpk", strFilePath.C_Str());

	if (strExpFilePath.IsEmpty()) return nExprot;
	fp = _wfopen(strExpFilePath.C_Str(), L"wb");
	if (!fp) return nExprot;

	// 初始化 MzPopupProgress 控件
	progress.SetRange(0, nCount);
	progress.SetCurrentValue(0);
	progress.SetTitleText(L"正在导出...");
	progress.StartProgress(m_hWnd, FALSE);

	if(fwrite(mark, sizeof(mark), 1, fp) <= 0) goto _finish;
	fpresets = new FilePreset[nCount];
	memset(fpresets, 0, sizeof(FilePreset)*nCount);
	fpreset = fpresets;
	if(fwrite(fpresets, sizeof(FilePreset), nCount, fp) < nCount) goto _finish;
	GetWorkPath(strPresetsDir.C_Str(), strPresetsDir.GetBufferSize());
	strPresetsDir += PRESET_PATH;
	int nIndexSel = 0;
	for (int i = 0; i < nCount; i++)
	{
		while (!m_list.isItemSelect(nIndexSel) && nIndexSel < m_list.GetItemCount())
		nIndexSel++;

		Preset* ps = m_list.getPreset(nIndexSel);
		if (!ps) continue;
		m_list.setItemSelect(nIndexSel, false);
		progress.SetCurrentValue(i);
		strProgressbarTitle = L"正在导出 : ";
		strProgressbarTitle += ps->name;
		progress.SetTitleText(strProgressbarTitle.C_Str());
		progress.UpdateProgress(FALSE);
		
		fpreset->preset = *ps;
		strFilePath = strPresetsDir + ps->name + L".jpg";
		fImg = _wfopen(strFilePath.C_Str(), L"rb");
		if (!fImg) continue;
		fseek(fImg, 0L, SEEK_END);
		fpreset->imageSize = ftell(fImg);
		if (fpreset->imageSize <= 0 )
		{
			fclose(fImg);
			continue;
		}
		fseek(fImg, 0, SEEK_SET);
		buf = new char[fpreset->imageSize];
		if (fread(buf, fpreset->imageSize, 1, fImg) == 1)
		{
			if (fwrite(buf, fpreset->imageSize, 1, fp) == 1)
			{
				nExprot++;
				fpreset++;
			}
		}
		fclose(fImg);
		delete [] buf;
	}
	progress.SetCurrentValue(nCount);
	strProgressbarTitle = L"一点点收尾工作...";
	progress.SetTitleText(strProgressbarTitle.C_Str());
	progress.UpdateProgress(FALSE);
	if (nExprot > 0)
	{
		if (fseek(fp, sizeof(mark), SEEK_SET) != 0 
			|| fwrite(fpresets, sizeof(FilePreset), nExprot, fp) != nExprot)
		{
			nExprot = 0;
		}
	}

_finish:
	fclose(fp);
	if (fpresets)
		delete [] fpresets;
	if (nExprot <= 0)
		DeleteFile(strExpFilePath.C_Str());
	progress.KillProgress();
	return nExprot;
}

LRESULT CLibraryWnd::MzDefWndProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	if (message == WM_LISTITEM_CLICK)
	{
		if (wParam == MZ_IDC_LIST)
		{
			showWaiting(m_hWnd);
			doLoadPreset(lParam);
			hideWaiting();
			EndModal(ID_OK);
			return 0;
		}
	}
	if (message == MZ_WM_WND_ACTIVATE)
	{
		if (wParam != WA_INACTIVE)
			m_GridMenu.EndGridMenu();
	}
	if (message == GetShellNotifyMsg_EntryLockPhone())
	{
		m_GridMenu.EndGridMenu();
	}

	return CMzWndEx::MzDefWndProc(message,wParam,lParam);
}

void CLibraryWnd::doSave()
{
	CMzString strName;
	CMzString strPresetsDir(MAX_PATH);
	CMzString strFilePath(MAX_PATH);
	Preset* ps = NULL;
	if (m_glWnd->getImageFileName().IsEmpty()) return;

	int firstSelect = m_list.getFirstSelect();

	if (firstSelect >= 0)
	{
		ps = m_list.getPreset(firstSelect);
		if (!ps) firstSelect = -1;
		strName = ps->name;
	}
	if (strName.IsEmpty())
	{
		bool bExistName = false;
		__RegetName:
		if (strName.IsEmpty() || bExistName)
		{
			strName = getDefaultPresetName(strName.C_Str());
		}
		bExistName = false;
		int ret = MyInputMessageBox( m_hWnd, L"请输入名称", strName);
		Invalidate();
		if (ret != ID_OK || strName.IsEmpty()) return;
		Preset* ps2 = getPresetByName(strName.C_Str());
		if (ps2)
		{
			bExistName = true;
			int msgRet = MzMessageBoxV2(m_hWnd, L"预设名已经存在,要覆盖吗?", MZV2_MB_YESNOCANCEL);
			if (msgRet == 1) ps = ps2; //第一个按钮 YES
			else if (msgRet == 3) return;	
			else goto __RegetName;
		}
	}
	showWaiting(m_hWnd);
	GetWorkPath(strPresetsDir.C_Str(), strPresetsDir.GetBufferSize());
	strPresetsDir += PRESET_PATH;
	strFilePath = strPresetsDir + strName + L".jpg";
	if (!(strFilePath == m_glWnd->getImageFileName()))
	{
		if (!CopyFile(m_glWnd->getImageFileName().C_Str(), strFilePath.C_Str(), false))
		{
			hideWaiting();
			//wprintf(L"Copy %s to %s failed\n", m_glWnd->getImageFileName().C_Str(), strFilePath.C_Str());
			MzMessageBoxV2(m_hWnd, L"保存失败了!");
			return;
		}
	}
	if (!ps) ps = new Preset;
	memset(ps, 0, sizeof(Preset));
	swprintf(ps->name, L"%s", strName.C_Str());

	BubbleSet* bs = ps->sets;
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		if (m_glWnd->getBubbleSet(i, bs->x, bs->y, bs->r))
			bs++;
	}
	if (firstSelect >= 0)
	{
		savePreset(firstSelect);
	}
	else
	{
		m_list.insertItem(*ps, 0);
		m_list.Invalidate();
		saveAll();
		delete ps;
	}
	hideWaiting();
}

void CLibraryWnd::doRename()
{
	CMzString strName;
	CMzString strFilePathOld(MAX_PATH);
	CMzString strFilePathNew(MAX_PATH);
	int firstSelect = m_list.getFirstSelect();
	if (firstSelect < 0) return;
	Preset* ps = m_list.getPreset(firstSelect);
	if (!ps) return;
	strName = ps->name;

	bool bExistName = false;
	__RegetName:

	if (strName.IsEmpty() || bExistName)
	{
		strName = getDefaultPresetName(strName.C_Str());
	}
	bExistName = false;
	int ret = MyInputMessageBox( m_hWnd, L"请输入新名称", strName);
	m_ToolBarPro.Invalidate();
	if (ret != ID_OK || strName.IsEmpty() || (strName.Compare((wchar_t*)ps->name) == 0)) return;
	if (getPresetByName(strName.C_Str()))
	{
		bExistName = true;
		int msgRet = MzMessageBoxV2(m_hWnd, L"预设名已经存在,请重新输入!", MZV2_MB_OKCANCEL);
		if (msgRet == 1) 
			goto __RegetName;
		else 
			return;
	}
	showWaiting(m_hWnd);
	GetWorkPath(strFilePathNew.C_Str(), strFilePathNew.GetBufferSize());
	strFilePathNew += PRESET_PATH;
	strFilePathOld = strFilePathNew + ps->name + L".jpg";
	strFilePathNew += (strName + L".jpg");
	if (!MoveFile(strFilePathOld.C_Str(), strFilePathNew.C_Str()))
	{
		hideWaiting();
		return;
	}
	swprintf(ps->name, L"%s", strName.C_Str());
	savePreset(firstSelect);
	hideWaiting();
	m_list.Invalidate();
}

void CLibraryWnd::doDelete()
{
	MzPopupProgress progress;
	CMzString strProgressbarTitle(30);
	CMzString strPresetsDir(MAX_PATH);
	CMzString strFilePath(MAX_PATH);
	int nDeleted = 0;
	int firstSelected = 0;
	progress.SetRange(0, m_list.getSelectedItemCount());
	progress.SetCurrentValue(0);
	progress.SetTitleText(L"正在删除...");
	progress.StartProgress(m_hWnd, FALSE);
	GetWorkPath(strPresetsDir.C_Str(), strPresetsDir.GetBufferSize());
	strPresetsDir += PRESET_PATH;

	firstSelected = m_list.getFirstSelect();
	while ( firstSelected > -1)
	{
		Preset* ps = m_list.getPreset(firstSelected);
		progress.SetCurrentValue(nDeleted++);
		if (ps) 
		{
			strProgressbarTitle = L"正在删除 : ";
			strProgressbarTitle += ps->name;
			progress.SetTitleText(strProgressbarTitle.C_Str());
			progress.UpdateProgress(FALSE);
			strFilePath = strPresetsDir + ps->name + L".jpg";
			DeleteFile(strFilePath.C_Str());
			m_list.RemoveItem(firstSelected);
		}
		firstSelected = m_list.getFirstSelect();
	}
	m_list.Invalidate();
	progress.SetCurrentValue(1);
	progress.SetRange(0, 1);
	strProgressbarTitle = L"一点点收尾工作...";
	progress.SetTitleText(strProgressbarTitle.C_Str());
	progress.UpdateProgress(FALSE);
	saveAll();
	progress.KillProgress();
}

bool CLibraryWnd::doLoadPreset( int nIndex, CglWnd* glWnd /*= NULL*/ )
{
	CMzString strFilePath(MAX_PATH);
	Preset* ps = m_list.getPreset(nIndex);
	if (!ps) return false;
	GetWorkPath(strFilePath.C_Str(), strFilePath.GetBufferSize());
	strFilePath += PRESET_PATH;
	strFilePath += ps->name;
	strFilePath += L".jpg";
	if (glWnd)
	{
		m_glWnd = glWnd;
	}
	if (!m_glWnd->setImage(strFilePath)) return false;
	m_glWnd->removeAllBubbles();
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		if (ps->sets[i].r >= MIN_BUBBLE_R && ps->sets[i].r <= MAX_BUBBLE_R)
			m_glWnd->addEditBubble(ps->sets[i].x, ps->sets[i].y, ps->sets[i].r);
	}
	m_crrtPreset = nIndex;
	g_config->SetValue(L"Last", L"Index", m_crrtPreset);
	return true;
}



CMzString CLibraryWnd::getDefaultPresetName( wchar_t* defaultPrefix )
{
	CMzString strName(20);
	CMzString strFormat;
	strFormat = (defaultPrefix && wcslen(defaultPrefix)) ? defaultPrefix : L"图片";
	if(strFormat[strFormat.Length () - 3] == L'_')
		strFormat[strFormat.Length () - 3] = L'\0';
	strFormat += L"_%02d";
	for (int i = 0; i < m_nMaxPresets; i++)
	{
		swprintf(strName.C_Str(), strFormat.C_Str(), i);
		if (!getPresetByName(strName.C_Str()))
			return strName;
	}
	return CMzString();
}

Preset* CLibraryWnd::getPresetByName( wchar_t* strName )
{
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		Preset* ps = m_list.getPreset(i);
		if (ps && wcscmp(strName, ps->name) == 0)
			return ps;
	}
	return NULL;
}

bool CLibraryWnd::saveCrrtPreset()
{
	if (m_crrtPreset < 0) return false;
	Preset* ps = NULL;
	ps = m_list.getPreset(m_crrtPreset);
	if (!ps) return false;
	BubbleSet* bs = ps->sets;
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		if (m_glWnd->getBubbleSet(i, bs->x, bs->y, bs->r))
			bs++;
	}
	return savePreset(m_crrtPreset);
}

void CLibraryWnd::setUseUserPic()
{
	m_crrtPreset = -1;
	m_list.unselectAll();
	g_config->SetValue(L"Last", L"Index", m_crrtPreset);
}

void showWaiting(HWND hWnd)
{
	RECT rect;
	rect.left = 218;
	rect.top = 338;
	rect.bottom = 382;
	rect.right = 262;
	MzBeginWaitDlg (hWnd, &rect, true);
}

void hideWaiting()
{
	MzEndWaitDlg();
}

bool selectFile(HWND hWnd,  CMzString &StrFile, TCHAR *filter, TCHAR*defDir /*= L""*/ )
{
	bool bSelect = false;
	IMzSelect *pSelect = NULL; 
	IFileBrowser *pFile = NULL;        

	CoInitializeEx(NULL, COINIT_MULTITHREADED );
	if ( SUCCEEDED( CoCreateInstance( CLSID_FileBrowser, NULL,CLSCTX_INPROC_SERVER ,IID_MZ_FileBrowser,(void **)&pFile ) ) )
	{     
		if( SUCCEEDED( pFile->QueryInterface( IID_MZ_Select, (void**)&pSelect ) ) )
		{
			TCHAR file[ MAX_PATH ] = { 0 };
			pFile->SetParentWnd( hWnd );
			pFile->SetOpenDirectoryPath(defDir); //如果不调用此函数则默认为根目录
			pFile->SetExtFilter(filter);                                      
			pFile->SetOpenDocumentType(DOCUMENT_SELECT_SINGLE_FILE); //应用根据需求进行文档打开方式的设置
			if( pSelect->Invoke() ) 
			{//各应用根据自己需求获取文档的返回值            
				StrFile = pFile->GetSelectedFileName();
				bSelect = true;
			}
			pSelect->Release();
		}     
		pFile->Release();
	} 
	CoUninitialize(); 
	return bSelect;
}