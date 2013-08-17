#include "MyImageContainer.h"

MyImageContainer::MyImageContainer(void)
{
}

MyImageContainer::~MyImageContainer(void)
{
	RemoveAll();
}

ImagingHelper* MyImageContainer::LoadImage( wchar_t* name, wchar_t* ext /*= NULL*/, wchar_t* path /*= NULL*/ )
{
	CMzString strFilePath(MAX_PATH);
	ImagingHelper* ret = findByName(name);
	if (ret) return ret;
	if (m_defaultPath.IsEmpty() && path == NULL) return NULL;

	swprintf(strFilePath.C_Str(), 
		L"%s\\%s%s",
		path == NULL ? m_defaultPath.C_Str() : path,
		name,
		ext == NULL ? L".png" : ext);
	ret = new ImagingHelper;
	if (ret->LoadImage(strFilePath.C_Str(), true, true))
	{
		MyImage img;
		img.name = name;
		img.img = ret;
		m_images.push_back(img);
		return ret;
	}
	delete ret;
	ret = NULL;
	return ret;
}

void MyImageContainer::setDefaultPath( const CMzString& strPath )
{
	m_defaultPath = strPath;
}

void MyImageContainer::RemoveAll()
{
	list<MyImage>::iterator it; // 声明一个迭代器

	for ( it = m_images.begin() ; it != m_images.end(); it++ )
	{
		(*it).img->Unload();
		delete (*it).img;
	}
	m_images.clear();
}

void MyImageContainer::RemoveImage( wchar_t* name )
{
	list<MyImage>::iterator it; 
	for ( it = m_images.begin() ; it != m_images.end(); it++ )
	{
		MyImage& img = *it;
		if (img.name.Compare(name) == 0)
		{
			img.img->Unload();
			delete img.img;
			m_images.erase(it);
			return;
		}
	}
}

ImagingHelper* MyImageContainer::findByName( wchar_t* name )
{
	list<MyImage>::iterator it; 
	for ( it = m_images.begin() ; it != m_images.end(); it++ )
	{
		MyImage& img = *it;
		if (img.name.Compare(name) == 0)
		{
			return img.img;
		}
	}
	return NULL;
}