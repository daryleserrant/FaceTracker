#include "StdAfx.h"
#include "GestureHistoryBuilder.h"

/* Default constructor
 */
GestureHistoryBuilder::GestureHistoryBuilder()
{
}

/* Constructor for Gesture History Builder
 */
GestureHistoryBuilder::GestureHistoryBuilder(GestureAnalyzer& analyzer)
{
	gestureAnalyzer = analyzer;
}

/* This function takes a frame and the corresponding frame number,
 * determines the gesture perfomed at that frame, and adds it to
 * to its gesture history list.
 */
void GestureHistoryBuilder::processFrame(Mat& frame, int frameNo)
{
	Gesture gesture = gestureAnalyzer.analyzeNextFaceInFrame(frame, frameNo);
	history.insert(pair<int,Gesture>(frameNo,gesture));
}

GestureHistoryBuilder::~GestureHistoryBuilder(void)
{
}
