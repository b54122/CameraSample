
// MVSTDlg.h : 头文件
#pragma once
//--------------------------------------------------------------
#include "VideoStreamShow.h"
#include "afxwin.h"
#include "CvvImage.h"

using namespace cv;
//--------------------------------------------------------------
// CSHUVideoPanoDlg 对话框
class VideoProc
{
public:
	VideoProc(CWnd* pParent = NULL);	// 标准构造函数

	enum { IDD = IDD_MVST_DIALOG }; // 对话框数据
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	//-----------------------------------------------------------------------------------------
public:
	CStatic *psub_VideoView[MAX_VIDEO_CHANNEL]; //显示静态文本类指针,用于显示视频
	CRect m_rect[MAX_VIDEO_CHANNEL];//显示视频的对象区域
	int m_Current_1;
	int m_Current_2;
	int m_Current_3;
	int m_Current_4;
	bool stop;

	HANDLE RealtimeHandle;
	HANDLE OfflineHandle;
	HANDLE CreateVideoHandle;

public:
	void  Set_Default_Config_Setting(void);//默认视频参数设定函数声明；
	void  DrawPicToHDC(IplImage *img, UINT ID);//OpenCV获取的视频在窗体中显示的函数声明
	BOOL  PreTranslateMessage(MSG* pMsg);

	void  PanoramaStiching(const Mat img_1, const Mat img_2); //存储预览全景图有关参数函数声明
	Mat   Blending( const Mat img_1, const Mat img_2, Mat imageturn , Mat H, //声明视频融合函数
	                int width, float inlier_minRx, float origin_x, int height, int drift);
protected:
	HICON m_hIcon;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	HRESULT OnNotifyIcon(WPARAM wParam,LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	//-----------------------------------------------------------------
	CStatic m_Wnd_View_CH1;//声明静态文本框类变量
	CStatic m_Wnd_View_CH2;
	CStatic m_Wnd_View_CH3;
	CStatic m_Wnd_View_CH4;
	CStatic m_Wnd_View_CH5;//用于显示全景图

	//-------------------------------------------------------------------
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnBnClickedButtonRealtime();
	afx_msg void OnBnClickedButtonOutline();
	afx_msg void OnBnClickedButtonCreatevideo();
	afx_msg void OnStnDblclickImageshowCH5();

private:
	CToolTipCtrl m_Mytip;
};
