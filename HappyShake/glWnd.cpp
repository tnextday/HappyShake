
#include "glWnd.h"
#include "shader/Fragment.h"
#include "shader/Vertex.h"
#include "glHelpers.h"
#include "image.h"
#include <ShellNotifyMsg.h>
#include "CHappyShakeWnd.h"
#include "DeviationMap.h"
#include <acc_api.h>
#include "IniMem.h"
//#define OUTPUTPTS

MZ_IMPLEMENT_DYNAMIC(CglWnd)

static int g_fps_count = 0;
static bool g_bDebug = false;

extern CIniMem *g_config;

// Handle to a program object
GLuint programObject;

// Attribute locations
GLint  positionLoc;
GLint  texCoordLoc;
GLint  viewprojLoc;

GLuint texture[TEXTURENUM];

/*
void esDPrint(const wchar_t* fmt, ...)
{
	wchar_t out[2048], *p = out;
	va_list ap;
	static int lastTick = 0;
	p += swprintf(p, L"[%10d] ", GetTickCount() - lastTick);
	lastTick = GetTickCount();
	va_start(ap, fmt);
	vswprintf(p, fmt, ap);
	va_end(ap);
	OutputDebugStringW(out);
}
void printGLError()
{

	GLenum errcode = glGetError();
	if (errcode == GL_NO_ERROR)
		wprintf(L"GL_NO_ERROR\n");
	else if (errcode == GL_INVALID_ENUM)
		wprintf(L"GL_INVALID_ENUM\n");
	else if (errcode == GL_INVALID_VALUE)
		wprintf(L"GL_INVALID_VALUE\n");
	else if (errcode == GL_INVALID_OPERATION)
		wprintf(L"GL_INVALID_OPERATION\n");
	else if (errcode == GL_STACK_OVERFLOW)
		wprintf(L"GL_STACK_OVERFLOW\n");
	else if (errcode == GL_STACK_UNDERFLOW)
		wprintf(L"GL_STACK_UNDERFLOW\n");
	else if (errcode == GL_OUT_OF_MEMORY)
		wprintf(L"GL_OUT_OF_MEMORY\n");
	else
		wprintf(L"Unknow Error Code\n");
}
*/

int getSng(int i)
{
	if (i > 0) return 1;
	else if (i == 0) return 0;
	else return -1;
}

//////////////////////////////////////////////////////////////////////////

GLuint LoadShaderBinary(GLenum type, const void *shaderSrc, int size)
{
	GLuint shader;

	// Create the shader object
	shader = glCreateShader(type);
	if(shader == 0)
		return 0;
	// Load the shader source
	glShaderBinary(1, &shader, (GLenum)0, shaderSrc, size);

	return shader;
}

bool loadAnyTexture(CMzString &filename, Texture* tex )
{
	TImage img;
	char strTexPath[_MAX_PATH];
	wcstombs(strTexPath, filename.C_Str(), _MAX_PATH);
	bool ret = false;

	if(img.LoadFromFile(strTexPath))
	{
		img.Bind2DTexture(tex->tex);
		ret = true;
	}
	tex->w = img.GetWidth() ;
	tex->h = img.GetHeight();
	return ret;
}

bool loadAnyTexture( CMzString &filename, int texName )
{
	Texture tex;
	tex.tex = texName;
	return loadAnyTexture(filename, &tex);
}


