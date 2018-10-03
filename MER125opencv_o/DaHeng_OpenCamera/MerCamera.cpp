#include<iostream>
#include<opencv2\opencv.hpp>
#include<GalaxyIncludes.h>
#include <conio.h>//�йؼ��̰���

class CSampleCaptureEventHandler : public ICaptureEventHandler

{

public:
	cv::Mat SrcCapture;
	void DoOnImageCaptured(CImageDataPointer&objImageDataPointer, void* pUserParam)

	{

		//std::cout << "�յ�һ֡ͼ��!" << std::endl;
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
	//�����¼��ص�����ָ��

	IDeviceOfflineEventHandler* pDeviceOfflineEventHandler = NULL;///<�����¼��ص�����

	IFeatureEventHandler* pFeatureEventHandler = NULL;///<Զ���豸�¼��ص�����

	ICaptureEventHandler* pCaptureEventHandler = NULL;///<�ɼ��ص�����



	//��ʼ��

	IGXFactory::GetInstance().Init();
	CGXDevicePointer objDevicePtr;//�豸���


	try
	{

		do
		{

			//ö���豸

			gxdeviceinfo_vector vectorDeviceInfo;

			IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);

			if (0 == vectorDeviceInfo.size())

			{

				std::cout << "�޿����豸!" << std::endl;

				break;

			}
			else
				std::cout << "�����豸����Ϊ: " << vectorDeviceInfo.size() << std::endl;

			//�������еĵ�һ���豸

			//CGXDevicePointer objDevicePtr;

			GxIAPICPP::gxstring strSN = vectorDeviceInfo[0].GetSN();

			//GxIAPICPP::gxstring strUserID = vectorDeviceInfo[0].GetUserID();

			//GxIAPICPP::gxstring strMAC = vectorDeviceInfo[0].GetMAC();

			//GxIAPICPP::gxstring strIP = vectorDeviceInfo[0].GetIP();
			objDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(strSN, GX_ACCESS_EXCLUSIVE);
			//�����豸����(�����������)
			CGXFeatureControlPointer objFeatureControlPtr = objDevicePtr->GetRemoteFeatureControl();
			objFeatureControlPtr->GetFloatFeature("ExposureTime")->SetValue(20000);
			//objFeatureControlPtr->GetEnumFeature("ExposureAuto")->SetValue("Continuous");
			objFeatureControlPtr->GetEnumFeature("BalanceWhiteAuto")->SetValue("Continuous");

			CGXStreamPointer objStreamPtr = objDevicePtr->OpenStream(0);//�ɼ���ָ��
			//ע��ɼ��ص�������ע���һ���������û�˽�в������û����Դ����κ�object����Ҳ������null
			//�û�˽�в����ڻص������ڲ���ԭʹ�ã������ʹ��˽�в��������Դ���null
			CSampleCaptureEventHandler* pCaptureEventHandler = new CSampleCaptureEventHandler();
			objStreamPtr->RegisterCaptureCallback(pCaptureEventHandler, NULL);
			//���òɼ�buffer����
			objStreamPtr->SetAcqusitionBufferNumber(10);

			//������ͨ���ɼ�
			objStreamPtr->StartGrab();

			//���豸���Ϳ�������
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


			//ͣ�ɡ�ע���ɼ��ص�����
			objFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();
			objStreamPtr->StopGrab();
			objStreamPtr->UnregisterCaptureCallback();
			delete pCaptureEventHandler;
	    	pCaptureEventHandler = NULL;
			//�ر���ͨ��
			objStreamPtr->Close();
		} while (0);
	}
	catch (CGalaxyException&e)

	{

		std::cout << "������: " << e.GetErrorCode() << std::endl;

		std::cout << "����������Ϣ: " << e.what() << std::endl;
	}

	catch (std::exception&e)

	{
		std::cout << "����������Ϣ: " << e.what() << std::endl;
	}




	objDevicePtr->Close();
	std::cin.get();
	return 0;
}