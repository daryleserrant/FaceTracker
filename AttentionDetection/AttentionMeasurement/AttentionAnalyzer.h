#include "GestureTypes.h"
#include "Property.h"
#pragma once
class AttentionAnalyzer
{
private:
	 AttentionAnalyzer(void);
	 AttentionAnalyzer(AttentionAnalyzer& other);
	~AttentionAnalyzer(void);

	static AttentionAnalyzer *instance;
	vector<GestureHistory> descriptors;

	double computeScore(GestureHistory& history);
public:
	Property<GestureHistory> baseline;
	static AttentionAnalyzer * getInstance();
	void addGestureHistory();
	void computeStatistics();
};

