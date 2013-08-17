#pragma once
#include <mzfc_inc.h>
#include <list>
using namespace std;


struct MyImage {
	CMzString		name;
	ImagingHelper*	img;
};

class MyImageContainer
{
public:
	MyImageContainer(void);
	~MyImageContainer(void);

	ImagingHelper*  LoadImage(wchar_t* name, wchar_t* ext = NULL, wchar_t* path = NULL);
	void setDefaultPath(const CMzString& strPath);
	void  RemoveAll ();
	void  RemoveImage (wchar_t* name);
	ImagingHelper* findByName(wchar_t* name);

protected:
	list<MyImage>	m_images;
	CMzString		m_defaultPath;
};
