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
	char buf[1000];
	for(int i = 5; i > 0; i--) {
		printf("captureing images: %d remaining\n", i);
		sleep(1);
		cap >> frame;
		sprintf(buf, "capture%d.jpg", i);
		imwrite(buf, frame);
	}
	
	return 0;
}
