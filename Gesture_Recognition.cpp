// OpenCVTesting1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include"opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <string>
#include <cstdio>
#include <chrono>
#include <thread>

using namespace cv;
using namespace std;
int videoCap(string&, bool);
bool fingerAngle(Point, Point, Point);
bool distanceTwoPoints(Point2f, Point, float);
int disTwoPoints(Point, Point);
int fingNum(Point2f, Point);
int main(int argc, const char** argv)
{
	string passcode = "";
	bool lockIt = true;
	cout << "Welcome to Quan's Security Box" << endl;
	while (1) {
		if (lockIt) {
			cout << "Status: unlocked\n";
			cout << "~~~\n";
			cout << "Press l to lock the box\n";
			char p;
			while ((p = getchar()) != 'l') {
				if (p == 101) return 0;
			}
			videoCap(passcode, lockIt);
			lockIt = false;
		}
		else {
			cout << "Status: locked\n";
			cout << "~~~\n";
			cout << "press u to unlock th box\n";
			char p;
			while ((p = getchar()) != 'u') {
				if (p == 101) return 0;
			}
			if (videoCap(passcode, lockIt) == 1) lockIt = true;
			else {
				cout << "~~~\n";
				cout << "Suspend access for 5s\n";
				chrono::seconds dura(5);
				this_thread::sleep_for(dura);

			}

		}

	}
	videoCap(passcode,lockIt);
	return 0;
}

