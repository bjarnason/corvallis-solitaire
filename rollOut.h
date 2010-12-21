/*
 * rollOut,h
 * corvallis-solitaire
 *
 * Created by Ronald Bjarnason on Tue Dec 21 2010
 * Copyright (c) 2010.  All rights reserved
 *
 */

#ifndef ROLLOUT_H
#define ROLLOUT_H

#include "main.h"
#include "rollOutUtil.h"

// --------
//
double rollOut( State * s[MAX_SEARCH_DEPTH], int base_depth, 
        RollOutPolicyInfo * hi_p, RollOutPolicyInfo * lo_p,
        double s_vals[MAX_SEARCH_DEPTH], int h_vals[MAX_SEARCH_DEPTH], int p_vals[MAX_SEARCH_DEPTH],
        time_t start, 
        bool check_time,
        int &last_node_index, 
        bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
        bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK], 
        int winning_path[MAX_SEARCH_DEPTH], bool note_path,
        int greedy_path[MAX_SEARCH_DEPTH], bool note_greedy_path,
        int which_p, int frontier )
{
  int i, j;
  double t_val;
  int greedy_action;
  int depth_offset = 0;
  int backtrackval = 0;

  double cur_score;
  if ( base_depth < frontier ) {
    cur_score = stateValue( s[base_depth+depth_offset], base_prop, base_rela, hi_p->policy, hi_p->policy_index );
  } else {
    cur_score = stateValue( s[base_depth+depth_offset], base_prop, base_rela, lo_p->policy, lo_p->policy_index );
  }

  double best_move_value = cur_score;
  int best_move_index = -1;

  //if ( rollOutCheckTime( check_time, start ) ) 
  //  return TOO_LONG;

  if ( rollOutCheckWin( s, base_depth+depth_offset, note_path, last_node_index ) )
    return POS_INF;

  s_vals[base_depth+depth_offset] = cur_score;
  h_vals[base_depth+depth_offset] = base_depth < frontier ? hi_p->h_level : lo_p->h_level;
  p_vals[base_depth+depth_offset] = base_depth < frontier ? which_p : which_p+1;

  if ( rollOutCheckLoop( s, base_depth+depth_offset, false, last_node_index, s_vals, h_vals, p_vals ) != -1 )
    return NEG_INF;
  
  if ( (base_depth < frontier) && ( rollOutEndCase( s, base_depth+depth_offset, note_path, last_node_index, hi_p->v, hi_p->v_index, hi_p->h_level ) ) )
  {
    return cur_score;
  } 
  else if ( (base_depth >= frontier) && ( rollOutEndCase( s, base_depth+depth_offset, note_path, last_node_index, lo_p->v, lo_p->v_index, lo_p->h_level ) ) )
  {
    return cur_score;
  }

  while ( best_move_value != NEG_INF && base_depth+depth_offset+1 < MAX_SEARCH_DEPTH )
  {
    //cout << "depth_offset = " << depth_offset << endl;

    best_move_value = NEG_INF;
    //best_move_value = cur_score;
    
    best_move_index = -1;
    
    // --------
    // for each action, get a value based on rollOut to determine the best action
    for ( i=0; i<=s[base_depth+depth_offset]->valid_actions_index; i++ )
    {
      if ( s[base_depth+depth_offset]->valid_actions[i]->available )
      {        
        s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], i );
        //s[base_depth+depth_offset+1]->prettyPrint();
        
        if ( base_depth < frontier ) 
          hi_p->h_level--;
        else 
          lo_p->h_level--;

        t_val = rollOut( s, base_depth+depth_offset+1, hi_p, lo_p, s_vals, h_vals, p_vals, start, check_time, last_node_index,
                 base_prop, base_rela, winning_path, note_path, greedy_path, note_greedy_path, which_p, frontier );
        if ( base_depth < frontier ) 
          hi_p->h_level++;
        else 
          lo_p->h_level++;
        
        if ( t_val == POS_INF )
        {
          if ( note_path )
          {
            winning_path[base_depth+depth_offset] = i;
            greedy_path[base_depth+depth_offset] = base_depth < frontier ? 
              greedyActionIndex( s[base_depth+depth_offset], base_prop, base_rela, hi_p->policy, hi_p->policy_index ):
              greedyActionIndex( s[base_depth+depth_offset], base_prop, base_rela, lo_p->policy, lo_p->policy_index ) ;
          }
          return POS_INF;
        }
        else if ( t_val == TOO_LONG )
        {
          return TOO_LONG;
        }
        if ( t_val > best_move_value )
        {
          best_move_value = t_val;
          best_move_index = i;
        }
      }
    }

    if ( note_path && base_depth < frontier ) greedy_action = greedyActionIndex( s[base_depth+depth_offset], base_prop, base_rela, hi_p->policy, hi_p->policy_index );
    else if ( note_path && base_depth >= frontier ) greedy_action = greedyActionIndex( s[base_depth+depth_offset], base_prop, base_rela, lo_p->policy, lo_p->policy_index );
    // --------
    
    if ( best_move_index == -1 )
    {
      // there were no possible actions from this state, or all options have already been visited
      if ( note_path ){ last_node_index = base_depth+depth_offset; }
      return cur_score;
    }
    
    // No solution was found
    // now that we know which action is the best one, take that action
    if ( note_path )
    {
      winning_path[base_depth+depth_offset] = best_move_index;
      greedy_path[base_depth+depth_offset] = greedy_action;
    }
    
    s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], best_move_index );
    
    depth_offset++;
  
    if ( base_depth < frontier )
      cur_score = stateValue( s[base_depth+depth_offset], base_prop, base_rela, hi_p->policy, hi_p->policy_index );
    else
      cur_score = stateValue( s[base_depth+depth_offset], base_prop, base_rela, lo_p->policy, lo_p->policy_index );
    
    s_vals[base_depth+depth_offset] = cur_score;
    h_vals[base_depth+depth_offset] = base_depth < frontier ? hi_p->h_level : lo_p->h_level;
    p_vals[base_depth+depth_offset] = base_depth < frontier ? which_p : which_p+1;
    
    j = rollOutCheckLoop( s, base_depth+depth_offset, note_path, last_node_index, s_vals, h_vals, p_vals );

    if ( j != -1 ) {
      //cout << "repeated choice at state " << base_depth+depth_offset << " after taking action " << best_move_index << endl;
      return NEG_INF;
      //exit( 0 );
    }
  }
  
  return best_move_value;
}

