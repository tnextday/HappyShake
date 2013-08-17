#include "UiMyList.h"

#define RECT_ARROW_ICON_LEFT			430
#define RECT_ARROW_ICON_WIDTH			50 


#define RECT_CONTENT_LEFT				20
#define RECT_CONTENT_WIDTH				450
#define RECT_CONTENT_TOP				26 
#define RECT_CONTENT_HEIGHT				57 



UiMyList::UiMyList()
{
	m_hWndMsgRev = NULL;
	SetItemHeight(80);
}

UiMyList::~UiMyList()
{

}

void UiMyList::DrawItem(HDC hdcDst, int nIndex, RECT* prcItem, RECT *prcWin, RECT *prcUpdate)
{
    MyListItem* pItem = getMyItem(nIndex);
    if (!pItem)
    {
        return;
    }
	if (nIndex == GetSelectedIndex())
		MzDrawSelectedBg(hdcDst,prcItem);

    //计算中间文字区域大小  
    RECT rcContent = {RECT_CONTENT_LEFT, RECT_CONTENT_TOP, RECT_CONTENT_LEFT + RECT_CONTENT_WIDTH, RECT_CONTENT_TOP + RECT_CONTENT_HEIGHT};

    ::OffsetRect(&rcContent,prcItem->left,prcItem->top);

    //! 绘制内容文本
    HFONT hFont = FontHelper::GetFont(GetTextSize());
    HGDIOBJ hOld = ::SelectObject(hdcDst,hFont);
	if (pItem->isSelect)
		::SetTextColor(hdcDst, RGB(0, 130, 214));
	else
		::SetTextColor(hdcDst, GetTextColor ());
    MzDrawText(hdcDst, pItem->preset.name, &rcContent, DT_LEFT|DT_TOP|DT_WORDBREAK);

    //! 绘制选择框
    RECT rcSelect = *prcItem;
    rcSelect.left += RECT_ARROW_ICON_LEFT;
    rcSelect.right = rcSelect.left + RECT_ARROW_ICON_WIDTH;
	ImagingHelper *pImg = m_imgContainer.LoadImage(GetMzResV2ModuleHandle(), pItem->isSelect? MZRESV2_IDR_PNG_CHECKBOX_SELECTED : MZRESV2_IDR_PNG_CHECKBOX_UNSELECTED, true);
    pImg->Draw(hdcDst,&rcSelect);
    ::SelectObject(hdcDst,hOld);
}


void UiMyList::OnRemoveItem(int nIndex)
{
    int itemHeight = CalcItemHeight(nIndex);
    m_nDeleteItemTopPos = itemHeight;
    m_nDeleteItem = nIndex;
    SetTimer(GetManager()->GetParentWnd(),MZ_TIMER_ID_DELETE_ANIMATION+GetID(),50,NULL);

    MyListItem* pItem = getMyItem(nIndex);
    if (pItem)
		delete pItem; 
}

int UiMyList::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == MZ_TIMER_ID_DELETE_ANIMATION + GetID())
    {
        if (m_nDeleteItemTopPos != 0)
        {
            /*m_deleteTopPos -= (m_deleteTopPos>>2);*/
            m_nDeleteItemTopPos = (m_nDeleteItemTopPos*3)>>2;
            if (m_nDeleteItemTopPos < 2)
            {
                m_nDeleteItemTopPos = 0;
                m_nDeleteItem = -1;
                KillTimer(GetManager()->GetParentWnd(),MZ_TIMER_ID_DELETE_ANIMATION+GetID());
            }
            Invalidate();
            Update();
        }
    }
    return UiList::OnTimer(nIDEvent);
}

int UiMyList::OnLButtonDown( UINT fwKeys, int xPos, int yPos )
{
	if (!IsMouseDownAtScrolling() && xPos < RECT_ARROW_ICON_LEFT)
	{
		int nIndex = CalcIndexOfPos(xPos, yPos);
		SetSelectedIndex(nIndex);
	}
	return UiList::OnLButtonDown(fwKeys, xPos, yPos);
}

