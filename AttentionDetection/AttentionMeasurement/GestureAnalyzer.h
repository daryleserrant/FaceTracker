#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "cv.h"
#include "highgui.h"
#include <algorithm>
#include "Track.h"
#include "Property.h"
#include "GestureTypes.h"
#include <math.h>
#pragma once
class GestureAnalyzer
{
private:
	Mat gray;                            // Current frame
	Mat prevGray;                        // Previous frame
	Gesture gesture;                     // Current gesture
	TrackIterator* iterator;             // Track Iterator
	bool initialized;                    // Has the analyzer been initialized
    list<Point2f> motionHistory;         // Motion vector history during the last x frames
	vector<float> horizontalStateData;   // Horizontal motion of face during current gesture state
	vector<float> verticalStateData;     // Vertical motion of face during current gesture state
	
	// The following boolean variables are used to compute the gesture state
	bool startedMovingLeft;
	bool startedMovingRight;
	bool startedMovingUp;
	bool startedMovingDown;

	/* This function computes the global optical flow motion vector
	 * of the face in the current frame.
	 * Input
	 *   roi - The face roi
	 */
	Point2f computeMotionVector(Rect roi);

	/* This function returns the most common optical flow vector
	 * by majority vote.
	 * Input
	 *  v - Optical flow vectors
	 */
	Point2f findMajority(vector<Point2f> v);

	/* This function computes the optical flow of the face in the 
	 * current frame.
	 * Input
	 *   roi - Face roi
	 * Returns
	 *   The optical flow vectors
	 */
	vector<Point2f> computeOpticalFlow(Rect roi);

	/* To eliminate noise, the motion vectors from the last several frames are kept. These
	 * vectors are then averaged to compute the global flow vector. This function simply updates
	 * the motion history using the most recent computed flow vector.
	 * Input
	 *   m - Optical flow vector
	 */
	void updateMotionHistory(Point2f m);

	/* This function simply averages the flow vectors computed in the last several frames to
	 * compute the global flow vector.
	 */
	Point2f computeGlobalFlowVector();
	
	/* This function determines the horizontal gesture based on the horizontal motion of the face.
	 */
	void determineHorizontalGesture();

	/* This function determines the vertical gesture based on the vertical motion of the face.
	 */
	void determineVerticalGesture();

	/* This function computes the determines the gesture the individual is currently showing
	 */
	void determineGestureState(Rect roi);

	/* This function draws the result of the gesture analysis on the frame.
	 * Input
	 *    frame - Frame image
	 *    face - Face roi
	 */
	void drawObjects(Mat& frame, Rect face);

public:
	
	// The following variables are thresholds used to determine the horizontal gesture
	// state
	Property<double> hStr;
	Property<double> hArea;
	Property<double> hStp;

	// The following variables are thresholds used to determine the vertical gesture
	// state
	Property<double> vStr;
	Property<double> vStp;
	Property<double> vArea;

	Property<Size> winSize;          // Window size used during Optical Flow Tracking
	Property<TermCriteria> term;     // Termination Criteria for Optical Flow Tracking
	Property<int> maxFeatures;       // Maximum number of features to track
	Property<int> histSize;          // The number of frames 

	/* Default constructor
	 */
	GestureAnalyzer();

	/* Main constructor. Initializes GestureAnalyzer with track iterator
	 */
	GestureAnalyzer(TrackIterator* iter);
	~GestureAnalyzer(void);

	/* This function will analyze the gesture of the next face in the Track. Faces are analyzed in
	 * order starting with the first face in the track.
	 * The next face that will be analyzed is determined by the TrackIterator passed to the
	 * GestureAnalyzer when it was created. If the frame number passed to this function does not
	 * match the frame number of the next face to be analyzed, the function will return a Gesture
	 * type with the values HORIZONTAL_UNKNOWN and VERTICAL_UNKNOWN.
	 * Input
	 *   frame - Frame image
	 *   frameNo - The frame number
	 * Return
	 *   The gesture state for the given frame.
	 */
	Gesture analyzeNextFaceInFrame(Mat& frame, int frameNo);
};

