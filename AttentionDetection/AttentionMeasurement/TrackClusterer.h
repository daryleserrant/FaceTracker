#include "cv.h"
#include "Track.h"
#include <algorithm>

using namespace std;
using namespace cv;

#pragma once
class TrackClusterer
{
private:
	Rect computeTrackCenter(Track& trk);

public:
	Property<int> thr;
	vector<Track> mergeTracks(vector<Track> faceSet, int trkThresh);
	TrackClusterer(void);
	~TrackClusterer(void);
};

