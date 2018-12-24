#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
#include "opencv2/calib3d/calib3d.hpp" 
#include<pylon/PylonIncludes.h>
using namespace cv;
using namespace std;
using namespace Pylon;
int main()
{
	Pylon::PylonInitialize();
	int corner_count = 0;
	cv::VideoCapture capture(1);
	capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, 900);
	char temp;
	while (1)
	{
		cv::Mat frame;
		cv::Mat grayImage;
		capture >> frame;
		imshow("thechess", frame);
		temp = cv::waitKey(30);
		if (temp >= 0)
		{
			if (temp == 's')break;
			std::string path_name = std::to_string(corner_count);
			path_name += ".jpg";
			cv::imwrite(path_name, frame);
			corner_count++;
		}
	}
	Pylon::PylonTerminate();
	return 0;
}
