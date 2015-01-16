// PlaneDiag_Demo_MFCDlg.cpp : 实现文件

#include "stdafx.h"
#include "PlaneDiag_Demo_MFC.h"
#include "PlaneDiag_Demo_MFCDlg.h"
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "cuda_converter.h"
BYTE * pt = NULL;

HWND ViewHWND;//*HWND表示窗口句柄*

HDC   hDC;//*MFC中的设备上下文句柄 编辑本义项,hDC是DC的句柄,API中的一个类似指针的数据类型*?
CDC* pDC;
CBrush *pBrush;
CBrush *pOldBrush;//选入设备描述表

//图像通道开关
int viIsValid;


int switchflag = 0;

/////////////////////////////////////////////////////////////////////////////////////////////
int viW = 720;  //1280; //1920;
int viH = 576;  //1080;
int vioffsetx = 5;    //250;
int vioffsety = 50;

////////////////////////////////////////////////////1///////////////////////////////////////

char viFilePath[256] = "D:\\demo1010\\fog7\\";
int viMaxFrameNum = 1;


/////////////////////////////////////////////////////////////////////////////////////////////

FILE *testhandle;
FILE *testhandler;

unsigned char *HImage;


BOOL SetTimeOut(UINT uTimeOut, int TimerInDex);//*设置定时器*
BOOL KillTimeOut(int TimerInDex);
UINT m_nTimerID[2000];

BOOL SetTimeOut(UINT uTimeOut, int TimerInDex)
{
	m_nTimerID[TimerInDex] = SetTimer(ViewHWND, TimerInDex, uTimeOut, 0);
	return m_nTimerID[TimerInDex];
}

BOOL KillTimeOut(int TimerInDex)
{

	KillTimer(ViewHWND, m_nTimerID[TimerInDex]);
	m_nTimerID[TimerInDex] = 0;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
///////////////////////////////////////////////////////////////////////////////////////////

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()//*向类中添加消息映射必要的结构体和函数声明*?
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)//*消息映射,为每个消息处理函数加入一个入口*
END_MESSAGE_MAP()//*宏结束消息映射*


// CPlaneDiag_Demo_MFCDlg 对话框


CPlaneDiag_Demo_MFCDlg::CPlaneDiag_Demo_MFCDlg(CWnd* pParent /*=NULL*/)
: CDialog(CPlaneDiag_Demo_MFCDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CPlaneDiag_Demo_MFCDlg::~CPlaneDiag_Demo_MFCDlg()
{

}

void CPlaneDiag_Demo_MFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_nSec);
}

BEGIN_MESSAGE_MAP(CPlaneDiag_Demo_MFCDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CPlaneDiag_Demo_MFCDlg::OnBnClickedConfigDiag)
	ON_BN_CLICKED(IDC_BUTTON2, &CPlaneDiag_Demo_MFCDlg::OnBnClickedStartDiag)
	ON_BN_CLICKED(IDC_BUTTON3, &CPlaneDiag_Demo_MFCDlg::OnBnClickedStopDiag)
	ON_BN_CLICKED(IDC_BUTTON4, &CPlaneDiag_Demo_MFCDlg::OnBnClickedInitDiag)
	//ON_BN_CLICKED(IDC_BUTTON5, &CPlaneDiag_Demo_MFCDlg::OnBnClickedButton5)//*窗口定义用*
	ON_WM_TIMER()

	ON_EN_CHANGE(IDC_EDIT1, &CPlaneDiag_Demo_MFCDlg::OnEnChangeEdit1)
END_MESSAGE_MAP()


// CPlaneDiag_Demo_MFCDlg 消息处理程序