// --------
//
bool HOptRollOut( State * s[MAX_SEARCH_DEPTH], int base_depth, 
        RollOutPolicyInfo * hi_p, RollOutPolicyInfo * lo_p,
        double s_vals[MAX_SEARCH_DEPTH], int h_vals[MAX_SEARCH_DEPTH], int p_vals[MAX_SEARCH_DEPTH],
        time_t start, 
        bool check_time,
        int &last_node_index, 
        bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
        bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK], 
        int winning_path[MAX_SEARCH_DEPTH], bool note_path,
        int greedy_path[MAX_SEARCH_DEPTH], bool note_greedy_path,
        int which_p, int frontier, State * test_states[SURVEY_SIZE], 
        State * t_state, MTRand * my_rand_gen )
{
  int i, j, k;
  double t_val;
  int greedy_action;
  int depth_offset = 0;
  int backtrackval = 0;
  int num_moves = 0;
  int num_face_down = 22;

  double avg_move_value; 
  double best_move_value = 0;
  int num_move_wins, best_move_wins;
  int best_move_index;

  int best_path[MAX_SEARCH_DEPTH];
  int t_best_path[MAX_SEARCH_DEPTH];
  int short_path = MAX_SEARCH_DEPTH+1;
  // compare apples to apples - shuffle your face down cards 10 times, and compare actions against those same states
  
  double cur_score = 0;

  //s[base_depth+depth_offset]->prettyPrint();

  while ( !s[base_depth+depth_offset]->win() && best_move_value != NEG_INF && base_depth+depth_offset+1 < MAX_SEARCH_DEPTH ) {
    best_move_value = NEG_INF;

    //cout << "step "<< depth_offset << endl;
    
    best_move_index = -1;
    
    // if we haven't revealed a face-down card (from the last decision process we made)
    // then go with the good decision we remembered from before
    // but only if the last decision process led to a winning game (indicated by short_path < MAX_SEARCH_DEPTH)
    
    while ( s[base_depth+depth_offset]->numFaceDownCards() == num_face_down && short_path < MAX_SEARCH_DEPTH ){

      //cout << "NON-REVEALING MOVE - taking action " << best_path[depth_offset] << " on step " << depth_offset << endl;
      s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], best_path[depth_offset] );
      winning_path[base_depth+depth_offset] = best_path[depth_offset];
      depth_offset++;
    }

    // we're about to shuffle all of the face-down cards, so store where the cards really are
    s[base_depth+depth_offset]->copyInto( t_state );

    for ( i=0; i<SURVEY_SIZE; i++ ){
      s[base_depth+depth_offset]->copyInto( test_states[i] );
      test_states[i]->shuffleFaceDownCards( my_rand_gen );
    }

    // --------
    // for each action, get a value based on hindsight optimization to determine the best action
    for ( i=0; i<=s[base_depth+depth_offset]->valid_actions_index; i++ )
    {
      //cout << " testing action " << i << endl;
      num_face_down = s[base_depth+depth_offset]->numFaceDownCards();
      if ( s[base_depth+depth_offset]->valid_actions[i]->available )
      {        
        winning_path[depth_offset] = i;

        num_move_wins = 0;
        avg_move_value = 0;
        short_path = MAX_SEARCH_DEPTH+1;

        for ( j=0; j<SURVEY_SIZE; j++ ){

          // mix up the face down cards
          test_states[j]->copyInto( s[base_depth+depth_offset] );

          s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], i );
          t_val = rollOut2( s, base_depth+depth_offset+1, hi_p, lo_p, s_vals, h_vals, p_vals, start, check_time, last_node_index,
                   base_prop, base_rela, winning_path, note_path, greedy_path, note_greedy_path, which_p );

          if ( t_val == POS_INF && num_face_down == 0 ) return true;
          if ( t_val == POS_INF ) num_move_wins++;
          else avg_move_value += t_val;

          if ( t_val == POS_INF && last_node_index < short_path ) {
            short_path = last_node_index;
            for ( k=0; k<=last_node_index; k++ ) t_best_path[k] = winning_path[k];
          }
        }

        if ( num_move_wins == SURVEY_SIZE ) avg_move_value = POS_INF;
        else avg_move_value = ( avg_move_value / ( SURVEY_SIZE - num_move_wins ) );

        if ( ( best_move_index == -1 ) || ( num_move_wins > best_move_wins  ) || ( num_move_wins == best_move_wins && avg_move_value > best_move_value  ) ){
          best_move_index = i;
          best_move_wins = num_move_wins;
          best_move_value = avg_move_value;
          if ( short_path < MAX_SEARCH_DEPTH ) for ( k=0; k<=short_path; k++ ) best_path[k] = t_best_path[k];
        }
      }
    }

    // --------
    
    // No solution was found
    if ( best_move_index == -1 ) return false;
    
    // now that we know which action is the best one, take that action
    if ( note_path ) {
      winning_path[base_depth+depth_offset] = best_move_index;
      greedy_path[base_depth+depth_offset] = greedy_action;
    }
    
    t_state->copyInto( s[base_depth+depth_offset] );
    s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], best_move_index );
    depth_offset++;
    
    j = rollOutCheckLoop( s, base_depth+depth_offset, note_path, last_node_index, s_vals, h_vals, p_vals );
    if ( j != -1 ) return false;
  }
  
  return s[base_depth+depth_offset]->win();
}

