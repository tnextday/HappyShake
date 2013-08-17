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

#include "stdio.h"
#include "stdlib.h"
#include "image.h"

#ifdef HAVE_JPEG
#include <jpeg/jpeglib.h>
#pragma comment(lib, "JpegLib.lib")
#endif // HAVE_JPEG_LIB

#ifdef HAVE_PNG
#include "lodepng.h"
#endif // HAVE_PNG


#pragma warning(disable : 4996)

struct rgb_t
{
	GLubyte r;
	GLubyte g;
	GLubyte b;
};


// 32-bit color type
struct rgba_t
{
	GLubyte r;
	GLubyte g;
	GLubyte b;
	GLubyte a;
};


// packed pixel formats
struct rgb565_t
{
	GLushort
		b : 5,
		g : 6,
		r : 5;
};

// 16-bit color with alpha type
struct rgba5551_t
{
	GLushort 
		a : 1,
		b : 5,
		g : 5,
		r : 5;
};

// 16-bit color with alpha type
struct rgba4444_t
{
	GLushort
		a : 4,
		b : 4,
		g : 4,
		r : 4;
};

struct la_t
{
	GLubyte l;
	GLubyte a;
};


#ifdef HAVE_BMP
// magic number identifies a bmp file; actually chars 'B''M'
// allowed type magics are 'BM'  for Windows;  
// OS2 allows:
//'BA' - Bitmap Array
//'CI' - Color Icon
//'CP' - Color Pointer (mouse cursor)
//'IC' - Icon
//'PT' - Pointer (mouse cursor)

enum BMPMagic 
{
	MB = 19778
}; 


enum BMPTypes
{
	BMP_NODATA		= 0,
	BMP_BLACKWHIET	= 1,
	BMP_GRAYSCALE	= 2,
	BMP_IA			= 3, 
	BMP_RGB			= 4, 
	BMP_RGBA		= 5,
};

// BMP format bits - at start of file is 512 bytes of pure garbage

struct bmpheader_t		// 12 byte
{
    short fileType;				// always MB
    unsigned short size[2];		// a dword for whole file size - make unsigned Feb 2002
    short reserved1;			// reserved for future purposes
	short reserved2;			// reserved for future purposes
    unsigned short offset[2];	// offset to image in bytes
//	long  offset;
};

struct bmpinfo_t 
{
    long width;			// width of the image in pixels
    long height;		// height of the image in pixels
    short planes;       // word: number of planes (always 1)
    short colorbits;    // word: number of bits used to describe color in each pixel
    long compression;	// compression used
    long imageSize;		// image size in bytes
    long XpixPerMeter;	// pixels per meter in X
    long YpixPerMeter;	// pixels per meter in Y
    long colorUsed;		// number of colors used
    long important;		// number of "important" colors
};	// 36 byte
#endif // HAVE_BMP

#ifdef HAVE_TGA
struct tgaheader_t
{
	GLubyte   idLength;
	GLubyte   colorMapType;
	GLubyte   imageType;
	GLubyte   colorMapSpec[5];
	GLushort  xOrigin;
	GLushort  yOrigin;
	GLushort  width;
	GLushort  height;
	GLubyte   bpp;
	GLubyte   imageDesc;
};


enum TGATypes
{
	TGA_NODATA = 0,
	TGA_INDEXED = 1,
	TGA_RGB = 2,
	TGA_GRAYSCALE = 3,
	TGA_INDEXED_RLE = 9,
	TGA_RGB_RLE = 10,
	TGA_GRAYSCALE_RLE = 11

};
#endif // HAVE_TGA

TImage::TImage() : m_pData(NULL), m_dataType(GL_UNSIGNED_BYTE)
{
}


TImage::~TImage()
{
	FreeData();

}

void TImage::FreeData()
{
	SafeDeleteArray(m_pData);
}

