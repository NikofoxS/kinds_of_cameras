#include"OpenHIKVISION.h"


OpenHIKVISION::~OpenHIKVISION()
{
	if (openflg)CloseHIK();
	return;
}

bool OpenHIKVISION::OpenHIK(unsigned int DeviceNum, std::string result)
{
	// Enum device
	int nRet = MV_OK;
	memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
	nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
	openflg = true;
	char temp_result[200];
	do
	{
		if (MV_OK != nRet || DeviceNum > stDeviceList.nDeviceNum)
		{
			sprintf_s(temp_result, sizeof(temp_result), "Can not find the device");
			break;
		}
		MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[DeviceNum];
		// Select device and create handle
		nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[DeviceNum]);
		if (MV_OK != nRet)
		{
			sprintf_s(temp_result, sizeof(temp_result), "Create Handle fail!nRet[0x%x]\n", nRet);
			break;
		}
		// open device
		nRet = MV_CC_OpenDevice(handle);
		if (MV_OK != nRet)
		{
			sprintf_s(temp_result, sizeof(temp_result), "Open Device fail! nRet [0x%x]\n", nRet);
			break;
		}
		// Detection network optimal package size(It only works for the GigE camera)
		if (stDeviceList.pDeviceInfo[DeviceNum]->nTLayerType == MV_GIGE_DEVICE)
		{
			int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
			if (nPacketSize > 0)
			{
				nRet = MV_CC_SetIntValue(handle, "GevSCPSPacketSize", nPacketSize);
				if (nRet != MV_OK)
				{
					sprintf_s(temp_result, sizeof(temp_result), "Warning: Set Packet Size fail nRet [0x%x]!", nRet);
				}
			}
			else
			{
				sprintf_s(temp_result, sizeof(temp_result), "Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
			}
			// Set trigger mode as off
			nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
			if (MV_OK != nRet)
			{
				sprintf_s(temp_result, sizeof(temp_result), "Set Trigger Mode fail! nRet [0x%x]\n", nRet);
				break;
			}
			// Get payload size
			MVCC_INTVALUE stParam;
			memset(&stParam, 0, sizeof(MVCC_INTVALUE));
			nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam);
			if (MV_OK != nRet)
			{
				sprintf_s(temp_result, sizeof(temp_result), "Get PayloadSize fail! nRet [0x%x]\n", nRet);
				break;
			}
			g_nPayloadSize = stParam.nCurValue;
			// Start grab image
			nRet = MV_CC_StartGrabbing(handle);
			if (MV_OK != nRet)
			{
				sprintf_s(temp_result, sizeof(temp_result), "Start Grabbing fail! nRet [0x%x]\n", nRet);
				break;
			}
			memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
			openflg = true;
		}
	} while (0);
	if (nRet != MV_OK)
	{
		result = temp_result;
		CloseHIK();
		return false;
	}
	else return true;
}

void OpenHIKVISION::CloseHIK()
{
	do
	{
		if (!openflg )
		{
			if (handle != NULL)break;
			return;
		}
		int nRet;
		// Stop grab image
		nRet = MV_CC_StopGrabbing(handle);
		// Close device
		nRet = MV_CC_CloseDevice(handle);
	} while (0);
	// Destroy handle
	MV_CC_DestroyHandle(handle);
	handle = NULL;
	openflg = false;
	return;
}

bool OpenHIKVISION::GetFrame(cv::Mat &image)
{
	int nRet = MV_OK;
	unsigned char * pData = (unsigned char *)malloc(sizeof(unsigned char) * (g_nPayloadSize));
	bool flg = false;
	do
	{
		if (pData == NULL)break;
		nRet = MV_CC_GetOneFrameTimeout(handle, pData, g_nPayloadSize, &stImageInfo, 1000);
		if (nRet != MV_OK || !Convert2Mat(image, &stImageInfo, pData))break;
		flg = true;
	} while (0);
	free(pData);
	pData = NULL;
	return flg;
}