BOOL CPlaneDiag_Demo_MFCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);//*允许应用程序为复制或修改而访问窗口菜单*?
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, FALSE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_SHOWMAXIMIZED);//*设置指定窗口的显示状态,窗口最大化*。

	ViewHWND = this->GetSafeHwnd();

	pDC = GetDC();//*检索整个屏幕的设备上下文环境*
	hDC = pDC->GetSafeHdc();//*返回输出设备上下文的句柄*

	FrameIndex = 0;
	viFramenum = 0;


	//开始诊断接口               

	// 帧计数初始设置

	Sleep(20);
	m_nSec = 1;
	UpdateData(FALSE);//*拷贝变量值到控件显示*

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CPlaneDiag_Demo_MFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPlaneDiag_Demo_MFCDlg::OnPaint()
{
	if (IsIconic())//*确定是否是最小化（图标化）的窗口,未最小化返回值零已最小化非零*
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);//*系统缺省的图标的高度和宽度*
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);//*此函数的执行？*
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CPlaneDiag_Demo_MFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPlaneDiag_Demo_MFCDlg::OnBnClickedConfigDiag()
{

}

void CPlaneDiag_Demo_MFCDlg::OnDestroy()
{

	::ReleaseDC(GetSafeHwnd(), hDC);//释放设备上下文环境（DC）供其他应用程序使用

	_CrtDumpMemoryLeaks();

	CDialog::OnDestroy();
}

BOOL CPlaneDiag_Demo_MFCDlg::DestroyWindow()
{
	return CDialog::DestroyWindow();
}

void CPlaneDiag_Demo_MFCDlg::OnBnClickedInitDiag()
{
	//DLL初始化接口
	viIsValid = 1;
	m_nSec = 0;
	
	curVisual = new BYTE[viW*viH * 3];
	EdgeVisual = new BYTE[viW*viH * 3];
	testhandle = fopen("D:\\time.log", "w");
	testhandler = fopen("D:\\timer.log", "w");
	UpdateData(FALSE);


}

void CPlaneDiag_Demo_MFCDlg::OnBnClickedStopDiag()
{
	int result;


	result = 0;
	delete[] curVisual;//先释放掉
	delete[] EdgeVisual;
	if (testhandle)
		fclose(testhandle);
	if (testhandler)
		fclose(testhandler);
	KillTimeOut(1);              /////////////////直接改成suspend线程或sleep
	Sleep(10);

	if (result == 1)
		m_nSec = 101010;
	else
		m_nSec = 3;

	UpdateData(FALSE);

}

void CPlaneDiag_Demo_MFCDlg::OnBnClickedStartDiag()
{

	if (viIsValid)
		GetVIData(viFramenum);

	SetTimeOut(10, 1);/////////////改成启动线程
}

void CPlaneDiag_Demo_MFCDlg::OnTimer(UINT_PTR nIDEvent)///////////////
{
	//int ret;
	GetEdge(curVisual, EdgeVisual, CurveBlob, 1,1);
	DisplayImage(viIsValid);


	if (viIsValid)
	{
		viFramenum = viFramenum + 1;
		if (viFramenum > viMaxFrameNum)
			viFramenum = 0;
		GetVIData(viFramenum);
		GetEdge(curVisual, EdgeVisual, CurveBlob, 1,1);
		m_nSec = viFramenum;
		UpdateData(FALSE);
	}


	CDialog::OnTimer(nIDEvent);
}

