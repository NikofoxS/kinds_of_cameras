#include"stdafx.h"
#include"MER_Series.h"

MerSeries::MerSeries()
{
	MerSwflg = false;
	pCaptureEventHandler = this;
	IGXFactory::GetInstance().Init();
}

MerSeries::~MerSeries()
{
	CloseMer();
	objDevicePtr->Close();
}

bool MerSeries::GetCamera(cv::Mat &image)
{
	if (!MerSwflg|| SrcCapture.empty()/*||SrcCapture.isContinuous()*/)return false;
	cv::Mat temp;
	temp.create(SrcCapture.size(), SrcCapture.type());
	image = SrcCapture;
	cv::flip(SrcCapture, temp, 0);
	image = temp;
	return true;
}

void MerSeries::DoOnImageCaptured(CImageDataPointer&objImageDataPointer, void* pUserParam)
{
	g_num_mutex.lock();
	if (objImageDataPointer->GetStatus() == GX_FRAME_STATUS_SUCCESS)//采图成功而且是完整帧
	{
		BYTE *m_pBuffer = (BYTE*)objImageDataPointer->ConvertToRGB24(GX_BIT_0_7, GX_RAW2RGB_NEIGHBOUR, true);
		cv::Mat temp(int(objImageDataPointer->GetHeight()), int(objImageDataPointer->GetWidth()), CV_8UC3, m_pBuffer);//nWidth=1292   nHeight=964
		SrcCapture = temp;
	}
	g_num_mutex.unlock();
}

bool MerSeries::OpenMer(size_t DeviceNum)
{
	gxdeviceinfo_vector vectorDeviceInfo;//枚举设备变量
	IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);//枚举设备列表
	if (0 == vectorDeviceInfo.size())
	{
		std::cout << "无可用设备！" << std::endl;
		return false;
	}
	GxIAPICPP::gxstring strSN = vectorDeviceInfo[DeviceNum].GetSN();//打开设备列表第n个设备。
	objDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(strSN, GX_ACCESS_EXCLUSIVE);
	//设置设备属性(详见开发包中)
	objFeatureControlPtr = objDevicePtr->GetRemoteFeatureControl();
	objFeatureControlPtr->GetFloatFeature("ExposureTime")->SetValue(10000);
	objFeatureControlPtr->GetEnumFeature("GainAuto")->SetValue("Continuous");
	objFeatureControlPtr->GetEnumFeature("BalanceWhiteAuto")->SetValue("Continuous");
	objStreamPtr = objDevicePtr->OpenStream(0);//采集类指针
	//注册采集回调函数，注意第一个参数是用户私有参数，用户可以传入任何object对象，也可以是null
	//用户私有参数在回调函数内部还原使用，如果不使用私有参数，可以传入null
	objStreamPtr->RegisterCaptureCallback(pCaptureEventHandler, NULL);
	//设置采集buffer个数
	objStreamPtr->SetAcqusitionBufferNumber(10);
	//开启流通道采集
	objStreamPtr->StartGrab();
	//给设备发送开采命令，开始回调采集
	objFeatureControlPtr->GetCommandFeature("AcquisitionStart")->Execute();
	MerSwflg = true;
	return true;
}

void MerSeries::CloseMer()
{
	if (!MerSwflg) return;
	objFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();
	objStreamPtr->StopGrab();
	objStreamPtr->UnregisterCaptureCallback();
	objStreamPtr->Close();
	MerSwflg = false;
}