// --------

// --------
//
bool HOptRollOut2( State * s[MAX_SEARCH_DEPTH], int base_depth, 
        RollOutPolicyInfo * hi_p, RollOutPolicyInfo * lo_p,
        double s_vals[MAX_SEARCH_DEPTH], int h_vals[MAX_SEARCH_DEPTH], int p_vals[MAX_SEARCH_DEPTH],
        time_t start, 
        bool check_time,
        int &last_node_index, 
        bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
        bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK], 
        int winning_path[MAX_SEARCH_DEPTH], bool note_path,
        int greedy_path[MAX_SEARCH_DEPTH], bool note_greedy_path,
        int which_p, int frontier, State * test_states[SURVEY_SIZE], 
        State * t_state, MTRand * my_rand_gen )
{
  int i, j, k;
  double t_val;
  int greedy_action;
  int depth_offset = 0;
  int backtrackval = 0;
  int num_moves = 0;
  int num_face_down = 22;

  double cur_move_value; 
  double cur_move_losing_value;
  double best_move_value = 0;
  double best_move_losing_value;
  int num_move_wins, best_move_wins;
  int best_move_index;

  int best_path[MAX_SEARCH_DEPTH];
  int t_best_path[MAX_SEARCH_DEPTH];
  int short_path = MAX_SEARCH_DEPTH+1;

  int upper_action;
  int num_total_samples;
  int num_samples[MAX_NUM_ACTIONS];
  int num_wins[MAX_NUM_ACTIONS];
  bool valid_actions[MAX_NUM_ACTIONS];
  double losing_values[MAX_NUM_ACTIONS];
  double z_score = Z_99;
  // compare apples to apples - shuffle your face down cards 10 times, and compare actions against those same states
  
  double cur_score = 0;

  //s[base_depth+depth_offset]->prettyPrint();

  while ( !s[base_depth+depth_offset]->win() && best_move_value != NEG_INF && base_depth+depth_offset+1 < MAX_SEARCH_DEPTH ) {

    best_move_value = NEG_INF;
    //short_path = MAX_SEARCH_DEPTH+1;
    //s[base_depth+depth_offset]->prettyPrint();

    //cout << "step "<< depth_offset << endl;
    
    best_move_index = -1;
    
    // if we haven't revealed a face-down card (from the last decision process we made)
    // then go with the good decision we remembered from before
    // but only if the last decision process led to a winning game (indicated by short_path < MAX_SEARCH_DEPTH)
    
    
    /*
    while ( s[base_depth+depth_offset]->numFaceDownCards() == num_face_down && short_path < MAX_SEARCH_DEPTH ){

      //cout << "NON-REVEALING MOVE - taking action " << best_path[depth_offset] << " on step " << depth_offset << endl;
      s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], best_path[depth_offset] );
      winning_path[base_depth+depth_offset] = best_path[depth_offset];
      depth_offset++;
      //s[base_depth+depth_offset]->prettyPrint();
    }
    */
    
    num_face_down = s[base_depth+depth_offset]->numFaceDownCards();

    // we're about to shuffle all of the face-down cards, so store where the cards really are
    s[base_depth+depth_offset]->copyInto( t_state );

    for ( i=0; i<SURVEY_SIZE; i++ ){
      s[base_depth+depth_offset]->copyInto( test_states[i] );
      test_states[i]->shuffleFaceDownCards( my_rand_gen );
    }

    // for a number of samples, sample according to the upper bound
    num_total_samples = 0;
    for ( i=0; i<MAX_NUM_ACTIONS; i++ ) {
      valid_actions[i] = false;
      losing_values[i] = num_samples[i] = num_wins[i] = 0;
    }
    for ( i=0; i<=s[base_depth+depth_offset]->valid_actions_index; i++ ) {
      if ( s[base_depth+depth_offset]->valid_actions[i]->available ){
        num_total_samples++;
        valid_actions[i] = true;
        //cout << i << " ";
      }
    }
    //cout << " are valid actions" << endl;

    num_total_samples = SURVEY_SIZE * num_total_samples;

    for ( i=0; i<num_total_samples; i++ ){
      upper_action = chooseUpper( num_samples, num_wins, valid_actions, z_score );
      //cout << " choosing action " << upper_action << endl;
      if ( upper_action == -1 ) return false;

      // to keep everything fair, test all of the actions on the same shuffles
      // for the first SURVEY_SIZE samples (the average number of samples per action)
      if ( num_samples[upper_action] < SURVEY_SIZE )
        test_states[num_samples[upper_action]]->copyInto( s[base_depth+depth_offset] );
      else{
        s[base_depth+depth_offset]->shuffleFaceDownCards( my_rand_gen );
      }
      s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], upper_action );

      t_val = rollOut2( s, base_depth+depth_offset+1, hi_p, lo_p, s_vals, h_vals, p_vals, start, check_time, last_node_index,
               base_prop, base_rela, winning_path, note_path, greedy_path, note_greedy_path, which_p );
      num_samples[upper_action]++;

      // if we can win a game when there are no more face down cards then we will always win the game
      if ( t_val == POS_INF && num_face_down == 0 ) return true;
      if ( t_val == POS_INF ) num_wins[upper_action]++;
      else losing_values[upper_action] += t_val;

      // if we've taken a significant number of samples, and won every time with one action, then we can commit
      if ( num_samples[upper_action] > SURVEY_SIZE && num_wins[upper_action] == num_samples[upper_action] ) break;

      /*  
      if ( t_val == POS_INF && last_node_index < short_path ) {
        short_path = last_node_index;
        for ( k=0; k<=last_node_index; k++ ) t_best_path[k] = winning_path[k];
      }
      */
    }
    // now that we've sampled all that we are going to sample, pick according to the mean performance
    //cout << "FINAL ACTION VALUES: ";
    for ( i=0; i<=s[base_depth+depth_offset]->valid_actions_index; i++ ){
      if ( s[base_depth+depth_offset]->valid_actions[i]->available ){
        cur_move_value = num_wins[i]/(double)num_samples[i];
        cur_move_losing_value = losing_values[i]/(num_samples[i] - num_wins[i] );
        //cout << i << "[" << cur_move_value << "," << cur_move_losing_value << "]  ";
        if ( ( best_move_index == -1 ) 
            || ( cur_move_value > best_move_value  ) 
            || ( cur_move_value == best_move_value && cur_move_losing_value > best_move_losing_value ) ){
          best_move_index = i;
          best_move_value = cur_move_value;
          best_move_losing_value = cur_move_losing_value;
        }
      }
    }
    //cout << " and choosing action " << best_move_index << endl;

    // No solution was found
    if ( best_move_index == -1 ) return false;
    
    // now that we know which action is the best one, take that action
    if ( note_path ) {
      winning_path[base_depth+depth_offset] = best_move_index;
      greedy_path[base_depth+depth_offset] = greedy_action;
    }
    
    t_state->copyInto( s[base_depth+depth_offset] );
    s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], best_move_index );
    depth_offset++;
    
    j = rollOutCheckLoop( s, base_depth+depth_offset, note_path, last_node_index, s_vals, h_vals, p_vals );
    if ( j != -1 ) return false;
  }
  
  return s[base_depth+depth_offset]->win();
}

