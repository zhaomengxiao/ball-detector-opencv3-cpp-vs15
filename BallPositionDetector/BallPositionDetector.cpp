// BallPositionDetector.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<iostream>
#include<vector>
#include<string>
// OpenCV Header
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// OpenNI Header
#include <OpenNI.h>

// namespace
using namespace std;
using namespace openni;
using namespace cv;


class CDevice {
public:
	const char* devicename;
	Device*       pDevice;
	VideoStream*  pDepthStream;
	VideoStream*  pColorStream;
	

	CDevice(int idx, const char* uri, const char* deviceName)
	{
		devicename = deviceName;
		

		// 创建Device
		pDevice = new Device();
		// 打开指定Uri设备
		pDevice->open(uri);

		// 创建深度数据量
		pDepthStream = new VideoStream();
		pDepthStream->create(*pDevice, SENSOR_DEPTH);
		// 创建彩色数据量
		pColorStream = new VideoStream();
		pColorStream->create(*pDevice, SENSOR_COLOR);
	


		// 创建OpenCV窗口
		namedWindow("depth", CV_WINDOW_AUTOSIZE);
		namedWindow("color", CV_WINDOW_AUTOSIZE);
		namedWindow("【bgr edges】", CV_WINDOW_AUTOSIZE);
		namedWindow("【mask edges】", CV_WINDOW_AUTOSIZE);
		// 开始    
		pDepthStream->start();
		pColorStream->start();
	}
};
Mat mImagebgrColor, mImagehsvColor, mask,resizeImg;
CvSize sz;
double scale = 2.5;//圖像放大倍數

