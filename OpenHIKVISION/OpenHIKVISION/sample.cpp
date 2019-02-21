#include"OpenHIKVISION.h"
#include <conio.h>

int main()
{
	OpenHIKVISION HIK;
	if (HIK.OpenHIK(0))
	{
		if (!HIK.SetExposureTime(40000.0f))//to set ExposureAuto  HIK.SetExposureAuto(2);
		{
			HIK.CloseHIK();
			return -1;
		}
		if (!HIK.SetGain(16.4f))//to set GainAuto  HIK.SetGainAuto(2);
		{
			HIK.CloseHIK();
			return -1;
		}
		cv::Mat frame;
		int ch;
		while (1)
		{
			if (HIK.GetFrame(frame))
			{
				if (_kbhit())
				{
					ch = _getch();
					if (ch == 113)break;//press'q' to exit
				}
				cv::namedWindow("camera_dis", CV_WINDOW_NORMAL);
				cv::imshow("camera_dis", frame);
				cv::waitKey(30);
			}
		}
		HIK.CloseHIK();
	}
	return 0;
}