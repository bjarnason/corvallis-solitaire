/*
 * initPolicy.h
 * corvallis-solitaire
 *
 * Created by Ronald Bjarnason on Tue Dec 21 2010
 * Copyright (c) 2010.  All rights reserved
 *
 */

#ifndef INITPOLICY_H
#define INITPOLICY_H

#include "main.h"

void initHandCodedHCPolicy( HCFeature * h_c_policy[NUM_HCP_FEATURES] )
{
  int i;
  int card_offset;
  
  for ( i=0; i<NUM_HCP_FEATURES; i++ )
  {
    h_c_policy[i]->base_features_index++;
    h_c_policy[i]->base_features[0] = i;
    if ( i < 312 )
    {
      // On Suit Blocking
      h_c_policy[i]->weight = -1;
    }
    else if ( i < 400 )
    {
      // Off Color Blocking
      h_c_policy[i]->weight = -5;
    }
    else if ( i < 413 )
    {
      // Group in Foundation
      h_c_policy[i]->weight = 0;
    }
    else if ( i < 439 )
    {
      // Similar Cards Face Down
      h_c_policy[i]->weight = -1;
      
      // face down aces weight = -13, twos = -12, threes = -11, ... etc
      card_offset = i - 413;
      //h_c_policy[i]->weight = -( SUIT_BREAK - ( card_offset / 2 ) );
    }
    else if ( i < 491 )
    {
      // In Foundation
      h_c_policy[i]->weight = 5;
    }
    else if ( i < 543 ) 
    {
      // In Stock Playable
      h_c_policy[i]->weight = 1;
    }
    else if ( i < 595 )
    {
      // In Tableau Face Down
      // h_c_policy[i]-> weight = 5;
      
      // face up cards in tableau weight: aces = -12, twos = -11, threes = -10, ... etc
      card_offset = i - 543;
      h_c_policy[i]->weight = ( card_offset % 13 ) - SUIT_BREAK;
    }
    else
    {
      cout << "ERROR in initHandCodedHCPolicy() 0 " << endl;
      exit ( 0 );
    }
  }
}
void initEmptyHCPolicy( HCFeature *h_c_policy[595] )
{
  for ( int i=0; i<595; i++ )
  {
    h_c_policy[i]->base_features_index++;
    h_c_policy[i]->base_features[0] = i;
    h_c_policy[i]->weight = 0;
  }
}
void printPolicy( HCFeature *policy[595] )
{
  int i;
  cout << "OSB: ";
  for ( i=0; i<312; i++ )
  {
    cout << policy[i]->weight << " ";
  }
  cout << endl << endl << "OCB: ";
  for ( i=312; i<400; i++ )
  {
    cout << policy[i]->weight << " ";
  }
  cout << endl << endl << "GIF: ";
  for ( i=400; i<413; i++ )
  {
    cout << policy[i]->weight << " ";
  }
  cout << endl << endl << "SCFD: ";
  for ( i=413; i<439; i++ )
  {
    cout << policy[i]->weight << " ";
  }
  cout << endl << endl << "IF: ";
  for ( i=439; i<491; i++ )
  {
    cout << policy[i]->weight << " ";
  }
  cout << endl << endl << "ISP: ";
  for (i=491; i<543; i++ )
  {
    cout << policy[i]->weight << " ";
  }
  cout << endl << endl << "ITFD: ";
  for (i=543; i<595; i++ )
  {
    cout << policy[i]->weight << " ";
  }
  cout << endl;

}

void initHandCodedHCPolicy2( HCFeature * h_c_policy[595] )
{
  int i;
  int card_offset;

  for ( i=0; i<595; i++ )
  {
    h_c_policy[i]->base_features_index++;
    h_c_policy[i]->base_features[0] = i;
    if ( i < 312 )
    {
      // On Suit Blocking
      h_c_policy[i]->weight = (-5);
    }
    else if ( i < 400 )
    {
      // Off Color Blocking
      h_c_policy[i]->weight = (-10);
    }
    else if ( i < 413 )
    {
      // Group in Foundation
      //h_c_policy[i]->weight = 5;
    }
    else if ( i < 439 )
    {
      // Similar Cards Face Down
      h_c_policy[i]->weight = (-5);
      
      // face down aces weight = -13, twos = -12, threes = -11, ... etc
      card_offset = i - 413;
      //h_c_policy[i]->weight = ( ( card_offset % 13 ) - SUIT_BREAK );
    }
    else if ( i < 491 )
    {
      // In Foundation
      //h_c_policy[i]->weight = 20;
      
      // Aces in foundation = 5, twos = 4, ... etc
      card_offset = i - 439;
      //h_c_policy[i]->weight = (5 - ( card_offset % 13 ));
      h_c_policy[i]->weight = 0;
    }
    else if ( i < 543 ) 
    {
      // In Stock Playable
      // h_c_policy[i]->weight = 10;
      //h_c_policy[i]->weight = 1;
    }
    else if ( i < 595 )
    {
      // In Tableau Face Down
      //h_c_policy[i]-> weight = 0;
      
      // face down cards in tableau weight: aces = -13, twos = -12, threes = -11, ... etc
      card_offset = i - 543;
      h_c_policy[i]->weight = ( ( card_offset % 13 ) - SUIT_BREAK );
    }
    else
    {
      cout << "ERROR in initHandCodedHCPolicy() 0 " << endl;
      exit ( 0 );
    }
  }
}

void initRandomPolicy( HCFeature * policy[595], MTRand * my_rand_gen )
{
  for ( int i=0; i<595; i++ )
  {
    policy[i]->weight = ( ( double ( my_rand_gen->randInt() % 201 ) ) / 100 ) - 1.0;
  }

}



#endif
