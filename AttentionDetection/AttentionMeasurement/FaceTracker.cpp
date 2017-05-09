#include "StdAfx.h"
#include "FaceTracker.h"
#include "TrackClusterer.h"
#include <math.h>

FaceTracker * FaceTracker::instance = 0;

/* Returns a vector of Tracks created during Face Tracking
 */
FaceTracker* FaceTracker::getInstance()
{
	if(!instance)
	{
		instance = new FaceTracker();
	}
	return instance;
}

/* Constructor
 */
FaceTracker:: FaceTracker()
{
	frontalMin.set(22);
	frontalMax.set(40);
	profileMin.set(29);
	profileMax.set(70);
	maxFeatures.set(500);

	term.set(TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
	winSize.set(Size(3,3));

	frontalCascade.load("..\\data\\haarcascade_frontalface_alt_tree.xml");
	profileCascade.load("..\\data\\haarcascade_profileface.xml");

	sharedThreshold.set(0.51);
}

/* Determines if a point is already listed in trackedPointList
 * Input:
 *   pt - Point
 * Return:
 *   True if the point exists in the list, false otherwise
 */
bool FaceTracker::contains(Point2f pt)
{
	vector<Point2f>::iterator it;
	for(it = trackedPointList.begin(); it != trackedPointList.end(); it++)
	{
		if(it->x == pt.x && it->y == pt.y)
			return true;
	}
	return false;
}

/* Detetermines if two rectanlges overlap
 * Input:
 *    r1 - First rectangle
 *    r2 - Second rectangle
 * Return:
 *    True if the rectangles overlap, false otherwise
 */
bool FaceTracker::overlaps(Rect r1, Rect r2)
{
	// First compute the intersection of both rectangles
	Rect intersection;

	intersection.x = (r1.x < r2.x) ? r2.x : r1.x;
	intersection.y = (r1.y < r2.y) ? r2.y : r1.y;
	intersection.width = (r1.x + r1.width < r2.x + r2.width) ? r1.x + r1.width: r2.x + r2.width;
	intersection.width -= intersection.x;
	intersection.height = (r1.y + r1.height < r2.y + r2.height) ? r1.y + r1.height : r2.y + r2.height;
	intersection.height -= intersection.y;

	// If the dimensions of the intersection are nonnegative, these rectangles overlap
	if ((intersection.width <= 0) || (intersection.height <= 0))
		return false;
	else
		return true;
}

/* Updates the point index list for each face track, while removing points that no longer lie inside face regions.
 */
void FaceTracker::updatePointLists()
{
	Point2f p;
	Rect r;
	unsigned int i,j,k;
	bool added;
	// Clear all FaceTrack point lists
	for(i = 0; i < faceSet.size(); i++)
	{
		faceSet[i].points.clear();
	}

	// Update the point lists for each face track
	for(i = k = 0; i < trackedPointList.size(); i++)
	{
		p = trackedPointList[i];
		added = false;
		
	    // Check to see if this point falls inside a face region
		for(j = 0; j < faceSet.size(); j++)
		{
			if (faceSet[j].currFace.contains(p))
			{
				faceSet[j].points.push_back(k);
				added = true;
				break;
			}
		}

		// If not, then remove it
		if (!added)
			continue;

		trackedPointList[k++] = trackedPointList[i];
	}

	// Update TrackedPointList size
	trackedPointList.resize(k);
}

/* Detects Frontal and Profile Faces in an image
 * Input:
 *	  img - An image
 *    detectedFaces - list that will contain the results of the detection.
 *					  list will be emptied before running the detector
 * Note: For performance purposes, the face detctor will be applied to a small rectangular region
 * containing the group of individuals.
 * TODO: Replace trusted Region with a background subtraction routine that will separate the people
 *       from the background. This is probably just as good for performance improvements as
 *       using a small rectangular region.
 */
void FaceTracker::detectFaces(Mat& img)
{
	// Clear any entries from detectedFaces
	faces.clear();
	vector<Rect> detectedFrontal;
	vector<Rect> detectedProfile;
	cvtColor(img,gray,CV_BGR2GRAY);
	equalizeHist(gray,gray);
	Rect r,s;
	unsigned int i,j,k,l;
	frontalCascade.detectMultiScale(gray,detectedFrontal,1.2,2,0|CV_HAAR_DO_CANNY_PRUNING,Size(frontalMin.get(),frontalMin.get()),Size(frontalMax.get(),frontalMax.get()));
	profileCascade.detectMultiScale(gray,detectedProfile,1.2,2,0|CV_HAAR_DO_CANNY_PRUNING,Size(profileMin.get(),profileMin.get()),Size(profileMax.get(),profileMax.get()));

	// Combine results
	for(i = 0; i < detectedFrontal.size(); i++)
	{
		r = detectedFrontal[i];
		faces.push_back(Rect(r.x,r.y,r.width,r.height));
	}

	for(j = 0; j < detectedProfile.size(); j++)
	{
		s = detectedProfile[j];
		faces.push_back(Rect(s.x,s.y,s.width,s.height));
	}

	// Remove Overlaping Rectanlges
	l = faces.size();
	for(i = 0; i < l; i++)
	{
		for(j = k = i+1; j < l; j++)
		{
			if(overlaps(faces[i],faces[j]))
				continue;
			faces[k++] = faces[j];
		}
		l = k;
	}
	faces.resize(l);
}

 /* Creates a new FaceTrack for the specified region. A file directory is also created in the project's working directory
 * that will store all the face images associated with the FaceTrack. This function goes ahead and saves an initial face image
 * into this directory.
 * Input:
 *    region - The face region
 *    num - FaceTrack id
 */
void FaceTracker::createTrack(Rect region,int num, Mat& img)
{
	Track newTrack;
	Point2f p;
	// Fill points list with the index of features contained within the face region
	for(unsigned int k = 0; k < trackedPointList.size(); k++)
	{
		p = trackedPointList[k];
		if(region.contains(p))
		{
			newTrack.points.push_back(k);
		}
	}

	newTrack.addFace(region,frameNumber.get());
	faceSet.push_back(newTrack);
}

/* Tracks points in subsequent frames using OpticalFlow
 * Input
 *   img - input image
 */
bool FaceTracker::trackPoints(Mat& img)
{
	double t;
	vector<uchar> status;
	vector<float> error;
	vector<Point2f> nextPoints;
	cvtColor(img,gray,CV_BGR2GRAY);
	equalizeHist(gray,gray);
	t = (double)cvGetTickCount();
	calcOpticalFlowPyrLK(prevGray,gray,trackedPointList,nextPoints,status,error,winSize.get(),3,term.get(),0,0.1);
	t = (double)cvGetTickCount() - t;
	printf("frameNumber %d ",frameNumber.get());
	printf("tracking time = %g ms\n", t/((double)cvGetTickFrequency()*1000.));
	size_t i,k;
	bool pointsLost = false;
	for(i = k = 0; i < nextPoints.size(); i++)
	{
		if(!status[i] || error[i] > 500)
		{
			pointsLost = true;
			continue;
		}
		nextPoints[k++] = nextPoints[i];
	}
	nextPoints.resize(k);
	swap(nextPoints,trackedPointList);
	return pointsLost;
}

/* Uses findGoodFeatures to identify interesing points to track
 * Input
 *    frame - input image
 *    regions - a list of face regions
 */
void FaceTracker::findGoodFeatures()
{
	double t = 0;
	vector<Point2f> features;
	Mat mask(gray.rows,gray.cols,CV_8UC1,Scalar(0));
	for(unsigned int i = 0; i < faces.size(); i++)
	{
		rectangle(mask,Point(faces[i].x,faces[i].y),Point(faces[i].x+faces[i].width,faces[i].y+faces[i].height),CV_RGB(255,255,255),-1);
	}
	goodFeaturesToTrack(gray,features,maxFeatures.get(),0.01,0.01,mask);
	if(features.size() > 0)
		cornerSubPix(gray,features,winSize.get(),Size(-1,-1),term.get());

	// Only add unique points to TrackedPointList
	for(unsigned int i = 0; i < features.size(); i++)
	{
		if (!contains(features[i]))
			trackedPointList.push_back(features[i]);
	}
}

/* Draws the face regions and interest points on an image
 * Input
 *   frame - input image
 */
void FaceTracker::drawObjects(Mat & frame)
{
	// First, draw the tracked points
	for(unsigned int i = 0; i < trackedPointList.size(); i++)
	{
		circle(frame,trackedPointList[i],2,CV_RGB(0,0,255),1,-1);
	}

	// Then draw the face regions
	for(unsigned int j = 0; j < faces.size(); j++)
	{
		Point p1 = Point(faces[j].x,faces[j].y);
		Point p2 = Point(faces[j].x + faces[j].width,faces[j].y + faces[j].height);
		rectangle(frame,p1,p2,CV_RGB(255,0,0));
	}
}

/* Returns the fraction of interest points shared by two face regions
 * Input
 *    region - face region detected in a recent frame
 *    track - a face track
 */
double FaceTracker::calcSharedFraction(Rect region,Track track)
{
	double size = track.points.size();
	double count = 0;
	Point2f p;
	for(unsigned int i = 0; i < track.points.size(); i++)
	{
		p = trackedPointList[track.points[i]];
		if (region.contains(p))
		{
			count++;
		}
	}
	return count / size;
}

/* This function will detect and track faces in the current frame. 
 * When given successive frames, this function will assemble one or more tracks.
 * Each track contains face regions belonging to the same individual.
 */
void FaceTracker::trackFaces(Mat& img)
{
	detectFaces(img);
	unsigned int x = faces.size();
	if(trackedPointList.empty())
	{
		findGoodFeatures();

		// Create a new FaceTrack for each face
		for(unsigned int j = 0; j < faces.size(); j++)
		{
			createTrack(faces[j],frameNumber.get(), img);
		}
	}
	else
	{
		// Use Lucas Kanade Optical Flow to find the points in the next frame 
		trackPoints(img);
			
		updatePointLists();

		// Add faces to their appropriate facetrack
		for(unsigned int i = 0; i < faces.size(); i++)
		{
			bool added = false;
			for(unsigned int j = 0; j < faceSet.size() && !added; j++)
			{
				if (calcSharedFraction(faces[i],faceSet[j]) >= sharedThreshold.get())
				{
					faceSet[j].addFace(faces[i],frameNumber.get());
					added = true;
				}
			}

			// Create a new face track if the face can't be fit into any face track 
			if(!added)
			{
				createTrack(faces[i],frameNumber.get(), img);
			}
		}

		// Update TrackPointList with new features detected in the face regions in the current frame
		findGoodFeatures();

		updatePointLists();
	}
	cv::swap(gray,prevGray);
	frameNumber.set(frameNumber.get() + 1);
	drawObjects(img);
}

/* During face tracking, the Face Tracker will frequently break it's track on an individual. 
 * Because of this a large number of face tracks will be created. This function uses
 * The K-Means clustering algorithm to connect these broken tracks together. 
 * Input
 *    trkThresh - After merging Tracks, the Face Tracker will discard any tracks whose size are below a
 *                threshold. Discarding these tracks will allow us to ignore any tracks generated from
 *                false face detections.
 */
void FaceTracker:: mergeTracks(int trkThresh)
{
	TrackClusterer clusterer = TrackClusterer();
	vector<Track> mergedTracks = clusterer.mergeTracks(faceSet, trkThresh);
	faceSet.clear();
	faceSet = mergedTracks;
}

/* Returns a vector of Tracks created during Face Tracking
 */
vector<Track> FaceTracker::getTracks()
{
	return faceSet;
}

void FaceTracker::setTracks(vector<Track> trk)
{
	faceSet = trk;
}