// --------

// --------
//
State * rollOutAnnotated( State * s[MAX_SEARCH_DEPTH], int base_depth, 
        RollOutPolicyInfo * p,
        double s_vals[MAX_SEARCH_DEPTH], int h_vals[MAX_SEARCH_DEPTH], int p_vals[MAX_SEARCH_DEPTH],
        int &last_node_index, 
        bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
        bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK], 
        int winning_path[MAX_SEARCH_DEPTH],
        int winning_search[MAX_SEARCH_DEPTH],
        State * t_state, State * b_state )
{
  int i, j;
  double t_val;
  int depth_offset = 0;

  double cur_score = stateValue( s[base_depth+depth_offset], base_prop, base_rela, p->policy, p->policy_index );
  s[base_depth+depth_offset]->value = cur_score;

  double best_move_value = cur_score;
  int best_move_index = -1;

  if ( rollOutCheckWin( s, base_depth+depth_offset, true, last_node_index ) )
  {
    //cout << "found win" << endl;
    s[base_depth+depth_offset]->value = POS_INF;
    return s[base_depth+depth_offset];
  }

  s_vals[base_depth+depth_offset] = cur_score;
  h_vals[base_depth+depth_offset] = p->h_level;
  p_vals[base_depth+depth_offset] = 0;
  if ( rollOutCheckLoop( s, base_depth+depth_offset, false, last_node_index, s_vals, h_vals, p_vals ) != -1 )
  {
    s[base_depth+depth_offset]->value = NEG_INF;
    return s[base_depth+depth_offset];
  }

  if ( rollOutEndCase( s, base_depth+depth_offset, true, last_node_index, p->v, p->v_index, p->h_level ) )
  {
    return s[base_depth+depth_offset];
  }

  while ( best_move_value != NEG_INF && base_depth+depth_offset+1 < MAX_SEARCH_DEPTH )
  {
    best_move_value = NEG_INF;
    best_move_index = -1;

    //cout << "choosing between actions:  " ;
    // --------
    // for each action, get a value based on rollOut to determine the best action
    for ( i=0; i<=s[base_depth+depth_offset]->valid_actions_index; i++ )
    {
      if ( s[base_depth+depth_offset]->valid_actions[i]->available )
      {        
        s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], i );
        
        p->h_level--;
        t_state->copyFrom(  rollOutAnnotated( s, base_depth+depth_offset+1, p, s_vals, h_vals, p_vals, last_node_index,
                 base_prop, base_rela, winning_path, winning_search, t_state, b_state ) );
        p->h_level++;

        //cout << i << "(" << t_state->value << ")  ";
        if ( t_state->value == POS_INF )
        {
          t_state->copyInto( b_state );
          winning_path[base_depth+depth_offset] = i;
          winning_search[base_depth+depth_offset] = p->h_level;
          return b_state;
        }
        if ( t_state->value > best_move_value )
        {
          t_state->copyInto( b_state );
          best_move_value = t_state->value;
          best_move_index = i;
        }
      }
    }
    // --------
    
    if ( best_move_index == -1 )
    {
      last_node_index = base_depth+depth_offset;
      return s[base_depth+depth_offset];
    }


    //cout << " and choosing action " << best_move_index << endl;
    
    // No solution was found
    // now that we know which action is the best one, take that action
    winning_path[base_depth+depth_offset] = best_move_index;
    winning_search[base_depth+depth_offset] = p->h_level;
    
    s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], best_move_index );
    
    depth_offset++;
    
    cur_score = stateValue( s[base_depth+depth_offset], base_prop, base_rela, p->policy, p->policy_index );
    s[base_depth+depth_offset]->value = cur_score;

    s_vals[base_depth+depth_offset] = cur_score;
    h_vals[base_depth+depth_offset] = p->h_level;
    p_vals[base_depth+depth_offset] = 0;
    
    j = rollOutCheckLoop( s, base_depth+depth_offset, true, last_node_index, s_vals, h_vals, p_vals );

    if ( j != -1 )
    {
      s[base_depth+depth_offset]->value = NEG_INF;
      return s[base_depth+depth_offset];
    }
  }
  
  //cout << "returning out the back" << endl;
  return b_state;
}// --------

