/*
 *  card.h
 *  solitaire
 *
 *  Created by Ronald Bjarnason on Tue Nov 22 2005.
 *  Copyright (c) 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef CARD_H
#define CARD_H

#include "global.h"

class Card
{
public:
	Card( int i );
	~Card();
	
	unsigned char* image;
	unsigned char* inv_image;
	
	unsigned char* gl_texture;
	unsigned char* inv_gl_texture;
	
	int iheight;
	int iwidth;
	
	int stack_id;					// identifies what stack I'm in
	int loc_in_stack;				// identifies my location within the stack
	int blocked_val;				// values my level of being blocked
	bool face_up;					// true if not face-down in a build stack
	bool deck_playable;				// true if in deck and playable
	int build_cards[2];				// the cards this card can be placed on in a build stack
	int suit_card;					// the card this card can be placed on in a suit stack
	
	int available;
	int movable;
	
	int playable_depth;				// an estimate of the number of moves needed to make card playable
	
	float x_pos;
	float y_pos;
	float z_pos;
	
	float home_x;
	float home_y;
	float home_z;
	
	unsigned DL_id_normal;
	unsigned DL_id_invert;
	
	int getXLoc();
	int getYLoc();
	int getZLoc();
	
	void setXLoc();
	void setYLoc();
	void setZLoc();
};
#endif
