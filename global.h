/*
 *  global.h
 *  solitaire
 *
 *  Created by Ronald Bjarnason on Tue Nov 22 2005.
 *  Copyright (c) 2005 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef GLOBAL_H
#define GLOBAL_H

//#include <Carbon/Carbon.h>
#include <iostream>
//#include <GLUT/glut.h>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include "mtRand.h"
#include <sys/time.h>

#define POS_INF					1000000
#define NEG_INF					-1000000
#define TOO_LONG				-1000001

/*
 * these are the two parameters 
 * that control partial games
 * (games with fewer than 7 stacks 
 *	and/or 13 cards in a suit )
 */
static bool print = false;

#define SUIT_SIZE				13	
#define NUM_B_STACKS				7	
#define NUM_CARDS				( 4 * SUIT_SIZE )

#define FIRST_B_STK				STK_B1
#define LAST_B_STK				( FIRST_B_STK + NUM_B_STACKS - 1 )
#define DECK_SIZE				( NUM_SUITS*SUIT_SIZE )

// 18 possible primary actions, with another two actions for each of those that is unavailable
#define MAX_NUM_ACTIONS			3*18

// for a typical 52 card deal, 21 cards will be face down initially (only counting the tableau)
#define MAX_FACE_DOWN_CARDS		21


// number of trajectories per tree
#define MAX_UCT_ROLLOUTS		512
// number of trees per decision
#define HOPT_UCT_TRIALS			32
// branching factor per tree (greater than 21 == infinity)
#define SPARSE_BRANCHING_FACTOR		32


#define MAX_UCT_NODES			100000
#define NUM_UCT_CHILDREN		(SPARSE_BRANCHING_FACTOR*MAX_NUM_ACTIONS)

#define PRINT_SOLUTION			false

#define NUM_SUITS				4
#define NUM_STACKS				12
#define STACK_SIZE				52
#define SUIT_BREAK				13
#define NO_CARD					-1
#define FULL_DECK				52
#define SURVEY_SIZE				100

#define NUM_HCP_FEATURES			595

#define NUM_UCT_FEATURE_TYPES			6
#define NUM_UCT_FEATURES			( (NUM_UCT_FEATURE_TYPES * FULL_DECK ) + 7)
#define NUM_UCT_WEIGHTS				( (NUM_UCT_FEATURE_TYPES * ( FULL_DECK/NUM_SUITS ) ) + 7)

#define RANDOM_POLICY_DEFAULT			0
#define PRIORITY_POLICY_DEFAULT			1
#define LEARNED_POLICY_DEFAULT			2

#define STK_B1					0
#define STK_B2					1
#define STK_B3					2
#define STK_B4					3
#define STK_B5					4
#define STK_B6					5
#define STK_B7					6
#define STK_DECK				7
#define STK_DIAM				8
#define STK_CLUB				9
#define STK_HEAR				10
#define STK_SPAD				11

#define ACE_DIAM				0
#define TWO_DIAM				1
#define THR_DIAM				2
#define FOU_DIAM				3
#define FIV_DIAM				4
#define SIX_DIAM				5
#define SEV_DIAM				6
#define EIG_DIAM				7
#define NIN_DIAM				8
#define TEN_DIAM				9
#define JAC_DIAM				10
#define QUE_DIAM				11
#define KIN_DIAM				12
#define ACE_CLUB				13
#define TWO_CLUB				14
#define THR_CLUB				15
#define FOU_CLUB				16
#define FIV_CLUB				17
#define SIX_CLUB				18
#define SEV_CLUB				19
#define EIG_CLUB				20
#define NIN_CLUB				21
#define TEN_CLUB				22
#define JAC_CLUB				23
#define QUE_CLUB				24
#define KIN_CLUB				25
#define ACE_HEAR				26
#define TWO_HEAR				27
#define THR_HEAR				28
#define FOU_HEAR				29
#define FIV_HEAR				30
#define SIX_HEAR				31
#define SEV_HEAR				32
#define EIG_HEAR				33
#define NIN_HEAR				34
#define TEN_HEAR				35
#define JAC_HEAR				36
#define QUE_HEAR				37
#define KIN_HEAR				38
#define ACE_SPAD				39
#define TWO_SPAD				40
#define THR_SPAD				41
#define FOU_SPAD				42
#define FIV_SPAD				43
#define SIX_SPAD				44
#define SEV_SPAD				45
#define EIG_SPAD				46
#define NIN_SPAD				47
#define TEN_SPAD				48
#define JAC_SPAD				49
#define QUE_SPAD				50
#define KIN_SPAD				51


