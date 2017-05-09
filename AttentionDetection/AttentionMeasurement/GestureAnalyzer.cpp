#include "StdAfx.h"
#include "GestureAnalyzer.h"
#include "TrackIterator.h"
#include <iostream>
#include <fstream>

ofstream textFileX;
ofstream textFileY;
GestureAnalyzer::GestureAnalyzer()
{
}

string printHGesture(HorizontalMotion hMotion)
{
	string result;
	switch(hMotion)
	{
	   case HFIXATION_STRAIGHT:
		   result = "STRAIGHT";
		   break;
	   case FIXATION_RIGHT:
		   result = "RIGHT";
		   break;
	   case FIXATION_LEFT:
		   result = "LEFT";
		   break;
	   case MOVE_LEFT:
		   result = "MOVE LEFT";
		   break;
	   case MOVE_RIGHT:
		   result = "MOVE RIGHT";
		   break;
	   default:
		   result = "UNKNOWN";
		   break;
	}
	return result;
}

string printVGesture(VerticalMotion vMotion)
{
	string result;
	switch(vMotion)
	{
	   case VFIXATION_STRAIGHT:
		   result = "STRAIGHT";
		   break;
	   case FIXATION_UP:
		   result = "UP";
		   break;
	   case FIXATION_DOWN:
		   result = "DOWN";
		   break;
	   case MOVE_UP:
		   result = "MOVE UP";
		   break;
	   case MOVE_DOWN:
		   result = "MOVE DOWN";
		   break;
	   default:
		   result = "UNKNOWN";
		   break;
	}
	return result;
}

