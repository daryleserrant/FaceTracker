#include "StdAfx.h"
#include "TrackClusterer.h"
#include <math.h>


Rect TrackClusterer::computeTrackCenter(Track & trk)
{
	Rect center;
	vector<Rect> faces = trk.getFaceROIs();
	int n = trk.getSize();
	for(int i = 0; i < n; i++)
	{
		center.x += faces[i].x;
		center.y += faces[i].y;
		center.width += faces[i].width;
		center.height += faces[i].height;
	}
	center.x /= n;
	center.y /= n;
	center.width /= n;
	center.height /= n;

	return center;
}

vector<Track> TrackClusterer::mergeTracks(vector<Track> faceSet, int trkThresh)
{
	int numClusters = 0;
	Mat dataSamples(faceSet.size(),8,CV_32F);
	Rect c;
	for(unsigned int i = 0; i < faceSet.size(); i++)
	{
		if(faceSet[i].getSize() > thr.get())
			numClusters++;
		dataSamples.at<float>(i,0) = faceSet[i].start.x;
		dataSamples.at<float>(i,1) = faceSet[i].start.y;
		dataSamples.at<float>(i,2) = faceSet[i].start.width;
		dataSamples.at<float>(i,3) = faceSet[i].start.height;
		dataSamples.at<float>(i,4) = faceSet[i].currFace.x;
	    dataSamples.at<float>(i,5) = faceSet[i].currFace.y;
	    dataSamples.at<float>(i,6) = faceSet[i].currFace.width;
	    dataSamples.at<float>(i,7) = faceSet[i].currFace.height;
	}

	Mat centers(numClusters,8,dataSamples.type());
	Mat labels;
	int idx;

	// Perform KMeans
	kmeans(dataSamples, numClusters, labels, TermCriteria( CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 15, 0.8),
	      15, KMEANS_RANDOM_CENTERS, centers);

	Track * mergedSet = new Track[numClusters];
	
	for(unsigned int j = 0; j < faceSet.size(); j++)
	{
		idx = labels.at<int>(j,0);
		mergedSet[idx].merge(faceSet[j]);
	}

	vector<Track> newFaceSet;
	for(int i = 0; i < numClusters; i++)
	{
		if (mergedSet[i].getSize() >= trkThresh)
		{
		   newFaceSet.push_back(mergedSet[i]);
		}
	}
	
	return newFaceSet;
}

TrackClusterer::TrackClusterer(void)
{
	thr.set(1);
}


TrackClusterer::~TrackClusterer(void)
{
}
