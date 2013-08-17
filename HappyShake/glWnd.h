#ifndef _glWnd_h_
#define _glWnd_h_

// Include MZFC header file
#include <mzfc_inc.h>
#include <gles2/gl2.h>
#include <EGL/egl.h>
#include <list>

using namespace std;

#define TEXTURENUM			10
//背景纹理id
#define TEXID_BG			0
//设置纹理id
#define TEXID_SET			1
//选中时的设置纹理id
#define TEXID_SET_SELECT	2
//
#define	TEXID_BUTTONS_BG	3
#define	TEXID_BUTTONS		4
//默认的摩擦力 0.98
#define DEFINE_FF			0.98
#define DEFINE_K			0.25

#define MAX_ACC_F			80
#define MIN_ACC_F			10
#define ACC_F_RATE			0.6

#define TIMER_DRAW  1
#define TIMER_PTS	2
#define TIMER_ACC	3

#define MAX_BUBBLES			5
#define DEFINE_EDITER_R		100
#define MAX_BUBBLE_R		200
#define MIN_BUBBLE_R		25
#define BUBBLE_ZOOM_STEP	5

#define WM_USER_EDIT_STATE_CHANGE		WM_USER + 1
#define WM_IDC_GLBUTTON					108

//分块数,x:y = 2:3
static const int columnPieces = 30;
static const int rowPieces = columnPieces*1.5;

enum EditBubbleState {
	ST_EDIT_ONSHOW = 0, 
	ST_EDIT_SHOW, 
	ST_EDIT_ONHIDE, 
	ST_NORMAL, 
	ST_ONREMOVE, 
	ST_REMOVED
};

struct Vertex {
	GLint x;
	GLint y;
	GLint z;
};

struct Texture {
	GLint tex;
	GLint w;
	GLint h;
};

struct Bubble {
	Bubble (){reset();}
	
	void reset();
	void show(int totalFarme = 10);
	void hide(int totalFarme = 10);
	void remove (int totalFarme = 10);
	void update();
	void setPos(int x, int y);
	//原始坐标
	GLint ox;
	GLint oy;
	//偏移后的坐标(相对值)
	GLint mx;
	GLint my;
	//顶点受外力
	GLfloat fx, fy;
	//顶点速度
	GLfloat vx;
	GLfloat vy;
	//半径
	GLint r;

	bool bSelect;
	//编辑状态
	EditBubbleState eState;
	GLfloat		zoomRate;
	int			crrtFrame;
	int			animateFrames;
};

enum ButtonState{
	BTN_NORMAL,
	BTN_MOVEING
};

struct glButton
{
	HWND	m_hWnd;
	int		m_tex;
	int		m_id;
	int		m_x;
	int		m_y;
	int		m_r;
	int		m_dx;
	int		m_dy;
	int		m_dr;
	bool	m_bSelect;
	bool	m_bShow;

	int			crrtFrame;
	int			animateFrames;
	ButtonState	m_state;
	float		m_rate;
	
	glButton();
	void setTexture(int texEnable, int r = 25);
	void setWnd(HWND hWnd, int id);

	void setPos(int x, int y);
	void moveTo(int x, int y, int totalFarme = 10);
	bool isInButton(int x, int y);
	void OnLButtonDown(UINT fwKeys, int xPos, int yPos);
	void OnLButtonUp(UINT fwKeys, int xPos, int yPos);
	int getR();
	int getX();
	int getY();
	void setVisible(bool b){m_bShow = b;};
	void draw();
};

struct glToolbar {
	// glToolbar
	glButton	m_btns[5];
	Texture	m_tbbg;	
	HWND	m_hWnd;
	int		m_x;
	int		m_y;
	int		m_dx;
	int		crrtFrame;
	int		animateFrames;
	float	m_rate;
	bool	m_bExpanded;
	bool	m_bAnimating;

	glToolbar();
	void init(HWND h);
	void OnLButtonDown(UINT fwKeys, int xPos, int yPos);
	void OnLButtonUp(UINT fwKeys, int xPos, int yPos);
	void moveTo(int x, int y, int totalFarme = 10);
	void setPos(int x, int y);
	void setExpanded(bool b, int duration = 12);
	void draw();
};