#define OFF_SUIT_BLOCK_PENALTY  20
#define ON_SUIT_BLOCK_PENALTY   5


#define F_IN_FOUNDATION				0
#define F_IN_STOCK_PLAYABLE			1
#define F_IN_STOCK_UNPLAYABLE		2
#define F_IN_TABLEAU_FACE_DOWN		3
#define F_IN_TABLEAU_FACE_UP		4
#define AF_FROM_FOUNDATION			5
#define AF_FROM_TABLEAU				6
#define AF_FROM_STOCK				7
#define AF_TO_FOUNDATION			8
#define AF_TO_TABLEAU				9
#define AF_REVEAL					10
#define NUM_BASE_PROPOSITIONS		5
#define FIRST_BASE_PROPOSITION		F_IN_FOUNDATION
#define LAST_BASE_PROPOSITION		F_IN_TABLEAU_FACE_UP
#define FIRST_A_PROPOSITION			AF_FROM_FOUNDATION
#define LAST_A_PROPOSITION			AF_REVEAL

#define F_STOCK_ABOVE				0
#define F_TABLEAU_ABOVE				1
#define F_FOUNDATION_ABOVE			2
#define NUM_BASE_RELATIONS			3
#define FIRST_BASE_RELATION			F_STOCK_ABOVE
#define LAST_BASE_RELATION			F_FOUNDATION_ABOVE

#define HCP_OSB						0
#define HCP_OCB						1
#define HCP_GIF						2
#define HCP_SCFD					3
#define HCP_IF						4
#define HCP_ISP						5
#define HCP_ITFU					6

// if you are going to put F_FOUNDATION_ABOVE back in as a feature you
// need to uncomment two lines in fillFeatureRelations() in state.cpp
// we've been running very successfully (and very quickly) with
// NUM_CONST_RELATIONS at 2 (only considering the BUILD_ON) relations

#define F_FOUNDATION_BUILD_ON		0
#define F_TABLEAU_BUILD_ON			1
#define F_SAME_SUIT					2
#define F_SAME_COLOR				3
#define F_LOWER_RANK				4
#define F_SAME_RANK					5
#define NUM_CONST_RELATIONS			6
#define FIRST_CONST_RELATION		F_SAME_SUIT
#define LAST_CONST_RELATION			F_LOWER_RANK

#define P_EX_P_FEAT					0
#define P_EX_N_FEAT					1
#define N_EX_P_FEAT					2
#define N_EX_N_FEAT					3

#define YES						1
#define NO						0
#define PROCESSING				-1
#define UNKNOWN					NEG_INF

#define NORMAL_IMAGE			true
#define INVERSE_IMAGE			false

#define TOP						748
#define RIGHT					525
#define CARD_Y_SIZE				84
#define CARD_X_SIZE				60
#define COL_OFFSET				CARD_X_SIZE / 4
#define ROW_OFFSET				CARD_Y_SIZE / 3

#define COL_1_X					0 * COL_OFFSET + 10
#define COL_2_X					1 * COL_OFFSET + 10
#define COL_3_X					2 * COL_OFFSET + 10
#define COL_4_X					3 * COL_OFFSET + 10
#define COL_5_X					4 * COL_OFFSET + 10
#define COL_6_X					5 * COL_OFFSET + 10
#define COL_7_X					6 * COL_OFFSET + 10
#define COL_8_X					7 * COL_OFFSET + 10
#define COL_9_X					8 * COL_OFFSET + 10
#define COL_10_X				9 * COL_OFFSET + 10
#define COL_11_X				10 * COL_OFFSET + 10
#define COL_12_X				11 * COL_OFFSET + 10
#define COL_13_X				12 * COL_OFFSET + 10
#define COL_14_X				13 * COL_OFFSET + 10
#define COL_15_X				14 * COL_OFFSET + 10
#define COL_16_X				15 * COL_OFFSET + 10
#define COL_17_X				16 * COL_OFFSET + 10
#define COL_18_X				17 * COL_OFFSET + 10
#define COL_19_X				18 * COL_OFFSET + 10
#define COL_20_X				19 * COL_OFFSET + 10
#define COL_21_X				20 * COL_OFFSET + 10
#define COL_22_X				21 * COL_OFFSET + 10
#define COL_23_X				22 * COL_OFFSET + 10
#define COL_24_X				23 * COL_OFFSET + 10
#define COL_25_X				24 * COL_OFFSET + 10
#define COL_26_X				25 * COL_OFFSET + 10
#define COL_27_X				26 * COL_OFFSET + 10
#define COL_28_X				27 * COL_OFFSET + 10
#define COL_29_X				28 * COL_OFFSET + 10
#define COL_30_X				29 * COL_OFFSET + 10
#define COL_31_X				30 * COL_OFFSET + 10
#define NUM_COLS				31

