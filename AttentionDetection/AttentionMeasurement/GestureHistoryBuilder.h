#include "GestureAnalyzer.h"

#pragma once
class GestureHistoryBuilder
{
private:
	GestureAnalyzer gestureAnalyzer;      // Gesture Analyzer
	map<int,Gesture> history;             // Dictionary of gestures in action during each frame number

public:
	Property<int> interval;               // Time interval used for constructing histogram bins

	/* Default constructor
	 */
	GestureHistoryBuilder();

	/* Constructor for Gesture History Builder
	 */
	GestureHistoryBuilder(GestureAnalyzer& analyzer);

	/* This function takes a frame and the corresponding frame number,
	 * determines the gesture perfomed at that frame, and adds it to
	 * to its gesture history list.
	 */
	void processFrame(Mat& frame, int frameNo);

	/* This function uses the gesture history information collected to build a histogram
	 * of gestures
	 */
	GestureHistory buildGestureHistory();
	~GestureHistoryBuilder(void);
};

