#pragma once
#ifndef OPEN_HIKVISION_H
#define OPEM_HIKVISION_H
#include "MvCameraControl.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include<string>
#include<stdio.h>

class OpenHIKVISION
{
public:
	OpenHIKVISION():handle (NULL),openflg (false),g_nPayloadSize (0),stImageInfo({ 0 })
	{}
	~OpenHIKVISION();
	bool OpenHIK(unsigned int DeviceNum, std::string result = "success");//Open Camera
	void CloseHIK();//Close Camera
	bool GetFrame(cv::Mat &image);// get one frame from camera with timeout=1000ms
	////-------------Open the camera before------------
	bool SetExposureAuto(unsigned int mode);//SetExposureAuto 0£ºOff ;1£ºOnce 2£ºContinuous
	bool SetExposureTime(float time);//¡Ý0.0£¬us
	bool SetGainAuto(unsigned int mode);//SetGainAuto 0£ºOff ;1£ºOnce 2£ºContinuous
	bool SetGain(float db); //¡Ý0.0£¬dB
	//-------------------------------------------------
private:
	void* handle;//Device handle
	volatile bool openflg;
	unsigned int g_nPayloadSize;
	MV_CC_DEVICE_INFO_LIST stDeviceList;
	MV_FRAME_OUT_INFO_EX stImageInfo;
	bool Convert2Mat(cv::Mat &OutputImage, MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char * pData);// convert data stream in Mat format
	int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight); //convert RGB in BGR format
	bool SetCamEnumValue(const char* strKey,unsigned int nValue);
	bool SetCamFloatExEnum(const char* strKey, const char* ExstrKey, unsigned int nValue);
};

#endif