#define ROW_1_Y					TOP - CARD_Y_SIZE - 10
#define ROW_2_Y					ROW_1_Y - CARD_Y_SIZE - 10
#define ROW_3_Y					ROW_2_Y - 10
#define ROW_4_Y					ROW_3_Y - 10
#define ROW_5_Y					ROW_4_Y - CARD_Y_SIZE - 10
#define ROW_6_Y					ROW_5_Y - 1 * ROW_OFFSET
#define ROW_7_Y					ROW_5_Y - 2 * ROW_OFFSET
#define ROW_8_Y					ROW_5_Y - 3 * ROW_OFFSET
#define ROW_9_Y					ROW_5_Y - 4 * ROW_OFFSET
#define ROW_10_Y				ROW_5_Y - 5 * ROW_OFFSET
#define ROW_11_Y				ROW_5_Y - 6 * ROW_OFFSET
#define ROW_12_Y				ROW_5_Y - 7 * ROW_OFFSET
#define ROW_13_Y				ROW_5_Y - 8 * ROW_OFFSET
#define ROW_14_Y				ROW_5_Y - 9 * ROW_OFFSET
#define ROW_15_Y				ROW_5_Y - 10 * ROW_OFFSET
#define ROW_16_Y				ROW_5_Y - 11 * ROW_OFFSET
#define ROW_17_Y				ROW_5_Y - 12 * ROW_OFFSET
#define ROW_18_Y				ROW_5_Y - 13 * ROW_OFFSET
#define ROW_19_Y				ROW_5_Y - 14 * ROW_OFFSET
#define ROW_20_Y				ROW_5_Y - 15 * ROW_OFFSET
#define ROW_21_Y				ROW_5_Y - 16 * ROW_OFFSET
#define ROW_22_Y				ROW_5_Y - 17 * ROW_OFFSET
#define ROW_23_Y				ROW_5_Y - 18 * ROW_OFFSET
#define ROW_24_Y				ROW_5_Y - 19 * ROW_OFFSET
#define ROW_25_Y				ROW_5_Y - 20 * ROW_OFFSET
#define ROW_26_Y				ROW_5_Y - 21 * ROW_OFFSET
#define ROW_27_Y				ROW_5_Y - 22 * ROW_OFFSET
#define ROW_28_Y				ROW_5_Y - 23 * ROW_OFFSET
#define ROW_29_Y				ROW_5_Y - 24 * ROW_OFFSET
#define ROW_30_Y				ROW_5_Y - 25 * ROW_OFFSET
#define ROW_31_Y				ROW_5_Y - 26 * ROW_OFFSET
#define NUM_ROWS				31


#define STK_B1_X				COL_1_X
#define STK_B2_X				COL_6_X
#define STK_B3_X				COL_11_X
#define STK_B4_X				COL_16_X
#define STK_B5_X				COL_21_X
#define STK_B6_X				COL_26_X
#define STK_B7_X				COL_31_X
#define STK_DECK_AVAIL_THIS		ROW_2_Y
#define STK_DECK_AVAIL_NEXT		ROW_3_Y
#define STK_DECK_Y				ROW_4_Y
#define STK_FIRST_BLD_Y			ROW_5_Y

#define STK_SUIT_Y				ROW_1_Y
#define STK_DIAM_X				COL_6_X
#define STK_CLUB_X				COL_11_X
#define STK_HEAR_X				COL_16_X
#define STK_SPAD_X				COL_21_X

#define HIGHEST_Z_VAL			60

#endif

