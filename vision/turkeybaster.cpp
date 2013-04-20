#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <set>
#include <map>
#include "disjoint.h"

#define __DEBUG__
#ifdef __DEBUG__
  #define debug_printf(...) printf(__VA_ARGS__)
  #define debug_imwrite(...) imwrite(__VA_ARGS__)
  #define debug_imshow(...) imshow(__VA_ARGS__)
#else
  #define debug_printf(...) 
  #define debug_imwrite(...) 
  #define debug_imshow(...) 
#endif

using namespace cv;
using namespace std;

class Point2D {
	public:
		int x, y;
		Point2D() {
			this->x = 0;
			this->y = 0;
		}

		Point2D(int x, int y) {
			this->x = x;
			this->y = y;
		}

		int compareX(Point2D &p) {
			return (this->x - p.x);
		}
		int compareY(Point2D &p) {
			return (this->y - p.y);
		}
	
		double distsq(Point2D &p) {
			return ((double)this->x-p.x)*((double)this->x-p.x) + 
				((double)this->y-p.y)*((double)this->y-p.y);
		}

		void operator= (const Point2D &p) {
			x = p.x;
			y = p.y;
		}
};

void orderthree(Point2D a, Point2D b, Point2D c, 
		Point2D &d, Point2D &e, Point2D &f);
double distsq(int x1, int y1, int x2, int y2);

