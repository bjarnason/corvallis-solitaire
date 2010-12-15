/*
 *  feature.h
 *  solitaire
 *
 *  Created by Ronald Bjarnason on Wed Feb 15 2006.
 *  Copyright (c) 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef FEATURE_H
#define FEAUTRE_H

#include "global.h"

class Feature
{
public:
	Feature ();
	~Feature ();
	
	void init();
	bool isSame( Feature * f );
	void copyInto( Feature * f );
	void prettyPrint();
	void printIndexToChar( int );
	void printStackIndexToChar( int );
	void printPropositionTypeToChar( int );
	void printRelationTypeToChar( int );
	void printConstRelationTypeToChar( int );
	void addFeatureAttributes( Feature * f );
	bool shareVariable( Feature * f );
	bool relatedByStaticRelation( Feature * f, bool const_rel[NUM_CONST_RELATIONS][FULL_DECK][FULL_DECK] );
	bool sufficientlyDifferent( Feature * f, int share_limit );
	
	double weight;
	double score;
	
	int count;
	
	bool positive_feature;
	int bp_conj[10][2];		// for each proposition conjunction, identify the proposition type and the card
	int bp_conj_index;
	int br_conj[10][3];		// for each relation conjunction, identify the relation type and the two cards
	int br_conj_index;
	int cr_conj[10][3];		// for each relation conjunction, identify the relation type and the two cards
	int cr_conj_index;
};

#endif