bool TImage::Flip()
{
	
	if (!m_pData)
		return false;

	rgba_t* tmpBits = new rgba_t[m_width];
	if (!tmpBits)
		return false;

	int lineWidth = m_width * 4;

	rgba_t* top = (rgba_t*)m_pData;
	rgba_t* bottom = (rgba_t*)(m_pData + lineWidth*(m_height-1));

	for (int i = 0; i < (m_height / 2); ++i)
	{
		memcpy(tmpBits, top, lineWidth); 
		memcpy(top, bottom, lineWidth);
		memcpy(bottom, tmpBits, lineWidth);

		top = (rgba_t*)((GLubyte*)top + lineWidth);
		bottom = (rgba_t* )((GLubyte*)bottom - lineWidth);
	}

	SafeDeleteArray(tmpBits);
	tmpBits = 0;

	return true; 
}


void TImage::SwapBlueAndRed()
{
	switch (m_colorDepth)
	{
		case 32:
		{
			GLubyte temp;
			int total = m_width * m_height;
			rgba_t* source = (rgba_t*)m_pData;

			for (int pixel = 0; pixel < total; ++pixel)
			{
				temp = source[pixel].b;
				source[pixel].b = source[pixel].r;
				source[pixel].r = temp;
			}
		} break;
		case 24:
		{
			GLubyte temp;
			int total = m_width * m_height;
			rgb_t* source = (rgb_t*)m_pData;

			for (int pixel = 0; pixel < total; ++pixel)
			{
				temp = source[pixel].b;
				source[pixel].b = source[pixel].r;
				source[pixel].r = temp;
			}
		} break;
		default:
		// ignore other color depths
		break;
	}

}

bool TImage::LoadFromFile(const char * filename)
{
	size_t l = strlen(filename);
#ifdef HAVE_BMP
	if(!_strnicmp(filename+(l-3), "bmp", 3))
		return LoadBMP(filename);
#endif // HAVE_BMP
#ifdef HAVE_TGA
	if(!_strnicmp(filename+(l-3), "tga", 3))
		return LoadTGA(filename);
#endif // HAVE_TGA
#ifdef HAVE_JPEG
	if(!_strnicmp(filename+(l-3), "jpg", 3) || !(_strnicmp(filename+(l-4), "jpeg", 4)))
		return LoadJPEG(filename);
#endif // HAVE_JPEG_LIB
#ifdef HAVE_PNG
	if(!_strnicmp(filename+(l-3), "png", 3))
		return LoadPNG(filename);
#endif // HAVE_PNG
	return false;
}


bool TImage::SaveToFile( const char* filename )
{
	size_t l = strlen(filename);
#ifdef HAVE_TGA
	if(!_strnicmp(filename+(l-3), "tga", 3))
		return SaveTGA(filename);
#endif // HAVE_TGA
#ifdef HAVE_JPEG
	if(!_strnicmp(filename+(l-3), "jpg", 3) || !(_strnicmp(filename+(l-4), "jpeg", 4)))
		return SaveJPEG(filename);
#endif // HAVE_JPEG_LIB
#ifdef HAVE_PNG
	if(!_strnicmp(filename+(l-3), "png", 3))
		return SavePNG(filename);
#endif // HAVE_PNG
	return false;
}


