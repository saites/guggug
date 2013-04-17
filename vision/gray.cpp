#include <iostream>
#include <cv.h>
#include <highgui.h>

using namespace cv;
using namespace std;

int
main(int argc, char *argv[]) {
	if(argc != 3) {
		cerr << "usage: " << argv[0] << " input output" << endl;
		exit(1);
	}

	Mat image;
	image = imread(argv[1], CV_LOAD_IMAGE_COLOR);

	if(!image.data) {
		cerr << "could not load file" << endl;
		exit(1);
	}
	Mat gray_image;
	cvtColor(image, gray_image, CV_BGR2GRAY);

	imwrite(argv[2], gray_image);

	return 0;
}
