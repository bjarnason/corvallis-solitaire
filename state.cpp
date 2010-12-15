/*
 *  state.cpp
 *  solitaire_IDA*
 *
 *  Created by Ronald Bjarnason on Wed Aug 24 2005.
 *  Copyright (c) 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "state.h"

using namespace std;

State::State()
{
	int i, j;
	
	all_stacks[STK_B1] = build_stack1;
	all_stacks[STK_B2] = build_stack2;
	all_stacks[STK_B3] = build_stack3;
	all_stacks[STK_B4] = build_stack4;
	all_stacks[STK_B5] = build_stack5;
	all_stacks[STK_B6] = build_stack6;
	all_stacks[STK_B7] = build_stack7;
	all_stacks[STK_DECK] = deck;
	all_stacks[STK_DIAM] = suit_stack_diam;
	all_stacks[STK_CLUB] = suit_stack_club;
	all_stacks[STK_HEAR] = suit_stack_hear;
	all_stacks[STK_SPAD] = suit_stack_spad;
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		cards[i] = new Card( i );
	}
	for ( i=0; i<MAX_NUM_ACTIONS; i++ )
	{
		valid_actions[i] = new Action();
	}
	previous_action = new Action();
	
	for ( i=0; i<NUM_STACKS; i++ )
	{
		for ( j=0; j<STACK_SIZE; j++ )
		{
			all_stacks[i][j] = -1;
		}
	}
	deck_offset = 0;
	last_played_deck_loc = -1;
	num_stock_turns = 0;
	
	my_cs = new CompactState();
	fillCompact( my_cs );
	value = NEG_INF;
}

State::~State()
{
	int i;
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		delete cards[i];
	}
	for ( i=0; i<MAX_NUM_ACTIONS; i++ )
	{
		delete valid_actions[i];
	}
	delete previous_action;
	delete my_cs;
}

void State::shuffleAndDeal( MTRand * my_rand_gen )
{
	int i, j;
	int t_deck[DECK_SIZE];
	int n_deck[DECK_SIZE];
	int n_deck_index = -1;
	int card_index;
	int cards_used = 0;
	
	deck_offset = 0;
	last_played_deck_loc = -1;
	
	for ( i=0; i<NUM_STACKS; i++ )
	{
		for ( j=0; j<STACK_SIZE; j++ )
		{
			all_stacks[i][j] = -1;
		}
		first_empty[i] = 0;
	}
	
	for ( i=0; i<DECK_SIZE; i++ )
	{
		// this eliminates non-used cards
		// this will be pointless in a regular deck, but essential in a half-deck
		t_deck[i] = ( ( i / SUIT_SIZE ) * SUIT_BREAK ) + ( i % SUIT_SIZE );
	}
	
	for ( i=DECK_SIZE; i>0; i-- )
	{
		//card_index = rand() % i;
		card_index = my_rand_gen->randInt() % i;
		//cout << card_index << " ";
		n_deck[++n_deck_index] = t_deck[card_index];
		t_deck[card_index] = t_deck[i-1];
	}
	//cout << endl;
	
	n_deck_index = -1;
	for ( i=FIRST_B_STK; i<=LAST_B_STK; i++ )
	{
		fillStackFromDeck( i, i+1, n_deck, n_deck_index );
		cards_used += i+1;
	}
	fillStackFromDeck( STK_DECK, DECK_SIZE-cards_used, n_deck, n_deck_index );
	
	generateDeckPlayable();
	generateAvailableActions();
	fillCompact( my_cs );
}

int State::numFaceDownCards()
{
	int face_down_count = 0;
	for ( int i=0; i<DECK_SIZE; i++ ){
		if ( !cards[i]->face_up ) face_down_count++; 
	}
	return face_down_count;
}

int State::numDeckCards()
{
	int deck_count = 0;
	for ( int i=0; i<DECK_SIZE; i++ ){
		if ( cards[i]->stack_id == STK_DECK ) deck_count++;
	}
	return deck_count;
}

int State::shuffleFaceDownCards( MTRand * my_rand_gen )
{
	int i, j;
	int t_deck[DECK_SIZE];
	int face_down_count = 0;
	int fd_cards[DECK_SIZE];

	int fd_stack_id[DECK_SIZE];		// tracks the stack_id's of the cards we identify as face-down
	int fd_stack_loc[DECK_SIZE];		// tracks the stack_loc's of the cards we identify as face-down

	int card_index;

	// if I'm reading this code correctly,
	// I just need to shuffle around the stack_id and loc_in_stack
	// of each card and then call fillStacksFromCards() and fillCompact()
	// set everything straight again
	
	// I shouldnt have to call 
	//   fillFirstEmptyFromStacks();
	//   generateDeckPlayable();
	//   generateAvailableActions();
	// because these things won't change by shuffling the face down cards

	//for ( i=0; i<DECK_SIZE; i++ ) cout << " card " << i << " stack_id = " << cards[i]->stack_id << ", loc_in_stack = " << cards[i]->loc_in_stack << endl;
	for ( i=0; i<DECK_SIZE; i++ ){
		if ( !cards[i]->face_up ){
			fd_cards[face_down_count] = i;
			fd_stack_id[face_down_count] = cards[i]->stack_id;
			fd_stack_loc[face_down_count] = cards[i]->loc_in_stack;
			face_down_count++;
		}
	}

	for ( i=face_down_count; i>0; i-- ){
		card_index = my_rand_gen->randInt() % i;

	 	//cout << " setting ";
		//printIndexToChar( fd_cards[i-1] );
		//cout << " to stack[" << fd_stack_id[card_index] << "] loc[" << fd_stack_loc[card_index] << "], previously occupied by";
		//printIndexToChar( fd_cards[card_index] );
		//cout << endl;

		cards[fd_cards[i-1]]->stack_id = fd_stack_id[card_index];
		cards[fd_cards[i-1]]->loc_in_stack = fd_stack_loc[card_index];

		swap( fd_stack_id[i-1], fd_stack_id[card_index] );
		swap( fd_stack_loc[i-1], fd_stack_loc[card_index] );
	}

	//for ( i=0; i<DECK_SIZE; i++ ) cout << " card " << i << " stack_id = " << cards[i]->stack_id << ", loc_in_stack = " << cards[i]->loc_in_stack << endl;

	fillStacksFromCards();
	fillCompact( my_cs );

	return face_down_count;
}

void State::dealIdeal()
{
	int n_deck[DECK_SIZE];
	int n_deck_index = -1;
	int i;
	int cards_used = 0;

	for ( i=0; i<DECK_SIZE; i++ )
	{
		n_deck[i] = DECK_SIZE-i-1;
	}

	for ( i=FIRST_B_STK; i<=LAST_B_STK; i++ )
	{
		fillStackFromDeck( i, i+1, n_deck, n_deck_index );
		cards_used += i+1;
	}
	fillStackFromDeck( STK_DECK, DECK_SIZE-cards_used, n_deck, n_deck_index );
	
	generateDeckPlayable();
	generateAvailableActions();
	fillCompact( my_cs );
}

void State::fillStackFromDeck( int dest, int num_cards, int * source_deck, int &source_deck_index )
{
	int i;
	int card_id;
	
	for ( i=0; i<num_cards; i++ )
	{
		card_id = source_deck[++source_deck_index];
		
		/*
		 * The following code is set in place to allow for playing with decks of different sizes
		 * a card in the queue will be skipped if it is not in the allowed set of cards
		 */
		if ( source_deck_index >= DECK_SIZE )
		{
			card_id = all_stacks[dest][i-1];
			//i++;
			break;
		}
		
		all_stacks[dest][i] = card_id;
		cards[card_id]->stack_id = dest;
		cards[card_id]->loc_in_stack = i;
		cards[card_id]->face_up = ( dest >= STK_DECK );
	}
	
	if ( dest >= FIRST_B_STK && dest <= LAST_B_STK && i > 0 )
	{
		//cards[card_id]->face_up = true;
		//cout << card_id << " " << all_stacks[dest][i-1] << endl;
		cards[all_stacks[dest][i-1]]->face_up = true;
	}
	
	first_empty[dest] = i;
}

void State::fillStacksFromCards()
{
	int i, j;
	
	for ( i=0; i<NUM_STACKS; i++ )
	{
		for ( j=0; j<STACK_SIZE; j++ )
		{
			all_stacks[i][j] = -1;
		}
	}
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		if ( cards[i]->stack_id != -1 )
		{
			all_stacks[cards[i]->stack_id][cards[i]->loc_in_stack] = i;
		}
	}
}

void State::fillFirstEmptyFromStacks()
{
	int i, j;
	
	for ( i=0; i<NUM_STACKS; i++ )
	{
		for ( j=STACK_SIZE-1; j>-1; j-- )
		{
			if ( all_stacks[i][j] != -1 )
			{
				first_empty[i] = j+1;
				break;
			}
			if ( j == 0 )
			{
				first_empty[i] = 0;
			}
		}
	}
}

void State::print7x5Strips()
{
	int i, j;

	cout << "(define (problem thoughtful-s7-t5)" << endl;
	cout << "(:domain thoughtful)" << endl;
	cout << "(:objects"<< endl;
	cout << "    C0 CA C2 C3 C4 C5 C6 C7" << endl;
	cout << "    D0 DA D2 D3 D4 D5 D6 D7" << endl;
	cout << "    H0 HA H2 H3 H4 H5 H6 H7" << endl;
	cout << "    S0 SA S2 S3 S4 S5 S6 S7" << endl;
	cout << " - card" << endl;
	cout << "    COLN0 COLN1 COLN2 COLN3 COLN4 COLN5" << endl;
	cout << " - colnum" << endl;
	cout << "    N0 N1 N2 N3 N4 N5 N6 N7" << endl;
	cout << " - num" << endl;
	cout << "    C D H S" << endl;
	cout << " - suit" << endl << ")" << endl;
	cout << "(:init" << endl;
	cout << "(VALUE C0 N0)" << endl << "(VALUE D0 N0)" << endl << "(VALUE H0 N0)" << endl << "(VALUE S0 N0)" << endl;
	cout << "(VALUE CA N1)" << endl << "(VALUE DA N1)" << endl << "(VALUE HA N1)" << endl << "(VALUE SA N1)" << endl;
	cout << "(VALUE C2 N2)" << endl << "(VALUE D2 N2)" << endl << "(VALUE H2 N2)" << endl << "(VALUE S2 N2)" << endl;
	cout << "(VALUE C3 N3)" << endl << "(VALUE D3 N3)" << endl << "(VALUE H3 N3)" << endl << "(VALUE S3 N3)" << endl;
	cout << "(VALUE C4 N4)" << endl << "(VALUE D4 N4)" << endl << "(VALUE H4 N4)" << endl << "(VALUE S4 N4)" << endl;
	cout << "(VALUE C5 N5)" << endl << "(VALUE D5 N5)" << endl << "(VALUE H5 N5)" << endl << "(VALUE S5 N5)" << endl;
	cout << "(VALUE C6 N6)" << endl << "(VALUE D6 N6)" << endl << "(VALUE H6 N6)" << endl << "(VALUE S6 N6)" << endl;
	cout << "(VALUE C7 N7)" << endl << "(VALUE D7 N7)" << endl << "(VALUE H7 N7)" << endl << "(VALUE S7 N7)" << endl;
	cout << "(COLSUCCESSOR COLN1 COLN0)" << endl;
	cout << "(COLSUCCESSOR COLN2 COLN1)" << endl;
	cout << "(COLSUCCESSOR COLN3 COLN2)" << endl;
	cout << "(COLSUCCESSOR COLN4 COLN3)" << endl;
	cout << "(COLSUCCESSOR COLN5 COLN4)" << endl;
	cout << "(SUCCESSOR N1 N0)" << endl;
	cout << "(SUCCESSOR N2 N1)" << endl;
	cout << "(SUCCESSOR N3 N2)" << endl;
	cout << "(SUCCESSOR N4 N3)" << endl;
	cout << "(SUCCESSOR N5 N4)" << endl;
	cout << "(SUCCESSOR N6 N5)" << endl;
	cout << "(SUCCESSOR N7 N6)" << endl;
	cout << "(SUIT C0 C)" << endl << "(SUIT D0 D)" << endl << "(SUIT H0 H)" << endl << "(SUIT S0 S)" << endl;
	cout << "(SUIT CA C)" << endl << "(SUIT DA D)" << endl << "(SUIT HA H)" << endl << "(SUIT SA S)" << endl;
	cout << "(SUIT C2 C)" << endl << "(SUIT D2 D)" << endl << "(SUIT H2 H)" << endl << "(SUIT S2 S)" << endl;
	cout << "(SUIT C3 C)" << endl << "(SUIT D3 D)" << endl << "(SUIT H3 H)" << endl << "(SUIT S3 S)" << endl;
	cout << "(SUIT C4 C)" << endl << "(SUIT D4 D)" << endl << "(SUIT H4 H)" << endl << "(SUIT S4 S)" << endl;
	cout << "(SUIT C5 C)" << endl << "(SUIT D5 D)" << endl << "(SUIT H5 H)" << endl << "(SUIT S5 S)" << endl;
	cout << "(SUIT C6 C)" << endl << "(SUIT D6 D)" << endl << "(SUIT H6 H)" << endl << "(SUIT S6 S)" << endl;
	cout << "(SUIT C7 C)" << endl << "(SUIT D7 D)" << endl << "(SUIT H7 H)" << endl << "(SUIT S7 S)" << endl;
	cout << "(KING C7)" << endl << "(KING D7)" << endl << "(KING H7)" << endl << "(KING S7)" << endl;
	cout << "(CANSTACK C2 D3)" << endl << "(CANSTACK C2 H3)" << endl;
	cout << "(CANSTACK S2 D3)" << endl << "(CANSTACK S2 H3)" << endl;
	cout << "(CANSTACK D2 C3)" << endl << "(CANSTACK D2 S3)" << endl;
	cout << "(CANSTACK H2 C3)" << endl << "(CANSTACK H2 S3)" << endl;
	cout << "(CANSTACK C3 D4)" << endl << "(CANSTACK C3 H4)" << endl;
	cout << "(CANSTACK S3 D4)" << endl << "(CANSTACK S3 H4)" << endl;
	cout << "(CANSTACK D3 C4)" << endl << "(CANSTACK D3 S4)" << endl;
	cout << "(CANSTACK H3 C4)" << endl << "(CANSTACK H3 S4)" << endl;
	cout << "(CANSTACK C4 D5)" << endl << "(CANSTACK C4 H5)" << endl;
	cout << "(CANSTACK S4 D5)" << endl << "(CANSTACK S4 H5)" << endl;
	cout << "(CANSTACK D4 C5)" << endl << "(CANSTACK D4 S5)" << endl;
	cout << "(CANSTACK H4 C5)" << endl << "(CANSTACK H4 S5)" << endl;
	cout << "(CANSTACK C5 D6)" << endl << "(CANSTACK C5 H6)" << endl;
	cout << "(CANSTACK S5 D6)" << endl << "(CANSTACK S5 H6)" << endl;
	cout << "(CANSTACK D5 C6)" << endl << "(CANSTACK D5 S6)" << endl;
	cout << "(CANSTACK H5 C6)" << endl << "(CANSTACK H5 S6)" << endl;
	cout << "(CANSTACK C6 D7)" << endl << "(CANSTACK C6 H7)" << endl;
	cout << "(CANSTACK S6 D7)" << endl << "(CANSTACK S6 H7)" << endl;
	cout << "(CANSTACK D6 C7)" << endl << "(CANSTACK D6 S7)" << endl;
	cout << "(CANSTACK H6 C7)" << endl << "(CANSTACK H6 S7)" << endl;
	cout << "(COLSPACE COLN0)" << endl;
	for ( i=FIRST_B_STK; i<=LAST_B_STK; i++ )
	{
		cout << "(BOTTOMCOL ";
		psCard( all_stacks[i][0] );
		cout << ")" << endl;
		for ( j=1; j<first_empty[i]; j++ )
		{
			cout << "(ON ";
			psCard( all_stacks[i][j] );
			cout << " ";
			psCard( all_stacks[i][j-1] );
			cout << ")" << endl;
		}
		cout << "(CLEAR ";
		psCard( all_stacks[i][first_empty[i]-1] );
		cout << ")" << endl;
		cout << "(FACEUP ";
		psCard( all_stacks[i][first_empty[i]-1] );
		cout << ")" << endl;
	}
	cout << "(BOTTOMTALON ";
	psCard( all_stacks[STK_DECK][0] );
	cout << ")" << endl;
	for ( j=1; j<first_empty[STK_DECK]; j++)
	{
		cout << "(ONTALON ";
		psCard( all_stacks[STK_DECK][j] );
		cout << " ";
		psCard( all_stacks[STK_DECK][j-1] );
		cout << ")" << endl;
	}
	cout << "(TOPTALON ";
	psCard( all_stacks[STK_DECK][first_empty[STK_DECK]-1] );
	cout << ")" << endl;
	cout << "(TALONPLAYABLE ";
	psCard( all_stacks[STK_DECK][2] );
	cout << ")" << endl;
	cout << "(HOME C0)" << endl << "(HOME D0)" << endl << "(HOME H0)" << endl << "(HOME S0)" << endl << ")" << endl;
	cout << "(:goal" << endl;
	cout << "(and" << endl;
	cout << "(HOME C7)" << endl << "(HOME D7)" << endl << "(HOME H7)" << endl << "(HOME S7)" << endl;
	cout << ")" << endl << ")" << endl << ")" << endl << endl;

}

