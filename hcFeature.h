/*
 *  hcFeature.h
 *  solitaire
 *
 *  Created by Ronald Bjarnason on Mon Jan 22 2007.
 *  Copyright (c) 2007 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef HCFEATURE_H
#define HCFEATURE_H

#include "global.h"

class HCFeature
{
public:
	HCFeature();
	~HCFeature();

	void init();
	bool isSame( HCFeature * f );
	
	bool sufficientlyDifferent( HCFeature * f, int share_limit );
	void copyInto( HCFeature * f );
	void prettyPrint();
	void printHCFeatureToChar( int feature_index );
	void printIndexToChar( int card_index );
	void addFeatureAttributes( int feature_index );
	
	int getOSBIndex( int card_mod );
	void HCPGetFeature( int HCP_index, int & feature, int & card1, int & card2 );
	int getOSBCard( int index );
	
	int count;
	int base_features_index;
	int base_features[10];
	
	double weight;
	double score;

};


#endif

