#include"MER_Series.h"
#include <conio.h>

int main()
{
	MerSeries capture;
	capture.OpenMer(0);
	cv::Mat frame;
	int ch;
	std::cout << "press 'q' to exit.\n";
	int i = 0;
	std::string path_name;
	while (1)
	{
		if (capture.GetCamera(frame))
		{
			if (_kbhit())
			{
				ch = _getch();
				if (ch == 113)break;//press'q' to exit
			}
			cv::imshow("camera", frame);
			cv::waitKey(10);
		}
	}
	system("pause");
	return 0;
}