void State::printFullStrips()
{
	int i, j;

	cout << "(define (problem thoughtful-s13-t7)" << endl;
	cout << "(:domain thoughtful)" << endl;
	cout << "(:objects"<< endl;
	cout << "    C0 CA C2 C3 C4 C5 C6 C7 C8 C9 CT CJ CQ CK" << endl;
	cout << "    D0 DA D2 D3 D4 D5 D6 D7 D8 D9 DT DJ DQ DK" << endl;
	cout << "    H0 HA H2 H3 H4 H5 H6 H7 H8 H9 HT HJ HQ HK" << endl;
	cout << "    S0 SA S2 S3 S4 S5 S6 S7 S8 S9 ST SJ SQ SK" << endl;
	cout << " - card" << endl;
	cout << "    COLN0 COLN1 COLN2 COLN3 COLN4 COLN5 COLN6 COLN7" << endl;
	cout << " - colnum" << endl;
	cout << "    N0 N1 N2 N3 N4 N5 N6 N7 N8 N9 N10 N11 N12 N13" << endl;
	cout << " - num" << endl;
	cout << "    C D H S" << endl;
	cout << " - suit" << endl << ")" << endl;
	cout << "(:init" << endl;
	cout << "(VALUE C0 N0)" << endl << "(VALUE D0 N0)" << endl << "(VALUE H0 N0)" << endl << "(VALUE S0 N0)" << endl;
	cout << "(VALUE CA N1)" << endl << "(VALUE DA N1)" << endl << "(VALUE HA N1)" << endl << "(VALUE SA N1)" << endl;
	cout << "(VALUE C2 N2)" << endl << "(VALUE D2 N2)" << endl << "(VALUE H2 N2)" << endl << "(VALUE S2 N2)" << endl;
	cout << "(VALUE C3 N3)" << endl << "(VALUE D3 N3)" << endl << "(VALUE H3 N3)" << endl << "(VALUE S3 N3)" << endl;
	cout << "(VALUE C4 N4)" << endl << "(VALUE D4 N4)" << endl << "(VALUE H4 N4)" << endl << "(VALUE S4 N4)" << endl;
	cout << "(VALUE C5 N5)" << endl << "(VALUE D5 N5)" << endl << "(VALUE H5 N5)" << endl << "(VALUE S5 N5)" << endl;
	cout << "(VALUE C6 N6)" << endl << "(VALUE D6 N6)" << endl << "(VALUE H6 N6)" << endl << "(VALUE S6 N6)" << endl;
	cout << "(VALUE C7 N7)" << endl << "(VALUE D7 N7)" << endl << "(VALUE H7 N7)" << endl << "(VALUE S7 N7)" << endl;
	cout << "(VALUE C8 N8)" << endl << "(VALUE D8 N8)" << endl << "(VALUE H8 N8)" << endl << "(VALUE S8 N8)" << endl;
	cout << "(VALUE C9 N9)" << endl << "(VALUE D9 N9)" << endl << "(VALUE H9 N9)" << endl << "(VALUE S9 N9)" << endl;
	cout << "(VALUE CT N10)" << endl << "(VALUE DT N10)" << endl << "(VALUE HT N10)" << endl << "(VALUE ST N10)" << endl;
	cout << "(VALUE CJ N11)" << endl << "(VALUE DJ N11)" << endl << "(VALUE HJ N11)" << endl << "(VALUE SJ N11)" << endl;
	cout << "(VALUE CQ N12)" << endl << "(VALUE DQ N12)" << endl << "(VALUE HQ N12)" << endl << "(VALUE SQ N12)" << endl;
	cout << "(VALUE CK N13)" << endl << "(VALUE DK N13)" << endl << "(VALUE HK N13)" << endl << "(VALUE SK N13)" << endl;
	cout << "(COLSUCCESSOR COLN1 COLN0)" << endl;
	cout << "(COLSUCCESSOR COLN2 COLN1)" << endl;
	cout << "(COLSUCCESSOR COLN3 COLN2)" << endl;
	cout << "(COLSUCCESSOR COLN4 COLN3)" << endl;
	cout << "(COLSUCCESSOR COLN5 COLN4)" << endl;
	cout << "(COLSUCCESSOR COLN6 COLN5)" << endl;
	cout << "(COLSUCCESSOR COLN7 COLN6)" << endl;
	cout << "(SUCCESSOR N1 N0)" << endl;
	cout << "(SUCCESSOR N2 N1)" << endl;
	cout << "(SUCCESSOR N3 N2)" << endl;
	cout << "(SUCCESSOR N4 N3)" << endl;
	cout << "(SUCCESSOR N5 N4)" << endl;
	cout << "(SUCCESSOR N6 N5)" << endl;
	cout << "(SUCCESSOR N7 N6)" << endl;
	cout << "(SUCCESSOR N8 N7)" << endl;
	cout << "(SUCCESSOR N9 N8)" << endl;
	cout << "(SUCCESSOR N10 N9)" << endl;
	cout << "(SUCCESSOR N11 N10)" << endl;
	cout << "(SUCCESSOR N12 N11)" << endl;
	cout << "(SUCCESSOR N13 N12)" << endl;
	cout << "(SUIT C0 C)" << endl << "(SUIT D0 D)" << endl << "(SUIT H0 H)" << endl << "(SUIT S0 S)" << endl;
	cout << "(SUIT CA C)" << endl << "(SUIT DA D)" << endl << "(SUIT HA H)" << endl << "(SUIT SA S)" << endl;
	cout << "(SUIT C2 C)" << endl << "(SUIT D2 D)" << endl << "(SUIT H2 H)" << endl << "(SUIT S2 S)" << endl;
	cout << "(SUIT C3 C)" << endl << "(SUIT D3 D)" << endl << "(SUIT H3 H)" << endl << "(SUIT S3 S)" << endl;
	cout << "(SUIT C4 C)" << endl << "(SUIT D4 D)" << endl << "(SUIT H4 H)" << endl << "(SUIT S4 S)" << endl;
	cout << "(SUIT C5 C)" << endl << "(SUIT D5 D)" << endl << "(SUIT H5 H)" << endl << "(SUIT S5 S)" << endl;
	cout << "(SUIT C6 C)" << endl << "(SUIT D6 D)" << endl << "(SUIT H6 H)" << endl << "(SUIT S6 S)" << endl;
	cout << "(SUIT C7 C)" << endl << "(SUIT D7 D)" << endl << "(SUIT H7 H)" << endl << "(SUIT S7 S)" << endl;
	cout << "(SUIT C8 C)" << endl << "(SUIT D8 D)" << endl << "(SUIT H8 H)" << endl << "(SUIT S8 S)" << endl;
	cout << "(SUIT C9 C)" << endl << "(SUIT D9 D)" << endl << "(SUIT H9 H)" << endl << "(SUIT S9 S)" << endl;
	cout << "(SUIT CT C)" << endl << "(SUIT DT D)" << endl << "(SUIT HT H)" << endl << "(SUIT ST S)" << endl;
	cout << "(SUIT CJ C)" << endl << "(SUIT DJ D)" << endl << "(SUIT HJ H)" << endl << "(SUIT SJ S)" << endl;
	cout << "(SUIT CQ C)" << endl << "(SUIT DQ D)" << endl << "(SUIT HQ H)" << endl << "(SUIT SQ S)" << endl;
	cout << "(SUIT CK C)" << endl << "(SUIT DK D)" << endl << "(SUIT HK H)" << endl << "(SUIT SK S)" << endl;
	cout << "(KING CK)" << endl << "(KING DK)" << endl << "(KING HK)" << endl << "(KING SK)" << endl;
	cout << "(CANSTACK C2 D3)" << endl << "(CANSTACK C2 H3)" << endl;
	cout << "(CANSTACK S2 D3)" << endl << "(CANSTACK S2 H3)" << endl;
	cout << "(CANSTACK D2 C3)" << endl << "(CANSTACK D2 S3)" << endl;
	cout << "(CANSTACK H2 C3)" << endl << "(CANSTACK H2 S3)" << endl;
	cout << "(CANSTACK C3 D4)" << endl << "(CANSTACK C3 H4)" << endl;
	cout << "(CANSTACK S3 D4)" << endl << "(CANSTACK S3 H4)" << endl;
	cout << "(CANSTACK D3 C4)" << endl << "(CANSTACK D3 S4)" << endl;
	cout << "(CANSTACK H3 C4)" << endl << "(CANSTACK H3 S4)" << endl;
	cout << "(CANSTACK C4 D5)" << endl << "(CANSTACK C4 H5)" << endl;
	cout << "(CANSTACK S4 D5)" << endl << "(CANSTACK S4 H5)" << endl;
	cout << "(CANSTACK D4 C5)" << endl << "(CANSTACK D4 S5)" << endl;
	cout << "(CANSTACK H4 C5)" << endl << "(CANSTACK H4 S5)" << endl;
	cout << "(CANSTACK C5 D6)" << endl << "(CANSTACK C5 H6)" << endl;
	cout << "(CANSTACK S5 D6)" << endl << "(CANSTACK S5 H6)" << endl;
	cout << "(CANSTACK D5 C6)" << endl << "(CANSTACK D5 S6)" << endl;
	cout << "(CANSTACK H5 C6)" << endl << "(CANSTACK H5 S6)" << endl;
	cout << "(CANSTACK C6 D7)" << endl << "(CANSTACK C6 H7)" << endl;
	cout << "(CANSTACK S6 D7)" << endl << "(CANSTACK S6 H7)" << endl;
	cout << "(CANSTACK D6 C7)" << endl << "(CANSTACK D6 S7)" << endl;
	cout << "(CANSTACK H6 C7)" << endl << "(CANSTACK H6 S7)" << endl;
	cout << "(CANSTACK C7 D8)" << endl << "(CANSTACK C7 H8)" << endl;
	cout << "(CANSTACK S7 D8)" << endl << "(CANSTACK S7 H8)" << endl;
	cout << "(CANSTACK D7 C8)" << endl << "(CANSTACK D7 S8)" << endl;
	cout << "(CANSTACK H7 C8)" << endl << "(CANSTACK H7 S8)" << endl;
	cout << "(CANSTACK C8 D9)" << endl << "(CANSTACK C8 H9)" << endl;
	cout << "(CANSTACK S8 D9)" << endl << "(CANSTACK S8 H9)" << endl;
	cout << "(CANSTACK D8 C9)" << endl << "(CANSTACK D8 S9)" << endl;
	cout << "(CANSTACK H8 C9)" << endl << "(CANSTACK H8 S9)" << endl;
	cout << "(CANSTACK C9 DT)" << endl << "(CANSTACK C9 HT)" << endl;
	cout << "(CANSTACK S9 DT)" << endl << "(CANSTACK S9 HT)" << endl;
	cout << "(CANSTACK D9 CT)" << endl << "(CANSTACK D9 ST)" << endl;
	cout << "(CANSTACK H9 CT)" << endl << "(CANSTACK H9 ST)" << endl;
	cout << "(CANSTACK CT DJ)" << endl << "(CANSTACK CT HJ)" << endl;
	cout << "(CANSTACK ST DJ)" << endl << "(CANSTACK ST HJ)" << endl;
	cout << "(CANSTACK DT CJ)" << endl << "(CANSTACK DT SJ)" << endl;
	cout << "(CANSTACK HT CJ)" << endl << "(CANSTACK HT SJ)" << endl;
	cout << "(CANSTACK CJ DQ)" << endl << "(CANSTACK CJ HQ)" << endl;
	cout << "(CANSTACK SJ DQ)" << endl << "(CANSTACK SJ HQ)" << endl;
	cout << "(CANSTACK DJ CQ)" << endl << "(CANSTACK DJ SQ)" << endl;
	cout << "(CANSTACK HJ CQ)" << endl << "(CANSTACK HJ SQ)" << endl;
	cout << "(CANSTACK CQ DK)" << endl << "(CANSTACK CQ HK)" << endl;
	cout << "(CANSTACK SQ DK)" << endl << "(CANSTACK SQ HK)" << endl;
	cout << "(CANSTACK DQ CK)" << endl << "(CANSTACK DQ SK)" << endl;
	cout << "(CANSTACK HQ CK)" << endl << "(CANSTACK HQ SK)" << endl;
	cout << "(COLSPACE COLN0)" << endl;
	for ( i=FIRST_B_STK; i<=LAST_B_STK; i++ )
	{
		cout << "(BOTTOMCOL ";
		psCard( all_stacks[i][0] );
		cout << ")" << endl;
		for ( j=1; j<first_empty[i]; j++ )
		{
			cout << "(ON ";
			psCard( all_stacks[i][j] );
			cout << " ";
			psCard( all_stacks[i][j-1] );
			cout << ")" << endl;
		}
		cout << "(CLEAR ";
		psCard( all_stacks[i][first_empty[i]-1] );
		cout << ")" << endl;
		cout << "(FACEUP ";
		psCard( all_stacks[i][first_empty[i]-1] );
		cout << ")" << endl;
	}
	cout << "(BOTTOMTALON ";
	psCard( all_stacks[STK_DECK][0] );
	cout << ")" << endl;
	for ( j=1; j<first_empty[STK_DECK]; j++)
	{
		cout << "(ONTALON ";
		psCard( all_stacks[STK_DECK][j] );
		cout << " ";
		psCard( all_stacks[STK_DECK][j-1] );
		cout << ")" << endl;
	}
	cout << "(TOPTALON ";
	psCard( all_stacks[STK_DECK][first_empty[STK_DECK]-1] );
	cout << ")" << endl;
	cout << "(TALONPLAYABLE ";
	psCard( all_stacks[STK_DECK][2] );
	cout << ")" << endl;
	cout << "(HOME C0)" << endl << "(HOME D0)" << endl << "(HOME H0)" << endl << "(HOME S0)" << endl << ")" << endl;
	cout << "(:goal" << endl;
	cout << "(and" << endl;
	cout << "(HOME CK)" << endl << "(HOME DK)" << endl << "(HOME HK)" << endl << "(HOME SK)" << endl;
	cout << ")" << endl << ")" << endl << ")" << endl << endl;
}

void State::psCard( int i )
{
	switch ( i ) 
	{
		case 0: cout << "DA"; break;
		case 1: cout << "D2"; break;
		case 2: cout << "D3"; break;
		case 3: cout << "D4"; break;
		case 4: cout << "D5"; break;
		case 5: cout << "D6"; break;
		case 6: cout << "D7"; break;
		case 7: cout << "D8"; break;
		case 8: cout << "D9"; break;
		case 9: cout << "DT"; break;
		case 10: cout << "DJ"; break;
		case 11: cout << "DQ"; break;
		case 12: cout << "DK"; break;
		case 13: cout << "CA"; break;
		case 14: cout << "C2"; break;
		case 15: cout << "C3"; break;
		case 16: cout << "C4"; break;
		case 17: cout << "C5"; break;
		case 18: cout << "C6"; break;
		case 19: cout << "C7"; break;
		case 20: cout << "C8"; break;
		case 21: cout << "C9"; break;
		case 22: cout << "CT"; break;
		case 23: cout << "CJ"; break;
		case 24: cout << "CQ"; break;
		case 25: cout << "CK"; break;
		case 26: cout << "HA"; break;
		case 27: cout << "H2"; break;
		case 28: cout << "H3"; break;
		case 29: cout << "H4"; break;
		case 30: cout << "H5"; break;
		case 31: cout << "H6"; break;
		case 32: cout << "H7"; break;
		case 33: cout << "H8"; break;
		case 34: cout << "H9"; break;
		case 35: cout << "HT"; break;
		case 36: cout << "HJ"; break;
		case 37: cout << "HQ"; break;
		case 38: cout << "HK"; break;
		case 39: cout << "SA"; break;
		case 40: cout << "S2"; break;
		case 41: cout << "S3"; break;
		case 42: cout << "S4"; break;
		case 43: cout << "S5"; break;
		case 44: cout << "S6"; break;
		case 45: cout << "S7"; break;
		case 46: cout << "S8"; break;
		case 47: cout << "S9"; break;
		case 48: cout << "ST"; break;
		case 49: cout << "SJ"; break;
		case 50: cout << "SQ"; break;
		case 51: cout << "SK"; break;
	}
}

