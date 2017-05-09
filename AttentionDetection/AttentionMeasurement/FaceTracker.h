#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "cv.h"
#include "highgui.h"
#include <algorithm>
#include "track.h"
#include "Property.h"

#pragma once
class FaceTracker
{
private:
	
	/* Constructor
	 */
	FaceTracker(void);
	FaceTracker(FaceTracker& other);
	~FaceTracker(void);
    
	/* Detetermines if two rectanlges overlap
     * Input:
     *    r1 - First rectangle
     *    r2 - Second rectangle
     * Return:
     *    True if the rectangles overlap, false otherwise
     */
	bool overlaps(Rect r1, Rect r2);

	/* Determines if a point is already listed in trackedPointList
	 * Input:
	 *   pt - Point
	 * Return:
	 *   True if the point exists in the list, false otherwise
	 */
	bool contains(Point2f pt);

   /* Updates the point index list for each face track, while removing points that no longer lie inside face regions.
    */
	void updatePointLists();

   /* Uses findGoodFeatures to identify interesing points to track
    * Input
    *    frame - input image
    *    regions - a list of face regions
    */
	void findGoodFeatures();

   /* Draws the face regions and interest points on an image
    * Input
    *   frame - input image
    */
	void drawObjects(Mat& frame);

   /* Creates a new FaceTrack for the specified region. A file directory is also created in the project's working directory
    * that will store all the face images associated with the FaceTrack. This function goes ahead and saves an initial face image
    * into this directory.
    * Input:
    *    region - The face region
    *    num - FaceTrack id
    */
	void createTrack(Rect region, int num, Mat& img);

   /* Tracks points in subsequent frames using OpticalFlow
    * Input
    *   img - input image
    */
	bool trackPoints(Mat& img);

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
	void detectFaces(Mat& img);

  /* Returns the fraction of interest points shared by two face regions
   * Input
   *    region - face region detected in a recent frame
   *    track - a face track
   */
	double calcSharedFraction(Rect region,Track track);

	// Private Members
	static FaceTracker* instance;          // Pointer to an instance of FaceTracker
	vector<Track> faceSet;                 // List of Tracks
	vector<Point2f> trackedPointList;      // List of points tracked throughout the video
	vector<Rect> faces;                    // Faces detected in current frame

	CascadeClassifier frontalCascade;      // Frontal Haar Cascade
	CascadeClassifier profileCascade;      // Profile Haar Cascade

	Mat gray;                              // Current frame image
	Mat prevGray;                          // Previous frame image

public:
	Property<int> frontalMin;              // Minimum frontal face size
	Property<int> frontalMax;              // Maximum frontal face size
	Property<int> profileMin;              // Minimum profile face size
	Property<int> profileMax;              // Maximum profile face size
	Property<int> maxFeatures;             // Maximum number of features to track
	Property<int> frameNumber;             // Current frame tracking
	Property<double> sharedThreshold;      // Track membership threshhold.
	Property<Size> winSize;                // Window size used during KLT feature tracking
	Property<TermCriteria> term;           // Termination criteria used during KLT feature tracking

	static FaceTracker * getInstance();    // retrive a pointer to FaceTracker

	/* This function will detect and track faces in the current frame. 
	 * When given successive frames, this function will assemble one or more tracks.
	 * Each track contains face regions belonging to the same individual.
	 */
	void trackFaces(Mat& img);

	/* During face tracking, the Face Tracker will frequently break it's track on an individual. 
	 * Because of this a large number of face tracks will be created. This function uses
	 * The K-Means clustering algorithm to connect these broken tracks together. 
	 * Input
	 *    trkThresh - After merging Tracks, the Face Tracker will discard any tracks whose size are below a
	 *                threshold. Discarding these tracks will allow us to ignore any tracks generated from
	 *                false face detections.
	 */
	void mergeTracks(int trkThresh);

	/* Returns a vector of Tracks created during Face Tracking
	 */
	vector<Track> getTracks();

	void setTracks(vector<Track> trk);
};

