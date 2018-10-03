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
	if (objImageDataPointer->GetStatus() == GX_FRAME_STATUS_SUCCESS)//��ͼ�ɹ�����������֡
	{
		BYTE *m_pBuffer = (BYTE*)objImageDataPointer->ConvertToRGB24(GX_BIT_0_7, GX_RAW2RGB_NEIGHBOUR, true);
		cv::Mat temp(int(objImageDataPointer->GetHeight()), int(objImageDataPointer->GetWidth()), CV_8UC3, m_pBuffer);//nWidth=1292   nHeight=964
		SrcCapture = temp;
	}
	g_num_mutex.unlock();
}

bool MerSeries::OpenMer(size_t DeviceNum)
{
	gxdeviceinfo_vector vectorDeviceInfo;//ö���豸����
	IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);//ö���豸�б�
	if (0 == vectorDeviceInfo.size())
	{
		std::cout << "�޿����豸��" << std::endl;
		return false;
	}
	GxIAPICPP::gxstring strSN = vectorDeviceInfo[DeviceNum].GetSN();//���豸�б��n���豸��
	objDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(strSN, GX_ACCESS_EXCLUSIVE);
	//�����豸����(�����������)
	objFeatureControlPtr = objDevicePtr->GetRemoteFeatureControl();
	objFeatureControlPtr->GetFloatFeature("ExposureTime")->SetValue(10000);
	objFeatureControlPtr->GetEnumFeature("GainAuto")->SetValue("Continuous");
	objFeatureControlPtr->GetEnumFeature("BalanceWhiteAuto")->SetValue("Continuous");
	objStreamPtr = objDevicePtr->OpenStream(0);//�ɼ���ָ��
	//ע��ɼ��ص�������ע���һ���������û�˽�в������û����Դ����κ�object����Ҳ������null
	//�û�˽�в����ڻص������ڲ���ԭʹ�ã������ʹ��˽�в��������Դ���null
	objStreamPtr->RegisterCaptureCallback(pCaptureEventHandler, NULL);
	//���òɼ�buffer����
	objStreamPtr->SetAcqusitionBufferNumber(10);
	//������ͨ���ɼ�
	objStreamPtr->StartGrab();
	//���豸���Ϳ��������ʼ�ص��ɼ�
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