void State::prettyPrint()
{
	if ( this == NULL ) 
	{
		cout << "this state is empty (NULL)" << endl;
		return;
	}
	int i, j;
	int max_height = max( first_empty, FIRST_B_STK, LAST_B_STK );
	
	printSuitStacks();
	printDeck();
	
	for ( j=FIRST_B_STK; j<=LAST_B_STK; j++ )
	{
		cout << " " << j+1 << "  "; 
	}
	cout << endl;
	for ( j=FIRST_B_STK; j<=LAST_B_STK; j++ )
	{
		cout << " -  "; 
	}
	cout << endl;
	
	//cout << " 1   2   3   4   5   6   7" << endl;
	//cout << " -   -   -   -   -   -   -" << endl;
	for ( i=0; i<=max_height; i++ )
	{
		for ( j=FIRST_B_STK; j<=LAST_B_STK; j++ )
		{
			printIndexToChar( all_stacks[j][i] );
		}
		cout << endl;
	}
	printAvailableActions();
	cout << "deck_offset = " << deck_offset << endl;
	cout << "last_played_deck_loc = " << last_played_deck_loc << endl;
	cout << "value = " << value << endl;
	cout << endl;
}

void State::printToFile( ofstream &of )
{	
	int i, j;
	int max_height = max( first_empty, FIRST_B_STK, LAST_B_STK );
	
	of << deck_offset << " deck_offset" << endl;
	of << last_played_deck_loc << " last_played_deck_loc" << endl << endl;
	
	printSuitStacks( of );
	printDeck( of );
	
	for ( j=FIRST_B_STK; j<=LAST_B_STK; j++ )
	{
		of << " " << j+1 << "  "; 
	}
	of << endl;
	for ( j=FIRST_B_STK; j<=LAST_B_STK; j++ )
	{
		of << " -  "; 
	}
	of << endl;
	
	for ( i=0; i<=max_height; i++ )
	{
		for ( j=FIRST_B_STK; j<=LAST_B_STK; j++ )
		{
			printIndexToChar( all_stacks[j][i], of );
		}
		of << endl;
	}
	
	of << endl;
}

void State::printSuitStacks()
{
	int i, j;
	int last_card = -1;
	int space_buffer = 15;
	
	printSpaces( space_buffer );
	cout << "di cl he sp";
	cout << endl;
	printSpaces( space_buffer );
	
	for ( j=STK_DIAM; j<=STK_SPAD; j++ )
	{
		for ( i=0; i<STACK_SIZE; i++ )
		{
			if ( all_stacks[j][i] != -1 )
			{
				last_card = all_stacks[j][i];
			}
		}
		if ( last_card != -1 )
		{
			printIndexToChar( last_card );
		}
		else
		{
			cout << "-- ";
		}
		last_card = -1;
	}
	cout << endl << endl;
}

void State::printSuitStacks( ofstream &of )
{
	int i, j;
	int last_card = -1;
	int space_buffer = 15;
	
	printSpaces( space_buffer, of );
	of << "di cl he sp";
	of << endl;
	printSpaces( space_buffer, of );
	
	for ( j=STK_DIAM; j<=STK_SPAD; j++ )
	{
		for ( i=0; i<STACK_SIZE; i++ )
		{
			if ( all_stacks[j][i] != -1 )
			{
				last_card = all_stacks[j][i];
			}
		}
		if ( last_card != -1 )
		{
			printIndexToChar( last_card, of );
		}
		else
		{
			of << "-- ";
		}
		last_card = -1;
	}
	of << endl << endl;
}

void State::printDeck()
{
	int i;
	
	for ( i=0; i<STACK_SIZE; i++ )
	{
		if ( all_stacks[STK_DECK][i] != -1 )
		{
			printIndexToChar( all_stacks[STK_DECK][i] );
		}
	}
	cout << endl << endl;
}

void State::printDeck( ofstream &of )
{
	int i;
	
	for ( i=0; i<STACK_SIZE; i++ )
	{
		if ( all_stacks[STK_DECK][i] != -1 )
		{
			printIndexToChar( all_stacks[STK_DECK][i], of );
		}
	}
	of << endl << endl;
}

void State::printAvailableActions()
{
	int i;
	
	cout << "Available actions: ";
	if ( valid_actions_index < 0 )
	{
		cout << "NONE";
	}
	cout << endl;
	
	for ( i=0; i<=valid_actions_index; i++ )
	{
		cout << i << ": ";
		printIndexToChar( valid_actions[i]->card_to_be_moved );
		cout << " to stack ";
		printStackIndexToChar( valid_actions[i]->new_stack );
		//cout << "  value = " << valid_actions[i]->value();
		if ( valid_actions[i]->available )
		{
			cout << "  available";
		}
		else
		{
			cout << "  not available";
		}
		if ( valid_actions[i]->dependent )
		{
			cout << ",  dependent on moving card ";
			printIndexToChar( valid_actions[i]->dependent_action_card_to_move );
			cout << " to ";
			printStackIndexToChar( valid_actions[i]->dependent_action_new_stack );
		}
		cout << endl;
	}
}

void State::printAvailableActions( ofstream &of )
{
	int i;
	
	of << "Available actions: ";
	if ( valid_actions_index < 0 )
	{
		of << "NONE";
	}
	of << endl;
	
	for ( i=0; i<=valid_actions_index; i++ )
	{
		printIndexToChar( valid_actions[i]->card_to_be_moved, of );
		of << " to stack ";
		printStackIndexToChar( valid_actions[i]->new_stack, of );
		//of << "  value = " << valid_actions[i]->value();
		of << endl;
	}
}

void State::printSpaces( int num )
{
	int i;
	for ( i=0; i<num; i++ )
	{
		cout << " ";
	}
}

void State::printSpaces( int num, ofstream &out )
{
	int i;
	for ( i=0; i<num; i++ )
	{
		out << " ";
	}
}

void State::printIndexToChar( int card_index )
{
	if ( card_index == -1 )
	{
		cout << "    ";
		return;
	}
	
	switch ( card_index % 13 )
	{
		case 0:
			cout << "A";
			break;
		case 1:
			cout << "2";
			break;
		case 2:
			cout << "3";
			break;
		case 3:
			cout << "4";
			break;
		case 4:
			cout << "5";
			break;
		case 5:
			cout << "6";
			break;
		case 6:
			cout << "7";
			break;
		case 7:
			cout << "8";
			break;
		case 8:
			cout << "9";
			break;
		case 9:
			cout << "T";
			break;
		case 10:
			cout << "J";
			break;
		case 11:
			cout << "Q";
			break;
		case 12:
			cout << "K";
			break;
	}
	switch ( card_index / 13 )
	{
		case 0:
			cout << "D";
			break;
		case 1:
			cout << "C";
			break;
		case 2:
			cout << "H";
			break;
		case 3:
			cout << "S";
			break;
	}
	if ( cards[card_index]->stack_id >= FIRST_B_STK && cards[card_index]->stack_id <= LAST_B_STK )
	{
		if ( cards[card_index]->face_up )
		{
			cout << "^";
		}
		else
		{
			cout << "_";
		}
	}
	if ( cards[card_index]->stack_id == STK_DECK )
	{
		if ( cards[card_index]->deck_playable )
		{
			cout << "*";
		}
		else
		{
			cout << " ";
		}
	}
	cout << " ";
}

void State::printIndexToChar( int card_index, ofstream &of )
{
	if ( card_index == -1 )
	{
		of << "    ";
		return;
	}
	
	switch ( card_index % 13 )
	{
		case 0:
			of << "A";
			break;
		case 1:
			of << "2";
			break;
		case 2:
			of << "3";
			break;
		case 3:
			of << "4";
			break;
		case 4:
			of << "5";
			break;
		case 5:
			of << "6";
			break;
		case 6:
			of << "7";
			break;
		case 7:
			of << "8";
			break;
		case 8:
			of << "9";
			break;
		case 9:
			of << "T";
			break;
		case 10:
			of << "J";
			break;
		case 11:
			of << "Q";
			break;
		case 12:
			of << "K";
			break;
	}
	switch ( card_index / 13 )
	{
		case 0:
			of << "D";
			break;
		case 1:
			of << "C";
			break;
		case 2:
			of << "H";
			break;
		case 3:
			of << "S";
			break;
	}
	if ( cards[card_index]->stack_id >= FIRST_B_STK && cards[card_index]->stack_id <= LAST_B_STK )
	{
		if ( cards[card_index]->face_up )
		{
			of << "^";
		}
		else
		{
			of << "_";
		}
	}
	if ( cards[card_index]->stack_id == STK_DECK )
	{
		if ( cards[card_index]->deck_playable )
		{
			of << "*";
		}
		else
		{
			of << " ";
		}
	}
	of << " ";
}

void State::printStackIndexToChar( int stack_id )
{
	switch ( stack_id )
	{
		case STK_B1:
			cout << "STK_B1";
			break;
		case STK_B2:
			cout << "STK_B2";
			break;
		case STK_B3:
			cout << "STK_B3";
			break;
		case STK_B4:
			cout << "STK_B4";
			break;
		case STK_B5:
			cout << "STK_B5";
			break;
		case STK_B6:
			cout << "STK_B6";
			break;
		case STK_B7:
			cout << "STK_B7";
			break;
		case STK_DECK:
			cout << "STK_DECK";
			break;
		case STK_DIAM:
			cout << "STK_DIAM";
			break;
		case STK_CLUB:
			cout << "STK_CLUB";
			break;
		case STK_HEAR:
			cout << "STK_HEAR";
			break;
		case STK_SPAD:
			cout << "STK_SPAD";
			break;
	}
}

void State::printStackIndexToChar( int stack_id, ofstream &of )
{
	switch ( stack_id )
	{
		case STK_B1:
			of << "STK_B1";
			break;
		case STK_B2:
			of << "STK_B2";
			break;
		case STK_B3:
			of << "STK_B3";
			break;
		case STK_B4:
			of << "STK_B4";
			break;
		case STK_B5:
			of << "STK_B5";
			break;
		case STK_B6:
			of << "STK_B6";
			break;
		case STK_B7:
			of << "STK_B7";
			break;
		case STK_DECK:
			of << "STK_DECK";
			break;
		case STK_DIAM:
			of << "STK_DIAM";
			break;
		case STK_CLUB:
			of << "STK_CLUB";
			break;
		case STK_HEAR:
			of << "STK_HEAR";
			break;
		case STK_SPAD:
			of << "STK_SPAD";
			break;
	}
}

void State::printHCPFeatures()
{
	bool hcp[NUM_HCP_FEATURES];
	bool bfp[NUM_BASE_PROPOSITIONS][FULL_DECK];
	bool bfr[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK];
	
	int feature, card1, card2, i;
	
	bool hcp_osb = false;
	bool hcp_ocb = false;
	bool hcp_gif = false;
	bool hcp_scfd = false;
	bool hcp_if = false;
	bool hcp_isp = false;
	bool hcp_itfu = false;
	
	fillFeaturePropositions( bfp );
	fillFeatureRelations( bfr );
	fillHCPFeatures( bfp, bfr, hcp );
	
	for ( i=0; i<NUM_HCP_FEATURES; i++ )
	{
		if ( hcp[i] )
		{
			HCPGetFeature( i, feature, card1, card2 );
			switch( feature )
			{
				case HCP_OSB:
					if ( !hcp_osb )
						cout << "On Suit Blocked";
					hcp_osb = true;
					cout << "(";
					printIndexToChar( card1 );
					cout << ",";
					printIndexToChar( card2 );
					cout << ") ";
					break;
				case HCP_OCB:
					if ( !hcp_ocb )
						cout << endl << "Off Color Blocked";
					hcp_ocb = true;
					cout << "(";
					printIndexToChar( card1 );
					cout << ",";
					printIndexToChar( card2 );
					cout << ") ";
					break;
				case HCP_GIF:
					if ( !hcp_gif )
						cout << endl << "Group in Foundation";
					hcp_gif = true;
					cout << "(";
					printIndexToChar( card1 );
					cout << ") ";
					break;
				case HCP_SCFD:
					if ( !hcp_scfd )
						cout << endl << "Similar Card Facedown";
					hcp_scfd = true;
					cout << "(";
					printIndexToChar( card1 );
					cout << ") ";
					break;
				case HCP_IF:
					if ( !hcp_if )
						cout << endl << "In Foundation";
					hcp_if = true;
					cout << "(";
					printIndexToChar( card1 );
					cout << ") ";
					break;
				case HCP_ISP:
					if ( !hcp_isp )
						cout << endl << "In Stock Playable";
					hcp_isp = true;
					cout << "(";
					printIndexToChar( card1 );
					cout << ") ";
					break;
				case HCP_ITFU:
					if ( !hcp_itfu )
						cout << endl << "In Tableau Faceup";
					hcp_itfu = true;
					cout << "(";
					printIndexToChar( card1 );
					cout << ") ";
					break;
				default:
				{
					cout << "ERROR in printHCPFeatures" << endl;
					exit( 0 );
				}
			}
		}
	}
	cout << endl;
}

/*void State::copyInto ( State * new_state, double f_weights[NUM_BASE_PROPOSITIONS][FULL_DECK] )
{
	copyInto( new_state );
	new_state->sortActions( f_weights );
}
*/

void State::copyFrom ( State * new_state )
{
	int i;

	for ( i=0; i<FULL_DECK; i++ )
	{
		cards[i]->stack_id = new_state->cards[i]->stack_id;
		cards[i]->loc_in_stack = new_state->cards[i]->loc_in_stack;
		cards[i]->face_up = new_state->cards[i]->face_up;
		cards[i]->deck_playable = new_state->cards[i]->deck_playable;
	}
	fillStacksFromCards();
	fillFirstEmptyFromStacks();

	deck_offset = new_state->deck_offset;
	last_played_deck_loc = new_state->last_played_deck_loc;
	value = new_state->value;

	new_state->previous_action->copyInto( previous_action );
	generateDeckPlayable();
	generateAvailableActions();
	new_state->my_cs->copyInto( my_cs );
}

void State::copyInto ( State * new_state )
{
	int i;
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		new_state->cards[i]->stack_id = cards[i]->stack_id;
		new_state->cards[i]->loc_in_stack = cards[i]->loc_in_stack;
		new_state->cards[i]->face_up = cards[i]->face_up;
		new_state->cards[i]->deck_playable = cards[i]->deck_playable;
	}
	new_state->fillStacksFromCards();
	new_state->fillFirstEmptyFromStacks();
	
	new_state->deck_offset = deck_offset;
	new_state->last_played_deck_loc = last_played_deck_loc;
	new_state->value = value;
	
	previous_action->copyInto( new_state->previous_action );
	
	new_state->generateDeckPlayable();
	new_state->generateAvailableActions();
	my_cs->copyInto( new_state->my_cs );
}
	
int State::transition( State * new_state, int action_index )
{
	if ( action_index == -1 )
		return 0;
	
	if ( ! valid_actions[action_index]->available )
	{
		cerr << "Tried to take action" << action_index << " (unavailable)" << endl;
		prettyPrint();
		exit( 1 );
	}
	
	if ( valid_actions[action_index]->dependent )
	{
		State * t_state = new State();
		transition( t_state, valid_actions[action_index]->dependent_action_card_to_move, valid_actions[action_index]->dependent_action_new_stack );
		t_state->transition( new_state, valid_actions[action_index]->card_to_be_moved, valid_actions[action_index]->new_stack );
		
		delete t_state;
		
	}
	else
	{
		transition( new_state, valid_actions[action_index]->card_to_be_moved, valid_actions[action_index]->new_stack );
	}
	valid_actions[action_index]->copyInto( new_state->previous_action );
	new_state->value = NEG_INF;
	
	return 1;
	
	//new_state->sortActions( f_weights );
}

