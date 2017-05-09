#include "StdAfx.h"
#include "TrackIterator.h"

/* Constructs and instance of TrackIterator using a pointer to a Track object
 */
TrackIterator::TrackIterator(const Track *t)
{
	trk = t;
	iter = trk->faces.begin();
}

/* Resets the TrackIterator to the beginning of the Track 
 */
void TrackIterator::reset()
{
	iter = trk->faces.begin();
}

/* Returns the face and frame number at the current position
 */
pair<int,Rect> TrackIterator::current()
{
	return *iter;
}

/* Advances the iterator and returns the face and frame number at the
 * new position.
 */
pair<int,Rect> TrackIterator::next()
{
	return *(++iter);
}

/* Has the TrackIterator reached the end?
 */
bool TrackIterator::isDone()
{
	return iter == trk->faces.end();
}