//-------響應鼠標左鍵點擊的回調函數------<>
//=======================================
void on_mouse(int event, int x, int y, int flags, void *ustc)
{
	char temp[16];
	static Point prept (-1, -1);//初始坐标 
	static Point curpt (-1, -1);//实时坐标 
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		sprintf_s(temp, "(%d,%d)", x, y);
		prept = Point(x, y);
		//·-·-·-·-·-·-列印鼠標所選點的HSV值-·-·-·-·-·-·-·-·-·-·-····
		int h = (int)mImagehsvColor.at<cv::Vec3b>(prept.y, prept.x)[0];
		int s = (int)mImagehsvColor.at<cv::Vec3b>(prept.y, prept.x)[1];
		int v = (int)mImagehsvColor.at<cv::Vec3b>(prept.y, prept.x)[2];
		cout << h << " ;" << s << " ;" << v << endl;
		//·-·-·-·-·-·-·-·-·-·-·-·-·-·-·-·-·-·-·-···········
		/*cout << mImagehsvColor.at<cv::Vec3b>(y, x)[0] << " " << mImagehsvColor.at<cv::Vec3b>(y, x)[1] << " " << mImagehsvColor.at<cv::Vec3b>(y, x)[2] << endl;*/
		putText(mImagehsvColor, temp, prept, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0, 255), 1, 8);
		imshow("hsv", mImagehsvColor);
		imwrite("E:\\catch.jpg", mImagehsvColor);
		waitKey(0);
	}
}
//========================================
//----------主函數------------------------------------<>
//=========================================
int main(int argc, char **argv)
{
	// 初始化OpenNI  
	OpenNI::initialize();

	// 获取设备信息  
	Array<DeviceInfo> aDeviceList;
	OpenNI::enumerateDevices(&aDeviceList);

	// 将每个设备信息用CDevice类封装  
	vector<CDevice>  vDevices;

	cout << "电脑上连接着 " << aDeviceList.getSize() << " 个体感设备." << endl;

	for (int i = 0; i < aDeviceList.getSize(); ++i)
	{
		cout << "设备 " << i << endl;
		const DeviceInfo& rDevInfo = aDeviceList[i];
		cout << "设备名： " << rDevInfo.getName() << endl;
		cout << "设备Id： " << rDevInfo.getUsbProductId() << endl;
		cout << "供应商名： " << rDevInfo.getVendor() << endl;
		cout << "供应商Id: " << rDevInfo.getUsbVendorId() << endl;
		cout << "设备URI: " << rDevInfo.getUri() << endl;

		// 封装类初始化，传入设备名和设备Uri 
		CDevice mDev(i, aDeviceList[i].getUri(), aDeviceList[i].getName());
		vDevices.push_back(mDev);
	}
	

	//----------------------------------------------------------//
	//-·-·-·-進入主迴圈 按Q 退出—·-·-·-·-·-·-·-·-·-//
	//----------------------------------------------------------//
	while (true)
	{
		for (vector<CDevice>::iterator itDev = vDevices.begin();
			itDev != vDevices.end(); ++itDev)
		{
			// 获取深度图像帧      
			VideoFrameRef vfDepthFrame;
			itDev->pDepthStream->readFrame(&vfDepthFrame);
			// 获取彩色图像帧 
			VideoFrameRef vfColorFrame;
			itDev->pColorStream->readFrame(&vfColorFrame);

			// 转换成 OpenCV 格式      
			const cv::Mat mImageDepth(vfDepthFrame.getHeight(),
				vfDepthFrame.getWidth(),
				CV_16UC1,
				const_cast<void*>(vfDepthFrame.getData())
			);

			const Mat mImageColor(vfColorFrame.getHeight(),
				vfColorFrame.getWidth(),
				CV_8UC3,
				const_cast<void*>(vfColorFrame.getData())
			);
			//彩色圖RGB TO BGR 
			/*Mat mImagebgrColor ,mImagehsvColor,mask;*/
			cvtColor(mImageColor, mImagebgrColor, CV_RGB2BGR);
			// 从 [0,Max] 转为 [0,255]      
			Mat mScaledDepth;
			mImageDepth.convertTo(mScaledDepth, CV_8U,
				255.0 / itDev->pDepthStream->getMaxPixelValue());
			// 显示图像      
			imshow("depth", mScaledDepth);
			imshow("color", mImagebgrColor);
			vfDepthFrame.release();
			vfColorFrame.release();
			
			// TRY---------轉換到hsv色彩空間鎖定黃色球 增加魯邦性----TRY//
			cvtColor(mImagebgrColor, mImagehsvColor, CV_BGR2HSV);
			
			inRange(mImagehsvColor,Scalar(20,100,0.0),Scalar(40,255,255),mask);
			imshow("mask", mask);
			/*int h = (int)mImagehsvColor.at<cv::Vec3b>(242, 344)[0];
			int s = (int)mImagehsvColor.at<cv::Vec3b>(242, 344)[1];
			int v = (int)mImagehsvColor.at<cv::Vec3b>(242, 344)[2];
			cout << h << " ;" << s << " ;" << v << endl;*/
			
			//----鼠標獲取坐標像素-------
			setMouseCallback("hsv", on_mouse,0);
			imshow("hsv", mImagehsvColor);
			

			 //END TRY---------轉換到hsv色彩空間鎖定黃色球 增加魯邦性----END TRY//
			
			Mat edges;  //定义转化的灰度图
			Mat circleimage = mask;
			/*cvtColor(circleimage, edges, CV_BGR2GRAY);*/
			GaussianBlur(circleimage, edges, Size(7, 7), 2, 2);//高斯滤波
			vector<Vec3f> circles;//霍夫圆
			HoughCircles(edges, circles, CV_HOUGH_GRADIENT, 1.5, circleimage.rows / 4, 40, 20, 0, 0);
			for (size_t i = 0; i < circles.size(); i++)
			{
				Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
				int radius = cvRound(circles[i][2]);
				//绘制圆心  
				circle(mImagebgrColor, center, 3, Scalar(0, 255, 0), -1, 8, 0);
				//绘制圆轮廓  
				circle(mImagebgrColor, center, radius, Scalar(155, 50, 255), 3, 8, 0);
			}
			imshow("edges",edges );

			//------------- resize image-----
			
			resize(mImagebgrColor,resizeImg,Size(mImagebgrColor.cols*scale, mImagebgrColor.rows*scale));
			
			imshow("【ball detector】", resizeImg);

			//``````````````````````````````````````````````````````````
			//Mat edges;  //定义转化的灰度图
			//Mat circleimage = mImagebgrColor;
			//cvtColor(circleimage, edges, CV_BGR2GRAY);
			//GaussianBlur(edges, edges, Size(7, 7), 2, 2);//高斯滤波
			//vector<Vec3f> circles;//霍夫圆
			//HoughCircles(edges, circles, CV_HOUGH_GRADIENT, 1.5, circleimage.rows/4, 200, 100, 0, 0);
			//for (size_t i = 0; i < circles.size(); i++)
			//{
			//	Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			//	int radius = cvRound(circles[i][2]);
			//	//绘制圆心  
			//	circle(circleimage, center, 3, Scalar(0, 255, 0), -1, 8, 0);
			//	//绘制圆轮廓  
			//	circle(circleimage, center, radius, Scalar(155, 50, 255), 3, 8, 0);
			//}
			//imshow("【bgr edges】", circleimage);
			




			
		}
		// 退出键    
		if (cv::waitKey(1) == 'q')
			break;
	}
	//----------------------------------------------------------//
	//-·-·-·-       退出主迴圈       —·-·-·-·-·-·-·-·-·-//
	//----------------------------------------------------------//
	// 停止时的操作  
	for (vector<CDevice>::iterator itDev = vDevices.begin();
		itDev != vDevices.end(); ++itDev)
	{
		itDev->pDepthStream->stop();
		itDev->pColorStream->stop();
		itDev->pDepthStream->destroy();
		itDev->pColorStream->destroy();
		delete itDev->pDepthStream;
		delete itDev->pColorStream;
		itDev->pDevice->close();
		delete itDev->pDevice;
	}
	OpenNI::shutdown();
	return 0;
}