GestureAnalyzer::GestureAnalyzer(TrackIterator* iter)
{
	iterator = iter;
	initialized = false;
	histSize.set(30);
	winSize.set(Size(12,12));
	term.set(TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
	hStr.set(0.05);
	hArea.set(2.0);
	hStp.set(0.01);
	vStr.set(0.05);
	vArea.set(2.0);
	vStp.set(0.01);
	maxFeatures.set(80);
	textFileX.open("vectorsX.txt");
	textFileY.open("vectorsY.txt");
}

/* To eliminate noise, the motion vectors from the last several frames are kept. These
 * vectors are then averaged to compute the global flow vector. This function simply updates
 * the motion history using the most recent computed flow vector.
 * Input
 *   m - Optical flow vector
 */
void GestureAnalyzer::updateMotionHistory(Point2f m)
{
	motionHistory.push_back(m);
	int size = motionHistory.size();
	while(size > histSize.get())
	{
		motionHistory.pop_front();
		size = motionHistory.size();
	}
}

double round(double n)
{
	double x = n * 1000;
	double wholePart = (int)x;
	double decimalPart;
	if (wholePart < 0)
	{
		decimalPart = (x + (wholePart*-1))*-1;
		if (decimalPart > 0.5)
			wholePart--;
	}
	else
	{
		decimalPart = x - wholePart;
		if (decimalPart > 0.5)
			wholePart++;
	}
	return wholePart / 1000;

}

float findMode(vector<float> v)
{
	if (v.empty())
	{
		return 0;
	}

	int* counter = new int[v.size()];
	int maxIdx = 0;
	for(unsigned int i = 0; i < v.size(); i++)
	{
		for(unsigned int j = 0; j < v.size(); j++)
		{
			if (v[i] == v[j])
			{
				counter[i]++;
			}
		}
	}

	for(unsigned int i = 0; i < v.size(); i++)
	{
		if (counter[i] > counter[maxIdx])
			maxIdx = i;
	}

	return v[maxIdx];
}

/* This function returns the most common optical flow vector
 * by mode.
 * Input
 *  v - Optical flow vectors
 */
Point2f GestureAnalyzer::findMajority(vector<Point2f> v)
{
	Point2f e;
	vector<float> x, y;
	// round values to 1 decimal place
 //   for (unsigned int x = 0; x < v.size(); x++)
	//{
	//	cout << v[x] << "\n";
	//}
	//cout << "\n";
	for(unsigned int i = 0; i < v.size(); i++)
	{
		v[i].x = round(v[i].x);
		v[i].y = round(v[i].y);
	}

	for(unsigned int i = 0; i < v.size(); i++)
	{
		x.push_back(v[i].x);
		y.push_back(v[i].y);
	}
	e.x = findMode(x);
	e.y = findMode(y);

	return e;
}

/* This function computes the optical flow of the face in the 
 * current frame.
 * Input
 *   roi - Face roi
 * Returns
 *   The optical flow vectors
 */
vector<Point2f> GestureAnalyzer::computeOpticalFlow(Rect roi)
{
	vector<Point2f> features;
	vector<Point2f> nextFeatures;
	vector<Point2f> flowVectors;
	vector<uchar> status;
	vector<float> error;
	float angle, hypotenuse, vx, vy;
	Mat mask(gray.rows,gray.cols,CV_8UC1,Scalar(0));
	rectangle(mask,Point(roi.x,roi.y),Point(roi.x+roi.width,roi.y+roi.height),CV_RGB(255,255,255),-1);
	goodFeaturesToTrack(gray,features,maxFeatures.get(), 0.01, 0.01, mask);
	if(features.size() > 0)
		cornerSubPix(gray,features,winSize.get(),Size(-1,-1),term.get());
	calcOpticalFlowPyrLK(prevGray,gray,features,nextFeatures,status,error,winSize.get(),3,term.get(),0,0.1);
	
	size_t i,k;
	bool pointsLost = false;
	for(i = k = 0; i < nextFeatures.size(); i++)
	{
		if(status[i])
		{
			angle = atan2(nextFeatures[i].y - features[i].y,nextFeatures[i].x - features[i].x);
			hypotenuse = sqrt(pow(nextFeatures[i].y - features[i].y,2) + pow(nextFeatures[i].x - features[i].x,2));
			vx = hypotenuse * cos(angle);
			vy = hypotenuse * sin(angle);
			flowVectors.push_back(Point2f(vx,vy));
		}
	}
	return flowVectors;
}

/* This function simply averages the flow vectors computed in the last several frames to
 * compute the global flow vector.
 */
Point2f GestureAnalyzer::computeGlobalFlowVector()
{
	Point2f averageFlow;
	list<Point2f>::iterator iter;
	for(iter = motionHistory.begin(); iter != motionHistory.end(); iter++)
	{
		averageFlow += *iter;
	}
	averageFlow.x /= motionHistory.size();
	averageFlow.y /= motionHistory.size();
	return averageFlow;
}

/* This function computes the global optical flow motion vector
 * of the face in the current frame.
 * Input
 *   roi - The face roi
 */
Point2f GestureAnalyzer::computeMotionVector(Rect roi)
{
	vector<Point2f> flowVectors = computeOpticalFlow(roi);

	Point2f majority = findMajority(flowVectors);
	updateMotionHistory(majority);
	return computeGlobalFlowVector();
}

/* This function determines the horizontal gesture based on the horizontal motion of the face.
 */
void GestureAnalyzer::determineHorizontalGesture()
{
	float sum = 0;
	float curr = horizontalStateData.back();
	bool stateChanged = true;

	for(unsigned int i = 0; i < horizontalStateData.size(); i++)
	{
		sum += horizontalStateData[i];
	}
	cout << "curr: " << curr << " sum: " << sum << " hStr: " << hStr.get() << " hstp: " << hStp.get() << " hArea: " << hArea.get() << " hGesture: " << printHGesture(gesture.hMotion) << "\n";
	switch(gesture.hMotion)
	{
	   case HFIXATION_STRAIGHT:
		   if(sum > hArea.get() && curr > hStr.get())
		   {
			   gesture.hMotion = MOVE_LEFT;
			   startedMovingLeft = true;
			   startedMovingRight = false;
		   }
		   else if(sum < -hArea.get() && curr < -hStr.get())
		   {
			   gesture.hMotion = MOVE_RIGHT;
			   startedMovingLeft = false;
			   startedMovingRight = true;
		   }
		   else
		   {
			   stateChanged = false;
		   }
		   break;
	   case FIXATION_RIGHT:
		   if(sum > hArea.get() && curr > hStr.get())
		   {
			   gesture.hMotion = MOVE_LEFT;
		   }
		   else
		   {
			   stateChanged = false;
		   }
		   break;
	   case FIXATION_LEFT:
		   if(sum < -hArea.get() && curr < -hStr.get())
		   {
			   gesture.hMotion = MOVE_RIGHT;
		   }
		   else
		   {
			   stateChanged = false;
		   }
		   break;
	   case MOVE_LEFT:
		   if(curr < hStp.get())
		   {
			   if (startedMovingLeft)
			   {
			       gesture.hMotion = FIXATION_LEFT;
			   }
			   if (startedMovingRight)
			   {
				   gesture.hMotion = HFIXATION_STRAIGHT;
			   }
		   }
		   else
		   {
			   stateChanged = false;
		   }
		   break;
	   case MOVE_RIGHT:
		   if(curr > -hStp.get())
		   {
			   if(startedMovingLeft)
			   {
			      gesture.hMotion = HFIXATION_STRAIGHT;
			   }
			   if(startedMovingRight)
			   {
				   gesture.hMotion = FIXATION_RIGHT;
			   }
		   }
		   else
		   {
			   stateChanged = false;
		   }
		   break;
	   default:
		   break;
	}

	if(stateChanged)
	{
		horizontalStateData.clear();
		horizontalStateData.push_back(curr);
	}
}

/* This function determines the vertical gesture based on the vertical motion of the face.
 */
void GestureAnalyzer::determineVerticalGesture()
{
	float sum = 0;
	float curr = verticalStateData.back();
	bool stateChanged = true;

	for(unsigned int i = 0; i < verticalStateData.size(); i++)
	{
		sum += verticalStateData[i];
	}

	switch(gesture.vMotion)
	{
	   	   case VFIXATION_STRAIGHT:
		   if(sum > vArea.get() && curr > vStr.get())
		   {
			   gesture.vMotion = MOVE_UP;
			   startedMovingUp = true;
			   startedMovingDown = false;
		   }
		   else if(sum < -vArea.get() && curr < -vStr.get())
		   {
			   gesture.vMotion = MOVE_DOWN;
			   startedMovingUp = false;
			   startedMovingDown = true;
		   }
		   else
		   {
			   stateChanged = false;
		   }
		   break;
	   case FIXATION_UP:
		   if(sum < -vArea.get() && curr < -vStr.get())
		   {
			   gesture.vMotion = MOVE_DOWN;
		   }
		   else
		   {
			   stateChanged = false;
		   }
		   break;
	   case FIXATION_DOWN:
		   if(sum > vArea.get() && curr > vStr.get())
		   {
			   gesture.vMotion = MOVE_UP;
		   }
		   else
		   {
			   stateChanged = false;
		   }
		   break;
	   case MOVE_UP:
		   if(curr < vStp.get())
		   {
			   if (startedMovingUp)
			   {
				   gesture.vMotion = FIXATION_UP;
			   }
			   if (startedMovingDown)
			   {
				   gesture.vMotion = VFIXATION_STRAIGHT;
			   }
		   }
		   else
		   {
			   stateChanged = false;
		   }
		   break;
	   case MOVE_DOWN:
		   if(curr > -vStp.get())
		   {
			   if (startedMovingUp)
			   {
				   gesture.vMotion = VFIXATION_STRAIGHT;
			   }
			   if (startedMovingDown)
			   {
			      gesture.vMotion = FIXATION_DOWN;
			   }
		   }
		   else
		   {
			   stateChanged = false;
		   }
		   break;
	   default:
		   break;
	}

	if(stateChanged)
	{
		verticalStateData.clear();
		verticalStateData.push_back(curr);
	}
}

/* This function computes the determines the gesture the individual is currently showing
 */
void GestureAnalyzer::determineGestureState(Rect roi)
{
	Point2f motionVector = computeMotionVector(roi);
	textFileX << motionVector.x << "\n"; 
    textFileY << motionVector.y << "\n";
	horizontalStateData.push_back(motionVector.x);
	verticalStateData.push_back(motionVector.y);
	determineHorizontalGesture();
	determineVerticalGesture();
}

/* This function draws the result of the gesture analysis on the frame.
 * Input
 *    frame - Frame image
 *    face - Face roi
 */
void GestureAnalyzer::drawObjects(Mat& frame, Rect face)
{
	Point p1 = Point(face.x,face.y);
	Point p2 = Point(face.x + face.width,face.y + face.height);
	rectangle(frame,p1,p2,CV_RGB(255,0,0));
	ostringstream oss;
	oss << "(H = " << printHGesture(gesture.hMotion) << ", V = " << printVGesture(gesture.vMotion) << ")";
	putText(frame,oss.str(),p1,CV_FONT_HERSHEY_SIMPLEX,0.3,CV_RGB(0,0,255));
}

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
Gesture GestureAnalyzer::analyzeNextFaceInFrame(Mat& frame, int frameNo)
{
	Gesture result;
	result.hMotion = HORIZONTAL_UNKNOWN;
	result.vMotion = VERTICAL_UNKNOWN; 
	if(!iterator->isDone())
	{
	   pair<int,Rect> curr = iterator->current();
	   if(curr.first == frameNo)
	   {
	       if(!initialized)
		   {
		      cvtColor(frame,gray,CV_BGR2GRAY);
			  equalizeHist(gray,gray);
			  gesture.hMotion = HFIXATION_STRAIGHT;
			  gesture.vMotion = VFIXATION_STRAIGHT;
			  result = gesture;
			  initialized = true;
		   }
		   else
		   {
			  cvtColor(frame,gray,CV_BGR2GRAY);
			  equalizeHist(gray,gray);
			  determineGestureState(curr.second);
			  result = gesture;
			  
		   }
		  iterator->next();
		  cv::swap(gray,prevGray);
		  drawObjects(frame,curr.second);
	   }
	}
	return result;
}

GestureAnalyzer::~GestureAnalyzer(void)
{
}