void drawTexture( Texture *tex, GLint posX, GLint posY )
{
	static const GLfloat texCoord[] =
	{
		0.0, 0.0,
		0.0, 1.0, 
		1.0, 1.0,
		1.0, 0.0,
	};
	GLint texVertex[12] = {0};
	GLint w,h;
	w = tex->w/2;
	h = tex->h/2;

	texVertex[0] = posX-w; texVertex[1] = posY+h; texVertex[2] = 0;
	texVertex[3] = posX-w; texVertex[4] = posY-h; texVertex[5] = 0;
	texVertex[6] = posX+w; texVertex[7] = posY-h; texVertex[8] = 0;
	texVertex[9] = posX+w; texVertex[10] = posY+h; texVertex[11] = 0;
	glBindTexture(GL_TEXTURE_2D, tex->tex);
	glVertexAttribPointer(positionLoc, 3, GL_INT, GL_FALSE, 0, texVertex);
	glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawSquare( GLint posX, GLint posY, GLint r, GLint posZ = 0 )
{
	static const GLfloat texCoord[] =
	{
		0.0, 0.0,
		0.0, 1.0, 
		1.0, 1.0,
		1.0, 0.0,
	};
	GLint texVertex[12] = {0};
	if (r <= 0) return;
	texVertex[0] = posX-r; texVertex[1] = posY+r; texVertex[2] = posZ;
	texVertex[3] = posX-r; texVertex[4] = posY-r; texVertex[5] = posZ;
	texVertex[6] = posX+r; texVertex[7] = posY-r; texVertex[8] = posZ;
	texVertex[9] = posX+r; texVertex[10] = posY+r; texVertex[11] = posZ;

	glVertexAttribPointer(positionLoc, 3, GL_INT, GL_FALSE, 0, texVertex);
	glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoord);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

CglWnd::CglWnd(void)
{
	m_Display = EGL_NO_DISPLAY;
	m_Surface = EGL_NO_DISPLAY;
	m_activeBubbles = 0;
	m_imageFileName.SetBufferSize(MAX_PATH);
}

CglWnd::~CglWnd(void)
{
	releaseGL();
	KillTimer(m_hWnd, TIMER_PTS);
	KillTimer(m_hWnd, TIMER_DRAW);
	KillTimer(m_hWnd, TIMER_ACC);
	MzAccClose();
}

BOOL CglWnd::OnInitDialog()
{
	// First invoke initialization of parent class
	if (!CMzWndEx::OnInitDialog())
	{
		return FALSE;
	}

	// 注册所有硬按键消息 WM_MZSH_ALL_KEY_EVENT
	//RegisterShellMessage(m_hWnd, WM_MZSH_ALL_KEY_EVENT);
	// 获取所有硬按键消息值
	//m_allkeyEvent = GetShellNotifyMsg_AllKeyEvent();

	//SetScreenAlwaysOn(m_hWnd);
	
	//开启acc功能
	MzAccOpen(); 

	m_ff = DEFINE_FF;
	m_k = DEFINE_K;
	m_bEdit = false;
	initEGL();
	initGL();
	m_toolbar.init(GetParent());

#ifdef OUTPUTPTS
	SetTimer(m_hWnd, TIMER_PTS, 1000, NULL);
#endif // OUTPUTPTS

	SetTimer(m_hWnd, TIMER_ACC, 100, NULL);
	SetTimer(m_hWnd, TIMER_DRAW, 30, NULL);
	m_bActive = true;
	return TRUE;
}


void GetWorkPath(TCHAR szPath[], int nSize)  
{  
	GetModuleFileName(NULL, szPath, nSize);  
	TCHAR *p = wcsrchr(szPath, '\\');    
	*p = 0;  
}


bool CglWnd::initEGL()
{
	 EGLContext	context = EGL_NO_CONTEXT;

	 EGLint m_Major = 0;
	 EGLint m_Minor = 0;

	 EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
	 
	 m_hdcWnd = GetDC(m_hWnd);

	 if( (m_Display = eglGetDisplay(m_hdcWnd))==EGL_NO_DISPLAY )
		 return false;

	 // initialize
	 if(eglInitialize(m_Display, &m_Major, &m_Minor)==EGL_FALSE)
		 return false;

	 // choose config
	 EGLConfig config = 0;
	 EGLint cfg_attr_list[] = { 
		 EGL_BUFFER_SIZE,    EGL_DONT_CARE,
		 EGL_RED_SIZE,       5,
		 EGL_GREEN_SIZE,     6,
		 EGL_BLUE_SIZE,      5,
		 EGL_DEPTH_SIZE,     8,
		 EGL_NONE
	 };

	 int num = 0;
	 if ( eglChooseConfig(m_Display, cfg_attr_list, &config, 1, &num) == EGL_FALSE || num == 0 )
	 {
		 return false;
	 }

	 if( (m_Surface = eglCreateWindowSurface(m_Display, config, m_hWnd, NULL))==EGL_NO_SURFACE )
		 return false;

	 if( (context = eglCreateContext(m_Display, config, EGL_NO_CONTEXT, contextAttribs))==EGL_NO_CONTEXT )
		 return false;

	 if( eglMakeCurrent(m_Display, m_Surface, m_Surface, context)==EGL_FALSE )
		 return false;
	 eglDestroyContext(m_Display, context);
	 return true;
}

int CglWnd::initGL()
{
	 GLuint vertexShader;
	 GLuint fragmentShader;
	 GLint linked;

	 // Load the vertex/fragment shaders
	 vertexShader = LoadShaderBinary(GL_VERTEX_SHADER, VertexVertex, sizeof(int)*VertexVertexLength);
	 fragmentShader = LoadShaderBinary(GL_FRAGMENT_SHADER, FragmentFragment, sizeof(int)*FragmentFragmentLength);
	 // Create the program object
	 programObject = glCreateProgram();
	 if(programObject == 0)
		 return 0;
	 glAttachShader(programObject, vertexShader);
	 glAttachShader(programObject, fragmentShader);

	 // Link the program
	 glLinkProgram(programObject);

	 // Check the link status
	 glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
	 if(!linked) 
	 {
		 GLint infoLen = 0;
		 glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

		 if(infoLen > 1)
		 {
			 char* infoLog = (char*)malloc(sizeof(char) * infoLen);
			 glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
			 // esLogMessage("Error linking program:\n%s\n", infoLog);
			 free(infoLog);
		 }
		 glDeleteProgram(programObject);
		 return FALSE;
	 }

	 glDeleteShader(vertexShader);
	 glDeleteShader(fragmentShader);

	 glUseProgram(programObject);
	 positionLoc = glGetAttribLocation(programObject, "a_vertexPos");
	 texCoordLoc = glGetAttribLocation(programObject, "a_texCoord");

	 viewprojLoc = glGetUniformLocation(programObject, "mvp" );

	 glClearColor(0.0f, 0.0f, 0.0f, 1.0f);			// 黑色背景
 	 glCullFace(GL_BACK);
 	 glEnable(GL_CULL_FACE);
	 glEnable(GL_BLEND);
	 //glBlendColor(0.0, 0.0, 0.0, 0.0);
	 //启用混合操作
	 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
	 
	 makeMainTexCoord();
	 makeMainTexIndices();

	 glGenTextures(TEXTURENUM, texture);

	 // texture:
	 loadTextures();
	 
	 glEnableVertexAttribArray(positionLoc);
	 glEnableVertexAttribArray(texCoordLoc);

	 ReSizeGLScene(480, 720);
	 return true;
}

void CglWnd::releaseGL()
{
	 glDeleteTextures(TEXTURENUM,texture);
	 glDeleteProgram(programObject);
	 eglDestroySurface(m_Display,m_Surface);
	 ReleaseDC(m_hWnd, m_hdcWnd);
}

// 重置OpenGL窗口大小
void CglWnd::ReSizeGLScene( GLsizei width, GLsizei height )
{
	if (height==0)										// 防止被零除
	{
		height=1;										// 将Height设为1
	}
	float mat_p[16];
	float mat_t[16];
	float mvp[16];
	float aspect = (float)width/height;

	glViewport(0,0,width,height);						// 重置当前的视口

	matIdentity(mvp);
	matIdentity(mat_p);
	matIdentity(mat_t);
	viewHeight = 720/2;
	viewWidth = viewHeight*aspect;
	// 设置视口的大小
	matPerspective((float*)mat_p, 90,  aspect, 200.0f, 500.0f);
	//设置镜头
	matLookAt((float*)mat_t, 0,0,viewHeight, 0,0,0, 0,1,0);

	matMult((float*)mvp, (float*)mat_p, (float*)mat_t);
	
	//z=0 平面的投影大小
	// set the transformation
	glUniformMatrix4fv(viewprojLoc, 1, GL_FALSE, (float*)mvp);
	resetMainTexVertex();
}

void CglWnd::OnTimer( UINT_PTR nIDEvent )
{
	if (m_bActive && nIDEvent == TIMER_DRAW)
	{
		drawGL();
	}
	else if (nIDEvent == TIMER_PTS)
	{
		WCHAR buf[100];
		//wsprintf(buf, L"FPS:%d ", g_fps_count);
		//SetWindowText(buf);
		wsprintf(buf, L"FPS:%d \n", g_fps_count);
		wprintf(buf);
		g_fps_count = 0;
	}
	else if (m_bActive && nIDEvent == TIMER_ACC)
	{
		updateBubblesF();
	}
		
}

void CglWnd::makeMainTexCoord()
{
	float pieceSizeColumn = 1.0f / columnPieces;
	float pieceSizeRow = 1.0f / rowPieces;
	float pieceCoordy = 1.0;
	int pointIndex = 0;
	float *texCoord = (float*)m_texCoord;
	for (int y = 0; y <= rowPieces; y++)
	{
		float pieceCoordx = 0.0;
		for (int x = 0; x <= columnPieces; x++)
		{
			*(texCoord++) = pieceCoordx;
			*(texCoord++) = pieceCoordy;
			pieceCoordx += pieceSizeColumn;
		}
		pieceCoordy -= pieceSizeRow;
	}
}

void CglWnd::resetMainTexVertex()
{
	m_pieceSize = viewWidth*2 / columnPieces;
	GLint pieceCoordy = viewHeight;
	int pointIndex = 0;
	GLint *texVertex = (GLint*)m_oTexVertex;
	for (int y = 0; y <= rowPieces; y++)
	{
		float pieceCoordx = -viewWidth;
		for (int x = 0; x <= columnPieces; x++)
		{
			*(texVertex++) = pieceCoordx;
			*(texVertex++) = pieceCoordy;
			*(texVertex++) = 0.0;		//z
			pieceCoordx += m_pieceSize;
		}
		pieceCoordy -= m_pieceSize;
	}
}


int CglWnd::drawGL()
{
	//esDPrint(L"DrawBegin\n");

	glClear(GL_COLOR_BUFFER_BIT);
	//复位坐标
	memcpy(m_dTexVertex, m_oTexVertex, sizeof(m_oTexVertex));
	if (!m_bEdit)
	{
		updateBubbles();
	}
	glVertexAttribPointer(positionLoc, 3, GL_INT, GL_FALSE, 0, m_dTexVertex);
	glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, m_texCoord);
	glBindTexture(GL_TEXTURE_2D, texture[TEXID_BG]);
	glDrawElements(GL_TRIANGLES /*GL_LINE_LOOP*/, columnPieces*rowPieces*3*2, GL_UNSIGNED_SHORT, m_indices);

	if (m_bEdit)
	{	
		int nHide = 0;
		Bubble *bubbleSelect = NULL;
		glBindTexture(GL_TEXTURE_2D, texture[TEXID_SET]);
		for (int i = 0; i < MAX_BUBBLES; i++)
		{
			Bubble *b = &m_bubbles[i];
			if (b->eState == ST_REMOVED) continue;
			b->update();
			if (b->eState == ST_NORMAL)
			{
				nHide++;
				continue;
			}
			if (b->bSelect)
				bubbleSelect = b;
			else
				drawSquare(b->ox, b->oy, b->r * b->zoomRate);
		}
		if (bubbleSelect)
		{
			glBindTexture(GL_TEXTURE_2D, texture[TEXID_SET_SELECT]);
			drawSquare(bubbleSelect->ox, bubbleSelect->oy, bubbleSelect->r * bubbleSelect->zoomRate);
		}
		
		if ( m_activeBubbles > 0)
			m_bEdit = nHide < m_activeBubbles;
	}
	m_toolbar.draw();
	//esDPrint(L"Draw\n");
	eglSwapBuffers(m_Display, m_Surface);
	g_fps_count++;
	return 1;
}
/************************************************************************/
/*	[0]----[3]
/*	 |		|
/*	 |		|
/*	 |		|
/*	[1]----[2]
/************************************************************************/
//生成图片定点索引
void CglWnd::makeMainTexIndices()
{
	GLushort *indice = (GLushort *)m_indices;
	int columnPoints = columnPieces + 1;
	for (int y = 0; y < rowPieces; y++)
	{
		for (int x = 0; x < columnPieces; x++)
		{
			//两个三角形绘制成一个四边形
			*(indice++) = y*columnPoints + x;			//[0]
			*(indice++) = (y+1)*columnPoints + x;		//[1]
			*(indice++) = y*columnPoints + x + 1;		//[3]

			*(indice++) = y*columnPoints + x + 1;		//[3]
			*(indice++) = (y+1)*columnPoints + x;		//[1]
			*(indice++) = (y+1)*columnPoints + x + 1;	//[2]
		}
	}
}

