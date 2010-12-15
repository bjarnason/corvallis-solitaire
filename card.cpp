/*
 *  card.cpp
 *  solitaire
 *
 *  Created by Ronald Bjarnason on Tue Nov 22 2005.
 *  Copyright (c) 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "card.h"


Card::Card( int i )
{ 
	stack_id = loc_in_stack = -1;  
	face_up = false; 
	deck_playable = false; 
	blocked_val = 0;
	available = UNKNOWN;
	movable = UNKNOWN;
	playable_depth = POS_INF;
	
	if( i % 26 < 13 )					// the card is red
	{
		build_cards[0] = ( i % 13 ) + 14;
		build_cards[1] = ( i % 13 ) + 40;
	}
	else								// the card is black
	{
		build_cards[0] = ( i % 26 ) - 12;
		build_cards[1] = ( i % 26 ) + 14;
	}
	if ( i % 13 != 0 )
	{
		suit_card = i-1;				// only non-aces get to be put on other cards in the suit stacks
	}
	else
	{
		suit_card = NO_CARD;			// aces don't have such luxuries
	}
	
	image = NULL;
	inv_image = NULL;
};

Card::~Card(){
	delete [] inv_image;
	delete [] image;
};

int Card::getXLoc()
{
	switch ( stack_id )
	{
		case STK_B1:
			return STK_B1_X;
			break;
		case STK_B2:
			return STK_B2_X;
			break;
		case STK_B3:
			return STK_B3_X;
			break;
		case STK_B4:
			return STK_B4_X;
			break;
		case STK_B5:
			return STK_B5_X;
			break;
		case STK_B6:
			return STK_B6_X;
			break;
		case STK_B7:
			return STK_B7_X;
			break;
		case STK_DIAM:
			return STK_DIAM_X;
			break;
		case STK_CLUB:
			return STK_CLUB_X;
			break;
		case STK_HEAR:
			return STK_HEAR_X;
			break;
		case STK_SPAD:
			return STK_SPAD_X;
			break;
		case STK_DECK:
			return ( loc_in_stack * COL_OFFSET + 10 );
			break;
	}
	return -1;
}

int Card::getYLoc()
{
	switch ( stack_id )
	{
		case STK_B1:
		case STK_B2:
		case STK_B3:
		case STK_B4:
		case STK_B5:
		case STK_B6:
		case STK_B7:
			return ROW_5_Y - ( loc_in_stack * ROW_OFFSET );
			break;
		case STK_DIAM:
		case STK_CLUB:
		case STK_HEAR:
		case STK_SPAD:
			return STK_SUIT_Y;
			break;
		case STK_DECK:
			if ( deck_playable )
			{
				return ( loc_in_stack % 3 == 2 ? STK_DECK_AVAIL_NEXT : STK_DECK_AVAIL_THIS );
			}
			else
			{
				return STK_DECK_Y;
			}
			break;
	}
	return -1;
}

int Card::getZLoc()
{
	return 2 * ( loc_in_stack + 1 );
}

void Card::setXLoc()
{
	x_pos = getXLoc();
}

void Card::setYLoc()
{
	y_pos = getYLoc();
}

void Card::setZLoc()
{
	z_pos = getZLoc();
}