void State::transition( State * new_state, int card, int stack )
{	
	int i;
	int old_stack = cards[card]->stack_id;
	int old_loc_in_stack = cards[card]->loc_in_stack;
	int new_stack_first_empty = first_empty[stack];

	int card_distance, stock_turns_to_end, num_cards_to_end;

	if ( old_stack == STK_DECK && last_played_deck_loc == -1 )
	{
		num_stock_turns = ( old_loc_in_stack + 1 ) / 3;
	}
	else if ( old_stack == STK_DECK && ( old_loc_in_stack != ( last_played_deck_loc - 1 ) ) )
	{
		num_cards_to_end = ( first_empty[STK_DECK]-1 ) - last_played_deck_loc;
		card_distance = old_loc_in_stack - last_played_deck_loc;

		// when cards can be played before we flip the talon
		// case 1: if the location of the played card mod 3 != 2
		// case 2: if it's the last card in the talon
		// case 3: if we've just played a card that evens out the deck-offset (back to zero)
		if ( ( old_loc_in_stack % 3 != 2 ) 										// case 1	
				|| ( old_loc_in_stack == first_empty[STK_DECK]-1 )						// case 2
				|| ( last_played_deck_loc % 3 == 0 && ( old_loc_in_stack > last_played_deck_loc ) ) ) 		// case 3
		{
			num_stock_turns = ( card_distance / 3 ) + 1;
		}
		else
		{
			if ( last_played_deck_loc == first_empty[STK_DECK] )
				stock_turns_to_end = 0;
			else
				stock_turns_to_end = num_cards_to_end / 3;

			num_stock_turns = stock_turns_to_end + ( ( old_loc_in_stack + 1 ) / 3 ) + 1;
		}
	}
	else num_stock_turns = 0;
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		new_state->cards[i]->stack_id = cards[i]->stack_id;
		new_state->cards[i]->loc_in_stack = cards[i]->loc_in_stack;
		new_state->cards[i]->face_up = cards[i]->face_up;
		new_state->cards[i]->deck_playable = cards[i]->deck_playable;
	}
	new_state->last_played_deck_loc = last_played_deck_loc;
	new_state->deck_offset = deck_offset;
	
	/*
	 * transferring a card from a build stack
	 */
	if ( old_stack >= FIRST_B_STK && old_stack <= LAST_B_STK )
	{
		for ( i=cards[card]->loc_in_stack; i<STACK_SIZE && all_stacks[old_stack][i] != -1; i++ )
		{
			new_state->cards[ all_stacks[old_stack][i] ]->stack_id = stack;
			new_state->cards[ all_stacks[old_stack][i] ]->loc_in_stack = new_stack_first_empty++;
		}
		if ( old_loc_in_stack > 0 )
		{
			new_state->cards[ all_stacks[old_stack][old_loc_in_stack-1] ]->face_up = true;
		}
	}
	/*
	 * transferring a card from a suit stack
	 */
	else if ( old_stack >= STK_DIAM && old_stack <= STK_SPAD )
	{
		new_state->cards[card]->stack_id = stack;
		new_state->cards[card]->loc_in_stack = new_stack_first_empty++;
	}
	/*
	 * transferring a card from the deck
	 */
	else if ( old_stack == STK_DECK )
	{
		new_state->cards[card]->stack_id = stack;
		new_state->cards[card]->loc_in_stack = new_stack_first_empty++;
		for ( i=old_loc_in_stack+1; i<STACK_SIZE && all_stacks[old_stack][i] != -1; i++ )
		{
			new_state->cards[ all_stacks[old_stack][i] ]->loc_in_stack--;
		}
		new_state->deck_offset = ( cards[card]->loc_in_stack % 3 == 2 ||
								   ( cards[card]->loc_in_stack < last_played_deck_loc &&
								   last_played_deck_loc - 1 != cards[card]->loc_in_stack ) ) ? 1 : ( deck_offset + 1 ) % 3;
		// new_state->deck_offset = ( deck_offset + 1 ) % 3;
		new_state->last_played_deck_loc = cards[card]->loc_in_stack;
	}
	
	new_state->fillStacksFromCards();
	new_state->fillFirstEmptyFromStacks();
	new_state->generateDeckPlayable();
	new_state->generateAvailableActions();
	new_state->fillCompact( new_state->my_cs );
}

void State::generateDeckPlayable()
{
	int i;
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		cards[i]->deck_playable = false;
	}
	
	/*
	 * Each third card in the deck is trivially available to play
	 * becuase you can always start clean in the deck
	 */
	for ( i=2; i<STACK_SIZE && all_stacks[STK_DECK][i] != NO_CARD; i+=3 )
	{
		cards[all_stacks[STK_DECK][i]]->deck_playable = true;
	}
	
	/*
	 * If a card is played then the card under it is now playable
	 */
	if ( last_played_deck_loc > 0 )
	{
		cards[all_stacks[STK_DECK][last_played_deck_loc-1]]->deck_playable = true;
	}
	
	/*
	 * If a deck card has been played then we can still play the 
	 * cards that will still be turned over in the rest of deck
	 * (under normal play)
	 */
	for ( i=last_played_deck_loc; i<STACK_SIZE && all_stacks[STK_DECK][i] != NO_CARD; i++ )
	{
		if ( ( i % 3 ) + deck_offset == 2 )
		{
			cards[all_stacks[STK_DECK][i]]->deck_playable = true;
		}
	}
	
	/*
	 * The last card in the deck is always trivially available
	 */
	if ( first_empty[STK_DECK] != 0 )
	{
		cards[all_stacks[STK_DECK][ first_empty[STK_DECK]-1] ]->deck_playable = true;
	}
}

/*
 * Fills the valid_actions structure with possible actions from this state
 * Each action is comprised of a card id (identifying which card to move)
 *  and a stack_id (identifying which stack to move it to)  In cases when
 *  the identified card is not the top-most-card on its stack, all cards
 *  above it are moved with it in a single action
 */
void State::generateAvailableActions()
{
	int i;
	
	generatePrimaryActions();
	generateSecondaryActions();
	//for ( i=0; i<=valid_actions_index; i++ )
	//{
	//	valid_actions[i]->setType();
	//}
	
	for ( i=0; i<=valid_actions_index; i++ )
	{
		if ( valid_actions[i]->new_stack == valid_actions[i]->old_stack )
		{
			cerr << "generated an action to same stack " << endl;
			prettyPrint();
			exit( 0 );
		}
	}
}

void State::generatePrimaryActions()
{
	int i, j;
	
	valid_actions_index = -1;
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		/*
		 * If the card is deck_playable OR
		 *    the card is face up in a build stack OR
		 *    the card is the top card in a suit stack THEN it can be placed on a build stack
		 */
		if ( cards[i]->deck_playable || 
			 ( cards[i]->face_up && cards[i]->stack_id >= FIRST_B_STK && cards[i]->stack_id <= LAST_B_STK ) ||
			 ( cards[i]->stack_id >= STK_DIAM && cards[i]->stack_id <= STK_SPAD &&  // card in suit stack
			   ( first_empty[cards[i]->stack_id]-1 == cards[i]->loc_in_stack ) &&   // top card in stack
			   ( cards[i]->loc_in_stack != 0 && cards[i]->loc_in_stack != 1 ) &&	// card not an ace or a two
			   ( ( all_stacks[STK_DIAM][cards[i]->loc_in_stack-2] == -1 ) ||		// 
				 ( all_stacks[STK_CLUB][cards[i]->loc_in_stack-2] == -1 ) ||		// the card is not
				 ( all_stacks[STK_HEAR][cards[i]->loc_in_stack-2] == -1 ) ||		// locked into suit
				 ( all_stacks[STK_SPAD][cards[i]->loc_in_stack-2] == -1 ) )			// 
			   ) 
			 ) 
		{
			/*
			 * If the card is a king AND 
			 *    it's not already directly on a build stack AND
			 *    there's an empty build stack to put it on  THEN find the first empty stack (they're all the same)
			 * else if the card is not a king, see if the two build_cards[] are open
			 */
			if ( ( i % 13 == ( SUIT_SIZE - 1 ) ) 
				 && ( cards[i]->loc_in_stack > 0 || cards[i]->stack_id == STK_DECK ) 
				 && ( min( first_empty, FIRST_B_STK, LAST_B_STK ) == 0 ) )
			{
				for ( j=FIRST_B_STK; j<=LAST_B_STK; j++ )
				{
					if ( first_empty[j] == 0 )
					{
						valid_actions[++valid_actions_index]->card_to_be_moved = i;
						valid_actions[valid_actions_index]->new_stack = j;
						valid_actions[valid_actions_index]->old_stack = cards[i]->stack_id;
						valid_actions[valid_actions_index]->reveal = 
							( cards[i]->loc_in_stack != 0 && 
							  cards[i]->stack_id >= FIRST_B_STK && cards[i]->stack_id <= LAST_B_STK &&
							  cards[all_stacks[cards[i]->stack_id][cards[i]->loc_in_stack-1]]->face_up == false );							
						
						break;		// no sense is having two redundant actions (king moving to two different stacks)
					}
				}
			}
			else if ( i % 13 != ( SUIT_SIZE - 1 ) && i % 13 != 0 )
			{
				for ( j=0; j<2; j++ )
				{
					if ( cards[cards[i]->build_cards[j]]->stack_id >= FIRST_B_STK && 
						 cards[cards[i]->build_cards[j]]->stack_id <= LAST_B_STK &&
						 first_empty[cards[cards[i]->build_cards[j]]->stack_id]-1 == cards[cards[i]->build_cards[j]]->loc_in_stack )
					{
						valid_actions[++valid_actions_index]->card_to_be_moved = i;
						valid_actions[valid_actions_index]->new_stack = cards[cards[i]->build_cards[j]]->stack_id;
						valid_actions[valid_actions_index]->old_stack = cards[i]->stack_id;
						if ( cards[i]->loc_in_stack != 0 && 
							 cards[i]->stack_id >= FIRST_B_STK && 
							 cards[i]->stack_id <= LAST_B_STK && 
							 all_stacks[cards[i]->stack_id][cards[i]->loc_in_stack-1] == -1 )
						{
							/*cout << "Action: ";
							printIndexToChar( i );
							cout << " moved to stack ";
							printStackIndexToChar( cards[cards[i]->build_cards[j]]->stack_id );
							cout << endl << endl;
							prettyPrint();*/
						}
						valid_actions[valid_actions_index]->reveal = 
							( cards[i]->loc_in_stack != 0 && 
							  cards[i]->stack_id >= FIRST_B_STK && cards[i]->stack_id <= LAST_B_STK &&
							  cards[all_stacks[cards[i]->stack_id][cards[i]->loc_in_stack-1]]->face_up == false );	
					}
				}
			}
		}
		
		/*
		 * If the card is deck playable OR
		 *    the card is the top card in a build stack THEN it can be placed on a suit stack
		 */
		if ( cards[i]->deck_playable ||
			 ( cards[i]->stack_id >= FIRST_B_STK && cards[i]->stack_id <= LAST_B_STK &&
			   ( first_empty[cards[i]->stack_id]-1 == cards[i]->loc_in_stack ) ) )
		{
			/*
			 * If the card is an ace, then it can be placed on a suit stack
			 * else check to see if the suit_stack card is already in a suit stack
			 */
			if ( i % 13 == 0 )
			{
				valid_actions[++valid_actions_index]->card_to_be_moved = i;
				valid_actions[valid_actions_index]->new_stack = ( i / 13 ) + STK_DIAM;
				valid_actions[valid_actions_index]->old_stack = cards[i]->stack_id;
				valid_actions[valid_actions_index]->reveal = 
					( cards[i]->loc_in_stack > 0 && 
					  cards[i]->stack_id >= FIRST_B_STK && cards[i]->stack_id <= LAST_B_STK &&
					  cards[all_stacks[cards[i]->stack_id][cards[i]->loc_in_stack-1]]->face_up == false );	
			}
			else if ( cards[cards[i]->suit_card]->stack_id == ( cards[i]->suit_card / 13 ) + STK_DIAM )
			{
				valid_actions[++valid_actions_index]->card_to_be_moved = i;
				valid_actions[valid_actions_index]->new_stack = cards[cards[i]->suit_card]->stack_id;
				valid_actions[valid_actions_index]->old_stack = cards[i]->stack_id;
				valid_actions[valid_actions_index]->reveal = 
					( cards[i]->loc_in_stack != 0 && 
					  cards[i]->stack_id >= FIRST_B_STK && cards[i]->stack_id <= LAST_B_STK &&
					  cards[all_stacks[cards[i]->stack_id][cards[i]->loc_in_stack-1]]->face_up == false );	
			}
		}
	}
	if ( valid_actions_index >= MAX_NUM_ACTIONS )
	{
		prettyPrint();
		cout << "GENERATED TOO MANY ACTIONS IN generatePrimaryActions() " << endl;
		exit ( 0 );
	}
	
	// if you can figure out a reason that this is here 
	// and not in generateAvailableActions, let me know 
	// (why should we exclude the secondary actions?)
	sortActions();
}

/*
 *  void State::generateSecondaryActions()
 *
 *		parameters:		none
 *
 *		return value:   void
 *
 *		side-effects:   Secondary actions are generated.  These are actions that can only be taken if a primary
 *						action is played first. All actions (primary and secondary) are marked as available 
 *						(true or false), dependent (true or false) and those that are dependent have a 
 *						dependent_action value set.  Meaningless actions (moving a card from face up card on a
 *						build stack to another, or back and forth from a build stack to a suit stack) are
 *						marked as NOT available.  These primary actions are paired with secondary actions that 
 *						make the primary action meaningful (by removing the option to undo the primary)
 */