// convert data stream in Mat format
bool OpenHIKVISION::Convert2Mat(cv::Mat &OutputImage, MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char * pData)
{
	cv::Mat srcImage;
	if (pstImageInfo->enPixelType == PixelType_Gvsp_Mono8)
	{
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC1, pData);
		OutputImage = srcImage.clone();
	}
	else if (pstImageInfo->enPixelType == PixelType_Gvsp_RGB8_Packed)
	{
		RGB2BGR(pData, pstImageInfo->nWidth, pstImageInfo->nHeight);
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pData);
		OutputImage = srcImage.clone();
	}
	else
	{
		MV_CC_PIXEL_CONVERT_PARAM stParam;
		memset(&stParam, 0, sizeof(MV_CC_PIXEL_CONVERT_PARAM));
		stParam.pSrcData = pData;
		stParam.nSrcDataLen = pstImageInfo->nFrameLen;
		stParam.enSrcPixelType = pstImageInfo->enPixelType;
		stParam.nWidth = pstImageInfo->nWidth;
		stParam.nHeight = pstImageInfo->nHeight;
														
		stParam.enDstPixelType = PixelType_Gvsp_BGR8_Packed;
																 
		stParam.nDstBufferSize = pstImageInfo->nWidth*pstImageInfo->nHeight * 3;
		unsigned char* pImage = (unsigned char*)malloc(stParam.nDstBufferSize);
		stParam.pDstBuffer = pImage;

		if (MV_CC_ConvertPixelType(handle, &stParam) != MV_OK)return false;
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pImage);

		OutputImage = srcImage.clone();
		free(pImage);
	}
	if (srcImage.data == NULL)
	{
		return false;
	}
	srcImage.release();
	return true;
}

int OpenHIKVISION::RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight)
{
	if (NULL == pRgbData)
	{
		return MV_E_PARAMETER;
	}

	for (unsigned int j = 0; j < nHeight; j++)
	{
		for (unsigned int i = 0; i < nWidth; i++)
		{
			unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];
			pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
			pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
		}
	}

	return MV_OK;
}

bool OpenHIKVISION::SetExposureAuto(unsigned int mode)
{
	if (!SetCamEnumValue("ExposureAuto", mode))return false;
	else return true;
}

bool OpenHIKVISION::SetExposureTime(float time)
{
	if (!SetCamFloatExEnum("ExposureTime", "ExposureAuto", time))return false;
	else return true;
}

bool OpenHIKVISION::SetGainAuto(unsigned int mode)
{
	if (!SetCamEnumValue("GainAuto", mode))return false;
	return true;
}

bool OpenHIKVISION::SetGain(float db)
{
	if (!SetCamFloatExEnum("Gain", "GainAuto", db))return false;
	else return true;
}

bool OpenHIKVISION::SetCamEnumValue(const char* strKey, unsigned int nValue)
{
	if (!openflg || nValue > 2)return false;
	int nRet = MV_CC_SetEnumValue(handle, strKey, nValue);
	if (MV_OK != nRet)return false;
	return true;
}

bool OpenHIKVISION::SetCamFloatExEnum(const char* strKey, const char* ExstrKey, float nValue)
{
	if (!openflg || nValue < 0.0f)return false;
	int nRet;
	MVCC_ENUMVALUE stParam;
	memset(&stParam, 0, sizeof(MVCC_INTVALUE));
	nRet = MV_CC_GetEnumValue(handle, ExstrKey, &stParam);
	if (MV_OK != nRet)return false;
	if (stParam.nCurValue == 2)
		if (!SetCamEnumValue(ExstrKey,0))return false;
	nRet = MV_CC_SetFloatValue(handle, strKey, nValue);
	if (MV_OK != nRet)return false;
	else return true;
}


bool OpenHIKVISION::IsConnected()
{
	return MV_CC_IsDeviceConnected(handle);
}
