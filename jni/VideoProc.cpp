#include <VideoProc.h>
#include <opencv2/core/core.hpp>
#include <string>
#include <vector>
using namespace cv;
using namespace std;

#define LOG_TAG "RealTimeVideoMosaicing"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

//全景拼接获取预览图像函数，把满意拼接参数存储，便于以视频形式显示全景图像（节约时间开销）
class  VideoProc::PanoramaStiching(const Mat img_1, const Mat img_2)
{
	//opencv2.4.X版本特征匹配
	initModule_nonfree();//初始化模块，使用SIFT或SURF时用到
	Ptr<FeatureDetector> detector = FeatureDetector::create( "SURF" );//创建SIFT特征检测器，可改成SURF/ORB
	Ptr<DescriptorExtractor> descriptor_extractor = DescriptorExtractor::create( "SURF" );//创建特征向量生成器，可改成SURF/ORB
	Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create( "BruteForce" );//创建特征匹配器
	if( detector.empty() || descriptor_extractor.empty() )
	    AfxMessageBox(_T("Fail to create detector!"),MB_ICONWARNING);

	//Detect the keypoints using SURF Detector
	vector<KeyPoint> m_LeftKey,m_RightKey;
	detector->detect( img_1, m_LeftKey );
	detector->detect( img_2, m_RightKey );

	//Calculate descriptors (feature vectors)
	Mat descriptors1,descriptors2;
	descriptor_extractor->compute( img_1, m_LeftKey, descriptors1 );
	descriptor_extractor->compute( img_2, m_RightKey, descriptors2 );

	//Matching descriptor vectors
	vector<DMatch> matches;//matching result
	descriptor_matcher->match( descriptors1, descriptors2, matches );

	//Quick calculation of max and min distances between keypoints
	double max_dist = 0;
	double min_dist = 100;
	for(int i=0; i<matches.size(); i++)
	{
		double dist = matches[i].distance;
		if(dist < min_dist) min_dist = dist;
		if(dist > max_dist) max_dist = dist;
	}

	//Use only "good" matches (i.e. whose distance is less than 0.2*max_dist )
	vector<DMatch> goodMatches;
	for(int i=0; i < matches.size(); i++)
	{
		if(matches[i].distance < 0.2 * max_dist)
		{
			goodMatches.push_back(matches[i]);
		}
	}


	// allocate memories
	Mat p1( goodMatches.size(), 2, CV_32F);
	Mat p2( goodMatches.size(), 2, CV_32F);

	// convert Keypoint to Mat
	Point2f pt;
	for (int i = 0; i < goodMatches.size(); i++)
	{
		pt = m_LeftKey[goodMatches[i].queryIdx].pt;
		p1.at<float>(i, 0) = pt.x;
		p1.at<float>(i, 1) = pt.y;

		pt = m_RightKey[goodMatches[i].trainIdx].pt;
		p2.at<float>(i, 0) = pt.x;
		p2.at<float>(i, 1) = pt.y;
	}

