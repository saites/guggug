#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <set>
#include <map>
#include "disjoint.h"

using namespace cv;
using namespace std;

int main() {
	VideoCapture cap(0);
	if(!cap.isOpened()) {
		fprintf(stderr, "something went wrong\n");
		exit(1);
	}

	Mat_<Vec3b> frame;
	cap >> frame;

	int numPixels = frame.rows * frame.cols;
	DisjointSet *dj = new_disjoint_set(numPixels);
	for(int i = 0; i < numPixels; i++) {
		disjoint_makeset(dj, i);
	}
	

	for(int i = 0; i < frame.rows; i++) {
		for(int j = 0; j < frame.cols; j++) {
			for(int k = 0; k < 3; k++) {
				if(frame(i,j)[k] > 127) {
					int thispixel = disjoint_find(dj, i*frame.cols + j);
					if(i > 0) {
						if(j > 0) {
							if(frame(i-1, j-1)[0] == 255) {
								int topleft = disjoint_find(dj,
										(i-1)*frame.cols + (j-1));
								if(topleft != thispixel) {
									disjoint_union(dj, topleft, thispixel);
								}
							}
						}
						if(frame(i-1, j)[0] == 255) {
							int top = disjoint_find(dj,
									(i-1)*frame.cols + j);
							if(top != thispixel) {
								disjoint_union(dj, top, thispixel);
							}
						}
					}
					if(j > 0) {
						if(frame(i, j-1)[0] == 255) {
							int left = disjoint_find(dj,
									i*frame.cols + (j-1));
							if(left != thispixel) {
								disjoint_union(dj, left, thispixel);
							}
						}
					}
					frame(i,j)[0] = 255;
					frame(i,j)[1] = 255;
					frame(i,j)[2] = 255;
					break;
				}
			}
		}
	}


	map<int, set<int>*> whites;
	map<int, set<int>*>::iterator it;
	for(int i = 0; i < frame.rows; i++) {
		for(int j = 0; j < frame.cols; j++) {
			if(frame(i,j)[0] == 255) {
				int thispixel = disjoint_find(dj, i*frame.cols + j);
				it = whites.find(thispixel);
				set<int> *pixelset;
				if(it == whites.end()) {
					pixelset = new set<int>();
					whites.insert(pair<int, set<int>* >(thispixel, pixelset));
				} else {
					pixelset = it->second;
				}
				pixelset->insert(i*frame.cols+j);
			}
		}
	}

	for(it = whites.begin(); it != whites.end(); it++) {
		set<int> *pixelset = it->second;
		for(set<int>::iterator sit = pixelset->begin(); sit != pixelset->end();
				sit++) {
			int i = *sit / frame.cols;
			int j = *sit % frame.rows;
			frame(i,j)[0] = 0;
			frame(i,j)[1] = 0;
			frame(i,j)[2] = 255;
		}
	}

	imwrite("output.jpg", frame);
	
	return 0;
}