void State::generateSecondaryActions()
{
	int i, j;
	int pri_card, under_pri, twin;
	int build_1, build_2;
	
	// Mark the primary actions
	
	// there are two types of meaningless actions (actions that can be trivially undone):
	// 1. moving a card from one face-up build stack card to another
	// 2. moving a card down from a suit stack to a build stack
	for ( i=0; i<=valid_actions_index; i++ )
	{
		if ( ( valid_actions[i]->old_stack <= LAST_B_STK &&							// 1. build stack
			   valid_actions[i]->new_stack <= LAST_B_STK &&							// to build stack
			   cards[valid_actions[i]->card_to_be_moved]->loc_in_stack != 0 &&		// without revealing a card
			   valid_actions[i]->reveal == false )				
			 || 
			 ( valid_actions[i]->old_stack >= STK_DIAM &&		// 2. from a suit stack
			   valid_actions[i]->new_stack <= LAST_B_STK ) )	// down to a build stack
		{
			valid_actions[i]->available = false;
			valid_actions[i]->dependent = false;
			valid_actions[i]->dependent_action_card_to_move = -1;
			valid_actions[i]->dependent_action_new_stack = -1;
		}
		else
		{
			valid_actions[i]->available = true;
			valid_actions[i]->dependent = false;
			valid_actions[i]->dependent_action_card_to_move = -1;
			valid_actions[i]->dependent_action_new_stack = -1;
		}
	}
	
	// generate secondary actions (and mark them)
	
	// there four different types of secondary actions -
	// each based on a primary action that is now marked as not available
	//  if a primary card is marked unavailable because it's moving back and forth on the build stacks
	//		1.  the card under the primary card may be able to be placed in a suit stack
	//		2.  we may want to place a different card on the card under the 
	//			primary card (same color and value as the primary)
	//  if a primary card is marked unavailable because it's moving from a suit stack to a build stack
	//		3.  we may want to place a card on the primary once it's in the build stack
	//		4.  we may also want to move the card under the primary from a suit stack to a build stack
	
	
	for ( i=0; i<=valid_actions_index; i++ )
	{
		if ( ! valid_actions[i]->available )
		{
			pri_card = valid_actions[i]->card_to_be_moved;
			under_pri = all_stacks[cards[pri_card]->stack_id][cards[pri_card]->loc_in_stack-1];
			
			if ( valid_actions[i]->old_stack <= LAST_B_STK )
			{
				twin = ( pri_card < 26 ? pri_card + 26 : pri_card - 26 );
				
				if ( cards[cards[under_pri]->suit_card]->stack_id == ( cards[under_pri]->suit_card / 13 ) + STK_DIAM )
				{
					valid_actions[++valid_actions_index]->card_to_be_moved = under_pri;
					valid_actions[valid_actions_index]->new_stack = ( cards[under_pri]->suit_card / 13 ) + STK_DIAM;
					valid_actions[valid_actions_index]->old_stack = cards[under_pri]->stack_id;
					valid_actions[valid_actions_index]->reveal = 
						( cards[under_pri]->loc_in_stack != 0 && 
						  cards[under_pri]->stack_id >= FIRST_B_STK && cards[under_pri]->stack_id <= LAST_B_STK &&
						  cards[all_stacks[cards[under_pri]->stack_id][cards[under_pri]->loc_in_stack-1]]->face_up == false );	
					valid_actions[valid_actions_index]->available = true;
					valid_actions[valid_actions_index]->dependent = true;
					valid_actions[valid_actions_index]->dependent_action_card_to_move = valid_actions[i]->card_to_be_moved;
					valid_actions[valid_actions_index]->dependent_action_new_stack = valid_actions[i]->new_stack;
				}
				else if ( ( cards[twin]->face_up && cards[twin]->stack_id <= LAST_B_STK ) ||
						  ( cards[twin]->deck_playable ) ||
						  ( cards[twin]->stack_id >= STK_DIAM && first_empty[cards[twin]->stack_id] == cards[twin]->loc_in_stack+1 ) )
				{
					valid_actions[++valid_actions_index]->card_to_be_moved = twin;
					valid_actions[valid_actions_index]->new_stack = cards[pri_card]->stack_id;
					valid_actions[valid_actions_index]->old_stack = cards[twin]->stack_id;
					valid_actions[valid_actions_index]->reveal = 
						( cards[twin]->loc_in_stack != 0 && 
						  cards[twin]->stack_id >= FIRST_B_STK && cards[twin]->stack_id <= LAST_B_STK &&
						  cards[all_stacks[cards[twin]->stack_id][cards[twin]->loc_in_stack-1]]->face_up == false );	
					valid_actions[valid_actions_index]->available = true;
					valid_actions[valid_actions_index]->dependent = true;
					valid_actions[valid_actions_index]->dependent_action_card_to_move = valid_actions[i]->card_to_be_moved;
					valid_actions[valid_actions_index]->dependent_action_new_stack = valid_actions[i]->new_stack;
				}
			}
			else if ( valid_actions[i]->old_stack >= STK_DIAM )
			{
				
				if( pri_card % 26 < 13 )					// pri_card is red
				{
					build_1 = ( pri_card % 13 ) + 12;		// these are the cards that can be placed onto the pri_card
					build_2 = ( pri_card % 13 ) + 38;
				}
				else										// pri_card is black
				{
					build_1 = ( pri_card % 26 ) - 14;		// these are the cards that can be placed onto the pri_card
					build_2 = ( pri_card % 26 ) + 12;
				}
				
				if ( build_1 % 13 != 0 && build_1 % 13 != 12 &&				// no point in moving aces or kings to a build stack
					 ( cards[build_1]->face_up && cards[build_1]->stack_id <= LAST_B_STK ) ||
					 ( cards[build_1]->deck_playable ) ||
					 ( cards[build_1]->stack_id >= STK_DIAM && first_empty[cards[build_1]->stack_id] == cards[build_1]->loc_in_stack+1 ) )
				{
					valid_actions[++valid_actions_index]->card_to_be_moved = build_1;
					valid_actions[valid_actions_index]->new_stack = valid_actions[i]->new_stack;
					valid_actions[valid_actions_index]->old_stack = cards[build_1]->stack_id;
					valid_actions[valid_actions_index]->reveal = 
						( cards[build_1]->loc_in_stack != 0 && 
						  cards[build_1]->stack_id >= FIRST_B_STK && cards[build_1]->stack_id <= LAST_B_STK &&
						  cards[all_stacks[cards[build_1]->stack_id][cards[build_1]->loc_in_stack-1]]->face_up == false );	
					valid_actions[valid_actions_index]->available = true;
					valid_actions[valid_actions_index]->dependent = true;
					valid_actions[valid_actions_index]->dependent_action_card_to_move = valid_actions[i]->card_to_be_moved;
					valid_actions[valid_actions_index]->dependent_action_new_stack = valid_actions[i]->new_stack;
				}
				else if ( build_2 % 13 != 0 && build_2 % 13 != 12 &&		// no point in moving aces or kings to a build stack
						  ( cards[build_2]->face_up && cards[build_2]->stack_id <= LAST_B_STK ) ||
						  ( cards[build_2]->deck_playable ) ||
						  ( cards[build_2]->stack_id >= STK_DIAM && first_empty[cards[build_2]->stack_id] == cards[build_2]->loc_in_stack+1 ) ) 
				{
					valid_actions[++valid_actions_index]->card_to_be_moved = build_2;
					valid_actions[valid_actions_index]->new_stack = valid_actions[i]->new_stack;
					valid_actions[valid_actions_index]->old_stack = cards[build_2]->stack_id;
					valid_actions[valid_actions_index]->reveal = 
						( cards[build_2]->loc_in_stack != 0 && 
						  cards[build_2]->stack_id >= FIRST_B_STK && cards[build_2]->stack_id <= LAST_B_STK &&
						  cards[all_stacks[cards[build_2]->stack_id][cards[build_2]->loc_in_stack-1]]->face_up == false );	
					valid_actions[valid_actions_index]->available = true;
					valid_actions[valid_actions_index]->dependent = true;
					valid_actions[valid_actions_index]->dependent_action_card_to_move = valid_actions[i]->card_to_be_moved;
					valid_actions[valid_actions_index]->dependent_action_new_stack = valid_actions[i]->new_stack;
				}
				else
				{
					for ( j=0; j<2; j++ )
					{
						if ( cards[cards[under_pri]->build_cards[j]]->stack_id >= FIRST_B_STK && 
							 cards[cards[under_pri]->build_cards[j]]->stack_id <= LAST_B_STK &&
							 first_empty[cards[cards[under_pri]->build_cards[j]]->stack_id]-1 == cards[cards[under_pri]->build_cards[j]]->loc_in_stack )
						{
							valid_actions[++valid_actions_index]->card_to_be_moved = under_pri;
							valid_actions[valid_actions_index]->new_stack = cards[cards[under_pri]->build_cards[j]]->stack_id;
							valid_actions[valid_actions_index]->old_stack = cards[under_pri]->stack_id;
							valid_actions[valid_actions_index]->reveal = false;
							valid_actions[valid_actions_index]->available = true;
							valid_actions[valid_actions_index]->dependent = true;
							valid_actions[valid_actions_index]->dependent_action_card_to_move = valid_actions[i]->card_to_be_moved;
							valid_actions[valid_actions_index]->dependent_action_new_stack = valid_actions[i]->new_stack;
						}
					}
				}
			}
		}
		if ( valid_actions_index >= MAX_NUM_ACTIONS )
		{
			prettyPrint();
			cout << "GENERATED TOO MANY ACTIONS IN generateSecondaryActions() " << endl;
			exit ( 0 );
		}
	}
}

int State::max( int * array, int start, int stop )
{
	int i, t_max;
	
	t_max = array[start];
	for ( i=start+1; i<=stop; i++ )
	{
		t_max = array[i] > t_max ? array[i] : t_max;
	}
	return t_max;
}

int State::min( int * array, int start, int stop )
{
	int i, t_min;
	
	t_min = array[start];
	for ( i=start+1; i<=stop; i++ )
	{
		t_min = array[i] < t_min ? array[i] : t_min;
	}
	return t_min;
}

/*
 * isSame( State * other_state )
 * returns true if other_state is exactly the same as this state
 * (isomorphs are not taken into consideration)
 */
bool State::isSame( State * other_state )
{
	bool same = true;
	int i;
	
	same = ( ( other_state->deck_offset == deck_offset ) && 
			 ( other_state->last_played_deck_loc == last_played_deck_loc ) );
	
	for ( i=0; i<FULL_DECK && same; i++ )
	{
		same = ( ( other_state->cards[i]->stack_id == cards[i]->stack_id ) &&
				 ( other_state->cards[i]->loc_in_stack == cards[i]->loc_in_stack ) );
	}
	return same;
}

/*
 * isFaceUpSame( State * other_state )
 * returns true if the face up cards in other_state are exactly the same
 */
bool State::isFaceUpSame( State * other_state )
{
	bool same = true;
	int i;
	
	same = ( ( other_state->deck_offset == deck_offset ) && 
			 ( other_state->last_played_deck_loc == last_played_deck_loc ) );
	
	for ( i=0; i<FULL_DECK && same; i++ )
	{
		// if both cards are face_down then consider them the same
		same = ( !cards[i]->face_up && !other_state->cards[i]->face_up ) || 
			( ( other_state->cards[i]->stack_id == cards[i]->stack_id ) &&
			 ( other_state->cards[i]->loc_in_stack == cards[i]->loc_in_stack ) );
	}
	return same;
}

/*
 * State::admissableHValue() returns an admissible heuristic value
 * calculated as the minimum number of moves necessary to secure a 
 * win from this state
 */
int State::admissibleHValue()
{
	return 52 - first_empty[STK_DIAM] - first_empty[STK_CLUB] - first_empty[STK_HEAR] - first_empty[STK_SPAD];
}

bool State::win()
{
	return ( first_empty[STK_DIAM] == SUIT_SIZE && first_empty[STK_CLUB] == SUIT_SIZE && 
			 first_empty[STK_HEAR] == SUIT_SIZE && first_empty[STK_SPAD] == SUIT_SIZE );
}

/*
void State::sortActions( double f_weights[NUM_BASE_PROPOSITIONS][FULL_DECK] )
{
	int t_card;
	int t_new_stack;
	int t_old_stack;
	bool t_reveal;
	int i, j;
	bool t_dependent;
	int t_dependent_action_card_to_move;
	int t_dependent_action_new_stack;
	bool t_available;
	int t_type;
	
	for ( i=0; i<=valid_actions_index; i++ )
	{
		for ( j=i+1; j<=valid_actions_index; j++ )
		{
			if ( f_weights[valid_actions[j]->type][valid_actions[j]->card_to_be_moved] > f_weights[valid_actions[i]->type][valid_actions[i]->card_to_be_moved] )
			//if ( valid_actions[j]->better( valid_actions[i] ) )
			//if ( rand() > rand() )
			{
				t_card = valid_actions[j]->card_to_be_moved;
				t_new_stack = valid_actions[j]->new_stack;
				t_old_stack = valid_actions[j]->old_stack;
				t_reveal = valid_actions[j]->reveal;
				t_dependent = valid_actions[j]->dependent;
				t_dependent_action_card_to_move = valid_actions[j]->dependent_action_card_to_move;
				t_dependent_action_new_stack = valid_actions[j]->dependent_action_new_stack;
				t_available = valid_actions[j]->available;
				t_type = valid_actions[j]->type;
				
				valid_actions[j]->card_to_be_moved = valid_actions[i]->card_to_be_moved;
				valid_actions[j]->new_stack = valid_actions[i]->new_stack;
				valid_actions[j]->old_stack = valid_actions[i]->old_stack;
				valid_actions[j]->reveal = valid_actions[i]->reveal;
				valid_actions[j]->dependent = valid_actions[i]->dependent;
				valid_actions[j]->dependent_action_card_to_move = valid_actions[i]->dependent_action_card_to_move;
				valid_actions[j]->dependent_action_new_stack = valid_actions[i]->dependent_action_new_stack;
				valid_actions[j]->available = valid_actions[i]->available;
				valid_actions[j]->type = valid_actions[i]->type;
				
				valid_actions[i]->card_to_be_moved = t_card;
				valid_actions[i]->new_stack = t_new_stack;
				valid_actions[i]->old_stack = t_old_stack;
				valid_actions[i]->reveal = t_reveal;
				valid_actions[i]->dependent = t_dependent;
				valid_actions[i]->dependent_action_card_to_move = t_dependent_action_card_to_move;
				valid_actions[i]->dependent_action_new_stack = t_dependent_action_new_stack;
				valid_actions[i]->available = t_available;
				valid_actions[i]->type = t_type;
			}
		}
	}	
} */

void State::fillFeaturePropositions( bool bfp[NUM_BASE_PROPOSITIONS][FULL_DECK] )
{
	/*
	 * #define F_IN_FOUNDATION				0
	 * #define F_IN_STOCK_PLAYABLE			1
	 * #define F_IN_STOCK_UNPLAYABLE		2
	 * #define F_IN_TABLEAU_FACE_DOWN		3
	 * #define F_IN_TABLEAU_FACE_UP			4
	 * #define AF_FROM_FOUNDATION			5
	 * #define AF_FROM_TABLEAU				6
	 * #define AF_FROM_STOCK				7
	 * #define AF_TO_FOUNDATION				8
	 * #define AF_TO_TABLEAU				9
	 * #define AF_REVEAL					10
	 */
	
	int i;
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		bfp[F_IN_FOUNDATION][i] = ( cards[i]->stack_id >= STK_DIAM );
		bfp[F_IN_STOCK_PLAYABLE][i] = ( cards[i]->stack_id == STK_DECK && cards[i]->deck_playable );
		bfp[F_IN_STOCK_UNPLAYABLE][i] = ( cards[i]->stack_id == STK_DECK && !cards[i]->deck_playable );
		bfp[F_IN_TABLEAU_FACE_DOWN][i] = ( cards[i]->stack_id <= LAST_B_STK && !cards[i]->face_up );
		bfp[F_IN_TABLEAU_FACE_UP][i] = ( cards[i]->stack_id <= LAST_B_STK && cards[i]->face_up );
		//bfp[AF_FROM_FOUNDATION][i] = ( previous_action->card_to_be_moved == i && previous_action->old_stack >= STK_DIAM );
		//bfp[AF_FROM_TABLEAU][i] = ( previous_action->card_to_be_moved == i && previous_action->old_stack <= LAST_B_STK );
		//bfp[AF_FROM_STOCK][i] = ( previous_action->card_to_be_moved == i && previous_action->old_stack == STK_DECK );
		//bfp[AF_TO_FOUNDATION][i] = ( previous_action->card_to_be_moved == i && previous_action->new_stack >= STK_DIAM );
		//bfp[AF_TO_TABLEAU][i] = ( previous_action->card_to_be_moved == i && previous_action->new_stack <= LAST_B_STK );
		//bfp[AF_REVEAL][i] = ( previous_action->card_to_be_moved == i && previous_action->reveal );
	}
}

void State::fillFeatureRelations( bool bfr[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK] )
{
	/*  
	 * #define F_FOUNDATION_ABOVE			0
	 * #define F_STOCK_ABOVE				1
	 * #define F_TABLEAU_ABOVE				2
	 */
	int i, j;
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		for ( j=0; j<FULL_DECK; j++ )
		{
			if ( cards[i]->stack_id == cards[j]->stack_id && cards[i]->loc_in_stack > cards[j]->loc_in_stack )
			{
				//bfr[F_FOUNDATION_ABOVE][i][j] = cards[i]->stack_id >= STK_DIAM;
				bfr[F_STOCK_ABOVE][i][j] = cards[i]->stack_id == STK_DECK;
				bfr[F_TABLEAU_ABOVE][i][j] = cards[i]->stack_id <= LAST_B_STK;
			}
			else
			{
				//bfr[F_FOUNDATION_ABOVE][i][j] = false;
				bfr[F_STOCK_ABOVE][i][j] = false;
				bfr[F_TABLEAU_ABOVE][i][j] = false;
			}
		}
	}
}