int UiMyList::OnMouseMove( UINT fwKeys, int xPos, int yPos )
{
	SetSelectedIndex(-1);
	return UiList::OnMouseMove(fwKeys, xPos, yPos );
}


int UiMyList::OnLButtonUp(UINT fwKeys, int xPos, int yPos)
{
	// 获得控件在鼠标放开前的一些鼠标的相关状态（在调用 UiLst::OnLButtonUp()）
	bool b1 = IsMouseDownAtScrolling();
	bool b2 = IsMouseMoved();

	int Ret = UiList::OnLButtonUp(fwKeys, xPos, yPos);

	int nIndex = CalcIndexOfPos(xPos, yPos);
	if((!b1) && (!b2) && xPos >= RECT_ARROW_ICON_LEFT)
	{
		// 计算鼠标所在位置的项的索引
		MyListItem* pItem = getMyItem(nIndex);
		if (pItem)
		{
			bool bSelect = pItem->isSelect;
			pItem->isSelect = !bSelect;

			InvalidateItem(nIndex);
			Update();
		}
	}
	if (!b1 && !b2 && nIndex == GetSelectedIndex() && GetItemCount())
	{
		if (m_hWndMsgRev == NULL) m_hWndMsgRev = GetParentWnd();
		::PostMessage(m_hWndMsgRev, WM_LISTITEM_CLICK, GetID(), nIndex);
	}
	return Ret;
}

bool UiMyList::isItemSelect( int nIndex )
{
	MyListItem* pItem = getMyItem(nIndex);
	if (pItem)
	{
		return pItem->isSelect;
	}
	return false;
}

void UiMyList::setItemSelect( int nIndex, bool bSelect )
{
	MyListItem* pItem = getMyItem(nIndex);
	if (pItem)
	{
		pItem->isSelect = bSelect;
	}
}

int UiMyList::getSelectedItemCount()
{
	int nSelected = 0;
	for (int i = 0; i < GetItemCount(); i++)
	{
		MyListItem* pItem = getMyItem(i);
		if (pItem && pItem->isSelect)
			nSelected++;
	}
	return nSelected;
}


MyListItem * UiMyList::getMyItem( int nIndex )
{
	ListItem* pItem = GetItem(nIndex);
	if (pItem)
	{
		return reinterpret_cast<MyListItem*>(pItem->Data);
	}
	return NULL;
}

Preset* UiMyList::getPreset( int nIndex )
{
	MyListItem * pItem = getMyItem(nIndex);
	if (pItem)
	{
		return &pItem->preset;
	}
	return NULL;
}

void UiMyList::selectAll()
{
	SetSelectedIndex(-1);
	for (int i = 0; i < GetItemCount(); i++)
	{
		setItemSelect(i, true);
	}
}

void UiMyList::inverseSelect()
{
	SetSelectedIndex(-1);
	for (int i = 0; i < GetItemCount(); i++)
	{
		MyListItem* pItem = getMyItem(i);
		if (pItem)
		{
			pItem->isSelect = !pItem->isSelect;
		}
	}
}

void UiMyList::unselectAll()
{
	SetSelectedIndex(-1);
	for (int i = 0; i < GetItemCount(); i++)
	{
		setItemSelect(i, false);
	}
}

int UiMyList::getFirstSelect()
{
	for (int i = 0; i < GetItemCount(); i++)
	{
		if (isItemSelect(i))
		{
			return i;
		}
	}
	return -1;
}

void UiMyList::insertItem( Preset &preset, int nPos/*=-1*/ )
{
	ListItem li;
	MyListItem *pData = new MyListItem;
	pData->preset = preset;
	li.Data = pData;
	InsertItem(li, nPos);
}

void UiMyList::addItem(Preset &preset)
{
	insertItem(preset, -1);
}