LRESULT CglWnd::MzDefWndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
// 	if (message == m_allkeyEvent)
// 	{
// 		if (LOWORD(wParam) == WPARAM_KEY_EVENT_CLICK_VOLUP)
// 			g_bDebug = true;
// 		else if (LOWORD(wParam) == WPARAM_KEY_EVENT_CLICK_VOLDOWN)
// 			g_bDebug = false;
// 	}
// 	if (message >= MZFC_WM_MESSAGE)
// 		printf("MZFC_WM_MESSAGE 0x%08x\n", message - MZFC_WM_MESSAGE);
// 	else if (message != WM_TIMER)
// 		printf("WM_MESSAGE 0x%08x\n", message);
	if (message == MZ_WM_WND_ACTIVATE)
	{
		setActive(wParam != WA_INACTIVE);
	}
	return CMzWndEx::MzDefWndProc(message,wParam,lParam);
}


void CglWnd::OnSize( int nWidth, int nHeight )
{
	if (nHeight > 16 && nWidth > 16)
	{
		//ReSizeGLScene(nWidth, nHeight);
	}
}

void CglWnd::setParentWnd( CHappyShakeMainWnd *pWnd )
{
	m_parent = pWnd;
}

void CglWnd::updateBubbles()
{
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		Bubble *b = &m_bubbles[i];
		if (b->eState != ST_NORMAL)
			continue;
		updateBubblePos(b);
		updateBubbleVertex(b);
	}
}