double randomRollOut( State * s[MAX_SEARCH_DEPTH], int base_depth, 
        RollOutPolicyInfo * hi_p, RollOutPolicyInfo * lo_p,
        double s_vals[MAX_SEARCH_DEPTH], int h_vals[MAX_SEARCH_DEPTH], int p_vals[MAX_SEARCH_DEPTH],
        time_t start, 
        bool check_time,
        int &last_node_index, 
        bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
        bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK], 
        int winning_path[MAX_SEARCH_DEPTH], bool note_path,
        int greedy_path[MAX_SEARCH_DEPTH], bool note_greedy_path,
        int which_p,
        MTRand * my_rand_gen)
{
  int i, j;
  double t_val;
  int greedy_action;
  int depth_offset = 0;
  int backtrackval = 0;
  
  double cur_score = my_rand_gen->randInt();
  
  double best_move_value = cur_score;
  int best_move_index = -1;
  
  if ( rollOutCheckTime( check_time, start ) ) 
    return TOO_LONG;
  
  if ( rollOutCheckWin( s, base_depth+depth_offset, note_path, last_node_index ) )
    return POS_INF;
  
  s_vals[base_depth+depth_offset] = cur_score;
  h_vals[base_depth+depth_offset] = hi_p->h_level;
  p_vals[base_depth+depth_offset] = which_p;
  if ( rollOutCheckLoop( s, base_depth+depth_offset, false, last_node_index, s_vals, h_vals, p_vals ) != -1 )
    return NEG_INF;
  
  if ( rollOutEndCase( s, base_depth+depth_offset, note_path, last_node_index, hi_p->v, hi_p->v_index, hi_p->h_level ) )
  {
    if ( hi_p->h_level == -1 || lo_p == NULL || base_depth+depth_offset <= backtrackval )
    {
      //s[base_depth+depth_offset]->prettyPrint();
      //cout << "-1: value of previous state: " << cur_score << endl;
      //cout << "     " << cur_score << endl;
      return cur_score;
    }
    else
    {
      return randomRollOut( s, base_depth+depth_offset-backtrackval, lo_p, NULL, s_vals, h_vals, p_vals, start, check_time, last_node_index,
              base_prop, base_rela, winning_path, note_path, greedy_path, note_greedy_path, which_p+1, my_rand_gen );
    }
    
    //  if ( hi_p->h_level == -1 || base_depth+depth_offset <= 1 )
    //    return cur_score;
    //  else
    //    return NEG_INF;
  }
  
  while ( best_move_value != NEG_INF && base_depth+depth_offset+1 < MAX_SEARCH_DEPTH )
  {
    if ( lo_p == NULL )
      best_move_value = NEG_INF;
    else
      best_move_value = cur_score;
    
    best_move_index = -1;
    
    // --------
    // for each action, get a value based on rollOut to determine the best action
    for ( i=0; i<=s[base_depth+depth_offset]->valid_actions_index; i++ )
    {
      if ( s[base_depth+depth_offset]->valid_actions[i]->available )
      {        
        s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], i );
        //s[base_depth+depth_offset+1]->prettyPrint();
        
        if ( hi_p->h_level == 1  && PRINT_ON )
        {
          cout << endl << "testing action " << i << endl << endl;
        }
        
        /*if ( hi_p->h_level == 0 && base_depth == 1 && depth_offset == 65 )
        {
          cout << "Here" << endl;
        }*/
        
        hi_p->h_level--;
        t_val = randomRollOut( s, base_depth+depth_offset+1, hi_p, lo_p, s_vals, h_vals, p_vals, start, check_time, last_node_index,
                 base_prop, base_rela, winning_path, note_path, greedy_path, note_greedy_path, which_p, my_rand_gen );
        hi_p->h_level++;
        
        if ( hi_p->h_level == 0 && PRINT_ON )
        {
          cout << "[" << base_depth << " : " << depth_offset << "] action " << i << " value = " << t_val << endl;
        }
        
        if ( t_val == POS_INF )
        {
          if ( note_path )
          {
            winning_path[base_depth+depth_offset] = i;
            greedy_path[base_depth+depth_offset] = greedyActionIndex( s[base_depth+depth_offset], base_prop, base_rela, hi_p->policy, hi_p->policy_index );
          }
          return POS_INF;
        }
        else if ( t_val == TOO_LONG )
        {
          return TOO_LONG;
        }
        if ( t_val > best_move_value )
        {
          best_move_value = t_val;
          best_move_index = i;
        }
      }
    }
    
    if ( note_path ) greedy_action = greedyActionIndex( s[base_depth+depth_offset], base_prop, base_rela, hi_p->policy, hi_p->policy_index );
    // --------
    
    if ( best_move_index == -1 )
    {
      if ( lo_p == NULL )
      {
        // there were no possible actions from this state - failed
        if ( note_path ){ last_node_index = base_depth+depth_offset; }
        return cur_score;
      }
      else
      {
        // there were no discovered moves that improved the heuristic, search with the lower policy
        return randomRollOut( s, base_depth+depth_offset, lo_p, NULL, s_vals, h_vals, p_vals, start, check_time, last_node_index,
                base_prop, base_rela, winning_path, note_path, greedy_path, note_greedy_path, which_p+1, my_rand_gen );
      }
    }

    cur_score = my_rand_gen->randInt();
    
    // No solution was found
    // now that we know which action is the best one, take that action
    if ( note_path )
    {
      winning_path[base_depth+depth_offset] = best_move_index;
      greedy_path[base_depth+depth_offset] = greedy_action;
    }
    
    //s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], best_move_index );
    int num_valid_actions = 0;
    for ( int ii = 0; ii<s[base_depth+depth_offset]->valid_actions_index; ii++ )
    {
      if ( s[base_depth+depth_offset]->valid_actions[ii]->available )
      {
        num_valid_actions++;
      }
    }

    // there are no possible actions from this state
    if ( num_valid_actions == 0 ) return cur_score;

    int random_action_choice = my_rand_gen->randInt() % num_valid_actions;
    for ( int ii=0; ii<s[base_depth+depth_offset]->valid_actions_index; ii++ )
    {
      if ( s[base_depth+depth_offset]->valid_actions[ii]->available && random_action_choice <= 0 )
      {
        s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], ii );
        break;
      }
      else
      {
        random_action_choice--;
      }
    }
    
    if ( PRINT_ON ) cout << "choosing action " << best_move_index << endl;
    if ( hi_p->h_level == 1 )
    {
      //s[base_depth+depth_offset+1]->prettyPrint();
    }
    depth_offset++;
    
    //s[base_depth+depth_offset]->prettyPrint();
    
    s_vals[base_depth+depth_offset] = cur_score;
    h_vals[base_depth+depth_offset] = hi_p->h_level;
    p_vals[base_depth+depth_offset] = which_p;
    
    j = rollOutCheckLoop( s, base_depth+depth_offset, note_path, last_node_index, s_vals, h_vals, p_vals );
    
    if ( j != -1 )
    {
      cout << "repeated choice a state " << base_depth+depth_offset << " after taking action " << best_move_index << endl;
      if ( lo_p == NULL ) cout << "using lower level policy" << endl;
      else cout << "using upper level policy" << endl;
      exit( 0 );
    }
  }
  
  return best_move_value;
}



