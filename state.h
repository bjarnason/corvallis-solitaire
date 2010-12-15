/*
 *  state.h
 *  solitaire_IDA*
 *
 *  Created by Ronald Bjarnason on Wed Aug 24 2005.
 *  Copyright (c) 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef STATE_H
#define STATE_H

#include "global.h"
#include "card.h"
#include "feature.h"
#include "hcFeature.h"
#include "compactState.h"
#include "mtRand.h"
#include "uctNode.h"
#include <string>

using namespace std;

class Action
{
public:
	Action(){};
	~Action(){};
	
	int card_to_be_moved;
	int new_stack;
	int old_stack;
	bool reveal;
	
	bool available;						// an action is marked as available if it cannot be trivially undone
	bool dependent;						// an action is markde as dependent if it is a secondary action (dependent on primary actions)
	int dependent_action_card_to_move;  // for secondary actions, the index of the primary action that must be taken prior to this move
	int dependent_action_new_stack;
	
	void copyInto( Action * new_action )
	{
		new_action->card_to_be_moved = card_to_be_moved;
		new_action->new_stack = new_stack;
		new_action->old_stack = old_stack;
		new_action->reveal = reveal;
		new_action->available = available;
		new_action->dependent = dependent;
		new_action->dependent_action_card_to_move = dependent_action_card_to_move;
		new_action->dependent_action_new_stack = dependent_action_new_stack;
	}
	
	bool better( Action * other )
	{
		int my_val = value();
		int his_val = other->value();
		return ( my_val > his_val || ( my_val == his_val && 
									   ( card_to_be_moved % 13 ) < ( other->card_to_be_moved % 13 ) ) );
	}
	
	int value ()
	{
		/*
		 *  Preference order
		 *  1. moves from build stack to suit stack that reveal a new card
		 *  2. moves to suit stacks
		 *  3. moves from build stack to build stack that reveal a new card
		 *  4. moves from deck to build
		 *  5. moves from suit stack to build stack
		 *  6. moves from build stack to build stack that do not reveal a new card
		 */
		if ( old_stack <= LAST_B_STK && new_stack >= STK_DIAM && reveal ) return 6;
		if ( new_stack >= STK_DIAM ) return 5;
		if ( old_stack <= LAST_B_STK && new_stack <= LAST_B_STK && reveal ) return 4;
		if ( old_stack == STK_DECK && new_stack <= LAST_B_STK ) return 3;
		if ( old_stack >= STK_DIAM && new_stack <= LAST_B_STK ) return 2;
		if ( old_stack <= LAST_B_STK && new_stack <= LAST_B_STK && ! reveal ) return 1;
		cerr << "ERROR - I didn't think of this type of move" << endl;
		return 0;
	}
};

class State
{
public:
	State ();
	~State ();
	
	void shuffleAndDeal( MTRand * my_rand_gen );
	int shuffleFaceDownCards( MTRand * my_rand_gen );
	int numFaceDownCards();
	int numDeckCards();
	void dealIdeal();
	void fillStacksFromCards();
	void fillStackFromDeck( int dest, int num_cards, int * source_deck, int &source_deck_index );
	void fillFirstEmptyFromStacks();
	void fillBlockedCards();
	void fillBlockedCardsHelper( int stack_id );
	//void copyInto( State * new_state, double[NUM_BASE_PROPOSITIONS][FULL_DECK] );
	void copyInto( State * new_state );
	void copyFrom( State * new_state );
	int transition( State * new_state, int action_index );
	void transition( State * new_state, int card, int stack );
	void generateDeckPlayable();
	
	void generateAvailableActions();
	void generatePrimaryActions();
	void generateSecondaryActions();
	
	void getThereFromHere( State * get_to_state, int &card_to_move, int &new_stack );
	void getThereFromHere( State * get_to_state, int &a_index );
	
	int cardMovable( int cardId );
	int cardAvailable( int cardId );
	
	bool isSame( State * other_state );
	bool isFaceUpSame( State * other_state );
	bool win();
	
	void print7x5Strips();
	void printFullStrips();
	void psCard( int i );
	void prettyPrint();
	void printSuitStacks();
	void printAvailableActions();
	void printStackIndexToChar( int stack_id );
	void printDeck();
	void printSpaces( int num );
	void printIndexToChar( int card_index );
	
	void printToFile( ofstream &out_file );
	void printSuitStacks( ofstream &out_file );
	void printAvailableActions( ofstream &out_file );
	void printStackIndexToChar( int stack_id, ofstream &out_file );
	void printDeck( ofstream &out_file );
	void printSpaces( int num, ofstream &out_file );
	void printIndexToChar( int card_index, ofstream &out_file );
	void printHCPFeatures();
	
	int admissibleHValue();
	double stateValue();
	double stateValue( double f_weights[NUM_BASE_PROPOSITIONS][FULL_DECK] );
	void fillBooleanFeatures();
	void fillFeaturePropositions( bool [NUM_BASE_PROPOSITIONS][FULL_DECK] );
	void fillFeatureRelations( bool [NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK] );
	void fillHCPFeatures( bool bfp[NUM_BASE_PROPOSITIONS][FULL_DECK], bool bfr[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK], bool hcp[NUM_HCP_FEATURES] );
	//void sortActions( double f_weights[NUM_BASE_PROPOSITIONS][FULL_DECK] );
	
	void fillCompact( CompactState * cs );
	void fillFromCompact( CompactState * cs );
	void fillUCTCompact( CompactState * cs );
	
	bool deadEnd();
	bool oldDeadEnd();
	
	int char2Card( char * in_card );
	long parseGameFile( string in_file, long start_loc );
	
	void sortActions();
	
	int max( int * array, int start, int stop );
	int min( int * array, int start, int stop );
	
	int getOSBIndex( int card_mod );
	void HCPGetFeature( int HCP_index, int & feature, int & card1, int & card2 );
	int getOSBCard( int index );

	//void setFeatureWeights( double f_weights[NUM_FEATURES][FULL_DECK] );
	
	Card * cards[FULL_DECK];
	Action * valid_actions[MAX_NUM_ACTIONS];
	Action * previous_action;
	int valid_actions_index;
	int deck_offset;
	int last_played_deck_loc;
	int first_empty[NUM_STACKS];
	int num_stock_turns;
	
	int build_stack1[STACK_SIZE];
	int build_stack2[STACK_SIZE];	
	int build_stack3[STACK_SIZE];
	int build_stack4[STACK_SIZE];
	int build_stack5[STACK_SIZE];
	int build_stack6[STACK_SIZE];
	int build_stack7[STACK_SIZE];
	int deck[STACK_SIZE];
	int suit_stack_diam[STACK_SIZE];
	int suit_stack_club[STACK_SIZE];
	int suit_stack_hear[STACK_SIZE];
	int suit_stack_spad[STACK_SIZE];
	int *all_stacks[12];

	int uct_best_action;
	
	CompactState * my_cs;
	UCTNode * my_uct_node;
	double value;
};



#endif