void CPlaneDiag_Demo_MFCDlg::DisplayImage(int viIsValid)//*显示图片*
{
	char         m_chBmpBuf1[2048];
	BITMAPINFO  *viBmpInfo;
	COLORREF     color;//*COLORREF类型用来描绘一个RGB颜色
	BOOL         bSuccess;
	char         text_char[24] = "";

	if (viIsValid)
	{
		//可见光显示
		viBmpInfo = (BITMAPINFO *)m_chBmpBuf1;
		viBmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		viBmpInfo->bmiHeader.biWidth = viW; //1024;
		viBmpInfo->bmiHeader.biHeight = viH; //768;
		viBmpInfo->bmiHeader.biPlanes = 1;
		viBmpInfo->bmiHeader.biBitCount = 24;
		viBmpInfo->bmiHeader.biCompression = BI_RGB;
		viBmpInfo->bmiHeader.biSizeImage = viW*viH * 3; //1024*768*3;
		viBmpInfo->bmiHeader.biXPelsPerMeter = 0;
		viBmpInfo->bmiHeader.biYPelsPerMeter = 0;
		viBmpInfo->bmiHeader.biClrUsed = 0;
		viBmpInfo->bmiHeader.biClrImportant = 0;

		bSuccess = StretchDIBits(hDC,                 // hDC
			vioffsetx,           // DestX
			vioffsety,           // DestY
			viW,                 // nDestWidth
			viH,                 // nDestHeight
			0,                   // SrcX
			0,                   // SrcY
			viW,                 // wSrcWidth
			viH,                 // wSrcHeight
			curVisual,           // lpBit
			viBmpInfo,           // lpBitsInfo
			DIB_RGB_COLORS,      // wUsage
			SRCCOPY);            // dwROP


		bSuccess = StretchDIBits(hDC,                 // hDC
			vioffsetx + viW,           // DestX
			vioffsety,           // DestY
			viW,                 // nDestWidth
			viH,                 // nDestHeight
			0,                   // SrcX
			0,                   // SrcY
			viW,                 // wSrcWidth
			viH,                 // wSrcHeight
			EdgeVisual,           // lpBit
			viBmpInfo,           // lpBitsInfo
			DIB_RGB_COLORS,      // wUsage
			SRCCOPY);            // dwROP



	}
}


void CPlaneDiag_Demo_MFCDlg::DrawRect(int offsetx, int offsety, RECT rect, COLORREF color, int h, int w, CString& text)
{
	int x, y;
	int minx, miny;
	int maxx, maxy;
	POINT point;

	minx = offsetx + rect.left;
	maxy = h - 1 - rect.top;    //
	maxy = offsety + maxy;
	maxx = offsetx + rect.right;
	miny = h - 1 - rect.bottom;//
	miny = offsety + miny;
	// pDC = GetDC();
	y = miny;
	for (x = minx; x < maxx; x++)
	{
		point.x = x;
		point.y = y;
		pDC->SetPixel(point, color);
	}

	y = maxy;
	for (x = minx; x < maxx; x++)
	{
		point.x = x;
		point.y = y;
		pDC->SetPixel(point, color);
	}

	x = minx;
	for (y = miny; y < maxy; y++)
	{
		point.x = x;
		point.y = y;
		pDC->SetPixel(point, color);
	}
	x = maxx;
	for (y = miny; y < maxy; y++)
	{
		point.x = x;
		point.y = y;
		pDC->SetPixel(point, color);
	}
	pDC->TextOut(maxx, maxy, text);

}

void CPlaneDiag_Demo_MFCDlg::DrawParts(int offsetx, int offsety, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, COLORREF color)
{
	int x, y;
	int minx, miny;
	int maxx, maxy;
	POINT point;

	x1 = x1 + offsetx;
	y1 = y1 + offsety;
	//y1 = viH-1-y1;

	x2 = x2 + offsetx;
	y2 = y2 + offsety;
	//y2 = viH-1-y2;

	x3 = x3 + offsetx;
	y3 = y3 + offsety;
	// y3 = viH-1-y3;

	x4 = x4 + offsetx;
	y4 = y4 + offsety;
	// y4 = viH-1-y4;
	//pDC = GetDC();
	CPen*   pOldPen;
	int   penWidth = 1;
	CPen   pen(PS_SOLID, penWidth, color);
	//CPen*   pOldPen;  
	pOldPen = pDC->SelectObject(&pen);
	//line1
	//pDC = GetDC();
	point.x = x1;
	point.y = y1;
	pDC->SetPixel(point, color);

	pDC->MoveTo(x1, y1);
	pDC->LineTo(x2, y2);

	//line2
	point.x = x2;
	point.y = y2;
	pDC->SetPixel(point, color);

	pDC->MoveTo(x2, y2);
	pDC->LineTo(x3, y3);

	//line3
	point.x = x3;
	point.y = y3;
	pDC->SetPixel(point, color);

	pDC->MoveTo(x3, y3);
	pDC->LineTo(x4, y4);

	//line4
	point.x = x4;
	point.y = y4;
	pDC->SetPixel(point, color);

	pDC->MoveTo(x4, y4);
	pDC->LineTo(x1, y1);

	pDC->SelectObject(&pOldPen);//
}

