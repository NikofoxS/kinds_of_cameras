#pragma once
//corpyright NikofoxS
#ifndef MER_SERIES_H
#define MER_SERIES_H
#include"stdafx.h"
#include<iostream>
#include<opencv2\opencv.hpp>
#include<GalaxyIncludes.h>
#include <conio.h>
#include <mutex>//Thread mutex

class MerSeries:public ICaptureEventHandler
{
public:
	MerSeries();
	~MerSeries();
	bool OpenMer(size_t DeviceNum);
	void CloseMer();
	bool GetCamera(cv::Mat &image);
private:
	bool MerSwflg;
	void DoOnImageCaptured(CImageDataPointer&objImageDataPointer, void* pUserParam);//reload callback
	CGXDevicePointer objDevicePtr;//�豸���
	CGXFeatureControlPointer objFeatureControlPtr;//�豸���Կ���ָ��
	cv::Mat SrcCapture;
	std::mutex g_num_mutex;
	ICaptureEventHandler* pCaptureEventHandler;
	CGXStreamPointer objStreamPtr;//�ɼ���ָ��
};
#endif