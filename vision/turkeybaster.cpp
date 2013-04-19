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
	
	imwrite("output-original.jpg", frame);

	//read through the image and join sets of white pixels
	for(int i = 0; i < frame.rows; i++) {
		for(int j = 0; j < frame.cols; j++) {
			for(int k = 0; k < 3; k++) {
				if(frame(i,j)[k] > 200) {
					if(i > 0) {
						if(j > 0) {
							if(frame(i-1, j-1)[0] == 255) {
								int thispixel = disjoint_find(dj, 
										i*frame.cols + j);
								int topleft = disjoint_find(dj,
										(i-1)*frame.cols + (j-1));
								if(topleft != thispixel) {
									disjoint_union(dj, topleft, thispixel);
								}
							}
						}
						if(frame(i-1, j)[0] == 255) {
							int thispixel = disjoint_find(dj, i*frame.cols+ j);
							int top = disjoint_find(dj,
									(i-1)*frame.cols + j);
							if(top != thispixel) {
								disjoint_union(dj, top, thispixel);
							}
						}
					}
					if(j > 0) {
						if(frame(i, j-1)[0] == 255) {
							int thispixel = disjoint_find(dj, i*frame.cols+ j);
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

	imwrite("output-white.jpg", frame);

	//build a map of pixelsets keyed on the dj_set_id, containing a set of all
	//the pixels in a group
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

	//just a handy array for handling coloring
	int r[3];
	int g[3];
	int b[3];
	r[0] = 0; r[1] = 0; r[2] = 255;
	g[0] = 0; g[1] = 255; g[2] = 0;
	b[0] = 255; b[1] = 0; b[2] = 0;
	int c = 0;

	//make a map of the sets keyed on set size, for handy sorting
	multimap<int, set<int>*> bySize;
	multimap<int, set<int>*>::reverse_iterator mit;
	for(it = whites.begin(); it != whites.end(); it++) {
		bySize.insert(pair<int, set<int>* >(it->second->size(), it->second));
	}

	//now look at the three biggest sets and make sure that they're roughly
	//circular. If so, mark them as the leds. Average the x and y coordinates
	//to find the center point of the leds
	int i;
	for(mit = bySize.rbegin(), i = 0; i < 3 && mit != bySize.rend(); 
		mit++, i++) {
		set<int> *pixelset = mit->second;
		c++;
		c %= 3;

		int xsum = 0, ysum = 0;
		for(set<int>::iterator sit = pixelset->begin(); sit != pixelset->end();
				sit++) {
			xsum += *sit / frame.cols;
			ysum += *sit % frame.cols;
			
		}

		for(set<int>::iterator sit = pixelset->begin(); sit != pixelset->end();
				sit++) {
			int i = *sit / frame.cols;
			int j = *sit % frame.cols;
			frame(i,j)[0] = b[c];
			frame(i,j)[1] = g[c];
			frame(i,j)[2] = r[c];
		}
	}

	imwrite("output.jpg", frame);
	
	return 0;
}
