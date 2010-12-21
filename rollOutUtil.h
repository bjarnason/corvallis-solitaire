/*
 * rollOutUtil.h
 * corvallis-solitaire
 *
 * Created by Ronald Bjarnason on Tue Dec 21 2010
 * Copyright (c) 2010. All rights reserved
 *
 */

#ifndef ROLLOUTUTIL_H
#define ROLLOUTUTIL_H

#include "main.h"

int greedyActionIndex( State * s, 
             bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
             bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK], 
             HCFeature * hc_feats[MAX_FEATURES], int hc_feat_index )
{
  int i;
  State * t = new State();
  double t_val;
  double greedy_val = NEG_INF;
  int greedy_action = -1;
  
  //cout << "choosing between actions: " ;
  for ( i=0; i<=s->valid_actions_index; i++ )
  {
    if ( s->valid_actions[i]->available )
    {
      s->transition( t, i );
      
      t_val = stateValue( t, base_prop, base_rela, hc_feats, hc_feat_index );
      //cout << i << "(" << t_val << ")  ";
      //t_val = t->stateValue();
      
      if ( t_val > greedy_val )
      {
        greedy_val = t_val;
        greedy_action = i;
      }
    }
  }
  //cout << " and choosing action " <<  greedy_action << endl;
  delete t;
  return greedy_action;
}

int greedyActionIndex( State * s[MAX_SEARCH_DEPTH], int base_depth, double uct_weights[NUM_UCT_WEIGHTS] )
{
  int i,j;
  int repeated_state = false;
  double best_action_value = INT_MIN;
  double action_value;
  int best_action = -1;
  double uct_features[NUM_UCT_FEATURES];

  for ( i=0; i<=s[base_depth]->valid_actions_index; i++) {
    if ( s[base_depth]->valid_actions[i]->available ) {

      fill_uct_features( s[base_depth], i, uct_features );
      action_value = uct_weighted_sum( uct_features, uct_weights );

      if ( best_action == -1 || action_value > best_action_value ){
        repeated_state = false;
        s[base_depth]->transition( s[base_depth+1], i );
        for ( j=base_depth; j>=0 && !repeated_state; j-- ) {
          if ( s[base_depth+1]->isFaceUpSame( s[j] ) ) {
            repeated_state = true;
          }
        }

        if ( !repeated_state ){
          best_action_value = action_value;
          best_action = i;
        }
      }
    }
  }

  return best_action;
}


/*
 * firstActionIndex( State*, MTRand, int search_depth );
 * returns the first valid (and available) action
 * that does not repeat a state already in the given trajectory history
 */
int firstActionIndex( State * s[MAX_SEARCH_DEPTH], int search_depth )
{
  int i, j;
  int available_actions_count = -1;
  int repeated_state = false;

  for ( i=0; i<=s[search_depth]->valid_actions_index; i++) {
    if ( s[search_depth]->valid_actions[i]->available ) {
      repeated_state = false;
      s[search_depth]->transition( s[search_depth+1], i );
      for ( j=0; j<search_depth; j++ ) {
        if ( s[search_depth+1]->isFaceUpSame( s[j] ) ) {
          repeated_state = true;
          break;
        }
      }
      if ( !repeated_state ) {
        return i;
      }
    }
  }
  return -1;
}


/*
 * randomActionIndex( State*, MTRand );
 * returns a random valid (and available) action
 * that does not repeat a state already in the given trajectory history
 */
int randomActionIndex( State * s[MAX_SEARCH_DEPTH], int search_depth, MTRand * my_rand_gen )
{
  int i, j;
  int available_actions_count = -1;
  int repeated_state = false;
  int good_actions[MAX_NUM_ACTIONS];

  for ( i=0; i<=s[search_depth]->valid_actions_index; i++) {
    if ( s[search_depth]->valid_actions[i]->available ) {
      repeated_state = false;
      s[search_depth]->transition( s[search_depth+1], i );
      for ( j=0; j<search_depth; j++ ) {
        if ( s[search_depth+1]->isFaceUpSame( s[j] ) ) {
          repeated_state = true;
          break;
        }
      }
      if ( !repeated_state ) {
        good_actions[++available_actions_count] = i;
      }
    }
  }

  if ( available_actions_count == -1 ) {
    //there are no available actions
    return -1;
  } else if ( available_actions_count == 0 ){
    // there is only one available actions
    return good_actions[0];
  }
  return good_actions[my_rand_gen->randInt() % available_actions_count];   // there are more than one avaialbe actions, so choose randomly

}

/*
 * randomActionIndex( State*, MTRand );
 * returns a random valid (and available) action
 */
