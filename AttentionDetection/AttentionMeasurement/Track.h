#include "cv.h"
#include "Property.h"
#include <algorithm>
#include <list>
#include <stdio.h>

using namespace cv;
using namespace std;
#pragma once

class TrackIterator;
class Track
{
private:
	Rect currFace;                  // Last face added to track
	vector<int> points;             // Index of points contained in currFace
	map<int,Rect> faces;            // Associative array of frame numbers and face detections
	Rect start;                     // First face added to track

	static int numTracks;           // Used to assign trackIDs at object creation

	/* This function merges the contents of another face track into the dictionary.
	 * TODO:
	 *    Throw away any face detections that come from a frame number already contained in the dictionary.
	 *    There should only be one face detection for each unique frame number.
	 */
	void merge(Track& other);

	friend class FaceTracker;
	friend class TrackIterator;
	friend class TrackClusterer;
public:

	Property<int> id;    // Track ID

	/* Constructor
	 */
	Track();
	~Track(void);

	/* This function adds a new face to the track
	 * Input
	 *   r - Face region
	 *   t - Frame number
	 */
	void addFace(Rect r, int t);
	
	/* This function returns the number of faces in the track
	 */
	int getSize();

	/* This function returns a list of frame numbers belonging to the face track
	 */
	vector<int> getFrames();

	/* This function returns a list of face regions belonging to the face track
	 */
	vector<Rect> getFaceROIs();

	/* This function saves the Track contents in a text file
	 * Input
	 *   filename - Name of the file
	 */
	void save(string& filename);

	/* This function loads a Track from file
	 * Input
	 *   filename - Name of text file
	 */
	void load(string& filename);

	/* This function returns an iterator to the Track
	 */
	TrackIterator *getIterator()const;
};