bool rollOutCheckTime( bool check_time, time_t start )
{
  // cut out if we've already taken too long
  if ( check_time )
  {
    time_t current_time;
    time( &current_time );
    if ( difftime( current_time, start ) > TIME_LIMIT )
    {
      //cout << "current time = " << current_time << "  start time = " << start << endl;
      //cout << "TOOK TOO LONG " << difftime( current_time, start) << ">" << TIME_LIMIT << endl;
      //last_node_index = base_depth+depth_offset;
      return true;
    }
  }
  return false;
}

bool rollOutCheckWin( State * s[MAX_SEARCH_DEPTH], int state_index )
{
  return s[state_index]->win();
}

bool rollOutCheckWin( State * s[MAX_SEARCH_DEPTH], int state_index, bool note_path, int &last_node_index )
{
  if ( s[state_index]->win() )
  {
    if ( false )
    {
      cout << state_index << " actions (1)" << endl;
      printSolution( s, state_index );
    }
    if ( note_path ){ last_node_index = state_index; }
    return true;
  }
  return false;
}

bool rollOutEndCase( State * s[MAX_SEARCH_DEPTH], int state_index, CompactState *v[VISITED_STATES], int &v_index, int h_level )
{
  if ( ( s[state_index]->deadEnd() ) || h_level == -1 || ( alreadyVisited( v, v_index, s[state_index], h_level ) ) )
    return true;
  return false;
}

bool rollOutEndCase( State * s[MAX_SEARCH_DEPTH], int state_index, bool note_path, int &last_node_index, CompactState * v[VISITED_STATES], int & v_index, int h_level )
{
  if ( ( s[state_index]->deadEnd() ) || h_level == -1 || ( alreadyVisited( v, v_index, s[state_index], h_level ) ) )
  {
    if ( note_path )
    { 
      last_node_index = state_index; 
    }
    //if ( h_level == -1 ) cout << "heuristic level = -1" << endl;
    //if ( s[state_index]->deadEnd() ) cout << "Dead End at level " << state_index << endl;
    //if ( alreadyVisited( v, v_index, s[state_index], h_level, check_visited ) ) cout << "Already Visited state " << state_index << " at heuristic level " << h_level << endl;
    //s[state_index]->prettyPrint();
    return true;
  }
  return false;
}

bool rollOutEndCase( State * s[MAX_SEARCH_DEPTH], int state_index, bool note_path, int &last_node_index, CompactState * v[VISITED_STATES], int & v_index, int h_level, 
    int winning_path[MAX_SEARCH_DEPTH], int losing_path[MAX_SEARCH_DEPTH], int &losing_path_index, int current_path[MAX_SEARCH_DEPTH] )
{
  
  if ( ( s[state_index]->deadEnd() ) || h_level == -1 || ( alreadyVisited( v, v_index, s[state_index], h_level ) ) )
  {
    if ( note_path )
    { 
      last_node_index = state_index; 
      if ( state_index > losing_path_index && h_level != -1 )
      {
        losing_path_index = state_index;
        //cout << endl << "*" << endl << "*" << endl << endl << "end case: new longest loss is : " << losing_path_index << endl;
        for ( int i=0; i<=state_index; i++ )
        {
          losing_path[i] = current_path[i];
        }
      }
    }
    //if ( h_level == -1 ) cout << "heuristic level = -1" << endl;
    //if ( s[state_index]->deadEnd() ) cout << "Dead End at level " << state_index << endl;
    //if ( alreadyVisited( v, v_index, s[state_index], h_level, check_visited ) ) cout << "Already Visited state " << state_index << " at heuristic level " << h_level << endl;
    //s[state_index]->prettyPrint();
    return true;
  }
  return false;
}