	//calculate F with RANSAC method
	vector<uchar> m_RANSACStatus; //用于存储RANSAC后每个点的状态
	findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC);

	// calculate wild point
	int OutlinerCount = 0;
	for (int i =0; i < goodMatches.size(); i++)
	{
		if (m_RANSACStatus[i] == 0)    //匹配区域外的点
			OutlinerCount++;
	}
	int InlinerCount = goodMatches.size() - OutlinerCount;   //匹配区域内的点个数


	vector<Point2f> m_LeftInlier;  m_LeftInlier.resize(InlinerCount); //提取的关键点内点
	vector<Point2f> m_RightInlier;  m_RightInlier.resize(InlinerCount);
	vector<DMatch>  m_InlierMatches;  m_InlierMatches.resize(InlinerCount); //内点匹配

	InlinerCount = 0;
	float inlier_minRx = img_1.cols;

	for (int i = 0; i < goodMatches.size(); i++)
	{
		if (m_RANSACStatus[i] != 0)
		{
			m_LeftInlier[InlinerCount].x = p1.at<float>(i, 0);
			m_LeftInlier[InlinerCount].y = p1.at<float>(i, 1);
			m_RightInlier[InlinerCount].x = p2.at<float>(i, 0);
			m_RightInlier[InlinerCount].y = p2.at<float>(i, 1);
			m_InlierMatches[InlinerCount].queryIdx = InlinerCount;
			m_InlierMatches[InlinerCount].trainIdx = InlinerCount;

			if(m_RightInlier[InlinerCount].x < inlier_minRx)
				inlier_minRx = m_RightInlier[InlinerCount].x;   //存储内点中右图最小横坐标
			InlinerCount++;
		}
	}

	//存储RANSAC得到的单应矩阵
	Mat H = findHomography( m_LeftInlier, m_RightInlier, RANSAC );

	//存储左图四角，变换到右图位置
	vector<Point2f> obj_corners(4);
	vector<Point2f> scene_corners(4);

	obj_corners[0] = Point(0,0);
	obj_corners[1] = Point( img_1.cols, 0 );
	obj_corners[2] = Point( img_1.cols, img_1.rows );
	obj_corners[3] = Point( 0, img_1.rows );

	perspectiveTransform( obj_corners, scene_corners, H);

	int drift = scene_corners[1].x; //储存偏移量


	//存储配准后四角的位置
	int width = int(max (abs(scene_corners[1].x), abs(scene_corners[2].x) ));
	int height= img_1.rows;
	float origin_x = 0,origin_y = 0;
	if(scene_corners[0].x < 0)
	{
		if (scene_corners[3].x<0)
			origin_x += min(scene_corners[0].x,scene_corners[3].x);
		else
			origin_x += scene_corners[0].x;
	}
	width -= int(origin_x);
	if(scene_corners[0].y < 0)
	{
		if (scene_corners[1].y)
			origin_y += min(scene_corners[0].y,scene_corners[1].y);
		else
			origin_y += scene_corners[0].y;
	}
	Mat imageturn =  Mat::zeros(width,height,img_1.type());


	//获取新的变换矩阵，使图像完整显示
	for (int i = 0;i < 4;i++)
	   scene_corners[i].x -= origin_x;

	Mat H_Pano = getPerspectiveTransform(obj_corners, scene_corners);
	warpPerspective(img_1,imageturn,H_Pano,Size(width,height));

	//blending
	int width_ol = width - int(inlier_minRx - origin_x);
	int start_x = int(inlier_minRx - origin_x);

	uchar* ptr = imageturn.data;
	double alpha = 0, beta = 1;
	for (int row = 0;row < height;row++)
	{
		ptr = imageturn.data + row*imageturn.step + (start_x)*imageturn.elemSize();
		for(int col = 0;col < width_ol;col++)
		{
			uchar* ptr_c1 = ptr + imageturn.elemSize1();
			uchar* ptr_c2 = ptr_c1 + imageturn.elemSize1();
			uchar* ptr2 = img_2.data + row*img_2.step + (col + int(inlier_minRx))*img_2.elemSize();
			uchar* ptr2_c1 = ptr2 + img_2.elemSize1();
			uchar* ptr2_c2 = ptr2_c1 + img_2.elemSize1();

			alpha = double(col)/double(width_ol);
			beta = 1 - alpha;

			if (*ptr == 0 && *ptr_c1 == 0 && *ptr_c2 == 0)
			{
				*ptr = (*ptr2);
				*ptr_c1 = (*ptr2_c1);
				*ptr_c2 = (*ptr2_c2);
			}

			*ptr = (*ptr)*beta + (*ptr2)*alpha;
			*ptr_c1 = (*ptr_c1)*beta + (*ptr2_c1)*alpha;
			*ptr_c2 = (*ptr_c2)*beta + (*ptr2_c2)*alpha;

			ptr += imageturn.elemSize();
		}
	}


	Mat img_result = Mat::zeros(height,width + img_2.cols - drift, img_1.type());
	uchar* ptr_r = imageturn.data;

	for (int row = 0;row < height;row++) {
		ptr_r = img_result.data + row*img_result.step;

		for(int col = 0;col < imageturn.cols;col++)
		{
			uchar* ptr_rc1 = ptr_r + imageturn.elemSize1();
			uchar* ptr_rc2 = ptr_rc1 + imageturn.elemSize1();
			uchar* ptr = imageturn.data + row*imageturn.step + col*imageturn.elemSize();
			uchar* ptr_c1 = ptr + imageturn.elemSize1();
			uchar* ptr_c2 = ptr_c1 + imageturn.elemSize1();

			*ptr_r = *ptr;
			*ptr_rc1 = *ptr_c1;
			*ptr_rc2 = *ptr_c2;

			ptr_r += img_result.elemSize();
		}

		ptr_r = img_result.data + row*img_result.step + imageturn.cols*img_result.elemSize();
		for(int col = imageturn.cols;col < img_result.cols;col++)
		{
			uchar* ptr_rc1 = ptr_r + imageturn.elemSize1();
			uchar* ptr_rc2 = ptr_rc1 + imageturn.elemSize1();

			uchar* ptr2 = img_2.data + row*img_2.step + (col - imageturn.cols + drift)*img_2.elemSize();
			uchar* ptr2_c1 = ptr2 + img_2.elemSize1();
			uchar* ptr2_c2 = ptr2_c1 + img_2.elemSize1();

			*ptr_r = *ptr2;
			*ptr_rc1 = *ptr2_c1;
			*ptr_rc2 = *ptr2_c2;

			ptr_r += img_result.elemSize();
		}
	}

	ofstream outFile;
	outFile.open("Pano.txt");
	outFile << width <<" "<< inlier_minRx <<" "<< origin_x <<" "<< height <<" "<< drift << " ";
	outFile.close();

	FileStorage fs("Pano.xml", FileStorage::WRITE);
	fs << "H_Pano" << H_Pano;
	fs << "imageturn" << imageturn;
	fs.release();

	imshow("Panorama",img_result);
	waitKey();
}

