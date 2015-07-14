#include "StdAfx.h"
#include "VideoStreamShow.h"
#include "JpegFile.h"

extern Video_Parame g_VideoSystemParamter;
//constructor
CVideoStreamShow::CVideoStreamShow(void)
{
	m_TotalChannel = 0;
	m_TotalDSPs = 0;
}
CVideoStreamShow::~CVideoStreamShow(void)
{
	VideoSystemExit();
}

Mat  CVideoStreamShow::GetFrame(int index,BYTE *DataBuffer)
{
 char szFile[256]="BGR";//此数组只要非空即可
	ULONG size=1024*1024;
	int result;

	if(TempBuffer)
	{
	  result = GetOriginalImage(m_hDSP[index],DataBuffer,&size);

	  if(result == 0)
	  {
		  ConvertYUVToJpgFile(szFile,DataBuffer);
	  }
	  else
	  {
		  AfxMessageBox(_T("Please wait a moment！"));
	  }

	  delete [] DataBuffer;
	  TempBuffer = NULL;
	}

	cv::Mat frame = cv::imread(szFile);
	return frame;
}
int  CVideoStreamShow::ConvertYUVToJpgFile(char *pszFile, BYTE *pSource)
{
	BOOL bSize = 0;
	if(pSource!= NULL )
	{
		int nWidth,nHeight;
		nWidth=*(short *)pSource;
		nHeight=*(short *)(pSource+2);

		BYTE *buf = new BYTE [3*(nWidth*nHeight)];
		int nSize = 3*nWidth*nHeight;
		BYTE *source = pSource+4;
		BYTE *dest1 = buf+nSize-nWidth*3;
		BYTE *linstart = source;
		if(buf)
		{
			int i,j;
			double R, G, B, y1,u1,v1,y2,u2,v2;
			for (i=0; i<nHeight; i++)
			{

				for (j=0; j<nWidth/2; j++)
				{
					y1 = source[0];
					y2 = source[2];
					v1 = source[3]-128;
					v2 = source[3]-128;
					u1 = source[1]-128;
					u2 = source[1]-128;

					R =(double)(y1 + 1.375 * v1);
					G =(double)(y1 - 0.34375 * u1 - 0.703125 * v1);
					B =(double)(y1 + 1.734375 * u1);
					R = max (0, min (255, R));
					G = max (0, min (255, G));
					B = max (0, min (255, B));

					dest1[0] = (BYTE)B;
					dest1[1] = (BYTE)G;
					dest1[2] = (BYTE)R;
					dest1+=3;
					R =(double)(y2 + 1.375 * v1);
					G =(double)(y2 - 0.34375 * u1 - 0.703125 * v1);
					B =(double)(y2 + 1.734375 * u1);
					R = max (0, min (255, R));
					G = max (0, min (255, G));
					B = max (0, min (255, B));

					dest1[0] = (BYTE)B;
					dest1[1] = (BYTE)G;
					dest1[2] = (BYTE)R;
					dest1+=3;
					source += 4;
				}
				dest1 -= 2*nWidth*3;

			}
            CJpegFile *pJpgFile  = new CJpegFile();
			if(pJpgFile)
			{
				pJpgFile->VertFlipBuf(buf,nWidth*3,nHeight);
				pJpgFile->BGRFromRGB(buf, nWidth, nHeight);
				bSize = pJpgFile->RGBToJpegFile(pszFile,buf,nWidth,nHeight,TRUE,75);

				delete pJpgFile;
				pJpgFile = NULL;
			}
			delete [] buf;
			buf = NULL;
	  }
	}

	return bSize;
}
/*bool CVideoStreamShow::JudgeOK(int index)
{
	if(m_hDSP[index]!= (HANDLE) 0xffffffff)
	{
		ULONG size=1024*1024;
		BYTE *TempBuffer=new BYTE[size];
		int result;
		if(TempBuffer)
		{
			result = GetOriginalImage(m_hDSP[index],TempBuffer,&size);
		}
		return result;
	 }
}

*/
