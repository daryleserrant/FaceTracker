#include "StdAfx.h"
#include "Track.h"
#include "TrackIterator.h"
#include <iostream>
#include <fstream>
#include <sstream>

int Track::numTracks = 0;

/* String tokenizer.
 */
vector<string> split(string str)
{
	string buf;
	stringstream ss(str);

	vector<string> tokens;

	while (ss >> buf)
		tokens.push_back(buf);

	return tokens;
}

/* Comparator for pair<int,Rect>
 */
struct comparator
{
   bool operator () (pair<int,Rect> arg0, pair<int,Rect> arg1)
   {
	   return arg0.first < arg1.first;
   }
} comparator;

/* Constructor
 */
Track::Track()
{
	id.set(numTracks++);
}

/* This function adds a new face to the track
 * Input
 *   r - Face region
 *   t - Frame number
 */
void Track::addFace(Rect r, int t)
{
	if(faces.size() == 0)
	{
		start = currFace = r;
	}
	else
	{
		currFace = r;
	}
	currFace = r;
	faces.insert(pair<int,Rect>(t,r));

}

/* This function merges the contents of another face track into the dictionary.
 */
void Track::merge(Track& other)
{
	map<int,Rect>::iterator oIT = other.faces.begin();
	map<int,Rect>::iterator it = faces.begin();
	int first, last;
	bool isFirst = true;

	for(oIT; oIT != other.faces.end(); oIT++)
	{
		faces.insert(*oIT);
	}
	
	for(it; it != faces.end(); it++)
	{
		if(isFirst)
		{
			first = it->first;
			isFirst = false;
		}
		last = it->first;
	}
	currFace = faces[last];
	start = faces[first];
}

/* This function returns the number of faces in the track
 */
int Track::getSize()
{
	return faces.size();
}

Track::~Track(void)
{
}

/* This function returns an iterator to the Track
 */
TrackIterator *Track::getIterator()const
{
	return new TrackIterator(this);
}

/* This function saves the Track contents in a text file
 * Input
 *   filename - Name of the file
 */
void Track::save(string& filename)
{
	ofstream outFile(filename);

	map<int,Rect>::iterator it;

	for(it = faces.begin(); it != faces.end(); it++)
	{
		outFile << it->first << " " << it->second.x << " " << it->second.y << " " << it->second.width << " " << it->second.height << "\n";
	}

	outFile.close();
}

/* This function returns a list of frame numbers belonging to the face track
 */
vector<int> Track::getFrames()
{
	vector<int> frames;
	map<int,Rect>::iterator it = faces.begin();
	while(it!= faces.end())
	{
		frames.push_back(it->first);
		it++;
	}
	return frames;
}

/* This function saves the Track contents in a text file
 * Input
 *   filename - Name of the file
 */
vector<Rect> Track::getFaceROIs()
{
	vector<Rect> faceROIs;
	map<int,Rect>::iterator it = faces.begin();
	while(it!= faces.end())
	{
		faceROIs.push_back(it->second);
		it++;
	}
	return faceROIs;
}

/* This function loads a Track from file
 * Input
 *   filename - Name of text file
 */
void Track::load(string& filename)
{
	ifstream inFile(filename);

	faces.clear();
	string line;
	string elem;
	int f,x,y,w,h;
	Rect n;
	int first;
	int last;
	bool isfirst = true;
	while(inFile.good())
	{
		getline(inFile,line);

		stringstream ss(line);

		getline(ss,elem, ' ');

		f = atoi(elem.c_str());
		
		getline(ss,elem, ' ');
		x = atoi(elem.c_str());
		
		getline(ss,elem, ' ');
		y = atoi(elem.c_str());

		getline(ss,elem, ' ');
		w = atoi(elem.c_str());

		getline(ss,elem, ' ');
		h = atoi(elem.c_str());

		n = Rect(x,y,w,h);
		if (n == Rect())
			break;
		faces.insert(pair<int,Rect>(f,n));
		
		if(isfirst)
		{
			first = f;
			isfirst = false;
		}
		last = f;
	}
	start = faces[first];
	currFace = faces[last];
	inFile.close();
}