void CglWnd::updateBubbleVertex( Bubble *b )
{
	Vertex *dv = (Vertex *)m_dTexVertex;
	Vertex *dv1;
	if (b->mx == 0 && b->my == 0)
		return;
	GLint ff;
	int tx,ty, bx, by;
	ff = b->ox - b->r;
	ff = ff < -viewWidth + m_pieceSize ? -viewWidth + m_pieceSize : ff;
	tx = (ff - (-viewWidth))/m_pieceSize;
	ff = b->oy + b->r;
	ff = ff > viewHeight - m_pieceSize ? viewHeight - m_pieceSize: ff;
	ty = abs(ff - viewHeight)/m_pieceSize;
	ff = b->ox + b->r;
	ff = ff > viewWidth - m_pieceSize ? viewWidth - m_pieceSize : ff;
	bx = (ff - (-viewWidth))/m_pieceSize;
	ff = b->oy - b->r;
	ff = ff < -viewHeight + m_pieceSize ? -viewHeight + m_pieceSize : ff;
	by = abs(ff - viewHeight)/m_pieceSize;
	for (int y = ty; y <=by; y++)
	{
		int ny = y*(columnPieces+1);
		for (int x = tx; x <= bx; x++)
		{
			dv1 = dv + ny + x;
			GLint dx = abs(b->ox - dv1->x);
			GLint dy = abs(b->oy - dv1->y);
			if (dx > b->r || dy > b->r)
				continue;
			float dr;
			dr = mapDeviationRate[(int)(dy*(nDeviationRate-1)/b->r)][(int)(dx*(nDeviationRate-1)/b->r )];
			dv1->x = dv1->x + b->mx * dr;
			dv1->y = dv1->y + b->my * dr;
		}
	}
}