#ifdef HAVE_BMP
bool TImage::LoadBMP(const char * filename)	// not work properly for 16bit and 8bit/256 color
{
	FILE *fp = fopen(filename, "rb");
    if (!fp) return false;

    fseek(fp, 0, SEEK_END);
    size_t filelen = ftell(fp);			// determine file size so we can fill it in later if FileSize == 0
    fseek(fp, 0, SEEK_SET);

    int ncolours=0;
    int ncomp=0;
    
    bmpheader_t hd;
    bmpinfo_t	inf;
    fread((char *)&hd, sizeof(bmpheader_t), 1, fp);
    if (hd.fileType == MB) 
	{
        long infsize;									// size of BMPinfo in bytes
        unsigned char *imbuff;							// image buffer
        fread((char *)&infsize, sizeof(long), 1, fp);	// insert inside 'the file is bmp' clause
        unsigned char * hdr=new unsigned char[infsize];	// to hold the new header

        fread((char *)hdr, 1, infsize-sizeof(long), fp);
        long hsiz = sizeof(inf);							// minimum of structure size & 
        if(infsize <= hsiz) hsiz=infsize;
        memcpy(&inf, hdr, hsiz);							// copy only the bytes I can cope with
		ncolours=inf.colorbits/8;

        SafeDeleteArray(hdr);
        long size = hd.size[1]*65536+hd.size[0];
        
        // handle size==0 in uncompressed 24-bit BMPs
        if (size == 0) size = filelen;

        if (inf.imageSize == 0)
			inf.imageSize=inf.width*inf.height*ncolours;
		
        imbuff = new unsigned char [inf.imageSize];		// read from disk
		fseek(fp, hd.offset[1]*65536+hd.offset[0], SEEK_SET);
        fread((char *)imbuff, sizeof(unsigned char), inf.imageSize, fp);

        switch (ncolours) 
		{
		case 0:
			ncomp = BMP_BLACKWHIET;
			inf.colorbits = 1;

        case 1:
            ncomp = BMP_GRAYSCALE;					// actually this is a 256 colour, paletted image
            inf.colorbits = 8;						// so this is how many bits there are per index
													// inf.ColorUsed=256; // and number of colours used
            break;
        case 2:
            ncomp = BMP_IA;
            break;
        case 3:
            ncomp = BMP_RGB;
            break;
        case 4:
            ncomp = BMP_RGBA;
            break;
        default:
			break;
        }

		FreeData();
		m_pData = new unsigned char [ncolours * inf.width * inf.height]; // to be returned
        
        unsigned long off=0;
        unsigned long rowbytes=ncolours * sizeof(unsigned char) * inf.width;
        unsigned long doff=rowbytes/4;
        if (rowbytes%4)		doff++;						// round up if needed
        doff*=4;										// to find dword alignment
        for(int j=0; j<inf.height; j++) 
		{
            if (ncomp>=1) 
				memcpy(m_pData+j*rowbytes, imbuff+off, rowbytes); // pack bytes closely
            off+=doff;
        }
        SafeDeleteArray(imbuff); // free the on-disk storage
 
        fclose(fp);

    } 
    else // else error in header
    {
        fclose(fp);
        return false;        
    }
    m_width = inf.width;
    m_height = inf.height;
    switch (ncomp) 
	{
    case BMP_GRAYSCALE:
        m_dataFormat = GL_LUMINANCE;
		m_dataType = GL_UNSIGNED_BYTE;
		m_colorDepth = 8;
        break;	
    case BMP_IA:
        m_dataFormat = GL_LUMINANCE_ALPHA;
		m_dataType = GL_UNSIGNED_BYTE;
		m_colorDepth = 16;	// ??
        break;
    case BMP_RGB:
        m_dataFormat = GL_RGB;
		m_dataType = GL_UNSIGNED_BYTE;
		m_colorDepth = 24;
        break;
    case BMP_RGBA:
        m_dataFormat = GL_RGBA;
		m_dataType = GL_UNSIGNED_BYTE;
		m_colorDepth = 32;
        break;
    default:
        m_dataFormat = GL_RGB;
		m_dataType = GL_UNSIGNED_BYTE;
		m_colorDepth = 24;
        break;
    }
	SwapBlueAndRed();
    return true;
}
#endif // HAVE_BMP

