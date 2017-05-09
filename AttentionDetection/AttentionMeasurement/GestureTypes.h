#include "cv.h"

using namespace cv;
using namespace std;
#pragma once

enum HorizontalMotion
{
	HFIXATION_STRAIGHT,
	FIXATION_RIGHT,
	FIXATION_LEFT,
	MOVE_LEFT,
	MOVE_RIGHT,
	HORIZONTAL_UNKNOWN
};

enum VerticalMotion
{
	VFIXATION_STRAIGHT,
	FIXATION_UP,
	FIXATION_DOWN,
	MOVE_UP,
	MOVE_DOWN,
	VERTICAL_UNKNOWN
};

struct Gesture
{
	HorizontalMotion hMotion;
	VerticalMotion vMotion;
};

typedef vector<Mat> GestureHistory;