int rollOutCheckLoop( State * s[MAX_SEARCH_DEPTH], int state_index, double s_vals[MAX_SEARCH_DEPTH], int h_vals[MAX_SEARCH_DEPTH], int p_vals[MAX_SEARCH_DEPTH] )
{
  for ( int i=state_index-1; i>=0; i-- )
  {
    if ( s_vals[state_index] == s_vals[i] && h_vals[state_index] <= h_vals[i] && p_vals[state_index] == p_vals[i] && s[state_index]->isSame( s[i] ) )
      return i;
  }
  return -1;
}

int rollOutCheckLoop( State * s[MAX_SEARCH_DEPTH], int state_index, bool note_path, int &last_node_index, double s_vals[MAX_SEARCH_DEPTH], int h_vals[MAX_SEARCH_DEPTH], int p_vals[MAX_SEARCH_DEPTH] )
{
  // --------
  // check for a repeated state in this action sequence (from here back to the beginning)
  // check for repeated states - if a state is repeated then we will be in an infinite loop
  // first check to see if the scores and the heuristic levels are the same (inexpensive)
  // if they are, then do the more expensive deep check
  // this still must be done even though we checked the Visited list, because the visited
  // list is often filled up, so we need to double check against the states already on our list
  for ( int i=state_index-1; i>=0; i-- )
  {
    if ( s_vals[state_index] == s_vals[i] && h_vals[state_index] <= h_vals[i] && p_vals[state_index] == p_vals[i] && s[state_index]->isSame( s[i] ) )
    {
      if ( note_path )
        last_node_index = state_index;
      return i;
    }
  }
  return -1;
}

int rollOutCheckLoop( State * s[MAX_SEARCH_DEPTH], int state_index, bool note_path, int &last_node_index, double s_vals[MAX_SEARCH_DEPTH], int h_vals[MAX_SEARCH_DEPTH], int p_vals[MAX_SEARCH_DEPTH],
    int winning_path[MAX_SEARCH_DEPTH], int losing_path[MAX_SEARCH_DEPTH], int &losing_path_index, int current_path[MAX_SEARCH_DEPTH] )
{
  // --------
  // check for a repeated state in this action sequence (from here back to the beginning)
  // check for repeated states - if a state is repeated then we will be in an infinite loop
  // first check to see if the scores and the heuristic levels are the same (inexpensive)
  // if they are, then do the more expensive deep check
  // this still must be done even though we checked the Visited list, because the visited
  // list is often filled up, so we need to double check against the states already on our list
  for ( int i=state_index-1; i>=0; i-- )
  {
    if ( s_vals[state_index] == s_vals[i] && h_vals[state_index] <= h_vals[i] && p_vals[state_index] == p_vals[i] && s[state_index]->isSame( s[i] ) )
    {
      if ( note_path )
      {
        if ( state_index > losing_path_index && !s[state_index]->win() )
        {
          losing_path_index = state_index;
        //  cout << endl << "*" << endl << "*" << endl << endl << "check loop: new longest loss is : " << losing_path_index << endl;
          for ( int i=0; i<=state_index; i++ )
          {
            losing_path[i] = current_path[i];
          }
        }
      }
        last_node_index = state_index;
      return i;
    }
  }
  return -1;
}

