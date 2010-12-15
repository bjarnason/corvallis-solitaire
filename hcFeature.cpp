/*
 *  hcFeature.cpp
 *  solitaire
 *
 *  Created by Ronald Bjarnason on Mon Jan 22 2007.
 *  Copyright (c) 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "hcFeature.h"

using namespace std;

HCFeature::HCFeature ()
{
	init();
}

HCFeature::~HCFeature()
{}

void HCFeature::init()
{
	count = 0;
	weight = 0.0;
	score = 0.0;
	base_features_index = -1;
}

bool HCFeature::isSame( HCFeature * f )
{
	int i;
	bool same = true;
	
	if ( base_features_index == f->base_features_index )
	{
		for ( i=0; i<=base_features_index; i++ )
		{
			if ( base_features[i] != f->base_features[i] )
			{
				same = false;
				break;
			}
		}
	}
	else
	{
		same = false;
	}
	return same;
}

bool HCFeature::sufficientlyDifferent( HCFeature * f, int share_limit )
{	
	int share_count = 0;
	int my_rule_total = base_features_index + 1;
	int his_rule_total = f->base_features_index + 1;
	int i;

	for ( i=0; i<=base_features_index; i++ )
	{
		if ( base_features[i] == f->base_features[i] )
		{
			share_count++;
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

void HCFeature::addFeatureAttributes( int feature_index )
{
	base_features[++base_features_index] = feature_index;
}

void HCFeature::copyInto( HCFeature * f )
{
	int i;
	
	f->weight = weight;
	f->score = score;
	f->base_features_index = base_features_index;
	
	for ( i=0; i<=base_features_index; i++ )
	{
		f->base_features[i] = base_features[i];
	}
}

void HCFeature::prettyPrint()
{
	int i;
	cout << "sc: " << score << "  ";
	cout << "wt: " << weight << "  ";
	cout << base_features_index + 1 << " total features:";
	
	for ( i=0; i<=base_features_index; i++ )
	{
		printHCFeatureToChar( base_features[i] );
	}
	cout << endl;
}

void HCFeature::printHCFeatureToChar( int feature_index )
{
	int feature, card1, card2;
	bool two_cards = false;
	
	HCPGetFeature( feature_index, feature, card1, card2 );
	switch ( feature )
	{
		case HCP_OSB:
			cout << "OnSuitBlocked [";
			two_cards = true;
			break;
		case HCP_OCB:
			cout << "OffColorBlocked [";
			two_cards = true;
			break;
		case HCP_GIF:
			cout << "GroupInFoundation [";
			break;
		case HCP_SCFD:
			cout << "SimilarCardsFaceDown [";
			break;
		case HCP_IF:
			cout << "InFoundation [";
			break;
		case HCP_ISP:
			cout << "InStockPlayable [";
			break;
		case HCP_ITFU:
			cout << "InTableauFaceUp [";
			break;
	}
	
	printIndexToChar( card1 );
	
	if ( two_cards )
	{
		cout << ",";
		printIndexToChar( card2 );
	}
	cout << "]" << endl;
}

void HCFeature::printIndexToChar( int card_index )
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

int HCFeature::getOSBIndex( int card_mod )
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

void HCFeature::HCPGetFeature( int HCP_index, int & feature, int & card1, int & card2 )
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

int HCFeature::getOSBCard( int index )
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