class CHappyShakeMainWnd;

// Application main form class derived rom CMzWndEx
class CglWnd: public CMzWndEx
{
	MZ_DECLARE_DYNAMIC(CglWnd);
public:
	CglWnd(void);
	~CglWnd(void);

	UINT m_allkeyEvent;

	void setParentWnd(CHappyShakeMainWnd *pWnd);

	void showEdit(bool bedit);

	void addEditBubble(int x = 0, int y = 0, int r = DEFINE_EDITER_R);
	bool canAdd(){return m_activeBubbles < MAX_BUBBLES;};
	void removeEditBubble();
	bool canRemove();
	void checkCanEdit();
	void zoomCrrtSelect(bool bIn);
	void removeAllBubbles();

	void setActive(bool bActive);
	bool isActive(){return m_bActive;};

	bool setImage(CMzString &filename);
	bool setImageFromDC(HDC dc, int width = 480, int height = 720);
	CMzString& getImageFileName(){return m_imageFileName;};
	
	bool getBubbleSet(int nIndex, int &x, int &y, int &r);

	int getActiveBubbleCount();

	int caclSelectBubble(int x, int y);
	bool selectImageFile();

private:

	GLint viewHeight;
	GLint viewWidth;

	GLfloat m_texCoord[(columnPieces+1)*(rowPieces+1)*2];
	GLushort m_indices[columnPieces*rowPieces*3*2]; //三角形顶点索引

	//原始坐标
	GLint m_oTexVertex[(columnPieces+1)*(rowPieces+1)*3];
	//需要绘制的坐标
	GLint m_dTexVertex[(columnPieces+1)*(rowPieces+1)*3];
	int	m_pieceSize;

	//EGL Units
	EGLDisplay	m_Display;
	EGLSurface	m_Surface;
	HDC			m_hdcWnd;

	CHappyShakeMainWnd *m_parent;

	bool	m_bEdit;
	GLfloat	m_ff; //摩擦力 速度衰减
	GLfloat	m_k; //弹力系数
	signed char m_LastXAxis ;
	signed char m_LastYAxis;

	Bubble	m_bubbles[MAX_BUBBLES];
	int		m_activeBubbles;
	bool	m_bActive;
	//上次鼠标坐标
	int		m_lastPosX;
	int		m_lastPosY;

	CMzString	m_imageFileName;

	glToolbar	m_toolbar;


private:
	bool initEGL();
	int initGL();
	void releaseGL();
	void loadTextures();
	int drawGL();
	void ReSizeGLScene(GLsizei width, GLsizei height);
	void makeMainTexCoord();
	void resetMainTexVertex();
	void makeMainTexIndices();

	void updateBubbleVertex(Bubble *b);
	void updateBubblePos(Bubble *b);
	void updateBubbles();
	//设置顶点所受外力
	void updateBubblesF();

protected:
	// Initialize main form
	virtual BOOL OnInitDialog();
	LRESULT MzDefWndProc(UINT message, WPARAM wParam, LPARAM lParam);

	// Over write MZFC's OnMzCommand function to dispose command notify
	//virtual void OnMzCommand(WPARAM wParam, LPARAM lParam);

	//     void OnCommond(WORD wNotifyCode, WORD wID, HWND hwndctl);
	//   
	//void OnLButtonDblClk(UINT fwKeys, int xPos, int yPos);
	void OnLButtonDown(UINT fwKeys, int xPos, int yPos);
	void OnLButtonUp(UINT fwKeys, int xPos, int yPos);

	void OnMouseMove(UINT fwKeys, int xPos, int yPos);
	//   
	//     void CglWnd::OnPaint(HDC hdc, LPPAINTSTRUCT ps);
	//   
	//     void CglWnd::OnSettingChange(DWORD wFlag, LPCTSTR pszSectionName);
	//   
	//     int CglWnd::OnShellHomeKey(UINT message, WPARAM wParam, LPARAM lParam);
	//   
	void OnSize(int nWidth, int nHeight); 
	void OnTimer(UINT_PTR nIDEvent);
};

void GetWorkPath(TCHAR szPath[], int nSize);

#endif // _glWnd_h_