void CglWnd::OnMouseMove( UINT fwKeys, int xPos, int yPos )
{
	//转换为Opengl的坐标系
	xPos = xPos - 240;
	yPos = 360 - yPos;
	if (m_bEdit)
	{
		for (int i = 0; i < MAX_BUBBLES; i++)
		{
			Bubble *b = &m_bubbles[i];
			if (b->eState == ST_EDIT_SHOW && b->bSelect)
			{
				b->setPos(b->ox + xPos - m_lastPosX, b->oy + yPos - m_lastPosY);
			}
		}
	}
	else
	{
		for (int i = 0; i < MAX_BUBBLES; i++)
		{
			Bubble *b = &m_bubbles[i];
			if (b->eState == ST_NORMAL && b->bSelect)
			{
				b->mx = xPos - b->ox;
				b->my = yPos - b->oy;
				int r2 = (b->mx*b->mx + b->my*b->my);
				int rr = b->r*0.6;
				if (r2 > rr*rr)
				{
					float scaleRate = rr/sqrt(r2);
					b->mx *= scaleRate;
					b->my *= scaleRate;
				}
			}
		}
	}

	m_lastPosX = xPos;
	m_lastPosY = yPos;
	
}

void CglWnd::OnLButtonDown( UINT fwKeys, int xPos, int yPos )
{
	//转换为Opengl的坐标系
	xPos = xPos - 240;
	yPos = 360 - yPos;
	m_lastPosX = xPos;
	m_lastPosY = yPos;
	int crrtSelect = caclSelectBubble(xPos, yPos);
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		if (crrtSelect >= 0)
			m_bubbles[i].bSelect = i == crrtSelect;
		else if (crrtSelect < 0 && !m_bEdit)
			m_bubbles[i].bSelect = false;
	}
	m_toolbar.OnLButtonDown(fwKeys, xPos, yPos);
}

void CglWnd::OnLButtonUp( UINT fwKeys, int xPos, int yPos )
{
	if (m_bEdit)
	{
		checkCanEdit();
	}
	else
	{
		for (int i = 0; i < MAX_BUBBLES; i++)
		{
			m_bubbles[i].bSelect = false;
		}
	}
	m_toolbar.OnLButtonUp(fwKeys, xPos - 240, 360 - yPos);
}

void CglWnd::updateBubblePos( Bubble *b )
{
	//合力
	GLfloat fx, fy;
	if (b->eState != ST_NORMAL || b->bSelect) return;
	int mr = b->r*0.6;
	//合力= 外力+弹力
	fx = b->fx - m_k*(float)b->mx;
	fy = b->fy - m_k*(float)b->my ;
	b->mx = b->mx + (b->vx + fx);
	b->my = b->my + (b->vy + fy);
	b->vx = (b->vx + fx)*m_ff;
	b->vy = (b->vy + fy)*m_ff;
	int r2 = b->mx*b->mx + b->my*b->my;
	if (r2 > mr*mr)
	{
		float scaleRate = mr/sqrt(r2);
		b->mx *= scaleRate;
		b->my *= scaleRate;
		b->vx *= scaleRate;
		b->vy *= scaleRate;
	}
	b->fx = 0;
	b->fy = 0;
}

void CglWnd::updateBubblesF()
{
	signed char xAxis, yAxis;
	int dxa,dya;
	//顶点受外力
	GLfloat aacfx = 0, aacfy = 0;
	MzAccGetY(&xAxis);
	dxa = xAxis - m_LastXAxis;
	if (abs(dxa) > MAX_ACC_F)
		dxa = getSng(dxa)*MAX_ACC_F;
	m_LastXAxis = xAxis;
	MzAccGetX(&yAxis);
	dya = yAxis - m_LastYAxis;
	if (abs(dya) > MAX_ACC_F)
		dxa = getSng(dya)*MAX_ACC_F;
	m_LastYAxis = yAxis;
	if (abs(dxa) > MIN_ACC_F)
		aacfx = dxa * ACC_F_RATE;
	if (abs(dya) > MIN_ACC_F)
		aacfy = dya * ACC_F_RATE;

	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		if (m_bubbles[i].eState == ST_NORMAL)
		{
			if (m_bubbles[i].bSelect) continue;
			if (aacfx != 0)
				m_bubbles[i].fx = aacfx;
			if (aacfy != 0)
				m_bubbles[i].fy = aacfy;
		}
	}
}

void CglWnd::showEdit( bool bedit )
{
	if(bedit) m_bEdit = true;

	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		if (m_bubbles[i].eState == ST_REMOVED)
			continue;
		m_bubbles[i].bSelect = false;
		if (bedit)
			m_bubbles[i].show();
		else
			m_bubbles[i].hide();
	}
	m_toolbar.setExpanded(bedit);
}

void CglWnd::addEditBubble( int x /*= 0*/, int y /*= 0*/, int r /*= 100*/ )
{
	if (!canAdd())  return;
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		if (m_bubbles[i].eState == ST_REMOVED)
		{
			m_bubbles[i].reset();
			m_bubbles[i].r = r;
			m_bubbles[i].setPos(x, y);
			m_bubbles[i].show();
			m_activeBubbles++;
			checkCanEdit();
			return;
		}
	}

}