int main(int argc, char *argv[]) {
#ifdef __DEBUG__
	if(argc != 1 && argc != 6) {
		printf(
		    "usage: %s center_thresh, x_thresh, size_thresh_max, min cam\n",
				argv[0]);
		exit(1);
	}
#else
	argc = 1;
#endif
	int CENTER_THRESH = 50;
	int X_THRESH = 5;
	int SIZE_THRESH_MAX = 3100;
	int SIZE_THRESH_MIN = 2800;
	int CAM = 0;
	if(argc == 6) {
		CENTER_THRESH = atoi(argv[1]);
		X_THRESH = atoi(argv[2]);
		SIZE_THRESH_MAX = atoi(argv[3]);
		SIZE_THRESH_MIN = atoi(argv[4]);
		CAM = atoi(argv[5]);
	}

	VideoCapture cap(CAM);
	if(!cap.isOpened()) {
		fprintf(stderr, "something went wrong\n");
		exit(1);
	}
	
	const char* windowname = "output";
	namedWindow(windowname, CV_WINDOW_AUTOSIZE);

	DisjointSet *dj = new_disjoint_set(1);
	map<int, set<int>*> whites;
	map<int, set<int>*>::iterator it;
	set<int> *pixelset;
	multimap<int, set<int>*> bySize;
	multimap<int, set<int>*>::reverse_iterator mit;
	set<int> xset;
	set<int> yset;
	while(true) {
		Mat_<Vec3b> frame;
		cap >> frame;

		//clear all the used memory
		for(it = whites.begin(); it != whites.end(); it++) {
			it->second->clear();
		}
		whites.clear();
		for(mit = bySize.rbegin(); mit != bySize.rend(); mit++) {
			mit->second->clear();
		}
		bySize.clear();
		xset.clear();
		yset.clear();

		int numPixels = frame.rows * frame.cols;
		free_disjoint_set(dj);
		dj = new_disjoint_set(numPixels);
		for(int i = 0; i < numPixels; i++) {
			disjoint_makeset(dj, i);
		}
		
		debug_imwrite("output-original.jpg", frame);

		//read through the image and join sets of white pixels
		for(int i = 0; i < frame.rows; i++) {
			for(int j = 0; j < frame.cols; j++) {
				for(int k = 0; k < 3; k++) {
					if(frame(i,j)[k] > 175) {
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
								int thispixel = disjoint_find(dj, 
										i*frame.cols+ j);
								int top = disjoint_find(dj,
										(i-1)*frame.cols + j);
								if(top != thispixel) {
									disjoint_union(dj, top, thispixel);
								}
							}
						}
						if(j > 0) {
							if(frame(i, j-1)[0] == 255) {
								int thispixel = disjoint_find(dj, 
										i*frame.cols+ j);
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

		debug_imwrite("output-white.jpg", frame);

		//build a map of pixelsets keyed on the dj_set_id, containing a set of
		//all the pixels in a group
		for(int i = 0; i < frame.rows; i++) {
			for(int j = 0; j < frame.cols; j++) {
				if(frame(i,j)[0] == 255) {
					int thispixel = disjoint_find(dj, i*frame.cols + j);
					it = whites.find(thispixel);
					//set<int> *pixelset;
					if(it == whites.end()) {
						pixelset = new set<int>();
						whites.insert(
								pair<int, set<int>* >(thispixel, pixelset));
					} else {
						pixelset = it->second;
					}
					pixelset->insert(i*frame.cols+j);
				}
			}
		}

#ifdef __DEBUG__
		//just a handy array for handling coloring
		int r[3];
		int g[3];
		int b[3];
		r[0] = 0; r[1] = 0; r[2] = 255;
		g[0] = 0; g[1] = 255; g[2] = 0;
		b[0] = 255; b[1] = 0; b[2] = 0;
		int c = 0;
#endif

		//make a map of the sets keyed on set size, for handy sorting
		//multimap<int, set<int>*> bySize;
		//multimap<int, set<int>*>::reverse_iterator mit;
		for(it = whites.begin(); it != whites.end(); it++) {
			bySize.insert(
					pair<int, set<int>* >(it->second->size(), it->second));
		}

		//now look at the three biggest sets and make sure that they're roughly
		//circular. Mark them as the leds. Average the x and y coordinates
		//to find the center point of the leds
		int i;
		Point2D leds[3];
		for(mit = bySize.rbegin(), i = 0; i < 3 && mit != bySize.rend(); 
			mit++, i++) {
			set<int> *pixelset = mit->second;

			int xsum = 0, ysum = 0;
			for(set<int>::iterator sit = pixelset->begin(); 
					sit != pixelset->end(); sit++) {
				xsum += *sit % frame.cols;
				ysum += *sit / frame.cols;
			}

			leds[i].x = xsum / pixelset->size();
			leds[i].y = ysum / pixelset->size();

#ifdef __DEBUG__
			c++;
			c %= 3;
			//color in the region so we can see it in the output
			for(set<int>::iterator sit = pixelset->begin(); sit != pixelset->end();
					sit++) {
				int i = *sit / frame.cols;
				int j = *sit % frame.cols;
				frame(i,j)[0] = b[c];
				frame(i,j)[1] = g[c];
				frame(i,j)[2] = r[c];
			}
#endif
		}

		debug_imwrite("output.jpg", frame);
		debug_imshow(windowname, frame);
		char a = (char)cvWaitKey(5);
		if(a == 27) break;

		debug_printf("points at (%d, %d), (%d, %d), (%d, %d)\n", 
				leds[0].x, leds[0].y, 
				leds[1].x, leds[1].y, 
				leds[2].x, leds[2].y);

		Point2D minP, midP, maxP;
		orderthree(leds[0], leds[1], leds[2], minP, midP, maxP);
		int avgX = (minP.x + maxP.x) / 2;
		if(midP.x > (frame.cols/2)+CENTER_THRESH) {
			debug_printf("moving right\n");
			//too far left
			//move right
			continue;
		}
		else if(midP.x < (frame.cols/2)-CENTER_THRESH) {
			debug_printf("moving left\n");
			//too far right
			//move left
			continue;
		}

		//centerX should be roughly cenetered between them
		//if not, the triangle is skew, and the person is not looking at the 
		//robot
		if(midP.x > avgX + X_THRESH || midP.x< avgX - X_THRESH) {
			debug_printf("person not looking at camera: m:%d a:%d\n",
					midP.x, avgX);
			continue;
		}

		//now we know the dots are centered in the frame and the viewer is 
		//looking at the robot. Compute the lengths of the segments, get the 
		//largest segment (which should be the base of the triangle) and try to 
		//make it acertain size
		double dist = minP.distsq(maxP);
		debug_printf("max: %lf\n", dist);

		if(dist > SIZE_THRESH_MAX) {
			debug_printf("backing up\n");
			//maxS is too large: need to back up
			continue;
		} else if(dist < SIZE_THRESH_MIN) {
			//maxS is too small: need to move forward
			continue;
		}

		debug_printf("Perfect!\n");
	}

	return 0;
}

void
orderthree(Point2D a, Point2D b, Point2D c, Point2D &d, Point2D &e, Point2D &f)
{
	if(a.x > b.x && a.x > c.x) {
		d = a;
		if(b.x > c.x) {
			e = b; f = c;
		} else {
			e = c; f = b;
		}
	} else if(b.x > a.x && b.x > c.x) {
		d = b;
		if(a.x > c.x) {
			e = a; f = c;
		} else {
			e = c; f = a;
		}
	} else {
		d = c;
		if(a.x > b.x) {
			e = a; f = b;
		} else {
			e = b; f = a;
		}
	}
}