#ifdef HAVE_TGA
bool TImage::LoadTGA(const char* filename)
{
	FILE* pFile;

	pFile = fopen(filename, "rb");
	if (!pFile)	return false;

	// read in the image type
	tgaheader_t tga;		// TGA header

	fread(&tga, sizeof(tgaheader_t), 1, pFile);

	// see if the type is one that we support
	if ((  (tga.imageType != TGA_RGB) 
		&& (tga.imageType != TGA_GRAYSCALE) 
		&& (tga.imageType != TGA_RGB_RLE) 
		&& (tga.imageType != TGA_GRAYSCALE_RLE) ) 
		|| (tga.colorMapType != 0) )
	{
		if (pFile) fclose(pFile);
		return NULL;
	}

	// store texture information
	m_width = tga.width;
	m_height = tga.height;

	// colormode -> 3 = BGR, 4 = BGRA
	int colorMode = tga.bpp / 8;

	// won't handle < 24 bpp for now
	if (colorMode < 3)
	{
		if(pFile) fclose(pFile);
		return NULL;
	}

	m_imageSize = m_width * m_height * colorMode;

	FreeData();
	// allocate memory for TGA image data
	m_pData = new GLubyte[m_imageSize];

	// read image data
	if (tga.imageType == TGA_RGB || tga.imageType == TGA_GRAYSCALE)
	{
		fread(m_pData, sizeof(GLubyte), m_imageSize, pFile);
	}
	else // must be RLE compressed
	{
		GLubyte id;
		GLubyte length;
		rgba_t color = { 0, 0, 0, 0 };
		GLuint i = 0;

		while(i < m_imageSize)
		{

			fread(&id, sizeof(char), 1, pFile);

			// see if this is run length data
			if(id & 0x80)
			{
				// find the run length
				length = (GLubyte)(id - 127);

				// next 3 (or 4) bytes are the repeated values

				fread(&color.b, sizeof(char), 1, pFile);
				fread(&color.g, sizeof(char), 1, pFile);
				fread(&color.r, sizeof(char), 1, pFile);

				if(colorMode == 4)
				{
					fread(&color.a, sizeof(char), 1, pFile);
				}

				// save everything in this run
				while(length > 0)
				{
					m_pData[i++] = color.b;
					m_pData[i++] = color.g;
					m_pData[i++] = color.r;
					if(colorMode == 4)
					{
						m_pData[i++] = color.a;
					}

					--length;
				}
			}
			else
			{
				// the number of non RLE pixels
				length = GLubyte(id + 1);

				while (length > 0)
				{

					fread(&color.b, sizeof(char), 1, pFile);
					fread(&color.g, sizeof(char), 1, pFile);
					fread(&color.r, sizeof(char), 1, pFile);

					if(colorMode == 4)
					{
						fread(&color.a, sizeof(char), 1, pFile);
					}
					m_pData[i++] = color.b;
					m_pData[i++] = color.g;
					m_pData[i++] = color.r;
					if(colorMode == 4)
					{
						m_pData[i++] = color.a;
					}

					--length;
				}
			}
		}
	}

	if (pFile) fclose(pFile);

	switch(tga.imageType)
	{
		case TGA_RGB:
		case TGA_RGB_RLE:
			if (3 == colorMode)
			{
				m_dataFormat = GL_RGB;
				m_dataType = GL_UNSIGNED_BYTE;
				m_colorDepth = 24;
			}
			else
			{
				m_dataFormat = GL_RGBA;
				m_dataType = GL_UNSIGNED_BYTE;
				m_colorDepth = 32;
			}
			break;
		case TGA_GRAYSCALE:
		case TGA_GRAYSCALE_RLE:
			m_dataFormat = GL_LUMINANCE;
			m_dataType = GL_UNSIGNED_BYTE;
			m_colorDepth = 8;
			break;
	}
	SwapBlueAndRed();

	return (m_pData != NULL);
}

bool TImage::SaveTGA(const char* filename)
{
	FILE* pFile;

	pFile = fopen(filename, "wb");
	if (!pFile)	return false;

	// read in the image type
	tgaheader_t tga;		// TGA header
	tga.bpp = m_colorDepth;
	tga.colorMapSpec[0] = tga.colorMapSpec[1] = tga.colorMapSpec[2] = tga.colorMapSpec[3] = tga.colorMapSpec[4] = 0;
	tga.colorMapType = 0;
	tga.height = m_height;
	tga.width = m_width;
	tga.idLength = 0;
	tga.imageDesc = 0;
	tga.imageType = TGA_RGB;
	tga.xOrigin = 0;
	tga.yOrigin = 0;

	SwapBlueAndRed();
	fwrite(&tga, sizeof(tgaheader_t), 1, pFile);
	fwrite(m_pData, sizeof(GLubyte), m_imageSize, pFile);
	if (pFile) fclose(pFile);
	return true;
}
#endif // HAVE_TGA