void CPlaneDiag_Demo_MFCDlg::GetVIData(int viFrameIndex)
{

	vi.ImageHeight = viH;
	vi.ImageWidth = viW;
	VisualImage = new MyBitmap();
	strcpy(VisualPath, viFilePath);

	_itoa(viFrameIndex, framestr, 10);//将整形转换为字符串 int--->char*
	strcpy(VisualPath1, VisualPath);
	strcat(VisualPath1, framestr);
	strcat(VisualPath1, ".bmp");

	VisualImage->OpenBitmap(VisualPath1, vi);

	int VIsize = VisualImage->m_lpInfoHeader->biSizeImage;
	memcpy(curVisual, VisualImage->m_pBits, VIsize);


	delete VisualImage;
}


void CPlaneDiag_Demo_MFCDlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

void CPlaneDiag_Demo_MFCDlg::GetEdge(unsigned char *curVisual, unsigned char *Edgevisual, LineAttributeBlob *CurveBlob, int BlackorWhite, int gpuorcpu)
{
	int m_width = viW;
	int m_height = viH;
	Edgeanalysis *m_edgeanalysis = new Edgeanalysis;
	m_edgeanalysis->init(viW, viH);
	int msize = viW*viH;

	double timeused;           //初始化GPU端计时值
	unsigned char *maskimage = new unsigned char[msize];
	unsigned char *Y = new unsigned char[msize];
	unsigned char r, g, b;
	//memset(maskimage, 0, msize*sizeof(unsigned char));

	const int cyb = int(0.114 * 219 / 255 * 65536 + 0.5);
	const int cyg = int(0.587 * 219 / 255 * 65536 + 0.5);
	const int cyr = int(0.299 * 219 / 255 * 65536 + 0.5);

	unsigned char *rr = new unsigned char[msize];
	unsigned char *gg = new unsigned char[msize];
	unsigned char *bb = new unsigned char[msize];

	for (int j = 0; j < m_height; j++)
	{
		for (int i = 0; i < m_width; i++)
		{
			r = curVisual[3 * j*m_width + 3 * i + 2];
			g = curVisual[3 * j*m_width + 3 * i + 1];
			b = curVisual[3 * j*m_width + 3 * i + 0];

			rr[j*m_width + i] = r;
			gg[j*m_width + i] = g;
			bb[j*m_width + i] = b;
			Y[j*m_width + i] = ((cyr*r + cyg*g + cyb*b + 0x10800) >> 16);
		}
	}

	memset(maskimage, 0, msize*sizeof(unsigned char));
	timeused = m_edgeanalysis->Bottom_Perceptual(Y, CurveBlob, maskimage, BlackorWhite, gpuorcpu,viW, viH);
	if (gpuorcpu)
	{
		fprintf(testhandle, "gpu time:%f\n", timeused);
	}
	else
	{
		fprintf(testhandler, "cpu time:%f\n", timeused);
	}
	for (int i = 0; i<msize; i++)
	{
		if (maskimage[i] == 1)
		{
			Edgevisual[3 * i + 2] = 255;
			Edgevisual[3 * i + 1] = 0;
			Edgevisual[3 * i + 0] = 0;
		}
		else if (maskimage[i] == 2)
		{
			Edgevisual[3 * i + 2] = 0;
			Edgevisual[3 * i + 1] = 255;
			Edgevisual[3 * i + 0] = 0;
		}
		else if (maskimage[i] == 3)
		{
			Edgevisual[3 * i + 2] = 0;
			Edgevisual[3 * i + 1] = 0;
			Edgevisual[3 * i + 0] = 255;
		}
		else
		{
			Edgevisual[3 * i + 2] = 255;//rr[i]; //255;
			Edgevisual[3 * i + 1] = 255;//gg[i]; //255;
			Edgevisual[3 * i + 0] = 255;//bb[i]; //255;
		}
	}
	delete[]maskimage;
	delete rr;
	delete gg;
	delete bb;
	delete m_edgeanalysis;
	delete[]Y;

}