int videoCap(string &passcode, bool lockIt) {
	int threshold_val = 70;
	VideoCapture cam(0);
	if (!cam.isOpened()) {
		cout << "ERROR not opened " << endl;
		return -1;
	}
	Mat img;
	Mat img_threshold;
	Mat img_roi;
	Mat roi_bound;
	namedWindow("Original_image", CV_WINDOW_AUTOSIZE);
	//namedWindow("Gray_image", CV_WINDOW_AUTOSIZE);
	namedWindow("Thresholded_image", CV_WINDOW_AUTOSIZE);
	//	namedWindow("ROI", CV_WINDOW_AUTOSIZE);
	createTrackbar("Contrast", "Original_image", &threshold_val, 225);
	string a;
	string star;
	string pass;
	int attempt = 0;
	int count = 0;
	bool passReveal = true;
	int sms = 0;
	while (1) {
		bool b = cam.read(img);
		flip(img, img, 1);
		if (!b) {
			cout << "ERROR : cannot read" << endl;
			return -1;
		}
		Rect roi(340, 100, 270, 270);
		Rect roi2(5, 5, 260, 260); //boundary check
		img_roi = img(roi);
		cvtColor(img_roi, img_threshold, CV_RGB2GRAY);
		GaussianBlur(img_threshold, img_threshold, Size(19, 19), 0.0, 0);
		threshold(img_threshold, img_threshold, threshold_val, 255, THRESH_BINARY_INV);
		rectangle(img, roi, Scalar(200, 200, 0), 2.3, 8, 0);
		//rectangle(img, roi2, Scalar(200, 200, 0), 2.3, 8, 0);
		//Mat se = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
		//dilate(img_threshold, img_threshold, se);
		vector<vector<Point> >contours;
		vector<Vec4i>hierarchy;
		findContours(img_threshold, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
		if (contours.size()>0) {
			size_t indexOfBiggestContour = -1;
			size_t sizeOfBiggestContour = 0;

			for (size_t i = 0; i < contours.size(); i++) {
				if (contours[i].size() > sizeOfBiggestContour) {
					sizeOfBiggestContour = contours[i].size();
					indexOfBiggestContour = i;
				}
			}

			vector<vector<int> >hull(contours.size());
			vector<vector<Point> >hullPoint(contours.size());
			vector<vector<Vec4i> >defects(contours.size());
			vector<vector<Point> >defectPoint(contours.size()); //finger defectPoint
			vector<vector<Point>> fingerTips(contours.size()); //count fingers
			vector<vector<Point> >contours_poly(contours.size());
			Point2f rect_point[4];
			vector<RotatedRect>minRect(contours.size());
			vector<Rect> boundRect(contours.size());
			for (size_t i = 0; i<contours.size(); i++) {
				if (contourArea(contours[i])>5000) {
					convexHull(contours[i], hull[i], true);
					convexityDefects(contours[i], hull[i], defects[i]);
					if (indexOfBiggestContour == i) {
						minRect[i] = minAreaRect(contours[i]);
						for (size_t k = 0; k<hull[i].size(); k++) {
							int ind = hull[i][k];
							hullPoint[i].push_back(contours[i][ind]);
						}
						count = 0;
						Point2f center1;
						float radius;
						minEnclosingCircle(contours[i], center1, radius);
						circle(img_roi, center1, 2, Scalar(255, 0, 0), 2);
						//circle(img_roi, center1, radius / 1.5, Scalar(255, 0, 0), 2);
						//vector<vector <Point> >defect_points(contours.size());
						vector<int> detGes;
						for (size_t k = 0; k < defects[i].size(); k++) {
							if (defects[i][k][3] > 4000 && count < 5 && distanceTwoPoints(center1, contours[i][defects[i][k][1]], radius / 1.5) && roi2.contains(contours[i][defects[i][k][1]])) {
								int p_start=defects[i][k][0];   
								int p_end = defects[i][k][1];
								int p_far = defects[i][k][2];
								/*
								if (fingerAngle(contours[i][p_start], contours[i][p_far], contours[i][p_start]) && (fingerTips[i].empty() || disTwoPoints(fingerTips[i].back(), contours[i][p_start]) > 30)) {
									fingerTips[i].push_back(contours[i][p_start]);
									circle(img_roi, contours[i][p_start], 3.5, Scalar(255, 255, 0), 2);
									line(img_roi, center1, fingerTips[i].back(), Scalar(0, 200, 0), 1.4, 0, 0);
									int num = fingNum(center1, fingerTips[i].back());
									string fingerNum = to_string(num);
									detGes.push_back(num);
									putText(img_roi, fingerNum, fingerTips[i].back(), CV_FONT_HERSHEY_SIMPLEX, 0.7, Scalar(127, 255, 212), 1, 8, false);
								}*/
								defectPoint[i].push_back(contours[i][p_far]);
								//fingerTips[i].push_back(contours[i][p_start]);
								fingerTips[i].push_back(contours[i][p_end]);
								//circle(img_roi, contours[i][p_start], 3.5, Scalar(255, 255, 0), 2);
								circle(img_roi, contours[i][p_end], 3.5, Scalar(255, 255, 0), 2);
								circle(img_roi, defectPoint[i].back(), 2, Scalar(64, 224, 208), 2);
								line(img_roi, center1, fingerTips[i].back(), Scalar(0, 200, 0), 1.4, 0, 0);
								int num = fingNum(center1, fingerTips[i].back());
								string fingerNum = to_string(num);
								detGes.push_back(num);
								putText(img_roi, fingerNum , fingerTips[i].back(), CV_FONT_HERSHEY_SIMPLEX, 0.7, Scalar(127, 255, 212), 1, 8, false);
								count++;
								//}
							}
						}
						//a = to_string(count);
						if (count == 1) {
							if (detGes.at(0) == 1) {
								a = "THUMB";
							}
							else if (detGes.at(0) == 2) {
								a = "POINTING";
							}
							else if (detGes.at(0) == 3) {
								a = "POINTING";
							}
							/*
							else if (detGes.at(0) == 4 || detGes.at(0) == 5) {
								a = "";
							}*/
							else {
								a = to_string(count);
							}
						}
						else if (count == 2) {
							if ((detGes.at(0) == 3 && detGes.at(1) == 2) || (detGes.at(0) == 3 && detGes.at(1) == 3) || (detGes.at(0) == 4 && detGes.at(1) == 2)) {
								a = "PEACE";
							}
							else if (detGes.at(0) == 2 && detGes.at(1) == 1 || detGes.at(0) == 3 && detGes.at(1) == 1) {
								a = "LOSER";
							}
							else if ((detGes.at(0) == 5 && detGes.at(1) == 2)) {
								a = "THE HORNS";
							}
							else if ((detGes.at(0) == 5 && detGes.at(1) == 1) || (detGes.at(0) == 4 && detGes.at(1) == 1)) {
								a = "HAND LOOSE";
							}
							else {
								a  = to_string(count);
							}
						}
						else if (count == 3) {
							if ((detGes.at(0) == 5 && detGes.at(1) == 2 && detGes.at(2) == 1) || (detGes.at(0) == 1 && detGes.at(1) == 5 && detGes.at(2) == 2)) {
								a = "I LOVE YOU";
							}
							else if (detGes.at(0) == 5 && detGes.at(1) == 4 && detGes.at(2) == 3 || detGes.at(0) == 4 && detGes.at(1) == 3 && detGes.at(2) == 3 || detGes.at(0) == 4 && detGes.at(1) == 4 && detGes.at(2) == 3 || detGes.at(0) == 5 && detGes.at(1) == 3 && detGes.at(2) == 3) {
								a = "OK";
							}
							else if (detGes.at(0) == 3 && detGes.at(1) == 2 && detGes.at(2) == 1 || detGes.at(0) == 3 && detGes.at(1) == 2 && detGes.at(2) == 2 || detGes.at(0) == 2 && detGes.at(1) == 3 && detGes.at(2) == 1 || detGes.at(0) == 1 && detGes.at(1) == 3 && detGes.at(2) == 2 || detGes.at(0) == 4 && detGes.at(1) == 3 && detGes.at(2) == 1) {
								a = "GERMAN THREE";
							}
							else {
								a = to_string(count);
							}
						}
						else if (count == 4) {
							if (detGes.at(0) == 5 && detGes.at(1) == 4 && detGes.at(2) == 3 && detGes.at(3) == 2) {
								a = "R4BIA";
							}
							else {
								a = to_string(count);
							}
						}
						else {
							a = to_string(count);
						}

						/*
						if (count == 1)
						strcpy_s(a, "ONE");
						else if (count == 2)
						strcpy_s(a, "TWO");
						else if (count == 3)
						strcpy_s(a, "THREE");
						else if (count == 4)
						strcpy_s(a, "FOUR");
						else if (count == 5)
						strcpy_s(a, "FIVE");
						else
						strcpy_s(a, "Welcome !!");
						*/
						putText(img, a, Point(350, 70), CV_FONT_HERSHEY_SIMPLEX, 1.2, Scalar(123, 0, 255), 2, 8, false);
						drawContours(img_threshold, contours, i, Scalar(127, 255, 212), 1.8, 8, vector<Vec4i>(), 0, Point());
						drawContours(img_threshold, hullPoint, i, Scalar(127, 255, 212), 1, 8, vector<Vec4i>(), 0, Point());
						drawContours(img_roi, hullPoint, i, Scalar(0, 250, 154), 1.5, 8, vector<Vec4i>(), 0, Point());
						/*
						approxPolyDP(contours[i], contours_poly[i], 3, false);
						boundRect[i] = boundingRect(contours_poly[i]);
						rectangle(img_roi, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 0, 0), 2, 8, 0);
						minRect[i].points(rect_point);
						for (size_t k = 0; k<4; k++) {
									line(img_roi, rect_point[k], rect_point[(k + 1) % 4], Scalar(0, 255, 0), 2, 8);
						}
									*/						
					}
				}
			}
			if (passReveal) putText(img, pass, Point(30, 110), FONT_HERSHEY_TRIPLEX, 0.7, Scalar(20, 0, 255), 2, 11, false);
			else putText(img, star, Point(150, 90), FONT_HERSHEY_TRIPLEX, 1, Scalar(20, 0, 255), 2, 11, false);
			if (sms > 0) {
				putText(img, "WRONG!!!", Point(90, 200), FONT_HERSHEY_DUPLEX, 2.5, Scalar(0, 0, 255), 3.5, 9, false);
				sms--;
			}
			int input = waitKey(25);
			if (lockIt == true && input == 13) {
				passcode = pass;
				destroyWindow("Original_image");
				destroyWindow("Thresholded_image");
				return 0;
			}
			else if (lockIt == false && input == 13) {
				if (passcode == pass) {
					destroyWindow("Original_image");
					destroyWindow("Thresholded_image");
					return 1;
				}
				else {
					sms = 20;
					attempt++;
					if (attempt == 4) {
						destroyWindow("Original_image");
						destroyWindow("Thresholded_image");
						return 0;
					}
				}
			}
			else if (input == 32) {
				pass += a;
				star += "*";
			}/*
			else if (input == 100) {
				pass.pop_back();
				star.pop_back();
			}*/
			else if (input == 100) {
				pass.clear();
				star.clear();
			}
			else if (input == 114) {
				passReveal = !passReveal;
			}

			//			imshow("ROI", img_roi);

		}
		imshow("Original_image", img);
		imshow("Thresholded_image", img_threshold);
	}
	return 0;
}

bool fingerAngle(Point f1, Point defect, Point f2) {
	float def_f1 = sqrt(pow(f1.x - defect.x, 2) + pow(f1.y - defect.y, 2));
	float def_f2 = sqrt(pow(f2.x - defect.x, 2) + pow(f2.y - defect.y, 2));

	float dot = (f1.x - defect.x) * (f2.x - defect.x) + (f1.y - defect.y) * (f2.y - defect.y); 
	if (dot / (def_f1 * def_f2) > 0.5 && dot / (def_f1 * def_f2) < 1) return true;
	return false;
}
 
bool distanceTwoPoints(Point2f cen, Point fin, float rad) {
	float def_f1 = sqrt(pow(cen.x - fin.x, 2) + pow(cen.y - fin.y, 2));
	if (def_f1 < rad) return false;
	return true;
}

int disTwoPoints(Point r, Point l) {
	return sqrt(pow(r.x - l.x, 2) + pow(r.y - l.y, 2));
}

int fingNum(Point2f c, Point f) {
	float d = f.x - c.x;
	if (d < -75) return 1;
	else if (d < -35) return 2;
	else if (d < 35) return 3;
	else if (d < 80) return 4;
	return 5;
}