void CglWnd::removeEditBubble()
{
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		if (m_bubbles[i].eState <= ST_EDIT_SHOW && m_bubbles[i].bSelect)
		{
			m_bubbles[i].remove();
			m_activeBubbles--;
			checkCanEdit();
			return;
		}
	}
}

bool CglWnd::canRemove()
{
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		if (m_bubbles[i].eState <= ST_EDIT_SHOW && m_bubbles[i].bSelect)
		{
			return true;
		}
	}
	return false;
}

void CglWnd::checkCanEdit()
{
	if (!m_bEdit) return;
	::SendMessage(GetParent(), WM_USER_EDIT_STATE_CHANGE, 0, 0);
}

void CglWnd::zoomCrrtSelect( bool bZoomIn )
{
	if (!m_bEdit) return;
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		Bubble *b = &m_bubbles[i];
		if (b->eState == ST_EDIT_SHOW && b->bSelect)
		{
			if (b->r <= MAX_BUBBLE_R - BUBBLE_ZOOM_STEP && bZoomIn)
				b->r += BUBBLE_ZOOM_STEP;
			if (b->r >= MIN_BUBBLE_R + BUBBLE_ZOOM_STEP && !bZoomIn)
				b->r -= BUBBLE_ZOOM_STEP;
			return;
		}
	}
}

void CglWnd::setActive( bool bActive )
{
	if (m_bActive == bActive) return;
	m_bActive = bActive;
	//printf("Debug: SetActive %s\n", bActive ? "true" : "false");
	//x,y是反的,因为m8工作在正旋90的状态
	MzAccGetX(&m_LastYAxis);
	MzAccGetY(&m_LastXAxis);
}

bool CglWnd::setImage( CMzString &filename )
{
	bool ret = loadAnyTexture(filename, texture[TEXID_BG]);
	if (ret) m_imageFileName = filename;
	return  ret;
}


bool CglWnd::setImageFromDC( HDC dc, int width /*= 480*/, int height /*= 720*/ )
{
	TImage img;
	CMzString strAppPath(_MAX_PATH);
	char strFilePath[_MAX_PATH];
	GetWorkPath(strAppPath.C_Str(), strAppPath.GetBufferSize());
	strAppPath += L"\\Data\\Last.jpg";
	wcstombs(strFilePath, strAppPath.C_Str(), _MAX_PATH);
	if (img.LoadFromDC(dc, 0, 0, width, height))
	{
		img.Bind2DTexture(texture[TEXID_BG]);
		img.SaveJPEG(strFilePath);
		m_imageFileName = strAppPath;
		return true;
	}
	return false;
}

void CglWnd::removeAllBubbles()
{
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		m_bubbles[i].eState = ST_REMOVED;
	}
	m_activeBubbles = 0;
	checkCanEdit();
}

bool CglWnd::getBubbleSet( int nIndex, int &x, int &y, int &r )
{
	if (nIndex >= MAX_BUBBLES) goto __false;
	Bubble *b = &m_bubbles[nIndex];
	if (b->eState >= ST_ONREMOVE) goto __false;
	x = b->ox;
	y = b->oy;
	r = b->r;
	return true;
__false:
	x = 0;
	y = 0;
	r = 0;
	return false;
}

int CglWnd::caclSelectBubble( int x, int y )
{
	int ret = -1;
	int nearest = 0x7FFFFFFF;
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		Bubble *b = &m_bubbles[i];
		if (b->eState == ST_EDIT_SHOW || b->eState == ST_NORMAL)
		{
			int r2 = ((x - b->ox)*(x - b->ox) + (y - b->oy)*(y - b->oy));
			if (r2 <= b->r*b->r && r2 < nearest)
			{
				nearest = r2;
				ret = i;
			}
		}
	}
	return ret;
}

int CglWnd::getActiveBubbleCount()
{
	int ret = 0;
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		Bubble *b = &m_bubbles[i];
		if (b->eState == ST_EDIT_SHOW || b->eState == ST_EDIT_ONSHOW || b->eState == ST_NORMAL)
			ret++;
	}
	return ret;
}

void CglWnd::loadTextures()
{
	CMzString strAppPath(_MAX_PATH);
	GetWorkPath(strAppPath.C_Str(), strAppPath.GetBufferSize());
	loadAnyTexture(strAppPath + L"\\Data\\0.png", texture[TEXID_SET]);
	loadAnyTexture(strAppPath + L"\\Data\\1.png", texture[TEXID_SET_SELECT]);
}

bool extractDir(IN LPTSTR path, OUT LPTSTR dir) 
{
	_tcscpy(dir, path);
	int nlen = _tcslen(dir);
	TCHAR* s = dir + nlen - 1;
	while (nlen)
	{
		if (*s == L'\\')
		{
			*s = L'\0';
			return true;
		}
		s--;
		nlen--;
	}
	return false;
}

