#include"OpenHIKVISION.h"
#include <conio.h>
#include<string>
#include<iostream>

int main()
{
	OpenHIKVISION HIK;
	if (HIK.IsConnected())std::cout << "The device is connected." << std::endl;
	else std::cout << "The device is unconnected." << std::endl;
	if (HIK.OpenHIK(0))
	{
		//if (!HIK.SetExposureTime(40000.0f))//to set ExposureAuto  HIK.SetExposureAuto(2);
		if (!HIK.SetExposureAuto(2))
		{
			HIK.CloseHIK();
			return -1;
		}
		//if (!HIK.SetGain(16.4f))//to set GainAuto  HIK.SetGainAuto(2);
		if (!HIK.SetGainAuto(2))
		{
			HIK.CloseHIK();
			return -1;
		}
		cv::Mat frame;
		int ch;

		int corner_count = 0;

		if (HIK.IsConnected())std::cout << "The device is connected." << std::endl;
		else std::cout << "The device is unconnected." << std::endl;

		while (1)
		{
			if (HIK.GetFrame(frame))
			{
				if (_kbhit())
				{
					corner_count++;
					std::string path_name;
					path_name = std::to_string(corner_count);
					path_name += ".bmp";
					cv::imwrite(path_name, frame);

					ch = _getch();
					if (ch == 113)break;//press'q' to exit
				}
				cv::namedWindow("camera_dis", CV_WINDOW_NORMAL);
				cv::imshow("camera_dis", frame);
				cv::waitKey(30);
			}
		}
		HIK.CloseHIK();
		if (HIK.IsConnected())std::cout << "The device is connected." << std::endl;
		else std::cout << "The device is unconnected." << std::endl;
	}
	std::cin.get();
	return 0;
}
