#include<iostream>
#include<opencv2\opencv.hpp>
#include<GalaxyIncludes.h>
#include <conio.h>//有关键盘按键

class CSampleCaptureEventHandler : public ICaptureEventHandler

{

public:
	cv::Mat SrcCapture;
	void DoOnImageCaptured(CImageDataPointer&objImageDataPointer, void* pUserParam)

	{

		//std::cout << "收到一帧图像!" << std::endl;
		//std::cout << "ImageInfo: " << objImageDataPointer->GetStatus() << std::endl;
		//std::cout << "ImageInfo: " << objImageDataPointer->GetWidth() << std::endl;
		//std::cout << "ImageInfo: " << objImageDataPointer->GetHeight() << std::endl;
		//std::cout << "ImageInfo: " << objImageDataPointer->GetPayloadSize() << std::endl;

		BYTE *m_pBuffer = (BYTE*)objImageDataPointer->ConvertToRGB24(GX_BIT_0_7, GX_RAW2RGB_NEIGHBOUR, true);
		cv::Mat temp(964, 1292, CV_8UC3, m_pBuffer, size_t(1292 * 3 * sizeof(char)));//nWidth=1292   nHeight=964
		SrcCapture = temp;
		//cv::imshow("the camera", SrcCapture);
	}
	cv::Mat GetCamera() 
	{
		return SrcCapture;
	}
};

int main()
{
	//声明事件回调对象指针

	IDeviceOfflineEventHandler* pDeviceOfflineEventHandler = NULL;///<掉线事件回调对象

	IFeatureEventHandler* pFeatureEventHandler = NULL;///<远端设备事件回调对象

	ICaptureEventHandler* pCaptureEventHandler = NULL;///<采集回调对象



	//初始化

	IGXFactory::GetInstance().Init();
	CGXDevicePointer objDevicePtr;//设备句柄


	try
	{

		do
		{

			//枚举设备

			gxdeviceinfo_vector vectorDeviceInfo;

			IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);

			if (0 == vectorDeviceInfo.size())

			{

				std::cout << "无可用设备!" << std::endl;

				break;

			}
			else
				std::cout << "可用设备数量为: " << vectorDeviceInfo.size() << std::endl;

			//打开链表中的第一个设备

			//CGXDevicePointer objDevicePtr;

			GxIAPICPP::gxstring strSN = vectorDeviceInfo[0].GetSN();

			//GxIAPICPP::gxstring strUserID = vectorDeviceInfo[0].GetUserID();

			//GxIAPICPP::gxstring strMAC = vectorDeviceInfo[0].GetMAC();

			//GxIAPICPP::gxstring strIP = vectorDeviceInfo[0].GetIP();
			objDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(strSN, GX_ACCESS_EXCLUSIVE);
			//设置设备属性(详见开发包中)
			CGXFeatureControlPointer objFeatureControlPtr = objDevicePtr->GetRemoteFeatureControl();
			objFeatureControlPtr->GetFloatFeature("ExposureTime")->SetValue(20000);
			//objFeatureControlPtr->GetEnumFeature("ExposureAuto")->SetValue("Continuous");
			objFeatureControlPtr->GetEnumFeature("BalanceWhiteAuto")->SetValue("Continuous");

			CGXStreamPointer objStreamPtr = objDevicePtr->OpenStream(0);//采集类指针
			//注册采集回调函数，注意第一个参数是用户私有参数，用户可以传入任何object对象，也可以是null
			//用户私有参数在回调函数内部还原使用，如果不使用私有参数，可以传入null
			CSampleCaptureEventHandler* pCaptureEventHandler = new CSampleCaptureEventHandler();
			objStreamPtr->RegisterCaptureCallback(pCaptureEventHandler, NULL);
			//设置采集buffer个数
			objStreamPtr->SetAcqusitionBufferNumber(10);

			//开启流通道采集
			objStreamPtr->StartGrab();

			//给设备发送开采命令
			objFeatureControlPtr->GetCommandFeature("AcquisitionStart")->Execute();


			bool flg=true;
			while (flg)
			{
				if (!(pCaptureEventHandler->GetCamera()).empty())
				{
					cv::imshow("the camera", pCaptureEventHandler->GetCamera());
					cv::waitKey(30);
				}

			}


			//停采、注销采集回调函数
			objFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();
			objStreamPtr->StopGrab();
			objStreamPtr->UnregisterCaptureCallback();
			delete pCaptureEventHandler;
	    	pCaptureEventHandler = NULL;
			//关闭流通道
			objStreamPtr->Close();
		} while (0);
	}
	catch (CGalaxyException&e)

	{

		std::cout << "错误码: " << e.GetErrorCode() << std::endl;

		std::cout << "错误描述信息: " << e.what() << std::endl;
	}

	catch (std::exception&e)

	{
		std::cout << "错误描述信息: " << e.what() << std::endl;
	}




	objDevicePtr->Close();
	std::cin.get();
	return 0;
}