void State::fillHCPFeatures( bool bfp[NUM_BASE_PROPOSITIONS][FULL_DECK], bool bfr[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK], bool hcp[NUM_HCP_FEATURES] )
{
	int i, j, suit_break;
	int hcp_index = -1;
	int card1_rank;
	int ocb1, ocb2;
	int card_below[FULL_DECK];
	/*
#define F_IN_FOUNDATION				0
#define F_IN_STOCK_PLAYABLE			1
#define F_IN_STOCK_UNPLAYABLE		2
#define F_IN_TABLEAU_FACE_DOWN		3
#define F_IN_TABLEAU_FACE_UP		4
	 
#define F_STOCK_ABOVE				0
#define F_TABLEAU_ABOVE				1
#define F_FOUNDATION_ABOVE			2
	 */
	
	// calculate the cards below each of the cards
	for ( i=0; i<FULL_DECK; i++ )
	{
		card_below[i] = all_stacks[cards[i]->stack_id][cards[i]->loc_in_stack-1];
	}
	
	// On Suit Blocking
	for ( i=0; i<FULL_DECK; i++ )
	{
		card1_rank = i % SUIT_BREAK;
		if ( card1_rank != 0 )
		{
			for ( j=suit_break; j<i; j++ )
			{
				//hcp[++hcp_index] = bfr[F_TABLEAU_ABOVE][i][j];
				hcp[++hcp_index] = bfr[F_TABLEAU_ABOVE][i][j] && ( card_below[i]!= -1 && bfp[F_IN_TABLEAU_FACE_DOWN][card_below[i]] );
			}
		}
		else
		{
			suit_break = i;
		}
	}
	
	// Off Color Blocking
	for ( i=0; i<FULL_DECK; i++ )
	{
		
		if( i % 26 < 13 )					// the card is red
		{
			ocb1 = ( i % 13 ) + 14;
			ocb2 = ( i % 13 ) + 40;
		}
		else								// the card is black
		{
			ocb1 = ( i % 26 ) - 12;
			ocb2 = ( i % 26 ) + 14;
		}
		
		card1_rank = i % SUIT_BREAK;
		if ( card1_rank != 0 && card1_rank != 12 )
		{
			//hcp[++hcp_index] = ( bfr[F_TABLEAU_ABOVE][i][ocb1] && bfp[F_IN_TABLEAU_FACE_DOWN][ocb1] );
			//hcp[++hcp_index] = ( bfr[F_TABLEAU_ABOVE][i][ocb2] && bfp[F_IN_TABLEAU_FACE_DOWN][ocb2] );
			hcp[++hcp_index] = ( bfr[F_TABLEAU_ABOVE][i][ocb1] && bfp[F_IN_TABLEAU_FACE_DOWN][ocb1] ) && ( card_below[i]!= -1 && bfp[F_IN_TABLEAU_FACE_DOWN][card_below[i]] );
			hcp[++hcp_index] = ( bfr[F_TABLEAU_ABOVE][i][ocb2] && bfp[F_IN_TABLEAU_FACE_DOWN][ocb2] ) && ( card_below[i]!= -1 && bfp[F_IN_TABLEAU_FACE_DOWN][card_below[i]] );
		}
	}
	
	// Group In Foundation
	for ( i=0; i<SUIT_BREAK; i++ )
	{
		hcp[++hcp_index] = ( bfp[F_IN_FOUNDATION][i+(0*SUIT_BREAK)] && bfp[F_IN_FOUNDATION][i+(1*SUIT_BREAK)] && 
							 bfp[F_IN_FOUNDATION][i+(2*SUIT_BREAK)] && bfp[F_IN_FOUNDATION][i+(3*SUIT_BREAK)] );
	}
	
	// Simiar Card Face Down
	for ( i=0; i<FULL_DECK/2; i++ )
	{
		hcp[++hcp_index] = bfp[F_IN_TABLEAU_FACE_DOWN][i] && bfp[F_IN_TABLEAU_FACE_DOWN][i+(FULL_DECK/2)];
	}
	
	// In Foundation
	for ( i=0; i<FULL_DECK; i++ )
	{
		hcp[++hcp_index] = bfp[F_IN_FOUNDATION][i];
	}
	
	// In Stock Playable
	for ( i=0; i<FULL_DECK; i++ )
	{
		hcp[++hcp_index] = bfp[F_IN_STOCK_PLAYABLE][i];
	}
	
	// In Tableau Face Down
	for ( i=0; i<FULL_DECK; i++ )
	{
		hcp[++hcp_index] = bfp[F_IN_TABLEAU_FACE_DOWN][i];
	}
	
	if ( hcp_index != NUM_HCP_FEATURES-1 )
	{
		cout << "ERROR in fillHCPFeatures() 0 " << endl;
		exit( 0 );
	}
}

void State::getThereFromHere( State * get_to_state, int &card_to_move, int &new_stack )
{
	int i;
	bool found_a_move = false;
	
	State * t_state = new State();
	
	for ( i=0; i<=valid_actions_index; i++ )
	{
		if ( valid_actions[i]->available )
		{
			transition( t_state, i );
			if ( t_state->isSame( get_to_state ) )
			{
				card_to_move = valid_actions[i]->card_to_be_moved;
				new_stack = valid_actions[i]->new_stack;
				found_a_move = true;
				if ( valid_actions[i]->dependent )
				{
					cout << "dependent on :" ;
					printIndexToChar( valid_actions[i]->dependent_action_card_to_move );
					cout << "->";
					printStackIndexToChar( valid_actions[i]->dependent_action_new_stack );
					cout << endl;
				}
				break;
			}
		}
	}
	if ( ! found_a_move )
	{
		cerr << "could not find a valid move to get from " << endl;
		prettyPrint();
		cerr << "to here" << endl;
		get_to_state->prettyPrint();
		cerr << endl << endl;
	}
	
	delete t_state;
}

void State::getThereFromHere( State * get_to_state, int &a_index )
{
	int i;
	bool found_a_move = false;
	
	State * t_state = new State();
	
	for ( i=0; i<=valid_actions_index; i++ )
	{
		if ( valid_actions[i]->available )
		{
			transition( t_state, valid_actions[i]->card_to_be_moved, valid_actions[i]->new_stack );
			if ( t_state->isSame( get_to_state ) )
			{
				a_index = i;
				found_a_move = true;
				break;
			}
		}
	}
	if ( ! found_a_move )
	{
		cerr << "could not find a valid move to get from " << endl;
		prettyPrint();
		cerr << "to here" << endl;
		get_to_state->prettyPrint();
		cerr << endl << endl;
		exit( 1 );
	}
	
	delete t_state;
}

int State::cardMovable( int card_id )
{
	return NO;
}

int State::cardAvailable( int card_id )
{
	int i;
	int t_card_movable;
	bool above_cards_movable = true;
	
	if ( cards[card_id]->available != UNKNOWN )
	{
		return cards[card_id]->available;
	}
	
	// only cards in the build stacks can be marked as available
	if ( cards[card_id]->stack_id > LAST_B_STK )
	{
		cards[card_id]->available = NO;
		return NO;
	}
	
	// mark this so that it's known during recursive calls
	cards[card_id]->available = PROCESSING;
	
	// check to see if we are the top card in a build stack
	if ( all_stacks[cards[card_id]->stack_id][cards[card_id]->loc_in_stack+1] == NO_CARD )
	{
		cards[card_id]->available = YES;
		return YES;
	}
	
	// check to see if all of the cards above us are movable
	for ( i=cards[card_id]->loc_in_stack+1; i != NO_CARD && i < STACK_SIZE; i++ )
	{
		t_card_movable = cardMovable ( all_stacks[cards[card_id]->stack_id][i] );
		switch ( t_card_movable )
		{
			case NO:
				cards[card_id]->available = NO;
				return NO;
				break;
			case PROCESSING:
				above_cards_movable = false;
				break;
			case YES:
				break;
			case UNKNOWN:
			default:
				cerr << "ERROR in State::cardAvailable()" << endl;
				break;
		}
	}
	
	if ( above_cards_movable )
	{
		cards[card_id]->available = YES;
		return YES;
	}	
	
	return PROCESSING;
}


bool State::oldDeadEnd()
{
	bool movable[FULL_DECK];
	bool receivable[FULL_DECK];
	bool move_candidate[FULL_DECK];
	bool suit_stackable[FULL_DECK];
	bool old_open_stack;
	bool open_stack;
	
	int i, j;
	int movable_total = 0;
	int receivable_total = 0;
	int move_candidate_total = 0;
	int suit_stackable_total = 0;
	
	int old_movable_total = 0;
	int old_receivable_total = 0;
	int old_move_candidate_total = 0;
	int old_suit_stackable_total = 0;
	
	int card_above;
	
	bool changed = true;
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		movable[i] = false;
		receivable[i] = false;
		move_candidate[i] = false;
		suit_stackable[i] = false;
	}
	open_stack = false;
	old_open_stack = false;
	
	// mark the tops of all the stacks as move_candidates and receivable
	// mark all deck_playable cards as move_candidates
	for ( i=0; i<FULL_DECK; i++ )
	{
		if ( cards[i]->stack_id <= LAST_B_STK && ( cards[i]->loc_in_stack+1 == first_empty[cards[i]->stack_id] ) )
		{
			move_candidate[i] = true;
			receivable[i] = true;
			move_candidate_total++;
			receivable_total++;
		}
		if ( cards[i]->stack_id == STK_DECK && cards[i]->deck_playable )
		{
			move_candidate[i] = true;
			move_candidate_total++;
		}
	}
	
	while ( changed )
	{
		changed = false;
		
		for ( i=0; i<FULL_DECK; i++ )
		{
			if ( move_candidate[i] && ! movable[i] )
			{
				// if it is an ace or if it's suit_card is suit_stackable,
				// then we can mark it as suit_stackable
				if ( i % 13 == 0 || suit_stackable[cards[i]->suit_card] )
				{
					suit_stackable[i] = true;
					movable[i] = true;
					if ( cards[i]->stack_id <= LAST_B_STK && cards[i]->loc_in_stack == 0 )
					{
						open_stack = true;
					}
				}
				// if the card is a king and we have an open stack, then this card is movable and receivable
				if ( i % 13 == ( SUIT_SIZE-1 ) && open_stack )
				{
					movable[i] = true;
					receivable[i] = true;
				}
			}
			if ( ( !movable[i] || !receivable[i] ) && move_candidate[i] && ( receivable[cards[i]->build_cards[0]] || receivable[cards[i]->build_cards[1]] ) )
			{
				movable[i] = true;
				receivable[i] = true;
				if ( cards[i]->stack_id <= LAST_B_STK && cards[i]->loc_in_stack == 0 )
				{
					open_stack = true;
				}
			}
			if ( !suit_stackable[i] && movable[i] && suit_stackable[cards[i]->suit_card] )
			{
				suit_stackable[i] = true;
			}
		}
		
		for ( i=0; i<FULL_DECK; i++ )
		{
			if ( cards[i]->stack_id == -1 )
			{
				card_above = -1;
			}
			else
			{
				card_above = all_stacks[cards[i]->stack_id][cards[i]->loc_in_stack+1];
			}
			if ( card_above != -1 )
			{
				if ( cards[i]->stack_id <= LAST_B_STK && movable[card_above] )
				{
					move_candidate[i] = true;
					receivable[i] = true;
				}
			}
			
			if ( cards[i]->stack_id == STK_DECK )
			{
				card_above = all_stacks[STK_DECK][cards[i]->loc_in_stack+1];
				if ( card_above != -1 )
				{
					if ( ! move_candidate[i] && movable[card_above] )
					{
						move_candidate[i] = true;
					}
				}
				if ( cards[i]->loc_in_stack % 3 == 0 )
				{
					for ( j=0; j<cards[i]->loc_in_stack; j++ )
					{
						if ( !move_candidate[i] && movable[all_stacks[STK_DECK][j]] && ( cards[all_stacks[STK_DECK][j]]->loc_in_stack % 3 ) == 2 )
						{
							move_candidate[i] = true;
						}
					}
				}
				if ( cards[i]->loc_in_stack % 3 == 1 )
				{
					for ( j=0; j<cards[i]->loc_in_stack; j++ )
					{
						if ( !move_candidate[i] && movable[all_stacks[STK_DECK][j]] && ( cards[all_stacks[STK_DECK][j]]->loc_in_stack % 3 ) == 1 )
						{
							move_candidate[i] = true;
						}
					}
				}
			}
		}
		
		
		 // reset the counters and check to see if anything has changed
		 // if it has, then we start the loop all over again
		
		movable_total = 0;
		receivable_total = 0;
		move_candidate_total = 0;
		suit_stackable_total = 0;
		for ( i=0; i<FULL_DECK; i++ )
		{
			if ( movable[i] ){ movable_total++; }
			if ( receivable[i] ){ receivable_total++; }
			if ( move_candidate[i] ){ move_candidate_total++;}
			if ( suit_stackable[i] ){ suit_stackable_total++;}
		}
		if ( movable_total != old_movable_total ||
			 receivable_total != old_receivable_total ||
			 move_candidate_total != old_move_candidate_total ||
			 suit_stackable_total != old_suit_stackable_total )
		{
			old_movable_total = movable_total;
			old_receivable_total = receivable_total;
			old_move_candidate_total = move_candidate_total;
			old_suit_stackable_total = suit_stackable_total;
			changed = true;
		}
	}
	
	if ( suit_stackable_total == DECK_SIZE )
	{
		return false;
	}
	
	for ( i=0; i<SUIT_SIZE; i++ )
	{
		for ( j=0; j<NUM_SUITS; j++ )
		{
			if ( false &&  ! suit_stackable[( 13*j )+i] )
			{
				cout << "couldn't play the ";
				printIndexToChar( ( 13*j )+i );
				cout << endl;
			}
		}
	}
	return true;
}

/*
 * this function checks to see if the current state is a dead end
 */

