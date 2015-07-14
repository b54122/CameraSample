
// MVSTDlg.h : ͷ�ļ�
#pragma once
//--------------------------------------------------------------
#include "VideoStreamShow.h"
#include "afxwin.h"
#include "CvvImage.h"

using namespace cv;
//--------------------------------------------------------------
// CSHUVideoPanoDlg �Ի���
class VideoProc
{
public:
	VideoProc(CWnd* pParent = NULL);	// ��׼���캯��

	enum { IDD = IDD_MVST_DIALOG }; // �Ի�������
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
	//-----------------------------------------------------------------------------------------
public:
	CStatic *psub_VideoView[MAX_VIDEO_CHANNEL]; //��ʾ��̬�ı���ָ��,������ʾ��Ƶ
	CRect m_rect[MAX_VIDEO_CHANNEL];//��ʾ��Ƶ�Ķ�������
	int m_Current_1;
	int m_Current_2;
	int m_Current_3;
	int m_Current_4;
	bool stop;

	HANDLE RealtimeHandle;
	HANDLE OfflineHandle;
	HANDLE CreateVideoHandle;

public:
	void  Set_Default_Config_Setting(void);//Ĭ����Ƶ�����趨����������
	void  DrawPicToHDC(IplImage *img, UINT ID);//OpenCV��ȡ����Ƶ�ڴ�������ʾ�ĺ�������
	BOOL  PreTranslateMessage(MSG* pMsg);

	void  PanoramaStiching(const Mat img_1, const Mat img_2); //�洢Ԥ��ȫ��ͼ�йز�����������
	Mat   Blending( const Mat img_1, const Mat img_2, Mat imageturn , Mat H, //������Ƶ�ںϺ���
	                int width, float inlier_minRx, float origin_x, int height, int drift);
protected:
	HICON m_hIcon;
	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	HRESULT OnNotifyIcon(WPARAM wParam,LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	//-----------------------------------------------------------------
	CStatic m_Wnd_View_CH1;//������̬�ı��������
	CStatic m_Wnd_View_CH2;
	CStatic m_Wnd_View_CH3;
	CStatic m_Wnd_View_CH4;
	CStatic m_Wnd_View_CH5;//������ʾȫ��ͼ

	//-------------------------------------------------------------------
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnBnClickedButtonRealtime();
	afx_msg void OnBnClickedButtonOutline();
	afx_msg void OnBnClickedButtonCreatevideo();
	afx_msg void OnStnDblclickImageshowCH5();

private:
	CToolTipCtrl m_Mytip;
};