int randomActionIndex( State * s, MTRand * my_rand_gen, bool non_repeat_actions[MAX_NUM_ACTIONS] )
{
  int i;
  int available_actions_count = 0;
  int good_actions[MAX_NUM_ACTIONS];

  for ( i=0; i<=s->valid_actions_index; i++ )
  {
    if ( s->valid_actions[i]->available && non_repeat_actions[i] ) {
      good_actions[available_actions_count++] = i;
    }
  }
  if ( available_actions_count == 0 ) return -1;
  else if ( available_actions_count == 1 ) return good_actions[0];
  return good_actions[my_rand_gen->randInt() % available_actions_count];
}

int randomUnchosenActionIndex( State * s, MTRand * my_rand_gen, bool non_repeat_actions[MAX_NUM_ACTIONS] )
{
  int i;
  int available_actions_count = 0;
  int good_actions[MAX_NUM_ACTIONS];

  for ( i=0; i<=s->valid_actions_index; i++ )
  {
    if ( s->valid_actions[i]->available && non_repeat_actions[i] && (s->my_uct_node->action_visits[i]==0) ) {
      good_actions[available_actions_count++] = i;
    }
  }
  if ( available_actions_count == 0 ) return -1;
  else if ( available_actions_count == 1 ) return good_actions[0];
  return good_actions[my_rand_gen->randInt() % available_actions_count];
}

/*
 * non_repeat_actions[] indicates which actions will not revisit a previous state
 */
int firstUnchosenActionIndex( State * s[MAX_SEARCH_DEPTH], int search_depth, bool non_repeat_actions[MAX_NUM_ACTIONS] )
{
  int i, j;
  int available_actions_count = -1;
  int repeated_state = false;

  for ( i=0; i<=s[search_depth]->valid_actions_index; i++) {
    if ( s[search_depth]->valid_actions[i]->available && non_repeat_actions[i] && (s[search_depth]->my_uct_node->action_visits[i]==0)) {
      return i;
    }
  }

  return -1;

}

int greedyUnchosenActionIndex( State * s[MAX_SEARCH_DEPTH], int base_depth, double uct_weights[NUM_UCT_WEIGHTS], bool non_repeat_actions[MAX_NUM_ACTIONS] )
{
  int i,j;
  int repeated_state = false;
  double best_action_value = INT_MIN;
  double action_value;
  int best_action = -1;
  double uct_features[NUM_UCT_FEATURES];

  for ( i=0; i<=s[base_depth]->valid_actions_index; i++) {
    if ( s[base_depth]->valid_actions[i]->available && non_repeat_actions[i] && (s[base_depth]->my_uct_node->action_visits[i]==0) ) {

      fill_uct_features( s[base_depth], i, uct_features );
      action_value = uct_weighted_sum( uct_features, uct_weights );

      if ( best_action == -1 || action_value > best_action_value ){
        best_action_value = action_value;
        best_action = i;
      }
    }
  }

  return best_action;
}

int epsilonTakePolicyAction( State * s[MAX_SEARCH_DEPTH], int search_depth, HCFeature* hc_feats[MAX_FEATURES], int hc_feat_index,
           bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
           bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK],
           MTRand * my_rand_gen )
{
  int best_a_index;

  if ( ( my_rand_gen->randInt() % 100 ) + 1 < ( EPSILON * 100 ) )
  {
    best_a_index = randomActionIndex( s, search_depth, my_rand_gen );
  }
  else
  {
    best_a_index = greedyActionIndex( s[search_depth], base_prop, base_rela, hc_feats, hc_feat_index );
  }
  if ( best_a_index != -1 ) s[search_depth]->transition( s[search_depth+1], best_a_index );
  return best_a_index;
}

int takePolicyAction( State * s[MAX_SEARCH_DEPTH], int search_depth, HCFeature* hc_feats[MAX_FEATURES], int hc_feat_index,
           bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
           bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK] )
{
  int i, best_a_index;
  double t_val;
  double best_a_val = NEG_INF;

  for ( i=0; i<=s[search_depth]->valid_actions_index; i++ )
  {
    if ( s[search_depth]->valid_actions[i]->available )
    {
      s[search_depth]->transition( s[search_depth+1], i );
      t_val = stateValue( s[search_depth+1], base_prop, base_rela, hc_feats, hc_feat_index );

      if ( t_val > best_a_val )
      {
        best_a_val = t_val;
        best_a_index = i;
      }
    }
  }

  s[search_depth]->transition( s[search_depth+1], best_a_index );

  if ( best_a_val == NEG_INF ) return -1;
  else return best_a_index;
}

int takeRandomAction( State * s[MAX_SEARCH_DEPTH], int search_depth, MTRand * my_rand_gen )
{
  int action = randomActionIndex( s, search_depth, my_rand_gen );

  if ( action == -1 ) return -1;

  s[search_depth]->transition( s[search_depth+1], action );
  return action;
}


#endif