//根据预览全景图获取的参数进行实时全景展示函数
Mat  VideoProc::Blending( const Mat img_1, const Mat img_2, Mat imageturn , Mat H, int width, float inlier_minRx, float origin_x, int height, int drift)
{
	IplImage* image=NULL;
	warpPerspective(img_1,imageturn,H,Size(width,height));
	//blending
	int width_ol = width - int(inlier_minRx - origin_x);
	int start_x = int(inlier_minRx - origin_x);

	uchar* ptr = imageturn.data;
	double alpha = 0, beta = 1;
	for (int row = 0;row < height;row++)
	{
		ptr = imageturn.data + row*imageturn.step + (start_x)*imageturn.elemSize();
		for(int col = 0;col < width_ol;col++)
		{
			uchar* ptr_c1 = ptr + imageturn.elemSize1();
			uchar* ptr_c2 = ptr_c1 + imageturn.elemSize1();
			uchar* ptr2 = img_2.data + row*img_2.step + (col + int(inlier_minRx))*img_2.elemSize();
			uchar* ptr2_c1 = ptr2 + img_2.elemSize1();
			uchar* ptr2_c2 = ptr2_c1 + img_2.elemSize1();

			alpha = double(col)/double(width_ol);
			beta = 1 - alpha;

			if (*ptr == 0 && *ptr_c1 == 0 && *ptr_c2 == 0)
			{
				*ptr = (*ptr2);
				*ptr_c1 = (*ptr2_c1);
				*ptr_c2 = (*ptr2_c2);
			}

			*ptr = (*ptr)*beta + (*ptr2)*alpha;
			*ptr_c1 = (*ptr_c1)*beta + (*ptr2_c1)*alpha;
			*ptr_c2 = (*ptr_c2)*beta + (*ptr2_c2)*alpha;

			ptr += imageturn.elemSize();
		}
	}


	Mat img_result = Mat::zeros(height,width + img_2.cols - drift, img_1.type());
	uchar* ptr_r = imageturn.data;

	for (int row = 0;row < height;row++) {
		ptr_r = img_result.data + row*img_result.step;

		for(int col = 0;col < imageturn.cols;col++)
		{
			uchar* ptr_rc1 = ptr_r + imageturn.elemSize1();
			uchar* ptr_rc2 = ptr_rc1 + imageturn.elemSize1();
			uchar* ptr = imageturn.data + row*imageturn.step + col*imageturn.elemSize();
			uchar* ptr_c1 = ptr + imageturn.elemSize1();
			uchar* ptr_c2 = ptr_c1 + imageturn.elemSize1();

			*ptr_r = *ptr;
			*ptr_rc1 = *ptr_c1;
			*ptr_rc2 = *ptr_c2;

			ptr_r += img_result.elemSize();
		}

		ptr_r = img_result.data + row*img_result.step + imageturn.cols*img_result.elemSize();
		for(int col = imageturn.cols;col < img_result.cols;col++)
		{
			uchar* ptr_rc1 = ptr_r + imageturn.elemSize1();
			uchar* ptr_rc2 = ptr_rc1 + imageturn.elemSize1();

			uchar* ptr2 = img_2.data + row*img_2.step + (col - imageturn.cols + drift)*img_2.elemSize();
			uchar* ptr2_c1 = ptr2 + img_2.elemSize1();
			uchar* ptr2_c2 = ptr2_c1 + img_2.elemSize1();

			*ptr_r = *ptr2;
			*ptr_rc1 = *ptr2_c1;
			*ptr_rc2 = *ptr2_c2;

			ptr_r += img_result.elemSize();
		}
	}

//    image = &IplImage(img_result);
//    DrawPicToHDC(image, IDC_ImageShow_CH_5);

	return img_result;
}

DWORD WINAPI ImageBuffer(LPVOID LpParameter)
{
	Mat img;
	for(int i = 0;i < 4;i++)
	{
		img = m_VideoStream.GetFrame(i);
		imgBuffer.push(img);
	}
	return TRUE;
}


