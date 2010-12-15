/*
 *  feature.cpp
 *  solitaire
 *
 *  Created by Ronald Bjarnason on Wed Feb 15 2006.
 *  Copyright (c) 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "feature.h"

using namespace std;

Feature::Feature ()
{
	init();
}

Feature::~Feature()
{}

void Feature::init()
{
	count = 0;
	weight = 0.0;
	score = 0.0;
	bp_conj_index = -1;
	br_conj_index = -1;
	cr_conj_index = -1;
}

bool Feature::isSame( Feature * f )
{
	int i;
	bool same = true;
	
	if ( bp_conj_index == f->bp_conj_index && br_conj_index == f->br_conj_index && cr_conj_index == f->cr_conj_index )
	{
		for ( i=0; i<=bp_conj_index; i++ )
		{
			if ( bp_conj[i][0] != f->bp_conj[i][0] || bp_conj[i][1] != f->bp_conj[i][1] )
			{
				same = false;
				break;
			}
		}
		if ( same )
		{
			for ( i=0; i<=br_conj_index; i++ )
			{
				if ( br_conj[i][0] != f->br_conj[i][0] || br_conj[i][1] != f->br_conj[i][1] || br_conj[i][2] != f->br_conj[i][2]  )
				{
					same = false;
					break;
				}
			}
			if ( same )
			{
				for ( i=0; i<=cr_conj_index; i++ )
				{
					if ( cr_conj[i][0] != f->cr_conj[i][0] || cr_conj[i][1] != f->cr_conj[i][1] || cr_conj[i][2] != f->cr_conj[i][2]  )
					{
						same = false;
						break;
					}
				}
			}
		}
	}
	else
	{
		same = false;
	}
	return same;
}

bool Feature::sufficientlyDifferent( Feature * f, int share_limit )
{
	int share_count = 0;
	int my_rule_total = bp_conj_index + br_conj_index + cr_conj_index + 3;
	int his_rule_total = f->bp_conj_index + f->br_conj_index + f->cr_conj_index + 3;
	int i, j;
	
	for ( i=0; i<=bp_conj_index; i++ )
	{
		for ( j=0; j<=f->bp_conj_index; j++ )
		{
			if ( bp_conj[i][0] == f->bp_conj[j][0] && bp_conj[i][1] == f->bp_conj[j][1] )
			{
				share_count++;
			}
		}
	}
	
	for ( i=0; i<=br_conj_index; i++ )
	{
		for ( j=0; j<=f->br_conj_index; j++ )
		{
			if ( br_conj[i][0] == f->br_conj[j][0] && br_conj[i][1] == f->br_conj[j][1] && br_conj[i][2] == f->br_conj[j][2] )
			{
				share_count++;
			}
		}
	}
	
	for ( i=0; i<=cr_conj_index; i++ )
	{
		for ( j=0; j<=f->cr_conj_index; j++ )
		{
			if ( cr_conj[i][0] == f->cr_conj[j][0] && cr_conj[i][1] == f->cr_conj[j][1] && cr_conj[i][2] == f->cr_conj[j][2] )
			{
				share_count++;
			}
		}
	}
	
	// the two features are not sufficiently different if:
	// 1. they share too many rules
	// 2. the limited number of rules they have are all in common (even though it may be less than the limit)
	if ( share_count > share_limit || ( my_rule_total == his_rule_total && share_count == my_rule_total ) )
	{
		return false;
	}
	
	return true;
}

void Feature::copyInto( Feature * f )
{
	int i;
	
	f->weight = weight;
	f->score = score;
	f->positive_feature = positive_feature;
	f->bp_conj_index = bp_conj_index;
	f->br_conj_index = br_conj_index;
	f->cr_conj_index = cr_conj_index;
	for ( i=0; i<=bp_conj_index; i++ )
	{
		f->bp_conj[i][0] = bp_conj[i][0];
		f->bp_conj[i][1] = bp_conj[i][1];
	}
	for ( i=0; i<=br_conj_index; i++ )
	{
		f->br_conj[i][0] = br_conj[i][0];
		f->br_conj[i][1] = br_conj[i][1];
		f->br_conj[i][2] = br_conj[i][2];
	}
	for ( i=0; i<=cr_conj_index; i++ )
	{
		f->cr_conj[i][0] = cr_conj[i][0];
		f->cr_conj[i][1] = cr_conj[i][1];
		f->cr_conj[i][2] = cr_conj[i][2];
	}
}

void Feature::prettyPrint()
{
	int i;
	cout << "sc: " << score << "  ";
	cout << "wt: " << weight << "  ";
	cout << bp_conj_index + br_conj_index + cr_conj_index + 3 << " total features:";
	for ( i=0; i<=bp_conj_index; i++ )
	{
		//cout << " type = " << bp_conj[i][0] << ", card = " << bp_conj[i][1];
		printPropositionTypeToChar( bp_conj[i][0] );
		cout << "[";
		printIndexToChar( bp_conj[i][1] );
		cout << "] ";
	}
	for ( i=0; i<=br_conj_index; i++ )
	{
		//cout << " type = " << br_conj[i][0] << ", card1 = " << br_conj[i][1] << ", card2 = " << br_conj[i][2];
		printRelationTypeToChar( br_conj[i][0] );
		cout << "[";
		printIndexToChar( br_conj[i][1] );
		cout << "] ";		
		cout << "[";
		printIndexToChar( br_conj[i][2] );
		cout << "] ";		
	}
	for ( i=0; i<=cr_conj_index; i++ )
	{
		//cout << " type = " << cr_conj[i][0] << ", card1 = " << cr_conj[i][1] << ", card2 = " << cr_conj[i][2];
		printConstRelationTypeToChar( cr_conj[i][0] );
		cout << "[";
		printIndexToChar( cr_conj[i][1] );
		cout << "] ";		
		cout << "[";
		printIndexToChar( cr_conj[i][2] );
		cout << "] ";
	}
	cout << endl;
}

void Feature::printIndexToChar( int card_index )
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
}

void Feature::printStackIndexToChar( int stack_id )
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

void Feature::printPropositionTypeToChar( int feature_type )
{
	switch( feature_type )
	{
		case F_IN_FOUNDATION:
			cout << "IN_FOUND";
			break;
		case F_IN_STOCK_PLAYABLE:
			cout << "IN_STOCK_P";
			break;
		case F_IN_STOCK_UNPLAYABLE:
			cout << "IN_STOCK_U";
			break;
		case F_IN_TABLEAU_FACE_DOWN:
			cout << "IN_TABL_FD";
			break;
		case F_IN_TABLEAU_FACE_UP:
			cout << "IN_TABL_FU";
			break;
		case AF_FROM_FOUNDATION:
			cout << "A_FROM_FOUND";
			break;
		case AF_FROM_TABLEAU:
			cout << "A_FROM_TABL";
			break;
		case AF_FROM_STOCK:
			cout << "A_FROM_STOCK";
			break;
		case AF_TO_FOUNDATION:
			cout << "A_TO_FOUND";
			break;
		case AF_TO_TABLEAU:
			cout << "A_TO_TABL";
			break;
		case AF_REVEAL:
			cout << "A_REVEAL";
			break;
	}
}

void Feature::printRelationTypeToChar( int relation_type )
{
	switch( relation_type )
	{
		case F_FOUNDATION_ABOVE:
			cout << "FOUND_ABOVE";
			break;
		case F_STOCK_ABOVE:
			cout << "STOCK_ABOVE";
			break;
		case F_TABLEAU_ABOVE:
			cout << "TABLEAU_ABOVE";
			break;
	}
}

void Feature::printConstRelationTypeToChar( int relation_type )
{
	switch( relation_type )
	{
		case F_FOUNDATION_BUILD_ON:
			cout << "F_BUILD_ON";
			break;	
		case F_TABLEAU_BUILD_ON:
			cout << "T_BUILD_ON";
			break;
		case F_SAME_SUIT:
			cout << "SAME_SUIT";
			break;
		case F_SAME_COLOR:
			cout << "SAME_COLOR";
			break;
		case F_LOWER_RANK:
			cout << "LOWER_RANK";
			break;
		case F_SAME_RANK:
			cout << "SAME_RANK";
			break;
			
	}
}

void Feature::addFeatureAttributes( Feature * f )
{
	int i, j;
	bool add;
		
	for ( i=0; i<=f->bp_conj_index; i++ )
	{
		add = true;
		for ( j=0; j<=bp_conj_index; j++ )
		{
			if ( f->bp_conj[i][0] == bp_conj[j][0] && f->bp_conj[i][1] == bp_conj[j][1] )
			{
				add = false;
			}
		}
		if ( add )
		{
			bp_conj[++bp_conj_index][0] = f->bp_conj[i][0];
			bp_conj[bp_conj_index][1] = f->bp_conj[i][1];
		}
	}
	
	for ( i=0; i<=f->br_conj_index; i++ )
	{
		add = true;
		for ( j=0; j<=br_conj_index; j++ )
		{
			if ( f->br_conj[i][0] == br_conj[j][0] && f->br_conj[i][1] == br_conj[j][1] && f->br_conj[i][2] == br_conj[j][2] )
			{
				add = false;
			}
		}
		if ( add )
		{
			br_conj[++br_conj_index][0] = f->br_conj[i][0];
			br_conj[br_conj_index][1] = f->br_conj[i][1];
			br_conj[br_conj_index][2] = f->br_conj[i][2];
		}
	}
	
	for ( i=0; i<=f->cr_conj_index; i++ )
	{
		add = true;
		for ( j=0; j<=cr_conj_index; j++ )
		{
			if ( f->cr_conj[i][0] == cr_conj[j][0] && f->cr_conj[i][1] == cr_conj[j][1] && f->cr_conj[i][2] == cr_conj[j][2] )
			{
				add = false;
			}
		}
		if ( add )
		{
			cr_conj[++cr_conj_index][0] = f->cr_conj[i][0];
			cr_conj[cr_conj_index][1] = f->cr_conj[i][1];
			cr_conj[cr_conj_index][2] = f->cr_conj[i][2];
		}
	}
}

bool Feature::shareVariable( Feature * f )
{
	bool return_value = false;
	int i, j;
	
	for ( i=0; i<=f->bp_conj_index; i++ )
	{
		for ( j=0; j<=bp_conj_index; j++ )
		{
			if ( f->bp_conj[i][1] == bp_conj[j][1] )
			{
				return_value = true;
			}
		}
		for ( j=0; j<=br_conj_index; j++ )
		{
			if ( f->bp_conj[i][1] == br_conj[j][1] || f->bp_conj[i][1] == br_conj[j][2]  )
			{
				return_value = true;
			}
		}
	}
	
	for ( i=0; i<=f->br_conj_index; i++ )
	{
		for ( j=0; j<=bp_conj_index; j++ )
		{
			if ( f->br_conj[i][1] == bp_conj[j][1] || f->br_conj[i][2] == bp_conj[j][1] )
			{
				return_value = true;
			}
		}
		for ( j=0; j<=br_conj_index; j++ )
		{
			if ( f->br_conj[i][1] == br_conj[j][1] || f->br_conj[i][1] == br_conj[j][2] ||
				 f->br_conj[i][2] == br_conj[j][1] || f->br_conj[i][2] == br_conj[j][2] )
			{
				return_value = true;
			}
		}
	}
	return return_value;
}

bool Feature::relatedByStaticRelation( Feature * f, bool const_rel[NUM_CONST_RELATIONS][FULL_DECK][FULL_DECK] )
{
	int i, j, k;
	
	for ( k=0; k<NUM_CONST_RELATIONS; k++ )
	{
		for ( i=0; i<=f->bp_conj_index; i++ )
		{
			for ( j=0; j<=bp_conj_index; j++ )
			{
				if ( const_rel[k][f->bp_conj[i][1]][bp_conj[j][1]] )
				{
					return true;
				}
			}
			for ( j=0; j<=br_conj_index; j++ )
			{
				if ( const_rel[k][f->bp_conj[i][1]][br_conj[j][1]] || const_rel[k][f->bp_conj[i][1]][br_conj[j][2]] )
				{
					return true;
				}
			}
		}
		
		for ( i=0; i<=f->br_conj_index; i++ )
		{
			for ( j=0; j<=bp_conj_index; j++ )
			{
				//if ( ( f->br_conj[i][1] / 13 ) == ( bp_conj[j][1] / 13 ) || ( f->br_conj[i][2] / 13 ) == ( bp_conj[j][1] / 13 ) )
				if ( const_rel[k][f->br_conj[i][1]][bp_conj[j][1]] || const_rel[k][f->br_conj[i][2]][bp_conj[j][1]] )
				{
					return true;
				}
			}
			for ( j=0; j<=br_conj_index; j++ )
			{
				if ( const_rel[k][f->br_conj[i][1]][br_conj[j][1]] || const_rel[k][f->br_conj[i][1]][bp_conj[j][2]] ||
					 const_rel[k][f->br_conj[i][2]][br_conj[j][1]] || const_rel[k][f->br_conj[i][2]][bp_conj[j][2]] )
				{
					return true;
				}
			}
		}
	}
	return false;
}



