#include "Track.h"

class TrackIterator
{
	const Track *trk;                     // Pointer to Track
	map<int,Rect>::const_iterator iter;   // Constant iterator to map
public:

	/* Constructs and instance of TrackIterator using a pointer to a Track object
	 */
	TrackIterator(const Track *t);

	/* Returns the face and frame number at the current position
	 */
	pair<int,Rect> current();

	/* Advances the iterator and returns the face and frame number at the
	 * new position.
	 */
	pair<int,Rect> next();

	/* Has the TrackIterator reached the end?
	 */
	bool isDone();

	/* Resets the TrackIterator to the beginning of the Track 
	 */
	void reset();

};