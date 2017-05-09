// AttentionMeasurement.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FaceTracker.h"
#include "GestureHistoryBuilder.h"
#include "TrackIterator.h"
#include "TrackClusterer.h"
#include <string>
#include <ostream>
#include <sstream>

using namespace std;

int main(int argc, char* argv[])
{
	namedWindow("Video", CV_WINDOW_AUTOSIZE);
	// video file path hardcoded. Change file path as needed
	VideoCapture capture("F:\\Videos\\classroom video library\\00023.avi"); 
	
	if (!capture.isOpened())
		return -1;

	// total number of video frames. Change as needed
	long number_of_frames = 9541;
	long current_frame;
	number_of_frames = (int) capture.get(CV_CAP_PROP_FRAME_COUNT);
	if (number_of_frames == 0)
		number_of_frames = 9541;
	Mat frame;

	// Frame number of start. Change as needed
	current_frame = 1353;
	FaceTracker* faceTracker = FaceTracker::getInstance();
	faceTracker->frameNumber.set(1353);
	cout << "Running Face Tracker..." << std::endl;
	while(current_frame != number_of_frames)
	{
		capture.set(CV_CAP_PROP_POS_FRAMES,current_frame);
		capture >> frame;
		faceTracker->trackFaces(frame);
		imshow("Video",frame);
		current_frame++;
		int c = waitKey(10);
		if(c == 27) break;
	}
	vector<Track> faceSet;
	// merge face tracks.
    faceTracker->mergeTracks(100);
	faceSet = faceTracker->getTracks();

	int size = faceSet.size();
	// Save face tracks to file
	for (int i = 0; i < size; i++)
	{
		ostringstream convert;
		convert << i;
		faceSet[i].save(convert.str()+".txt");
	}
}