DWORD WINAPI RealtimePanorama(LPVOID LpParameter)
{
	IplImage* image = NULL;
	CSHUVideoPanoDlg* MvstDlg = (CSHUVideoPanoDlg *)LpParameter;
	clock_t Start,Finish;
	double TotalTime;
	CString FrameInfo;

	FileStorage fs_1 , fs_2, fs_3;
	fs_1.open("1_2.xml", FileStorage::READ);
	fs_2.open("3_4.xml", FileStorage::READ);
	fs_3.open("Pano.xml", FileStorage::READ);

	Mat H_1, imageturn_1;
	fs_1["H_1"] >> H_1;
	fs_1["imageturn_1"] >> imageturn_1;

	Mat H_2, imageturn_2;
	fs_2["H_2"] >> H_2;
	fs_2["imageturn_2"] >> imageturn_2;

	Mat H_Pano, imageturn;
	fs_3["H_Pano"] >> H_Pano;
	fs_3["imageturn"] >> imageturn;

	int width_1, height_1, drift_1, width_2, height_2, drift_2, width, height, drift;
	float inlier_minRx_1, origin_x_1, inlier_minRx_2, origin_x_2, inlier_minRx, origin_x;

	ifstream inFile;

	inFile.open("1_2.txt");
	if( !inFile.is_open( ) )
	{
		AfxMessageBox(_T("Could not open file！"),MB_ICONWARNING);
		return FALSE;
	}
	inFile >> width_1 >> inlier_minRx_1 >> origin_x_1 >> height_1 >> drift_1;
	inFile.close();

	inFile.open("3_4.txt");
	if( !inFile.is_open( ) )
	{
		AfxMessageBox(_T("Could not open file！"),MB_ICONWARNING);
		return FALSE;
	}
	inFile >> width_2 >> inlier_minRx_2 >> origin_x_2 >> height_2 >> drift_2;
	inFile.close();

	inFile.open("Pano.txt");
	if( !inFile.is_open( ) )
	{
		AfxMessageBox(_T("Could not open file！"),MB_ICONWARNING);
		return FALSE;
	}
	inFile >> width >> inlier_minRx >> origin_x >> height >> drift;
	inFile.close();

	Mat result,result_1,result_2;
	Mat img[4];

	HANDLE Handle_buffer = CreateThread(NULL,0,ImageBuffer,NULL,0,NULL);
	CloseHandle(Handle_buffer);

	while(RealtimeFlag)
	{
		if(imgBuffer.size() == 4)
		{
			Start = clock();
			Handle_buffer = CreateThread(NULL,0,ImageBuffer,NULL,0,NULL);
			CloseHandle(Handle_buffer);

			img[0] = imgBuffer.front(); imgBuffer.pop();
			img[1] = imgBuffer.front(); imgBuffer.pop();
			img[2] = imgBuffer.front(); imgBuffer.pop();
			img[3] = imgBuffer.front(); imgBuffer.pop();

			result_1 = MvstDlg->Blending(img[0],img[1],imageturn_1,H_1,width_1,inlier_minRx_1,origin_x_1,height_1,drift_1);
			result_2 = MvstDlg->Blending(img[2],img[3],imageturn_2,H_2,width_2,inlier_minRx_2,origin_x_2,height_2,drift_2);
			result = MvstDlg->Blending(result_1,result_2,imageturn,H_Pano,width,inlier_minRx,origin_x,height,drift);

			image = &IplImage(result);
	    	MvstDlg->DrawPicToHDC(image, IDC_ImageShow_CH_5);
			Finish = clock();
			TotalTime = (double)(Finish - Start)/CLOCKS_PER_SEC;

			FrameInfo.Format("Frame rate : %6.3f fps",(double)(1/TotalTime));
			SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_FRAMERATE,FrameInfo);
		}
	}
	while(imgBuffer.size() > 0)
	{
		imgBuffer.pop();
	}
	return TRUE;
}

JNIEXPORT jintArray JNICALL Java_net_johnhany_grayprocessjni_VideoProc_mosaicProc(JNIEnv* env, jclass obj,
																				jintArray buf1, jintArray buf2, jintArray buf3, jintArray buf4,
																				jint w, jint h){
    jint *cbuf;
    cbuf = env->GetIntArrayElements(buf, false);
    if(cbuf == NULL){
        return 0;
    }

    Mat imgData(h, w, CV_8UC4, (unsigned char*)cbuf);

    uchar* ptr = imgData.ptr(0);
    for(int i = 0; i < w*h; i ++){
        int grayScale = (int)(ptr[4*i+2]*0.299 + ptr[4*i+1]*0.587 + ptr[4*i+0]*0.114);
        ptr[4*i+1] = grayScale;
        ptr[4*i+2] = grayScale;
        ptr[4*i+0] = grayScale;
    }

    int size=w * h;
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, cbuf);
    env->ReleaseIntArrayElements(buf, cbuf, 0);
    return result;
}
