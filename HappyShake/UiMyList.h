/************************************************************************/
/*
* Copyright (C) Meizu Technology Corporation Zhuhai China
* All rights reserved.
* 中国珠海, 魅族科技有限公司, 版权所有.
*
* This file is a part of the Meizu Foundation Classes library.
* Author:    Lynn
* Create on: 2009-8-12
*/
/************************************************************************/

//! 此代码演示了：
//! 以短信列表样式为例,演示如何使用UiList派生类实现复杂的列表控件
//! 使用ImagingHelper,实现图像的加载与绘制

#include <Windows.h>
#include <mzfc\MzString.h>
#include <mzfc_inc.h>

#define WM_LISTITEM_CLICK	WM_USER + 10

struct BubbleSet {
	int		x;
	int		y;
	int		r;
};

struct Preset {
	wchar_t		name[20];		//最多20个字
	BubbleSet	sets[5];
};

class MyListItem
{
public:
	MyListItem():isSelect(false){};
    Preset preset;       
    bool isSelect;
};

class UiMyList : 
    public UiList
{
public:
    UiMyList();
    ~UiMyList();
    
	bool isItemSelect(int nIndex);
	void setItemSelect(int nIndex, bool bSelect);
	void selectAll();
	void unselectAll();
	void inverseSelect();
	int getSelectedItemCount();
	int getFirstSelect();
	void addItem(Preset &preset);
	void insertItem (Preset &preset, int nPos=-1);
	MyListItem * getMyItem (int index);
	Preset* getPreset(int nIndex);

	void setMessageReceiveHandle(HWND hWnd){m_hWndMsgRev = hWnd;};
	HWND messageReceiveHandle() {return m_hWndMsgRev;};

private:
	HWND	m_hWndMsgRev;
    ImageContainer m_imgContainer;

protected:
	virtual int OnTimer(UINT_PTR nIDEvent);
	virtual void OnRemoveItem(int nIndex);

	virtual int OnLButtonDown(UINT fwKeys, int xPos, int yPos);
	virtual int OnMouseMove (UINT fwKeys, int xPos, int yPos);
	virtual int OnLButtonUp(UINT fwKeys, int xPos, int yPos);

	virtual void DrawItem(HDC hdcDst, int nIndex, RECT* prcItem, RECT *prcWin, RECT *prcUpdate);
};