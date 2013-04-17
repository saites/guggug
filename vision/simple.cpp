#include <cv.h>
#include <highgui.h>
#include <stdio.h>

using namespace cv;
using namespace std;

int main() {
	VideoCapture cap(0);
		if(!cap.isOpened()) {
			fprintf(stderr, "something went wrong\n");
			exit(1);
		}

	Mat frame;
	cap >> frame;

	imwrite("capture.jpg", frame);
	
	return 0;
}
