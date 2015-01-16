// PlaneDiag_Demo_MFCDlg.h : 头文件
//

#pragma once

#include "Bitmap.h"
#include "EdgeAnalysis.h"

// CPlaneDiag_Demo_MFCDlg 对话框
class CPlaneDiag_Demo_MFCDlg : public CDialog
{
	// 构造
public:
	CPlaneDiag_Demo_MFCDlg(CWnd* pParent = NULL);	// 标准构造函数
	virtual ~CPlaneDiag_Demo_MFCDlg();

	// 对话框数据
	enum { IDD = IDD_PLANEDIAG_DEMO_MFC_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持



	// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedConfigDiag();
	afx_msg void OnBnClickedStartDiag();
	afx_msg void OnBnClickedStopDiag();
	afx_msg void OnBnClickedInitDiag();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnDestroy();
	virtual BOOL DestroyWindow();

	void DisplayImage(int viIsValid);

	ImageDefine vi;

	MyBitmap   *VisualImage;
	char	 framestr[20];
	int      ImageNumber;

	int      FrameIndex;
	int      viFramenum;

	char     VisualPath[1024];
	char     VisualPath1[1024];

	unsigned char *curVisual;
	unsigned char *grayVisual;
	unsigned char *EdgeVisual;
	LineAttributeBlob  CurveBlob[MAXBLOBNUMBER];

	int m_nSec;
	void GetVIData(int viFameIndex);

	void DrawRect(int offsetx, int offsety, RECT rect, COLORREF color, int h, int w, CString& text);
	void DrawParts(int offsetx, int offsety, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, COLORREF color);
	void GetEdge(unsigned char *curVisual, unsigned char *Edgevisual, LineAttributeBlob *CurveBlob, int BlackorWhite,int gpuorcpu);
public:
	afx_msg void OnEnChangeEdit1();
};