#ifdef HAVE_WINDOWS_DC
bool TImage::LoadFromDC( HDC srcDC, int top, int left, int width, int height )
{
	bool ret = false;
	m_colorDepth = 24;
	HDC hMemDC;
	//bmp是要4字节对齐的
	int fixLineSize = ((width * m_colorDepth) >> 5) << 2;
	char* bitsMem = NULL;
	BITMAPINFOHEADER bih;
	memset(&bih,0,sizeof(bih));
	bih.biSize = sizeof(bih);
	bih.biWidth = width;
	bih.biHeight = height;
	bih.biPlanes=1;
	bih.biBitCount=24;

	BITMAPINFO bitmapInfo;
	memset((void *)&bitmapInfo,0,sizeof(BITMAPINFO));
	bitmapInfo.bmiHeader=bih;

	HBITMAP hDibBitmap = CreateDIBSection(0,&bitmapInfo,DIB_RGB_COLORS,(void **)&bitsMem,NULL,0);
	hMemDC = CreateCompatibleDC(0);

	if(hDibBitmap != 0)
	{
		::SelectObject(hMemDC,hDibBitmap);
		if (!BitBlt(hMemDC,0,0, width,height, srcDC, top, left, SRCCOPY))
			goto _realse;
	}
	else
		goto _realse;

	FreeData();
	int lineSize = m_colorDepth / 8 * width;
	m_imageSize = lineSize * height;
	m_pData = new unsigned char [m_imageSize]; 
	//DIB数据是4字节对齐的,所以我们要去掉多余数据
	int offset = 0, offset1 = 0;
	for (int h = 0; h < height; h++)
	{
		memcpy(m_pData+offset1, bitsMem+offset, lineSize);
		offset += fixLineSize;
		offset1 += lineSize;
	}

	m_width = width;
	m_height = height;
	m_dataType = GL_UNSIGNED_BYTE;
	m_dataFormat = GL_RGB;
	SwapBlueAndRed();
	ret = true;
_realse:
	DeleteObject(hDibBitmap);
	DeleteDC(hMemDC);
	SafeDelete(bitsMem);
	return ret;
}


bool TImage::LoadTextureFromDC( HDC srcDC, int top, int left, int width, int height, int tex_name)
{
	TImage img;
	if(img.LoadFromDC(srcDC, top, left, width, height))
	{
		img.Bind2DTexture(tex_name);
		return true;
	}
	else
		return false;
}

#endif // HAVE_WINDOWS_DC

#ifdef HAVE_JPEG

bool TImage::LoadJPEG( const char* filename, bool bFast /*= false*/)
{
	FILE* file = fopen(filename, "rb");  //open the file
	struct jpeg_decompress_struct info;  //the jpeg decompress info
	struct jpeg_error_mgr err;           //the error handler

	info.err = jpeg_std_error(&err);     //tell the jpeg decompression handler to send the errors to err
	jpeg_create_decompress(&info);       //sets info to all the default stuff

	//if the jpeg file didnt load exit
	if(!file)
	{
		fprintf(stderr, "Error reading JPEG file %s!!!", filename);
		return false;
	}

	jpeg_stdio_src(&info, file);    //tell the jpeg lib the file we'er reading

	jpeg_read_header(&info, TRUE);   //tell it to start reading it

	//if it wants to be read fast or not
	if(bFast)
		info.do_fancy_upsampling = FALSE;

	jpeg_start_decompress(&info);    //decompress the file

	//set the x and y
	m_width = info.output_width;
	m_height = info.output_height;

	m_dataFormat = GL_RGB;
	if(info.num_components == 4) m_dataFormat = GL_RGBA;

	m_imageSize = m_width * m_height * 3;
	FreeData();
	m_pData = new unsigned char [m_imageSize]; 
	//read turn the uncompressed data into something ogl can read

	int row_stride = m_width * 3;
	BYTE* p1 = m_pData + m_imageSize - row_stride;
	BYTE** p2 = &p1;
	
	while(info.output_scanline < info.output_height)
	{
		jpeg_read_scanlines(&info, p2, 1);
		*p2 -= row_stride;
	}

	jpeg_finish_decompress(&info);   //finish decompressing this file

	fclose(file);                    //close the file

	return true;
}