bool State::deadEnd()
{
	bool movable[FULL_DECK];					// a card is movable is it can be legally moved from its current location
	bool buildable[FULL_DECK];					// a card is buildable if it is at the top of a build stack
	bool move_candidate[FULL_DECK];				// a card is a move candidate if it can be considered for a move
	bool suit_stackable[FULL_DECK];				// a card is suit-stackable if it can be moved to the suit stack
	bool open_stack;							// open-stack is true if one or more of the build stacks are empty
	
	int low_deck_movable[2] = {POS_INF-1, POS_INF};		// we need to keep track of the earliest two playable stack cards
	int i;
	int total_suit_stackable = 0;
	
	int card_above;
	int my_stack;
	int my_stack_loc;
	bool is_top_card;
	bool is_build_card;
	bool is_deck_card;
	bool is_suit_card;
	
	bool changed = true;
	
	int count = 0;
	
	/*
	 * initially all values are false when they are made true, they will remain true
	 */
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		movable[i] = false;
		buildable[i] = false;
		move_candidate[i] = false;
		suit_stackable[i] = false;
	}
	open_stack = false;
	
	/*
	 * check all of the build stacks - if any of them have no cards, mark open_stack as true
	 */
	for ( i=FIRST_B_STK; i<=LAST_B_STK; i++ )
	{
		if ( first_empty[i] == 0 )
		{
			open_stack = true;
		}
	}
	
	/*
	 * mark the tops of the build stacks as move-candidates and buildable
	 * mark all cards in the suit stacks as suit-stackable
	 * mark the tops of the suit stacks as move-candidates
	 * mark all deck-playable cards as move-candidates
	 */
	for ( i=0; i<FULL_DECK; i++ )
	{
		/*
		 * these are the values that we use frequently in checking -
		 * these make notation easier 
		 */
		card_above = ( cards[i]->stack_id == -1 ) ? -1 : all_stacks[cards[i]->stack_id][cards[i]->loc_in_stack+1];		// the card above cards[i] (may be -1)
		my_stack = cards[i]->stack_id;												// the stack of cards[i]
		my_stack_loc = cards[i]->loc_in_stack;										// the stack location of cards[i]
		is_top_card = ( first_empty[my_stack]-1 == my_stack_loc );					// true if cards[i] is the top card in its stack
		is_build_card = ( my_stack >= 0 && my_stack <= STK_B7 );					// true if cards[i] is in one of the seven build stacks
		is_deck_card = ( my_stack == STK_DECK );									// true if cards[i] is in the deck
		is_suit_card = ( my_stack >= STK_DIAM );									// true if cards[i] is in one of the four suit stacks
		
		// tops of build stacks are move-candidates and buildable
		if ( is_build_card && is_top_card )
		{
			move_candidate[i] = true;
			buildable[i] = true;
		}
		
		// face-up build cards are move-candidates
		if ( is_build_card && cards[i]->face_up )
		{
			move_candidate[i] = true;
		}
		
		// cards in suit stacks are suit-stackable
		if ( is_suit_card )
		{
			suit_stackable[i] = true;
			
			// tops of suit stacks are move-candidates
			if ( is_top_card )
			{
				move_candidate[i] = true;
			}
		}
		
		// deck-playable cards are move-candidates
		if ( is_deck_card && cards[i]->deck_playable )
		{
			move_candidate[i] = true;
		}
	}
	
	while ( changed )
	{
		changed = false;
		count++;
		
		if ( false )
		{
			cout << "loop " << count << endl;
		}
		
		for ( i=0; i<FULL_DECK; i++ )
		{
			/*
			 * these are the values that we use frequently in checking -
			 * these make notation easier 
			 */
			card_above = ( cards[i]->stack_id == -1 ) ? -1 : all_stacks[cards[i]->stack_id][cards[i]->loc_in_stack+1];		// the card above cards[i] (may be -1)
			
			// the stack of cards[i]
			my_stack = cards[i]->stack_id;
			
			// the stack location of cards[i]
			my_stack_loc = cards[i]->loc_in_stack;		
			
			// true if cards[i] is the top card in its stack
			is_top_card = ( first_empty[my_stack]-1 == my_stack_loc );					
			
			// true if cards[i] is in one of the seven build stacks OR if the card has been made buildable in previous iterations
			is_build_card = ( ( my_stack >= 0 && my_stack <= STK_B7 ) || buildable[i] );
			
			// true if cards[i] is in the deck
			is_deck_card = ( my_stack == STK_DECK );
			
			// true if cards[i] is in one of the four suit stacks OR if the card has been made suit-stackable in previous iterations
			is_suit_card = ( ( my_stack >= STK_DIAM ) || suit_stackable[i] );									
			
			
			/*
			 * there are two ways a card can become suit-stackable
			 * 1. An ace is a move-candiate ( case 1 )
			 * 2. A non-ace is a move-candidate whose suit-card is suit-stackable
			 *       and is either not in the build stack, or whose card_above is movable ( case 2 )
			 */
			if ( !suit_stackable[i] && 
				 move_candidate[i] && 
				 ( i % 13 == 0 ||															// case 1
				   ( suit_stackable[cards[i]->suit_card] && 
					 ( !( my_stack >= 0 && my_stack <= STK_B7 ) || is_top_card || movable[card_above] ) ) ) )			// case 2		( changed 20 feb 2007 )
			{
				suit_stackable[i] = true;
				changed = true;
			}
			
			/*
			 * there are four ways a card can become movable
			 * 1. An ace is a move-candidate (case 1)
			 * 2. A non-ace is a move-candidate ( in a build stack and is either the top card or it's card_above is movable )
			 *      or in the deck and its suit-card is suit-stackable (case 2)
			 * 3. A king is a move-candidate, and open-stack is true (case 3)
			 * 4. A non-king is a move-candidate, and one of its build cards is buildable (case 4)
			 */
			
			if ( !movable[i] && move_candidate[i] &&
				 (
				  i % 13 == 0 ||																							// case 1
				  ( is_build_card && suit_stackable[cards[i]->suit_card] && ( is_top_card || movable[card_above] ) ) ||		// case 2  (changed 20 feb 2007)
				  ( is_deck_card && suit_stackable[cards[i]->suit_card] ) ||												// case 2
				  ( i % 13 == ( SUIT_SIZE-1 ) && open_stack ) ||															// case 3
				  ( i % 13 != ( SUIT_SIZE-1 ) && ( buildable[cards[i]->build_cards[0]] ||									// case 4
												   buildable[cards[i]->build_cards[1]] ) ) ) )								// case 4
			{
				movable[i] = true;
				changed = true;
				cards[i]->playable_depth = count;
				
				/*
				 * if we've changed which cards are movable, we may have
				 * to update the lowest movable cards in the deck
				 */
				if ( is_deck_card )
				{
					// low_deck_movable[0] is always the lowest card that has been moved
					// low_deck_movable[1] is always the second lowest card that has been moved
					// swap without temporary variables      b = ( a+b ) - ( a = b );
					low_deck_movable[1] = my_stack_loc < low_deck_movable[1] ? my_stack_loc : low_deck_movable[1];
					low_deck_movable[0] = low_deck_movable[1] < low_deck_movable[0] ?
						( low_deck_movable[1]+low_deck_movable[0] ) - ( low_deck_movable[1] = low_deck_movable[0] ) : low_deck_movable[0];
				}
			}
			/*
			 * there are nine ways a card can become a move-candidate
			 * 1. A card in the deck is the last card in the deck (case 1)
			 * 2. A card in the deck whose (zero-based) location in the deck MOD 3 is 2 (case 2)
			 * 3. A card in the deck whose (zero-based) location in the deck MOD 3 is 0 
			 *    and at least one card before it is movable (case 3)
			 * 4. A card in the deck whose (zero-based) location in the deck MOD 3 is 1
			 *    and at least two cards before it is movable (case 4)
			 * 5. A card in the deck and the card directly above it is movable (case 5)
			 * 6. A card is in a build stack and is the top card in that stack (case 6)
			 * 7. A card is in a build stack and the card above it is movable (case 7)
			 * 8. A card is in a suit stack and is the top card in that stack (case 8)
			 * 9. A card is in a suit stack and the cards above it is movable (case 9)
			 */
			if ( !move_candidate[i] &&
				 ( ( is_deck_card && is_top_card ) ||																// case 1
				   ( is_deck_card && my_stack_loc % 3 == 2 ) ||														// case 2
				   ( is_deck_card && my_stack_loc % 3 == 0 && ( low_deck_movable[0] < my_stack_loc ||				// case 3
																low_deck_movable[1] < my_stack_loc ) ) ||
				   ( is_deck_card && my_stack_loc % 3 == 1 && ( low_deck_movable[0] < my_stack_loc &&				// case 4
																low_deck_movable[1] < my_stack_loc ) ) ||
				   ( is_deck_card && card_above >= 0 && movable[card_above] ) ||									// case 5
				   ( is_build_card && is_top_card ) ||																// case 6
				   ( is_build_card && card_above >= 0 && movable[card_above] ) ||									// case 7
				   ( is_suit_card && is_top_card ) ||																// case 8
				   ( is_suit_card && card_above >= 0 && movable[card_above] ) ) )									// case 9
			{
				move_candidate[i] = true;
				changed = true;
			}
			
			/*
			 * there are four ways a card can become buildable
			 * 1. A card in a build stack is the top card in that stack
			 * 2. A card in a build stack and the card above it is movable
			 * 3. A king is a move-candidate, not in a build stack, and open-stack is true (case 3)
			 * 4. A non-king is a move-candidate, and one of its build cards is buildable (case 4)
			 */
			if ( !buildable[i] &&
				 ( ( is_build_card && is_top_card ) ||																	// case 1
				   ( is_build_card && card_above >= 0 && movable[card_above] ) ||										// case 2
				   ( move_candidate[i] && i % 13 == ( SUIT_SIZE-1 ) && open_stack && !is_build_card ) ||									// case 3
				   ( move_candidate[i] && i % 13 != ( SUIT_SIZE-1 ) && ( buildable[cards[i]->build_cards[0]] ||			// case 4
																		 buildable[cards[i]->build_cards[1]] ) ) ) )	// case 4
			{
				buildable[i] = true;
				changed = true;
			}
			/*
			 * there is one way open_stack can become true
			 * 1. The bottom card of a build stack is movable
			 */
			if ( !open_stack &&
				 ( ( all_stacks[STK_B1][0] != -1 && movable[all_stacks[STK_B1][0]] ) ||
				   ( all_stacks[STK_B2][0] != -1 && movable[all_stacks[STK_B2][0]] ) ||
				   ( all_stacks[STK_B3][0] != -1 && movable[all_stacks[STK_B3][0]] ) ||
				   ( all_stacks[STK_B4][0] != -1 && movable[all_stacks[STK_B4][0]] ) ||
				   ( all_stacks[STK_B5][0] != -1 && movable[all_stacks[STK_B5][0]] ) ||
				   ( all_stacks[STK_B6][0] != -1 && movable[all_stacks[STK_B6][0]] ) ||
				   ( all_stacks[STK_B7][0] != -1 && movable[all_stacks[STK_B7][0]] ) ) )
			{
				open_stack = true;
				changed = true;
			}
		}
	}
	for ( i=0; i<FULL_DECK; i++ )
	{
		if ( suit_stackable[i] ) 
		{
			total_suit_stackable++;
		}
	}
	
	if ( false )
	{
		for ( i=0; i<FULL_DECK; i++ )
		{
			printIndexToChar( i );
			cout << " made movable on iteration " << cards[i]->playable_depth << " of deadEnd() check" << endl;
		}
		for ( i=1; i<=count; i++ )
		{
			int play_count = 0;
			for ( int j=ACE_DIAM; j<=KIN_SPAD; j++ )
			{
				if ( cards[j]->playable_depth == i )
				{
					play_count++;
				}
			}
			cout << play_count << " cards made playable on iteration " << i << endl;
		}
	}
	
	if ( false )
	{
		cout << "Card M  B  MC SS" << endl;
		for ( i=0; i<FULL_DECK; i++ )
		{
			printIndexToChar( i );
			cout << ( movable[i] ? "T" : " " ) << " ";
			cout << ( buildable[i] ? "T" : " " ) << " " ;
			cout << ( move_candidate[i] ? "T" : " " ) << " " ;
			cout << ( suit_stackable[i] ? "T" : " " ) << " " << endl;
		}
	}
	
	//ofstream out_file( "dead_end2.txt", ios::app );
	if ( DECK_SIZE != total_suit_stackable )
	{
		if ( false )
		{
			//printToFile( out_file );
		}
		return true;
	}
	//out_file.close();
	
	// begin added 13 January 2006
	bool twin_in_same_stack;
	bool on_suit_blocked;
	bool twin_on_suit_blocked;
	bool off_suit_blocked;
	int my_index, twin_index;
	int my_suit, twin_suit;
	int cur_card;
	int j, k;
	
	for ( i=0; i<NUM_B_STACKS; i++ )
	{
		for ( j=2; j<STACK_SIZE && all_stacks[i][j] != -1; j++ )
		{
			twin_in_same_stack = false;
			on_suit_blocked = false;
			twin_on_suit_blocked = false;
			off_suit_blocked = false;
			
			my_index = all_stacks[i][j];
			twin_index = ( my_index > 25 ? my_index - 26 : my_index + 26 );
			my_suit = my_index / 13;
			twin_suit = twin_index / 13;
			
			for ( k=j-1; k>=0; k-- )
			{
				cur_card = all_stacks[i][k];
				if ( cur_card == twin_index ) twin_in_same_stack = true;
				if ( cur_card / 13 == my_suit && cur_card < my_index ) on_suit_blocked = true;
				if ( twin_in_same_stack && cur_card / 13 == twin_suit && cur_card < twin_index ) twin_on_suit_blocked = true;
				if ( cur_card == cards[my_index]->build_cards[0] || cur_card == cards[my_index]->build_cards[1] )
				{
					if ( ! cards[cur_card]->face_up && twin_in_same_stack ) off_suit_blocked = true;
				}
			}
			
			if ( on_suit_blocked && twin_on_suit_blocked && off_suit_blocked )
			{
				return true;
			}
		}
	}
	// end added 13 January 2006
	return false;
}

int State::char2Card( char * in_card )
{
	if ( strncmp( in_card, "--", 2 ) == 0 ) return -1;
	else if ( strncmp( in_card, "AD", 2 ) == 0 ) return ACE_DIAM;
	else if ( strncmp( in_card, "2D", 2 ) == 0 ) return TWO_DIAM;
	else if ( strncmp( in_card, "3D", 2 ) == 0 ) return THR_DIAM;
	else if ( strncmp( in_card, "4D", 2 ) == 0 ) return FOU_DIAM;
	else if ( strncmp( in_card, "5D", 2 ) == 0 ) return FIV_DIAM;
	else if ( strncmp( in_card, "6D", 2 ) == 0 ) return SIX_DIAM;
	else if ( strncmp( in_card, "7D", 2 ) == 0 ) return SEV_DIAM;
	else if ( strncmp( in_card, "8D", 2 ) == 0 ) return EIG_DIAM;
	else if ( strncmp( in_card, "9D", 2 ) == 0 ) return NIN_DIAM;
	else if ( strncmp( in_card, "TD", 2 ) == 0 ) return TEN_DIAM;
	else if ( strncmp( in_card, "JD", 2 ) == 0 ) return JAC_DIAM;
	else if ( strncmp( in_card, "QD", 2 ) == 0 ) return QUE_DIAM;
	else if ( strncmp( in_card, "KD", 2 ) == 0 ) return KIN_DIAM;
	else if ( strncmp( in_card, "AC", 2 ) == 0 ) return ACE_CLUB;
	else if ( strncmp( in_card, "2C", 2 ) == 0 ) return TWO_CLUB;
	else if ( strncmp( in_card, "3C", 2 ) == 0 ) return THR_CLUB;
	else if ( strncmp( in_card, "4C", 2 ) == 0 ) return FOU_CLUB;
	else if ( strncmp( in_card, "5C", 2 ) == 0 ) return FIV_CLUB;
	else if ( strncmp( in_card, "6C", 2 ) == 0 ) return SIX_CLUB;
	else if ( strncmp( in_card, "7C", 2 ) == 0 ) return SEV_CLUB;
	else if ( strncmp( in_card, "8C", 2 ) == 0 ) return EIG_CLUB;
	else if ( strncmp( in_card, "9C", 2 ) == 0 ) return NIN_CLUB;
	else if ( strncmp( in_card, "TC", 2 ) == 0 ) return TEN_CLUB;
	else if ( strncmp( in_card, "JC", 2 ) == 0 ) return JAC_CLUB;
	else if ( strncmp( in_card, "QC", 2 ) == 0 ) return QUE_CLUB;
	else if ( strncmp( in_card, "KC", 2 ) == 0 ) return KIN_CLUB;
	else if ( strncmp( in_card, "AH", 2 ) == 0 ) return ACE_HEAR;
	else if ( strncmp( in_card, "2H", 2 ) == 0 ) return TWO_HEAR;
	else if ( strncmp( in_card, "3H", 2 ) == 0 ) return THR_HEAR;
	else if ( strncmp( in_card, "4H", 2 ) == 0 ) return FOU_HEAR;
	else if ( strncmp( in_card, "5H", 2 ) == 0 ) return FIV_HEAR;
	else if ( strncmp( in_card, "6H", 2 ) == 0 ) return SIX_HEAR;
	else if ( strncmp( in_card, "7H", 2 ) == 0 ) return SEV_HEAR;
	else if ( strncmp( in_card, "8H", 2 ) == 0 ) return EIG_HEAR;
	else if ( strncmp( in_card, "9H", 2 ) == 0 ) return NIN_HEAR;
	else if ( strncmp( in_card, "TH", 2 ) == 0 ) return TEN_HEAR;
	else if ( strncmp( in_card, "JH", 2 ) == 0 ) return JAC_HEAR;
	else if ( strncmp( in_card, "QH", 2 ) == 0 ) return QUE_HEAR;
	else if ( strncmp( in_card, "KH", 2 ) == 0 ) return KIN_HEAR;
	else if ( strncmp( in_card, "AS", 2 ) == 0 ) return ACE_SPAD;
	else if ( strncmp( in_card, "2S", 2 ) == 0 ) return TWO_SPAD;
	else if ( strncmp( in_card, "3S", 2 ) == 0 ) return THR_SPAD;
	else if ( strncmp( in_card, "4S", 2 ) == 0 ) return FOU_SPAD;
	else if ( strncmp( in_card, "5S", 2 ) == 0 ) return FIV_SPAD;
	else if ( strncmp( in_card, "6S", 2 ) == 0 ) return SIX_SPAD;
	else if ( strncmp( in_card, "7S", 2 ) == 0 ) return SEV_SPAD;
	else if ( strncmp( in_card, "8S", 2 ) == 0 ) return EIG_SPAD;
	else if ( strncmp( in_card, "9S", 2 ) == 0 ) return NIN_SPAD;
	else if ( strncmp( in_card, "TS", 2 ) == 0 ) return TEN_SPAD;
	else if ( strncmp( in_card, "JS", 2 ) == 0 ) return JAC_SPAD;
	else if ( strncmp( in_card, "QS", 2 ) == 0 ) return QUE_SPAD;
	else if ( strncmp( in_card, "KS", 2 ) == 0 ) return KIN_SPAD;
	else return -1;
}