bool CglWnd::selectImageFile()
{
	CMzString strFilePath(MAX_PATH);
	TCHAR defDir[MAX_PATH] = {0};
	bool bSelect = false;   
	bool ret = false;
	g_config->GetValue(L"Last", L"Path", L"\\Disk\\Photo", defDir, MAX_PATH);
	bSelect = selectFile(m_hWnd, strFilePath, CMzString(L"*.jpg;*.png;"), defDir);

	if (bSelect)
	{
		showWaiting(m_hWnd);
		ImagingHelper img;
		MemoryDC memDc;
		if (img.LoadImage(strFilePath.C_Str(), true, true))
		{
			HDC dc;
			if (img.GetImageWidth() == 480 && img.GetImageHeight() == 720)
			{
				dc = img.GetDC();
			}
			else
			{
				memDc.Create(480, 720);
				RECT r;
				HBRUSH hBrush = CreateSolidBrush(RGB(0,0,0));
				SetRect(&r, 0, 0, 480, 720);
				FillRect(memDc.GetDC(), &r, hBrush);
				DeleteObject(hBrush);
				int w,h;
				if (img.GetImageWidth()*3 > img.GetImageHeight()*2)
				{
					w = 480;
					h = w*img.GetImageHeight()/img.GetImageWidth();
				}
				else
				{
					h = 720;
					w = h*img.GetImageWidth()/img.GetImageHeight();
				}
				SetRect(&r, 240 - w/2, 360 - h/2, 240 + w/2, 360 + h/2);
				img.Draw(memDc.GetDC(), &r, true, true);
				dc = memDc.GetDC();
			}

			if (setImageFromDC(dc))
			{
				ret = true;
				removeAllBubbles();
				if (extractDir(strFilePath.C_Str(), defDir))
					g_config->SetValue(L"Last", L"Path", defDir);
				
			}
		}
		hideWaiting();
	}
	return ret;
}

void Bubble::reset()
{
	ox = 0;
	mx = 0;
	oy = 0;
	my = 0;
	r = DEFINE_EDITER_R;
	fx = 0;
	fy = 0;
	vx = 0;
	vy = 0;
	zoomRate = 0.1;
	bSelect = false;
	eState = ST_REMOVED;
}

void Bubble::show( int totalFarme /*= 10*/ )
{
	crrtFrame = 0;
	animateFrames = totalFarme;
	eState = ST_EDIT_ONSHOW;
	bSelect = false;
}

void Bubble::hide( int totalFarme /*= 10*/ )
{
	crrtFrame = 0;
	animateFrames = totalFarme;
	eState = ST_EDIT_ONHIDE;
	//默认状态时,随机初始个状态
	if (mx != ox && my != oy && fx ==0 && fy == 0 && vx == 0 && vy == 0)
	{
		//速度+-40
		vx = rand()%80 - 40;
		vy = rand()%80 - 40;
	}
	bSelect = false;
}

void Bubble::remove( int totalFarme /*= 10*/ )
{
	crrtFrame = 0;
	animateFrames = totalFarme;
	eState = ST_ONREMOVE;
	bSelect = false;
}

void Bubble::update()
{
	crrtFrame++;
	if (crrtFrame >= animateFrames && eState % 2 == 0) 
	{
		eState  = (EditBubbleState)(eState + 1);
		if (eState == ST_EDIT_SHOW) zoomRate = 1.0;;
	}
	switch (eState)
	{
	case ST_EDIT_ONSHOW:
		{
			float t = (float)crrtFrame/animateFrames - 1.0;
			float s = 1.70158;
			zoomRate = t*t*((s+1)*t+ s) + 1;
		}
		break;
	case ST_EDIT_ONHIDE:
	case ST_ONREMOVE:
		{
			float t = (float)crrtFrame / animateFrames - 1.0;
			zoomRate = t*t*t*t;
		}
		break;
	}
}

void Bubble::setPos( int x, int y )
{
	if (x < -240) x = -240;
	else if (x > 240) x = 240;
	ox = x;
	mx = 0;
	if (y < -360) y = -360;
	else if (y >360) y = 360;
	oy = y;
	my = 0;
}

void glButton::draw()
{
	if (!m_bShow) return;
	if (m_state != BTN_NORMAL) 
	{
		crrtFrame++;
		if (crrtFrame >= animateFrames) 
		{
			m_state = BTN_NORMAL;
			m_x += m_dx;
			m_y += m_dy;
		}
		if (m_state == BTN_MOVEING)
		{
			float t = (float)crrtFrame / animateFrames - 1.0;
			m_rate = 1.0 - t*t*t*t;
		}
	}
	glBindTexture(GL_TEXTURE_2D, m_tex);
	drawSquare(getX(), getY(), getR());
}


int glButton::getY()
{
	return m_state == BTN_NORMAL ? m_y : m_y + m_dy*m_rate;
}

int glButton::getX()
{
	return m_state == BTN_NORMAL ? m_x : m_x + m_dx*m_rate;
}

int glButton::getR()
{
	return m_bSelect ? m_r*1.5 : m_r;
}

void glButton::OnLButtonUp( UINT fwKeys, int xPos, int yPos )
{
	if (m_bShow && m_bSelect && isInButton(xPos, yPos))
	{
		SendMessage(m_hWnd, MZ_WM_COMMAND, WM_IDC_GLBUTTON, m_id);
	}
	m_bSelect = false;
}

