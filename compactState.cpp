/*
 *  compactState.cpp
 *  solitaire
 *
 *  Created by Ronald Bjarnason on Mon Jan 29 2007.
 *  Copyright (c) 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "compactState.h"
#include "global.h"

using namespace std;


CompactState::CompactState()
{}


CompactState::~CompactState()
{}


void CompactState::setStack( int card, int stack )
{
	int clear_stack = ( 1 << 9 ) + 31;

	cards[card] &= clear_stack;
	cards[card] |= ( stack << 5 );
}

void CompactState::setLocInStack( int card, int loc_in_stack )
{
	int clear_loc = ( 1 << 9 ) + ( 15 << 5 );
	
	cards[card] &= clear_loc;
	cards[card] |= loc_in_stack;
}

void CompactState::setFaceUp( int card, bool face_up )
{
	int clear_face_up = ( 15 << 5 ) + 31;
	
	cards[card] &= clear_face_up;
	
	if ( face_up ) cards[card] |= ( 1 << 9 );
}

void CompactState::setDeckOffset( int offset )
{
	int clear_offset = 31;				// clear_offset = 0000 0000 0001 1111
	
	deck_state &= clear_offset;			// the & operation will perserve the bits storing the 'last-played-deck-loc' information
	
	deck_state |= ( offset << 5 );		// the | operation will set the value of the offset in the sixth and seventh bits (from the bottom)
}

void CompactState::setLastPlayedDeckLoc( int loc )
{
	int clear_loc = ( 3 << 5 );			// clear_loc = 0000 0000 0110 0000
	
	deck_state &= clear_loc;			// the & operation will preserve the two bits storing the 'offset' information
	
	deck_state |= loc;					// the | operation will set the value of loc in the bottom 5 bits
}


int CompactState::getStack( int card )
{
	return ( ( cards[card] >> 5 ) & 15 );
}

int CompactState::getLocInStack( int card )
{
	return ( cards[card] & 31 );
}

bool CompactState::getFaceUp( int card )
{
	return ( ( cards[card] >> 9 ) & 1 );
}

int CompactState::getDeckOffset()
{
	return ( deck_state >> 5 );
}

int CompactState::getLastPlayedDeckLoc()
{
	return ( deck_state & 31 );
}


bool CompactState::isSame( CompactState * cs )
{
	int i;
	
	// changes are most likely to occur in lesser-valued cards - so check them first
	for ( i=0; i<FULL_DECK/NUM_SUITS; i++ )
	{
		if ( cs->cards[i+(0*SUIT_BREAK)] != cards[i+(0*SUIT_BREAK)] || cs->cards[i+(1*SUIT_BREAK)] != cards[i+(1*SUIT_BREAK)] ||
			 cs->cards[i+(2*SUIT_BREAK)] != cards[i+(2*SUIT_BREAK)] || cs->cards[i+(3*SUIT_BREAK)] != cards[i+(3*SUIT_BREAK)] )
		{
			return false;
		}
	}
	
	if ( cs->deck_state != deck_state )
		return false;
	
	return true;
}

bool CompactState::isSame( CompactState * cs, int new_level )
{
	/*if ( isSame( cs ) )
	{
		if ( roll_out_level_explored >= current_level )
		{
			return true;
		}
	}
	return false;
	*/
	return ( isSame( cs ) && ( roll_out_level_explored >= new_level ) );
}

/*
 * bool CompactState::isFaceUpSame( CompactState * cs )
 * like CompactState::isSame(*cs) except that now we are only concerned with cards that are face up
 */
bool CompactState::isFaceUpSame( CompactState * cs )
{
	int i, j;
	int card;
	
	// changes are most likely to occur in lesser-valued cards - so check them first
	for ( i=0; i<FULL_DECK/NUM_SUITS; i++ ) {
		for ( j=0; j<NUM_SUITS; j++ ) {
			card = i+(j*SUIT_BREAK);
			// if either state has this specific card as face-up then check that it matches 
			if ( ( getFaceUp( card ) || cs->getFaceUp( card ) ) && ( cards[card] != cs->cards[card] ) ) return false;
		}
	}
	
	if ( cs->deck_state != deck_state ) return false;
	
	return true;
}

void CompactState::copyInto( CompactState * cs )
{
	int i;
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		cs->cards[i] = cards[i];
	}
	cs->deck_state = deck_state;
}

void CompactState::copyInto( CompactState * cs, int roll_out_level )
{
	int i;
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		cs->cards[i] = cards[i];
	}
	cs->deck_state = deck_state;
	roll_out_level_explored = roll_out_level;
}