long State::parseGameFile( string in_file, long start_loc )
{
	string num;
	
	long end_location;
	
	int space_buffer = 15;							// from printSuitStacks (should probably be global)
	int card_id, counter;
	int i, j;
	int card_loc_in_stack;
	
	bool card_deck_playable;
	
	char * eat_line = new char[512];				// there's no way a line should be this long
	char * in_card = new char[4];
	char * asterisk = new char[2];
	char * in_buffer = new char[15];				// this is why it should be global
	char * deck_num = new char[2];
	
	ifstream input;
	input.open( in_file.c_str(), ifstream::in );
	
	//make sure we got the file opened up ok...
	if( !input.is_open() )
	{
		cout << "could not open file " << in_file << endl;
		exit( 0 );
	}
	
	input.seekg( start_loc );
	
	// clear out all of our storage
	for ( i=0; i<FULL_DECK; i++ )
	{
		cards[i]->stack_id = -1;
		cards[i]->loc_in_stack = -1;  
		cards[i]->face_up = false; 
		cards[i]->deck_playable = false; 
		cards[i]->blocked_val = 0;
		cards[i]->available = UNKNOWN;
		cards[i]->movable = UNKNOWN;
		cards[i]->playable_depth = POS_INF;
	}
	
	input.read( deck_num, 2 );
	deck_offset = atoi( deck_num );
	input.getline( eat_line, 256 );					// eat the rest of the deck_offset line
	input.read( deck_num, 2 );
	last_played_deck_loc = atoi( deck_num );
	input.getline( eat_line, 256);					// eat the rest of the last_played_deck_card line
	input.getline( eat_line, 256);					// eat the blank line
													// read in the suit stacks
	input.getline( eat_line, 256 );					// eat the "di cl he sp" line
	
	input.read( in_buffer, space_buffer+1 );
	for ( i=STK_DIAM; i<=STK_SPAD; i++ )
	{
		input.read( in_card, 1 );
		input.read( in_card, 2 );
		card_id = char2Card( in_card );
		if ( card_id != -1 )
		{
			for ( ( j=( card_id / (int)13 ) * 13 ); j<=card_id; j++ )
			{
				card_loc_in_stack = j%13;
				cards[j]->loc_in_stack = card_loc_in_stack;
				cards[j]->stack_id = i;
			}
		}
	}
	
	input.getline( eat_line, 256 );					// eat the rest of the suit stack line
													//input.getline( eat_line, 256 );					// eat the empty line line
	
	// read in the deck
	counter = 0;
	input.read( in_card, 2 );
	card_id = char2Card( in_card );
	while ( card_id != -1 )
	{
		input.read( asterisk, 2 );
		card_deck_playable = ( strncmp( asterisk, "*", 1 ) == 0 );
		
		cards[card_id]->stack_id = STK_DECK;
		cards[card_id]->loc_in_stack = counter;
		cards[card_id]->deck_playable = card_deck_playable;
		cards[card_id]->face_up = true;							// changed 2-20
		counter++;
		input.read( in_card, 2 );
		card_id = char2Card( in_card );
	}
	
	// read in the build stacks
	input.getline( eat_line, 256 );					// eat the " 1   2   3   4" line
	input.getline( eat_line, 256 );					// eat the " -   -   -   -" line
	
	counter = 0;
	bool reading_b_stk = true;
	while ( reading_b_stk )
	{
		reading_b_stk = false;
		for ( i=STK_B1; i<STK_B1+NUM_B_STACKS; i++ )
		{
			if ( input.read( in_card, 4 ) )
			{
				card_id = char2Card( in_card );
				if ( card_id != -1 )
				{
					reading_b_stk = true;
					cards[card_id]->stack_id = i;
					cards[card_id]->loc_in_stack = counter;
				}
				if ( in_card[2] == '^' )
				{
					cards[card_id]->face_up = true;
				}
			}
		}
		counter++;
		input.getline( eat_line, 256 );					// eat the rest of the build stack line
	}
	
	end_location = input.tellg();
	input.close();
	
	fillStacksFromCards();
	fillFirstEmptyFromStacks();
	generateAvailableActions();
	fillCompact( my_cs );
	if ( false )
	{
		prettyPrint();
	}
	
	delete in_card;
	delete in_buffer;
	delete eat_line;
	delete asterisk;
	delete deck_num;
	
	return end_location;
}

int State::getOSBIndex( int card_mod )
{
	if ( card_mod <= 0 || card_mod > 12 )
	{
		cout << "ERROR in getOSBIndex " << endl;
		exit ( 0 );
	}
	else if ( card_mod == 1 ) return 0;
	else if ( card_mod == 2 ) return 1;
	else return getOSBIndex( card_mod - 1 ) + card_mod - 1;
}

void State::HCPGetFeature( int HCP_index, int & feature, int & card1, int & card2 )
{
	int feature_mod;
	int card1_mod, card1_suit, card1_rank;
	int card2_rank;
	int suit_mod;
	
	if ( HCP_index < 0 || HCP_index > 594 )
	{
		cout << "ERROR in HCPGetFeature" << endl;
		exit ( 0 );
	}
	else if ( HCP_index <= 311 )
	{
		feature = HCP_OSB;
		card1_mod = HCP_index % 78;
		card1_suit = HCP_index / 78;
		card1_rank = getOSBCard( card1_mod );
		card1 = card1_rank + ( SUIT_BREAK * card1_suit );
		card2_rank = card1_mod - getOSBIndex( card1_rank );
		card2 = card2_rank + ( SUIT_BREAK  * card1_suit );
	}
	else if ( HCP_index <= 399 )
	{
		feature_mod = HCP_index - 312;
		feature = HCP_OCB;
		card1_mod = feature_mod % 22;
		card1_suit = feature_mod / 22;
		card1_rank = ( card1_mod / 2 ) + 1;
		suit_mod = card1_mod % 2;
		card1 = card1_rank + ( SUIT_BREAK * card1_suit );
		card2_rank = card1_rank + 1;
		switch ( card1_suit )
		{
			case 0:
			case 2:
				card2 = card2_rank + ( SUIT_BREAK * ( !suit_mod ? 1 : 3 ) );
				break;
			case 1:
			case 3:
				card2 = card2_rank + ( SUIT_BREAK * ( !suit_mod ? 0 : 2 ) );
				break;
		}
	}
	else if ( HCP_index <= 412 )
	{
		feature = HCP_GIF;
		feature_mod = HCP_index - 400;
		card1 = feature_mod;
		card2 = -1;
	}
	else if ( HCP_index <= 438 )
	{
		feature = HCP_SCFD;
		feature_mod = HCP_index - 413;
		card1_rank = feature_mod % 13;
		card1_suit = feature_mod / 13;
		card1 = card1_rank + ( card1_suit * SUIT_BREAK );
		card2 = -1;
	}
	else if ( HCP_index <= 490 )
	{
		feature = HCP_IF;
		feature_mod = HCP_index - 439;
		card1 = feature_mod;
		card2 = -1;
	}
	else if ( HCP_index <= 542 )
	{
		feature = HCP_ISP;
		feature_mod = HCP_index - 491;
		card1 = feature_mod;
		card2 = -1;
	}
	else if ( HCP_index <= 594 )
	{
		feature = HCP_ITFU;
		feature_mod = HCP_index - 543;
		card1 = feature_mod;
		card2 = -1;
	}
}

int State::getOSBCard( int index )
{
	int i;
	int j = 12;
	
	if ( index >= 78 || index < 0 )
	{
		cout << "ERROR in getOSBCard" << endl;
		exit ( 0 );
	}
	
	for ( i=66;;i-=j)
	{
		if ( index >= i ) return j;
		j--;
	}
	return -1;
}

void State::fillCompact( CompactState * cs )
{
	int i;
	
	for ( i=0; i<FULL_DECK; i++ )
	{
		cs->setStack( i, cards[i]->stack_id );
		cs->setLocInStack( i, cards[i]->loc_in_stack );
		cs->setFaceUp( i, cards[i]->face_up );
	}
	
	cs->setDeckOffset( deck_offset );
	if ( last_played_deck_loc == -1 )
		cs->setLastPlayedDeckLoc( 31 );
	else
		cs->setLastPlayedDeckLoc( last_played_deck_loc );
}

void State::fillUCTCompact( CompactState * cs )
{
	int i;
	
	// for our UCT compact states, we need to preserve the hidden nature of the face-down cards
	for ( i=0; i<FULL_DECK; i++ ) {
		if ( cards[i]->face_up ){
			cs->setStack( i, cards[i]->stack_id );
			cs->setLocInStack( i, cards[i]->loc_in_stack );
			cs->setFaceUp( i, cards[i]->face_up );
		} else {
			cs->setStack( i, 0 );
			cs->setLocInStack( i, 0 );
			cs->setFaceUp( i, cards[i]->face_up );
		}
	}
	
	cs->setDeckOffset( deck_offset );
	if ( last_played_deck_loc == -1 )
		cs->setLastPlayedDeckLoc( 31 );
	else
		cs->setLastPlayedDeckLoc( last_played_deck_loc );
}

void State::fillFromCompact( CompactState * cs )
{
	int i;
	
	for ( i=0; i<DECK_SIZE; i++ )
	{
		cards[i]->stack_id = cs->getStack( i );
		cards[i]->loc_in_stack = cs->getLocInStack( i );
		cards[i]->face_up = cs->getFaceUp( i );
		//cout << " for card " << i << " stack = " << cards[i]->stack_id << " loc in stack = " << cards[i]->loc_in_stack;
		//cout << " and face up = " << cards[i]->face_up << endl;
	}
	
	fillStacksFromCards();
	fillFirstEmptyFromStacks();

	//cout << " stacks and first empties filled from compact" << endl;
	
	deck_offset = cs->getDeckOffset();
	last_played_deck_loc = cs->getLastPlayedDeckLoc();
	last_played_deck_loc = ( last_played_deck_loc == 31 ? -1 : last_played_deck_loc );

	//cout << " last played deck loc filled" << endl;
	
	generateDeckPlayable();
	generateAvailableActions();
	fillCompact( my_cs );
}



//void State::setFeatureWeights( double f_weights[NUM_BASE_PROPOSITIONS][FULL_DECK] )
//{
//	int i, j;
//	for ( i=0; i<NUM_BASE_PROPOSITIONS; i++ )
//	{
//		for ( j=0; j<FULL_DECK; j++ )
//		{
//			feature_weights[i][j] = f_weights[i][j];
//		}
//	}
//}
	
/*
 int main (int argc, char * const argv[]) {
	 
	 ifstream input;
	 char * buffer1 = new char[4];
	 
	 input.open( "../space_test.txt", ifstream::in );
	 
	 if ( !input.is_open() )
	 {
		 cout << "closed" << endl;
		 exit( 0 );
	 }
	 
	 input.read( buffer1, 4 );
	 while ( !input.eof() )
	 {
		 cout << buffer1 << " " << input.gcount() << endl;
		 input.read( buffer1, 4 );
	 }
	 
	 return 0;
 }*/

void State::sortActions( )
{
	int t_card;
	int t_new_stack;
	int t_old_stack;
	bool t_reveal;
	int i, j;
	
	for ( i=0; i<=valid_actions_index; i++ )
	{
		for ( j=i+1; j<=valid_actions_index; j++ )
		{
			if ( valid_actions[j]->better( valid_actions[i] ) )
			{
				t_card = valid_actions[j]->card_to_be_moved;
				t_new_stack = valid_actions[j]->new_stack;
				t_old_stack = valid_actions[j]->old_stack;
				t_reveal = valid_actions[j]->reveal;
				
				valid_actions[j]->card_to_be_moved = valid_actions[i]->card_to_be_moved;
				valid_actions[j]->new_stack = valid_actions[i]->new_stack;
				valid_actions[j]->old_stack = valid_actions[i]->old_stack;
				valid_actions[j]->reveal = valid_actions[i]->reveal;
				
				valid_actions[i]->card_to_be_moved = t_card;
				valid_actions[i]->new_stack = t_new_stack;
				valid_actions[i]->old_stack = t_old_stack;
				valid_actions[i]->reveal = t_reveal;
			}
		}
	}	
}

double State::stateValue()
{
	double return_value = 0.0;
	int i;
	
	fillBlockedCards();
	
	// --------
	// the number of cards in the suit_stacks
	for ( i=0; i<FULL_DECK; i++ )
	{
		if ( cards[i]->stack_id >= STK_DIAM && cards[i]->stack_id <= STK_SPAD )
		{
			return_value += 5;
		}
	}
	// --------
	
	
	// --------
	// give points for each time all cards of a value are banked (keeps things even)
	for ( i=0; i<SUIT_SIZE; i++ )
	{
		if ( cards[i]->stack_id >= STK_DIAM && cards[i]->stack_id <= STK_SPAD &&
			 cards[i+13]->stack_id >= STK_DIAM && cards[i+13]->stack_id <= STK_SPAD &&
			 cards[i+26]->stack_id >= STK_DIAM && cards[i+26]->stack_id <= STK_SPAD &&
			 cards[i+39]->stack_id >= STK_DIAM && cards[i+39]->stack_id <= STK_SPAD )
		{
			return_value += 10;
		}
	}
	// --------
	
	// --------
	// include blocked card and face down penalties
	for ( i=0; i<FULL_DECK; i++ )
	{
		return_value -= cards[i]->blocked_val;
		if ( cards[i]->stack_id >= FIRST_B_STK && cards[i]->stack_id <= LAST_B_STK && ( ! cards[i]->face_up ) )
		{
			// --------
			// penalize each face down ace -13, each two -12, each three -11, ... etc
			return_value-= ( SUIT_SIZE - ( i % 13 ) );
			// --------
		}
	}
	// --------
	
	// --------
	// include penalty for two cards of same suit and value both being face-down
	for ( i=0; i<FULL_DECK/2; i++ )
	{
		if ( cards[i]->stack_id >= FIRST_B_STK && cards[i]->stack_id <= LAST_B_STK && ! cards[i]->face_up &&
			 cards[i+26]->stack_id >= FIRST_B_STK && cards[i+26]->stack_id <= LAST_B_STK && ! cards[i]->face_up )
		{
			return_value -= 20;
		}
	}
	
	// --------
	// get penalized a point for each card in the deck that is not playable
	for ( i=0; i<STACK_SIZE; i++ )
	{
		if ( cards[i]->stack_id == STK_DECK && !cards[i]->deck_playable )
		{
			return_value -= 1;
		}
	}
	// --------
	
	return return_value;
}

void State::fillBlockedCards()
{
	int i;
	for ( i=0; i<FULL_DECK; i++ )
	{
		cards[i]->blocked_val = 0;
	}
	
	// check the build stacks for blocked cards
	// b_1 will never have face-down cards, so no point in checking
	for ( i=FIRST_B_STK; i<=LAST_B_STK; i++ )
	{
		fillBlockedCardsHelper( i );
	}
}

void State::fillBlockedCardsHelper( int stack_id )
{
	int i, j;
	bool first_face_up = false;
	
	for ( i=0; i<STACK_SIZE; i++ )
	{
		if ( ! first_face_up )
		{
			// if this card is not an empty spot
			if ( all_stacks[stack_id][i] != NO_CARD )
			{
				// if this card is face-up, it will be the last one we check
				if ( cards[all_stacks[stack_id][i]]->face_up )
				{
					first_face_up = true;
				}
				
				// only move on if the card is not a king ( kings can't block anything )
				if ( all_stacks[stack_id][i] % 13 != ( SUIT_SIZE - 1 ) )
				{
					for ( j=i-1; j>=0; j-- )
					{
						if ( ( ( all_stacks[stack_id][i] % 26 < 13 ) &&		// if the card is red
							   ( all_stacks[stack_id][j] == ( all_stacks[stack_id][i] % 13 ) + 14 || 
								 all_stacks[stack_id][j] == ( all_stacks[stack_id][i] % 13 ) + 40 ) ) ||
							 ( ( all_stacks[stack_id][i] % 26 > 12 ) &&		// if the card is black
							   ( all_stacks[stack_id][j] == ( all_stacks[stack_id][i] % 13 ) - 12 ||
								 all_stacks[stack_id][j] == ( all_stacks[stack_id][i] % 13 ) + 14 ) ) )
						{
							cards[all_stacks[stack_id][i]]->blocked_val += OFF_SUIT_BLOCK_PENALTY;
						}
						
						if ( ( all_stacks[stack_id][i] % 13 == all_stacks[stack_id][j] % 13 ) &&
							 ( all_stacks[stack_id][i] > all_stacks[stack_id][j] ) )
						{
							cards[all_stacks[stack_id][i]]->blocked_val += ON_SUIT_BLOCK_PENALTY;
						}
					}
				}
			}
		}
	}
}


	