void glButton::OnLButtonDown( UINT fwKeys, int xPos, int yPos )
{
	m_bSelect = m_bShow && isInButton(xPos, yPos);
}

bool glButton::isInButton( int x, int y )
{
	return (abs(x - m_x) <= m_r*2) && (abs(m_y - y) <= m_r*2);
}

void glButton::moveTo( int x, int y, int totalFarme /*= 10*/ )
{
	crrtFrame = 0;
	animateFrames = totalFarme;
	m_dx = x - m_x;
	m_dy = y - m_y;
	m_state = BTN_MOVEING;
}

void glButton::setPos( int x, int y )
{
	m_y = y;
	m_x = x;
}

glButton::glButton()
{
	m_hWnd = NULL;
	m_tex = 0;
	m_id = 0;
	m_r = 0;
	setPos(0, 0);
	m_bSelect = false;
	m_state = BTN_NORMAL;
	setVisible(true);
}

void glButton::setTexture( int texEnable, int r )
{
	m_tex = texEnable;
	m_r = r;
}

void glButton::setWnd( HWND hWnd, int id )
{
	m_hWnd = hWnd;
	m_id = id;
}


void glToolbar::OnLButtonDown( UINT fwKeys, int xPos, int yPos )
{
	for (int i = 0; i < 5; i++)
	{
		m_btns[i].OnLButtonDown(fwKeys, xPos, yPos);
	}
}

void glToolbar::OnLButtonUp( UINT fwKeys, int xPos, int yPos )
{
	for (int i = 0; i < 5; i++)
	{
		m_btns[i].OnLButtonUp(fwKeys, xPos, yPos);
	}
}


void glToolbar::draw()
{
	if (m_bAnimating)
	{
		crrtFrame++;
		if (crrtFrame >= animateFrames)
		{
			m_bAnimating = false;
			m_x += m_dx;
		}
		float t = (float)crrtFrame / animateFrames - 1.0;
		m_rate = 1.0 - t*t*t*t;
	}
	drawTexture(&m_tbbg, m_bAnimating ? m_x + m_dx*m_rate : m_x, m_y);
	for (int i = 0; i < 5; i++)
	{
		m_btns[i].draw();
	}
}

void glToolbar::init( HWND h )
{
	CMzString strAppPath(_MAX_PATH);
	CMzString strTexPath(_MAX_PATH);
	GetWorkPath(strAppPath.C_Str(), strAppPath.GetBufferSize());

	m_hWnd = h;
	for (int i = 0; i <= 5; i++)
	{
		wsprintf(strTexPath.C_Str(), L"%s\\Data\\btn_%d.png", strAppPath.C_Str(), i);
		loadAnyTexture(strTexPath, texture[TEXID_BUTTONS+i]);
	}
	
	loadAnyTexture(strAppPath + L"\\Data\\dock.png", &m_tbbg);
	for (int i = 0; i < 5; i++)
	{
		m_btns[i].setWnd(m_hWnd,i);
		m_btns[i].setTexture(texture[TEXID_BUTTONS+i]);
	}
	setExpanded(false, 0);
}

glToolbar::glToolbar()
{
	crrtFrame = 0;
	animateFrames = 0;
	m_bAnimating = false;
	m_y = -360 + 35;
	m_tbbg.tex = texture[TEXID_BUTTONS_BG];
}

void glToolbar::setExpanded( bool b, int duration /*= 12*/ )
{
	m_bAnimating = (duration > 0) && (b != m_bExpanded);
	m_bExpanded = b;
	
	if (m_bExpanded)
	{
		m_btns[0].setTexture(texture[TEXID_BUTTONS]);
		if (m_bAnimating)
			moveTo(480/2 - 235, -360 + 35, duration);
		else
			setPos(480/2 - 235, -360 + 35);
	}
	else
	{
		m_btns[0].setTexture(texture[TEXID_BUTTONS+5]);
		if (m_bAnimating)
			moveTo(480/2 - 235 + 410, -360 + 35, duration);
		else
			setPos(480/2 - 235 + 410, -360 + 35);
	}

}

void glToolbar::setPos( int x, int y )
{
	m_bAnimating = false;
	m_x = x;
	m_y = y;
	crrtFrame = 0;
	animateFrames = 0;
	for (int i = 0; i < 5; i++)
	{
		m_btns[i].setPos(m_x - 250 + 474/5/2 + 474/5*i, y);
	}
}

void glToolbar::moveTo( int x, int y, int totalFarme /*= 30*/ )
{
	crrtFrame = 0;
	animateFrames = totalFarme;
	m_dx = x - m_x;
	m_bAnimating = true;
	for (int i = 0; i < 5; i++)
	{
		int duration;
		if (m_bExpanded)
			duration = totalFarme + 1*i;
		else
			duration = totalFarme;
		
		//474为toolbar宽度
		m_btns[i].moveTo(x - 250 + 474/5/2 + 474/5*i, y, duration);
	}
}