double rollOut2( State * s[MAX_SEARCH_DEPTH], int base_depth, 
        RollOutPolicyInfo * hi_p, RollOutPolicyInfo * lo_p,
        double s_vals[MAX_SEARCH_DEPTH], int h_vals[MAX_SEARCH_DEPTH], int p_vals[MAX_SEARCH_DEPTH],
        time_t start, 
        bool check_time,
        int &last_node_index, 
        bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
        bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK], 
        int winning_path[MAX_SEARCH_DEPTH], bool note_path,
        int greedy_path[MAX_SEARCH_DEPTH], bool note_greedy_path,
        int which_p )
{
  int i, j;
  double t_val;
  int greedy_action;
  int depth_offset = 0;
  int backtrackval = 0;
  
  double cur_score = stateValue( s[base_depth+depth_offset], base_prop, base_rela, hi_p->policy, hi_p->policy_index );

  double best_move_value = cur_score;
  int best_move_index = -1;

  if ( rollOutCheckTime( check_time, start ) ) 
    return TOO_LONG;

  if ( rollOutCheckWin( s, base_depth+depth_offset, note_path, last_node_index ) )
    return POS_INF;

  s_vals[base_depth+depth_offset] = cur_score;
  h_vals[base_depth+depth_offset] = hi_p->h_level;
  p_vals[base_depth+depth_offset] = which_p;
  if ( rollOutCheckLoop( s, base_depth+depth_offset, false, last_node_index, s_vals, h_vals, p_vals ) != -1 )
    return NEG_INF;
  
  if ( rollOutEndCase( s, base_depth+depth_offset, note_path, last_node_index, hi_p->v, hi_p->v_index, hi_p->h_level ) )
  {
    if ( hi_p->h_level == -1 || lo_p == NULL || base_depth+depth_offset <= backtrackval )
    {
      //s[base_depth+depth_offset]->prettyPrint();
      //cout << "-1: value of previous state: " << cur_score << endl;
      //cout << "     " << cur_score << endl;
      return cur_score;
    }
    else
    {
      return rollOut2( s, base_depth+depth_offset-backtrackval, lo_p, NULL, s_vals, h_vals, p_vals, start, check_time, last_node_index,
              base_prop, base_rela, winning_path, note_path, greedy_path, note_greedy_path, which_p+1 );
    }
    
  //  if ( hi_p->h_level == -1 || base_depth+depth_offset <= 1 )
  //    return cur_score;
  //  else
  //    return NEG_INF;
  }

  while ( best_move_value != NEG_INF && base_depth+depth_offset+1 < MAX_SEARCH_DEPTH )
  {
    if ( lo_p == NULL )
      best_move_value = NEG_INF;
    else
      best_move_value = cur_score;
    
    best_move_index = -1;
    
    // --------
    // for each action, get a value based on rollOut to determine the best action
    for ( i=0; i<=s[base_depth+depth_offset]->valid_actions_index; i++ )
    {
      if ( s[base_depth+depth_offset]->valid_actions[i]->available )
      {        
        s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], i );
        //s[base_depth+depth_offset+1]->prettyPrint();
        
        if ( hi_p->h_level == 1  && PRINT_ON )
        {
          cout << endl << "testing action " << i << endl << endl;
        }
        
        /*if ( hi_p->h_level == 0 && base_depth == 1 && depth_offset == 65 )
        {
          cout << "Here" << endl;
        }*/
        
        hi_p->h_level--;
        t_val = rollOut2( s, base_depth+depth_offset+1, hi_p, lo_p, s_vals, h_vals, p_vals, start, check_time, last_node_index,
                 base_prop, base_rela, winning_path, note_path, greedy_path, note_greedy_path, which_p );
        hi_p->h_level++;
        
        if ( hi_p->h_level == 0 && PRINT_ON )
        {
          cout << "[" << base_depth << " : " << depth_offset << "] action " << i << " value = " << t_val << endl;
        }
        
        if ( t_val == POS_INF )
        {
          if ( note_path )
          {
            winning_path[base_depth+depth_offset] = i;
            greedy_path[base_depth+depth_offset] = greedyActionIndex( s[base_depth+depth_offset], base_prop, base_rela, hi_p->policy, hi_p->policy_index );
          }
          return POS_INF;
        }
        else if ( t_val == TOO_LONG )
        {
          return TOO_LONG;
        }
        if ( t_val > best_move_value )
        {
          best_move_value = t_val;
          best_move_index = i;
        }
      }
    }

    if ( note_path ) greedy_action = greedyActionIndex( s[base_depth+depth_offset], base_prop, base_rela, hi_p->policy, hi_p->policy_index );
    // --------
    
    if ( best_move_index == -1 )
    {
      if ( lo_p == NULL )
      {
        // there were no possible actions from this state - failed
        if ( note_path ){ last_node_index = base_depth+depth_offset; }
        return cur_score;
      }
      else
      {
        // there were no discovered moves that improved the heuristic, search with the lower policy
        return rollOut2( s, base_depth+depth_offset, lo_p, NULL, s_vals, h_vals, p_vals, start, check_time, last_node_index,
                base_prop, base_rela, winning_path, note_path, greedy_path, note_greedy_path, which_p+1 );
      }
    }
    
    // No solution was found
    // now that we know which action is the best one, take that action
    if ( note_path )
    {
      winning_path[base_depth+depth_offset] = best_move_index;
      greedy_path[base_depth+depth_offset] = greedy_action;
    }
    
    s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], best_move_index );
    
    if ( PRINT_ON ) cout << "choosing action " << best_move_index << endl;
    if ( hi_p->h_level == 1 )
    {
      //s[base_depth+depth_offset+1]->prettyPrint();
    }
    depth_offset++;
    
    //s[base_depth+depth_offset]->prettyPrint();
    
    cur_score = stateValue( s[base_depth+depth_offset], base_prop, base_rela, hi_p->policy, hi_p->policy_index );
    
    s_vals[base_depth+depth_offset] = cur_score;
    h_vals[base_depth+depth_offset] = hi_p->h_level;
    p_vals[base_depth+depth_offset] = which_p;
    
    j = rollOutCheckLoop( s, base_depth+depth_offset, note_path, last_node_index, s_vals, h_vals, p_vals );

    if ( j != -1 )
    {
      cout << "repeated choice at state " << base_depth+depth_offset << " after taking action " << best_move_index << endl;
      if ( lo_p == NULL ) cout << "using lower level policy" << endl;
      else cout << "using upper level policy" << endl;
      exit( 0 );
    }
  }
  
  if ( base_depth+depth_offset+1 >= MAX_SEARCH_DEPTH )
  {
    cout << " MAX_SEARCH_DEPTH exceeded" << endl;
    /*
    ofstream out_file( "out.txt", ios::out );
    if ( !out_file)
    {
      cout << "out.txt could not be opened" << endl;
      exit ( 0 );
    }
    s[0]->printToFile( out_file );
    */
    s[MAX_SEARCH_DEPTH-1]->prettyPrint();

    for ( i=1; i<MAX_SEARCH_DEPTH; i++ )
    {
      j = rollOutCheckLoop( s, i, false, last_node_index, s_vals, h_vals, p_vals );
      if ( j != -1 )
      {
        cout << "repeated states " << i << " and " << j << endl;
        s[i]->prettyPrint();
        s[j]->prettyPrint();
      }
    }
    //cout << "ran out of space" << endl;
    return NEG_INF;
  }
  
  return best_move_value;
}


#endif
