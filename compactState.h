/*
 *  compactState.h
 *  solitaire
 *
 *  Created by Ronald Bjarnason on Mon Jan 29 2007.
 *  Copyright (c) 2007 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef COMPACT_STATE_H
#define COMPACT_STATE_H

#include "global.h"

class CompactState
{
public:
	CompactState();
	~CompactState();
	
	void setStack( int card, int stack );
	void setLocInStack( int card, int loc_in_stack );
	void setFaceUp( int card, bool face_up );
	void setDeckOffset( int offset );
	void setLastPlayedDeckLoc( int loc );
	
	int getStack( int card );
	int getLocInStack( int card );
	bool getFaceUp( int card );
	int getDeckOffset();
	int getLastPlayedDeckLoc();
	
	bool isSame( CompactState * cs );
	bool isSame( CompactState * cs, int new_level );
	bool isFaceUpSame( CompactState * cs );
	void copyInto( CompactState * cs );
	void copyInto( CompactState * cs, int roll_out_level );
	
	int roll_out_level_explored;
private:
	int cards[FULL_DECK];
	int deck_state;
};

#endif

