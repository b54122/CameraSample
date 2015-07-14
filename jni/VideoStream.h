#pragma once
//----------------------------------------------------------------------
#include<opencv2\core\core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
//---------------------------------------------------------------------
using namespace cv;
class CVideoStream
{
public:

	int  ConvertYUVToJpgFile(char *pszFile, BYTE *pSource);
	bool JudgeOK(int index);
	//-----------------------
	void StartVideoDisplay(int index,HWND hwnd,RECT *rect);
	void StopVideoDisplay(int index);
	Mat GetFrame(int index);
	//------------------------
	HANDLE m_hDSP[MAX_VIDEO_CHANNEL];
	int m_TotalDSPs;
	int m_TotalChannel;
	int m_ShowIndex[MAX_VIDEO_CHANNEL];
	CVideoStreamShow(void);
	~CVideoStreamShow(void);
};

