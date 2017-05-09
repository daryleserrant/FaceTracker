# FaceTracker
An OpenCV C++ program that tracks faces in video. Will output text files (one for each face) containing the bounding box
locations of each face at each frame in the video.

## Prerequisites
Microsoft Visual Studio 2010

## Instructions
1. Open the Visual Studio 2010 Solution File (.sln)
2. Modify line 19 in AttentionMeasurement.cpp to point to video file you'd like to track faces in.
3. Modify the TRK_THRESH variable to control the quality of the face track merging. After face tracking is completed. The program will attempt to merge broken face tracks together using the KMeans clustering algorithm. Any tracks with less than TRK_THRESH faces will be considered as false positives and will be discarded.
4. Build the solution.
5. Run the project. To terminate tracking, hit the Escape Key.
