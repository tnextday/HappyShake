/*
-----------------------------------------------------------------------------
This source file is part of Dflora (Dancing Flora) OpenGL|ES 1.1 version

Copyright (c) 2004-2005 Changzhi Li < richardzlee@163.com >

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free 
Software Foundation; either version 2 of the License, or (at your option) any 
later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more 
details.

You should have received a copy of the GNU Lesser General Public License along 
with this program; if not, write to the Free Software Foundation, Inc., 59 
Temple Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <gles/gles2/gl2.h>

#define		HAVE_BMP
#define		HAVE_TGA
#define		HAVE_JPEG
#define		HAVE_PNG
#define		HAVE_WINDOWS_DC


#ifdef WIN32
#pragma warning( disable : 4514 4100 4505 )
#endif

#ifdef HAVE_WINDOWS_DC
#include <windows.h>
#endif // HAVE_WINDOWS_DC

#define  SafeDelete(p) {if(p) {delete p; p=NULL;}}			// for shorter code
#define  SafeDeleteArray(p) {if(p) {delete[] p; p=NULL;}}	// for shorter code

/** Wrapper class for image file reading and writing.
	@remarks
	Now support bmp and tga file, used by texture creation.
	Modified from Qualcomm QpenGL|ES examples.
	@note
	Not all kinds of bmp and tga format image files are perfectly supported.
	But it works well for this demo.
	@see
	Texture
*/

class TImage
{
public:
	TImage();

	virtual ~TImage();

	bool	LoadFromFile(const char* filename);
	bool	SaveToFile(const char* filename);
#ifdef HAVE_BMP
	bool	LoadBMP(const char* filename);
#endif // HAVE_BMP
	
#ifdef HAVE_TGA
	bool	LoadTGA(const char* filename);
	bool	SaveTGA(const char* filename);
#endif // HAVE_TGA

#ifdef HAVE_JPEG
	bool	LoadJPEG(const char* filename, bool bFast = false);
	bool	SaveJPEG(const char* filename, int quality = 70);
#endif // HAVE_JPEG_LIB

#ifdef HAVE_PNG
	bool	LoadPNG(const char* filename);
	bool	SavePNG(const char* filename);
#endif // HAVE_PNG_LIB

	// returns the image's width, in pixels
	int   GetWidth() const { return m_width; }
	// returns the image's height in pixels
	int   GetHeight() const { return m_height; }

	// returns the pixel format of the data using OpenGL pixel format types
	GLenum  GetDataFormat() const { return m_dataFormat; }
	// returns the data type used to store color info using the OpenGL data type
	int   GetDataType() const { return m_dataType; }

	// returns a pointer to the image data
	GLubyte*  GetData() const { return m_pData; }

	// swaps the red and blue components of every color
	void  SwapBlueAndRed();
	
	void Bind2DTexture(int tex_name);

	static bool LoadTextureFromFile(const char* filename, int tex_name);
#ifdef HAVE_WINDOWS_DC
	bool LoadFromDC(HDC srcDC, int top, int left, int width, int height);
	static bool LoadTextureFromDC(HDC srcDC, int top, int left, int width, int height, int tex_name);
#endif // HAVE_WINDOWS_DC

private:
	bool  Flip();

	GLubyte*  m_pData;

	GLint   m_width;
	GLint	m_height;
	GLint	m_colorDepth;
	GLenum  m_dataFormat;
	GLint   m_dataType;
	GLuint  m_imageSize;

	GLuint  m_redMask;
	GLuint  m_greenMask;
	GLuint  m_blueMask;

private:
	void  FreeData();
};

#endif 