bool TImage::SaveJPEG( const char* filename, int quality /*= 70*/)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* More stuff */
	FILE * outfile;		/* target file */
	int row_stride;		/* physical row width in image buffer */

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	if ((outfile = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		return false;
	}
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = m_width; 	/* image width and height, in pixels */
	cinfo.image_height = m_height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */

	jpeg_set_defaults(&cinfo);

	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	jpeg_start_compress(&cinfo, TRUE);

	row_stride = m_width * 3;	/* JSAMPLEs per row in image_buffer */

	BYTE* p1 = m_pData + m_imageSize - row_stride;
	BYTE** p2 = &p1;

	while (cinfo.next_scanline < cinfo.image_height) {
		jpeg_write_scanlines(&cinfo, p2, 1);
		*p2 -= row_stride;
	}
	jpeg_finish_compress(&cinfo);

	fclose(outfile);

	jpeg_destroy_compress(&cinfo);
	return true;
}
#endif // HAVE_JPEG_LIB

#ifdef HAVE_PNG
bool TImage::LoadPNG( const char* filename )
{
	unsigned char* buffer;
	size_t buffersize;
	LodePNG_Decoder decoder;
	bool ret = true;

	FreeData();
	LodePNG_loadFile(&buffer, &buffersize, filename); /*load the image file with given filename*/
	LodePNG_Decoder_init(&decoder);
	LodePNG_decode(&decoder, &m_pData, &m_imageSize, buffer, buffersize); /*decode the png*/
	
	if(decoder.error) 
	{
		printf("loadpng error: %d\n", decoder.error);
		ret = false;
	}

	m_width = decoder.infoPng.width;
	m_height = decoder.infoPng.height;
	m_dataType = GL_UNSIGNED_BYTE;
	switch(decoder.infoPng.color.colorType)
	{
	case 0:					/*grey*/
		m_dataFormat = GL_LUMINANCE;
		break;
	case 2:					/*RGB*/
		m_dataFormat = GL_RGB;
		break;
	case 4:					/*grey + alpha*/
		m_dataFormat = GL_LUMINANCE_ALPHA;
		break;
	case 6:					/*RGBA*/
		m_dataFormat = GL_RGBA;
		break;
	default:		//failed
		ret = false;
	}
	m_colorDepth =  LodePNG_InfoColor_getBpp(&decoder.infoPng.color);

	free(buffer);
	LodePNG_Decoder_cleanup(&decoder);
	return ret;
}

bool TImage::SavePNG( const char* filename )
{
	unsigned char* buffer;
	size_t buffersize;
	LodePNG_Encoder encoder;

	/*create encoder and set settings and info (optional)*/
	LodePNG_Encoder_init(&encoder);
	encoder.settings.zlibsettings.windowSize = 2048;
	//LodePNG_Text_add(&encoder.infoPng.text, "Comment", "Created with LodePNG");
	switch(m_dataFormat)
	{
	case GL_RGBA:						/*RGBA*/
		encoder.infoRaw.color.colorType = 6;
		break;
	case GL_LUMINANCE:					/*grey*/
		encoder.infoRaw.color.colorType = 0;
		break;
	case GL_LUMINANCE_ALPHA:			/*grey + alpha*/
		encoder.infoRaw.color.colorType = 4;
		break;
	case GL_RGB:							/*RGB*/
		encoder.infoRaw.color.colorType = 2;
		break;
	default:
		LodePNG_Encoder_cleanup(&encoder);
		return false;
	}
	/*encode and save*/
	LodePNG_encode(&encoder, &buffer, &buffersize, m_pData, m_width, m_height);
	LodePNG_saveFile(buffer, buffersize, filename);

	/*cleanup*/
	LodePNG_Encoder_cleanup(&encoder);
	free(buffer);
	return true;
}
#endif // HAVE_PNG


void TImage::Bind2DTexture( int tex_name )
{
	glBindTexture(GL_TEXTURE_2D, tex_name);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, m_dataFormat, m_width, m_height, 
		0, m_dataFormat, m_dataType, m_pData);
}

bool TImage::LoadTextureFromFile( const char* filename, int tex_name)
{
	TImage img;
	if (img.LoadFromFile(filename))
	{
		img.Bind2DTexture(tex_name);
		return true;
	}
	else
		return false;
}

