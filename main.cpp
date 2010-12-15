
#include "main.h"

#define PRINT_ON    false

using namespace std;

int main (int argc, char * const argv[]) {
  
  int my_random;
  // get random seed based on timestamp and process id
  my_random = ( time ( NULL ) ) + getpid();
  //my_random = 1254278266;


  int i, j, k;
  
  int test_set_size = TEST_SET_SIZE;            //  these may be reset
  double learning_rate = LEARNING_RATE;            //  by the command line
  
  int clp_index = 0;              // helps parse command line parameters

  int first_game = 0;
  
  //bool found_good_training_state;
  bool win;
  
  State * search_states[MAX_SEARCH_DEPTH];
  State * t_search_states[MAX_SEARCH_DEPTH];
  State * choice_point[MAX_SEARCH_DEPTH];
  State * test_set[TEST_SET_SIZE];
  State * training_set[TRAINING_SET_SIZE];
  
  RollOutPolicyInfo * lo_p = new RollOutPolicyInfo();
  RollOutPolicyInfo * hi_p = new RollOutPolicyInfo();
  RollOutPolicyInfo * random_p = new RollOutPolicyInfo();
  RollOutPolicyInfo * empty_p = new RollOutPolicyInfo();

  RollOutPolicyInfo * p_array[MAX_POLICIES];
  
  int winning_path_length[TRAINING_SET_SIZE];
  int losing_path_length[TRAINING_SET_SIZE];
  int * winning_paths[TRAINING_SET_SIZE];
  int * greedy_paths[TRAINING_SET_SIZE];
  int * losing_paths[TRAINING_SET_SIZE];
  int * winning_search[TRAINING_SET_SIZE];

  double state_values[MAX_SEARCH_DEPTH];    // the history of state values
  int h_values[MAX_SEARCH_DEPTH];        // the history of search depths
  int p_types[MAX_SEARCH_DEPTH];        // the history of used policies
  int cur_path[MAX_SEARCH_DEPTH];
  
  bool prop[NUM_BASE_PROPOSITIONS][FULL_DECK];
  bool rel[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK];
  bool const_rel[NUM_CONST_RELATIONS][FULL_DECK][FULL_DECK];

  State * survey_states[SURVEY_SIZE];

  UCTNode * uctRoot;
  
  int loss_count;

  int choice_level;
  
  int test_games_won;
  int test_dead_ends;
  int tested_games;
  
  time_t c_start;
  time_t final_start, final_end;

  struct timeval tvstart, tvstop;
  
  int paths_different;
  
  int empty_policy_wins = 0;
  int random_policy_wins = 0;
  int learned_policy_wins = 0;
  int hc_policy_wins = 0;
  int hc2_policy_wins = 0;

  double game_result;
  double total_secs = 0;
  double l0_game_secs;
  double l1_game_secs;
  double l2_game_secs;

  int upper_level = LEVEL_ZERO;
  int lower_level = LEVEL_ZERO;
  int level_frontier = -1; // by default, use the lower level policy all the time

//  gettimeofday( &tvstart, NULL );

  fillConstRelations( const_rel );
  
  initHandCodedHCPolicy( lo_p->policy );
  lo_p->policy_index = NUM_HCP_FEATURES-1;
  
  initHandCodedHCPolicy2( hi_p->policy );
  hi_p->policy_index = NUM_HCP_FEATURES-1;
  
  initEmptyHCPolicy( empty_p->policy );
  empty_p->policy_index = NUM_HCP_FEATURES-1;
  
  double choice_point_values[MAX_SEARCH_DEPTH];
  
  while ( argc - clp_index > 1 )
  {
    if ( strcmp( argv[clp_index+1], "-rs" ) == 0 && ( argc - clp_index > 2 ) )
    {
      my_random = atoi( argv[clp_index+2] );
    }
    else if ( strcmp( argv[clp_index+1], "-lr" ) == 0 && ( argc - clp_index > 2 ) )
    {
      learning_rate = atof( argv[clp_index+2] );
    }
    else if ( strcmp( argv[clp_index+1], "-fg" ) == 0 && ( argc - clp_index > 2 ) )
    {
      first_game = atoi( argv[clp_index+2] );
    }
    else if ( strcmp( argv[clp_index+1], "-ul" ) == 0 && ( argc - clp_index > 2 ) )
    {
      upper_level = atoi( argv[clp_index+2] );
    }
    else if ( strcmp( argv[clp_index+1], "-ll" ) == 0 && ( argc - clp_index > 2 ) )
    {
      lower_level = atoi( argv[clp_index+2] );
    }
    else if ( strcmp( argv[clp_index+1], "-fr" ) == 0 && ( argc - clp_index > 2 ) )
    {
      level_frontier = atoi( argv[clp_index+2] );
    }

    else
    {
      cout << "Usage:  [executable] -rs [random_seed]  -lr [learning_rate] " << endl;
      exit( 0 );
    }
    clp_index += 2;
  }
  
  if ( test_set_size > TEST_SET_SIZE )
  {
    cout << "test set must be no larger than " << TEST_SET_SIZE << endl;
    exit( 1 );
  }
  
  cout << "random seed = " << my_random << endl;
  /*cout << "first game = " << first_game << endl;
  cout << "learning rate = " << learning_rate << endl;
  cout << "test set size = " << test_set_size << endl;
  cout << "training set size = " << TRAINING_SET_SIZE << endl;
  cout << "max search depth = " << MAX_SEARCH_DEPTH << endl;
  cout << "upper level = " << upper_level << endl;
  cout << "lower level = " << lower_level << endl;
  cout << "level fronteir = " << level_frontier << " states " << endl;
  if ( CHECK_TIME ) cout << "time limit = " << TIME_LIMIT << " seconds" << endl;
  else cout << "no time limit" << endl;
  cout << "repeated state memory size = " << VISITED_STATES << " states" << endl;
  if ( ROLLOUT_RESORT_GREEDY ) cout << "rollout resorting to greedy if win not found" << endl;
  else cout << "rollout choosing highest valued path from search if win not found" << endl;

  cout << "Positive Margin = " << POS_MARGIN << "     Negative Margin = " << NEG_MARGIN << endl;
  cout << "Learning Rate = " << LEARNING_RATE << endl;
  cout << "Epsilon = " <<  EPSILON << endl;
  cout << "Lambda = " << LAMBDA << endl;
  cout << "Gamma = " << GAMMA << endl;
  */
  
  cout << endl << endl << "9 Apr 2009" << endl;
  cout << "Sparse UCT " << endl;
  cout << "max uct rollouts = " << MAX_UCT_ROLLOUTS << "(# of trajectories for each UCT tree) " << endl;
  cout << "max uct tree size = " << MAX_UCT_NODES << endl;
  cout << "H0pt trials per action = " << HOPT_UCT_TRIALS << "(# of UCT trees per action decision)" << endl;
  cout << "Sparse Branching Factor = " << SPARSE_BRANCHING_FACTOR << "(# of branches per action)" << endl;
  //cout << "sample size = " << SURVEY_SIZE << " per action" << endl;
  
  //srand( my_random );
  //init_genrand( (unsigned long)my_random );
  MTRand * my_rand_gen = new MTRand( my_random );
  
  for ( i=0; i<TRAINING_SET_SIZE; i++ ) {
    training_set[i] = new State();
    winning_paths[i] = new int[MAX_SEARCH_DEPTH];
    greedy_paths[i] = new int[MAX_SEARCH_DEPTH];
    losing_paths[i] = new int[MAX_SEARCH_DEPTH];
    winning_search[i] = new int[MAX_SEARCH_DEPTH];
  }
  for ( i=0; i<TEST_SET_SIZE; i++ ) {
    test_set[i] = new State();
  }
  for ( i=0; i<MAX_SEARCH_DEPTH; i++ ) {
    search_states[i] = new State();
    t_search_states[i] = new State();
  }

  for ( i=0; i<MAX_POLICIES; i++ ) {
    p_array[i] = new RollOutPolicyInfo();
  }

  for ( i=0; i<SURVEY_SIZE; i++) {
    survey_states[i] = new State();
  }

  uctRoot = new UCTNode();

  State * last_state = new State();
  State * temp_state1 = new State();
  State * temp_state2 = new State();

  /*initHandCodedHCPolicy2( p_array[0]->policy );
  p_array[0]->policy_index = NUM_HCP_FEATURES-1;
  p_array[0]->id = 0;
  p_array[0]->search_depth = DEPTH_LIMIT;

  initHandCodedHCPolicy( p_array[1]->policy );
  p_array[1]->policy_index = NUM_HCP_FEATURES-1;
  p_array[1]->id = 1;
  p_array[1]->search_depth = NO_LIMIT;*/

  /*initHandCodedHCPolicy2( p_array[0]->policy );
  p_array[0]->policy_index = NUM_HCP_FEATURES-1;
  p_array[0]->id = 0;
  p_array[0]->search_depth = 5;

  initEmptyHCPolicy( p_array[1]->policy );
  p_array[1]->policy_index = NUM_HCP_FEATURES -1;
  p_array[1]->id = 1;
  p_array[1]->search_depth = 30;
  
  initHandCodedHCPolicy( p_array[2]->policy );
  p_array[2]->policy_index = NUM_HCP_FEATURES-1;
  p_array[2]->id = 2;
  p_array[2]->search_depth = NO_LIMIT;*/

  /*initEmptyHCPolicy( p_array[0]->policy );
  p_array[0]->policy_index = NUM_HCP_FEATURES-1;
  p_array[0]->id = 0;
  p_array[0]->search_depth = NO_LIMIT;
  
  initHandCodedHCPolicy2( p_array[1]->policy );
  p_array[1]->policy_index = NUM_HCP_FEATURES-1;
  p_array[1]->id = 1;
  p_array[1]->search_depth = 5;

  initHandCodedHCPolicy( p_array[2]->policy );
  p_array[2]->policy_index = NUM_HCP_FEATURES-1;
  p_array[2]->id = 2;
  p_array[2]->search_depth = NO_LIMIT;*/

  initHandCodedHCPolicy( p_array[0]->policy );
  p_array[0]->policy_index = NUM_HCP_FEATURES-1;
  p_array[0]->id = 0;
  p_array[0]->search_depth = NO_LIMIT;
  
  initEmptyHCPolicy( p_array[1]->policy );
  p_array[1]->policy_index = NUM_HCP_FEATURES-1;
  p_array[1]->id = 1;
  p_array[1]->search_depth = NO_LIMIT;

  initEmptyHCPolicy( p_array[2]->policy );
  p_array[2]->policy_index = NUM_HCP_FEATURES-1;
  p_array[2]->id = 2;
  p_array[2]->search_depth = NO_LIMIT;

  // initialize test set
  for ( i=0; i<test_set_size; i++) 
  {
    test_set[i]->shuffleAndDeal( my_rand_gen );
  }

  // initialize training set
  //for ( i=0; i<TRAINING_SET_SIZE; i++ ){
    //training_set[i]->shuffleAndDeal( my_rand_gen );
  //}
  
  bool hcp[NUM_HCP_FEATURES];
  
  test_games_won = 0;
  int test_games_lost = 0;
  int total_training_states = 0;
  int total_training_games = 0;
  double initial_value;
  double initial_wins = 0;
  double initial_losses = 0;
  double base_result = 0;
  int base_losses = 0;
  int base_wins = 0;
  int best_policy_score = 0;
  
  int take_over_depth, t_depth, valid_examples;
  double step_penalty;


  bool loss_turn = true;

  int L0_games_won = 0;
  double L0_total_secs = 0;
  int L1_games_won = 0;
  double L1_total_secs = 0;
  int L2_games_won = 0;
  double L2_total_secs = 0;

  /*search_states[0]->shuffleAndDeal( my_rand_gen );
  search_states[0]->prettyPrint();

  search_states[0]->shuffleFaceDownCards( my_rand_gen );
  search_states[0]->prettyPrint();
  search_states[0]->transition( search_states[1], 0 );
  search_states[1]->prettyPrint();

  search_states[1]->shuffleFaceDownCards( my_rand_gen );
  search_states[1]->prettyPrint();
  search_states[1]->transition( search_states[0], 0 );
  search_states[0]->prettyPrint();

  search_states[0]->shuffleFaceDownCards( my_rand_gen );
  search_states[0]->prettyPrint();
  search_states[0]->transition( search_states[1], 2 );
  search_states[1]->prettyPrint();

  search_states[1]->shuffleFaceDownCards( my_rand_gen );
  search_states[1]->prettyPrint();
  search_states[1]->transition( search_states[0], 2 );
  search_states[0]->prettyPrint();

  search_states[0]->shuffleFaceDownCards( my_rand_gen );
  search_states[0]->prettyPrint();
  search_states[0]->transition( search_states[1], 2 );
  search_states[1]->prettyPrint();

  search_states[1]->shuffleFaceDownCards( my_rand_gen );
  search_states[1]->prettyPrint();
  search_states[1]->transition( search_states[0], 0 );
  search_states[0]->prettyPrint();

  search_states[0]->shuffleFaceDownCards( my_rand_gen );
  search_states[0]->prettyPrint();
  search_states[0]->transition( search_states[1], 2 );
  search_states[1]->prettyPrint();

  search_states[1]->shuffleFaceDownCards( my_rand_gen );
  search_states[1]->prettyPrint();
  search_states[1]->transition( search_states[0], 0 );
  search_states[0]->prettyPrint();

  search_states[0]->shuffleFaceDownCards( my_rand_gen );
  search_states[0]->prettyPrint();
  search_states[0]->transition( search_states[1], 2 );
  search_states[1]->prettyPrint();

  search_states[1]->shuffleFaceDownCards( my_rand_gen );
  search_states[1]->prettyPrint();
  search_states[1]->transition( search_states[0], 2 );
  search_states[0]->prettyPrint();

  goto clean_up;
  */
  int l0_wins = 0;
  double l0_total_time = 0;

  int l1_wins = 0;
  double l1_total_time = 0;

  int l2_wins = 0;
  double l2_total_time = 0;

  int l11_wins = 0;
  double l11_total_time = 0;


  /*
   *
   */
  int actions_chosen[MAX_SEARCH_DEPTH];
  int depth;
  int cur_index = 0;
  bool repeat = false;
  bool fail = false;
  bool valid_action = true;
  int simple_wins, simple_dead_ends, simple_repeats, other;
  simple_wins = simple_dead_ends = simple_repeats = other = 0;

  l0_total_time =0;


  //int randomActionIndex( State * s[MAX_SEARCH_DEPTH], int search_depth, MTRand * my_rand_gen )

  /*int next_action;
  simple_wins = 0;
  for ( i=0; i<1000000; i++ ){

    depth = 0;
    search_states[0]->shuffleAndDeal( my_rand_gen );
    next_action = 0;
    win = false;

    while ( depth < MAX_SEARCH_DEPTH-1 ){

      if ( search_states[depth]->win() ) {
        win = true;
        break;
      } 
      if ( search_states[depth]->numFaceDownCards() == 0 ){
        if ( greedyFinish ( search_states, depth ) ) {
          win = true;
          break;
        }
      }

      //next_action = randomActionIndex( search_states, depth, my_rand_gen );
      next_action = firstActionIndex( search_states, depth );
      if ( next_action == -1 ) break;
      search_states[depth]->transition( search_states[depth+1], next_action );
      depth++;
    }

    if ( (i+1) % 10000 == 0 ) cout << " current tally: " << simple_wins << " out of " << i+1 << " games " << endl;
    if ( win ){
      //cout << "Win with random in 0 seconds" << endl;
      simple_wins++;
    }
    //else cout << "Loss with random in 0 seconds" << endl;
  }
  cout << " Final tally: " << simple_wins << " out of " << i << " games " << endl;

goto clean_up;*/

  
  
//**************
//Begin Learning Section
  
  
  double uct_features[NUM_UCT_FEATURES];
  double uct_weights[NUM_UCT_WEIGHTS];
  int greedy_action;
  int training_set_size = 1000000;
  test_set_size = 100000;
  int decay_divisor = 1;
  learning_rate = LEARNING_RATE;
  int feature_offset = 0;
  int feature_index = 0;

  cout << "GAMMA = " << GAMMA << ",  LAMBDA = " << LAMBDA << ",  EPSILON = " << EPSILON << ",  LEARNING_RATE = " << LEARNING_RATE << endl;
  cout << "POS_REWARD = " << POS_REWARD << ",  NEG_REWARD = " << NEG_REWARD << endl;

  random_uct_feat_weights( uct_weights, my_rand_gen );
  for ( i=0; i<training_set_size; i++ ){
    if ( i % 50000 == 0 ) cout << "training " << i << endl;
    search_states[0]->shuffleAndDeal( my_rand_gen );

    //train_epsilon_greedy( search_states, uct_features, uct_weights, my_rand_gen );
    //cout << " prior " << endl;
    train_td_lambda( search_states, uct_features, uct_weights, my_rand_gen, learning_rate );
    //cout << "after training" << endl;

    // each hundredth of our way through the training set, decrease the learning rate
    if ( i % (training_set_size/100) == 99 ) learning_rate -= (learning_rate/100);
  }
  cout << "done training entirely" << endl;
  

  // since all of the suits are isomorphic (and more importantly because our features are linear and not relational),
  //  we should be able to increase accuracy by averaging the learned values for each card value over the 4 suits
  //  don't average over the final seven values - they are not over cards and suits
  double average_value;
  for ( i=0; i<NUM_UCT_FEATURE_TYPES; i++ ){
    for ( j=0; j<SUIT_SIZE; j++ ){
      average_value = uct_weights[(i*FULL_DECK)+(0*SUIT_SIZE)+j] + uct_weights[(i*FULL_DECK)+(1*SUIT_SIZE)+j]+
          uct_weights[(i*FULL_DECK)+(2*SUIT_SIZE)+j] + uct_weights[(i*FULL_DECK)+(3*SUIT_SIZE)+j];
      uct_weights[(i*FULL_DECK)+(0*SUIT_SIZE)+j] = average_value;
      uct_weights[(i*FULL_DECK)+(1*SUIT_SIZE)+j] = average_value;
      uct_weights[(i*FULL_DECK)+(2*SUIT_SIZE)+j] = average_value;
      uct_weights[(i*FULL_DECK)+(3*SUIT_SIZE)+j] = average_value;
    }
  }
  
  //default_uct_feat_weights( uct_weights );
  
  
  cout << " FINAL WEIGHT VALUES: " << endl;
  for ( i=0; i<NUM_UCT_FEATURE_TYPES; i++ ){
    for ( j=0; j<SUIT_BREAK; j++ ){
      cout << uct_weights[feature_index++] << ", ";
    }
    cout << endl;
  }
  for ( i=0; i<7; i++ ){
    cout << uct_weights[feature_index++] << ", ";
  }
  
  //td17_learned_weights( uct_weights );
  l0_wins = 0;
  l1_wins = 0;
  l2_wins = 0;
  for ( i=0; i<test_set_size; i++ ){
    search_states[0]->shuffleAndDeal( my_rand_gen );
    //search_states[0]->copyInto( t_state );

    if ( follow_greedy_policy( search_states, uct_weights ) ){
      l0_wins++;
    }
    if ( follow_random_policy( search_states, my_rand_gen ) ){
      l1_wins++;
    }
    if ( follow_first_action_policy( search_states ) ){
      l2_wins++;
    }

    if ( (i+1)%1000 == 0 ) cout << l0_wins << " (learned policy) and " << l1_wins << " (random policy) and " << l2_wins << " (priority) wins of " << i+1 << " games " << endl;
  }

goto clean_up;
// END Learning Section
//****************************************


//****************************************
//BEGIN General Testing Section 
/*
  double uct_weights[NUM_UCT_WEIGHTS];
  //td17_learned_weights( uct_weights );

  for ( i=0; i<1000; i++) 
  {
    test_set[i]->copyInto( search_states[0] );
    //cout << " Num valid actions = " << search_states[0]->valid_actions_index+1 << endl;
    gettimeofday( &tvstart, NULL );
    win = hOptUctPolicy( search_states, uctRoot, my_rand_gen, temp_state1, temp_state2, AVG_REW_UCT_UPDATE, RANDOM_POLICY_DEFAULT, uct_weights ); 
    gettimeofday( &tvstop, NULL );

    l0_game_secs = (double)( ( ( tvstop.tv_sec - tvstart.tv_sec ) * 1000000 ) + ( tvstop.tv_usec - tvstart.tv_usec ) ) / 1000000;
    l0_total_time += l0_game_secs;


    if ( win ){
      cout << "Win with RANDOM in " << l0_game_secs << " seconds  " << endl;
      l0_wins++;
    }
    else cout << "Loss with RANDOM in " << l0_game_secs << " seconds  " << endl;
  }  // if you want to include the next section put this brace just above the next goto
*/


    /*
    test_set[i]->copyInto( search_states[0] );
    gettimeofday( &tvstart, NULL );
    win = hOptUctPolicy( search_states, uctRoot, my_rand_gen, temp_state1, temp_state2, AVG_REW_UCT_UPDATE, PRIORITY_POLICY_DEFAULT, uct_weights ); 
    gettimeofday( &tvstop, NULL );

    l1_game_secs = (double)( ( ( tvstop.tv_sec - tvstart.tv_sec ) * 1000000 ) + ( tvstop.tv_usec - tvstart.tv_usec ) ) / 1000000;
    l1_total_time += l1_game_secs;

    if ( win ){
      cout << "Win with PRIORITY in " << l1_game_secs << " seconds" << endl;
      l1_wins++;
    }  
    else cout << "Loss with PRIORITY in " << l1_game_secs << " seconds" << endl;
  

    test_set[i]->copyInto( search_states[0] );
    gettimeofday( &tvstart, NULL );
    win = hOptUctPolicy( search_states, uctRoot, my_rand_gen, temp_state1, temp_state2, AVG_REW_UCT_UPDATE, LEARNED_POLICY_DEFAULT, uct_weights ); 
    gettimeofday( &tvstop, NULL );

    l2_game_secs = (double)( ( ( tvstop.tv_sec - tvstart.tv_sec ) * 1000000 ) + ( tvstop.tv_usec - tvstart.tv_usec ) ) / 1000000;
    l2_total_time += l2_game_secs;

    if ( win ){
      cout << "Win with LEARNED in " << l2_game_secs << " seconds" << endl;
      l2_wins++;
    }
    else cout << "Loss with LEARNED in " << l2_game_secs << " seconds" << endl;
    cout << endl;
    */

//goto clean_up;
// END General Testing Section
//******************************************

clean_up:
      
  // --------
  // clean up our messes before we leave
  for ( i=0; i<MAX_SEARCH_DEPTH; i++ )
  {
    delete search_states[i];
    delete t_search_states[i];
  }
  for ( i=0; i<TEST_SET_SIZE; i++ )
  {
    delete test_set[i];
  }
  for ( i=0; i<TRAINING_SET_SIZE; i++ )
  {
    delete training_set[i];
    delete winning_paths[i];
    delete greedy_paths[i];
    delete losing_paths[i];
    delete winning_search[i];
  }

  delete uctRoot;

  delete last_state;
  delete temp_state1;
  delete temp_state2;
  
  delete hi_p;
  delete lo_p;
  delete random_p;
  delete empty_p;
  
  delete my_rand_gen;
  
  return 0;
}

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

/*
 *
 */
void printSolution( State * s[MAX_SEARCH_DEPTH], int base_depth )
{
  int i;
  int card_to_move;
  int new_stack;
  
  s[0]->prettyPrint();
  for ( i=0; i<base_depth; i++ )
  {
    s[i]->getThereFromHere( s[i+1], card_to_move, new_stack );
    cout << i+1 << ": ";
    s[i]->printIndexToChar( card_to_move );
    cout << "->";
    s[i]->printStackIndexToChar( new_stack );
    cout << " (with "<< s[i]->num_stock_turns << " stock turns)";
    cout << endl;
  }
  cout << endl;
}

void storeSolution( State * s[MAX_SEARCH_DEPTH], int base_depth, int winning_path[TRAINING_SET_SIZE][MAX_SEARCH_DEPTH], int training_game )
{
  int i;
  int a_index;
  
  for ( i=0; i<base_depth; i++ )
  {
    s[i]->getThereFromHere( s[i+1], a_index );
    winning_path[training_game][i] = a_index;
  }
}


bool alreadyVisited( CompactState * visited[VISITED_STATES], int &visited_index, State * test_state, int roll_out_level )
{
  int i;

  for ( i=visited_index; i>=0; i-- )
  {
    if ( test_state->my_cs->isSame( visited[i], roll_out_level ) )
    {
      //cout << endl << "REPEAT at " << i << endl << endl;
      //cout << "repeat" << endl;
      return true;
    }
  }
  if ( visited_index < VISITED_STATES - 1 )
  {
    //cout << "storing # " << visited_index+1 << endl;
    test_state->my_cs->copyInto( visited[++visited_index], roll_out_level );
  }
  
  return false;
}

int fillTrainingStatesFromFile( string in_file, State ** in_states )
{
  int i;
  int num_examples;
  long file_loc = 0;
  char * eat_line = new char[512];        // there's no way a line should be this long
  
  ifstream input;
  input.open( in_file.c_str(), ifstream::in );
  
  //make sure we got the file opened up ok...
  if( !input.is_open() )
  {
    cout << "could not open file " << in_file << endl;
    exit( 0 );
  }
  
  input.getline( eat_line, 256 );          // read the number of examples from the first line of the file
  num_examples = atoi( eat_line );
  
  file_loc = input.tellg();
  input.close();
  
  for ( i=0; i<num_examples; i++ )
  {
    cout << i << endl;
    file_loc = in_states[i]->parseGameFile( in_file, file_loc );
  }
  
  return num_examples;
}


void fillConstRelations( bool c[NUM_CONST_RELATIONS][FULL_DECK][FULL_DECK] )
{
  int i, j, k;
  
  for ( i=0; i<NUM_CONST_RELATIONS; i++ )
  {
    for ( j=0; j<FULL_DECK; j++ )
    {
      for ( k=0; k<FULL_DECK; k++ )
      {
        c[i][j][k] = false;
      }
    }
  }
  
  i = F_SAME_SUIT;
  for ( j=0; j<FULL_DECK; j++ )
  {
    for ( k=0; k<FULL_DECK; k++ )
    {
      if ( j != k )   // disallow relations defined over the same card
      {
        if ( F_SAME_SUIT < NUM_CONST_RELATIONS && ( j / 13 ) == ( k / 13 ) )  // integer division by 13 yields the suit of a card
        {
          c[F_SAME_SUIT][j][k] = true;
          //cout << "same suit: " <<  j << " and " << k << endl;
        }
        
        if ( F_SAME_COLOR < NUM_CONST_RELATIONS &&  ( j % 26 < 13 ) == ( k % 26 < 13 ) )
        {
          c[F_SAME_COLOR][j][k] = true;
          //cout << "same color: " << j << " and " << k << endl;
        }
        
        if ( F_LOWER_RANK < NUM_CONST_RELATIONS &&  ( j % 13 ) < ( k % 13 ) )
        {
          c[F_LOWER_RANK][j][k] = true;
          //cout << "lower rank: " << j << " lower than " << k << endl;
        }
        if ( F_SAME_RANK < NUM_CONST_RELATIONS && ( j % 13 ) == ( k % 13 ) )
        {
          c[F_SAME_RANK][j][k] = true;
          // cout << "same rank: " << j << " and " << k << endl;
        }
      }
    }
    if (  F_TABLEAU_BUILD_ON < NUM_CONST_RELATIONS )
    {
      if ( j % 26 < 13 )          // the card is red
      {
        c[F_TABLEAU_BUILD_ON][j][ ( j%13 ) + 14] = true;
        c[F_TABLEAU_BUILD_ON][j][ ( j%13 ) + 40] = true;
      }
      else                  // the card is black
      {
        c[F_TABLEAU_BUILD_ON][j][ ( j%26 ) - 12] = true;
        c[F_TABLEAU_BUILD_ON][j][ ( j%26 ) + 14] = true;
      }
    }
    
    if (  F_FOUNDATION_BUILD_ON < NUM_CONST_RELATIONS && ( j % 13 ) != 0 )
    {
      c[F_FOUNDATION_BUILD_ON][j][j-1] = true;
    }
  }
}

double stateValue( State * s, 
           bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
           bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK], 
           HCFeature * hc_feats[MAX_FEATURES], int hc_feat_index )
{
  int i;
  double return_value = 0;
  bool hcp[NUM_HCP_FEATURES];
  
  //s->prettyPrint();
  s->fillFeaturePropositions( base_prop );
  s->fillFeatureRelations( base_rela );
  //cout << "filling hand coded features" << endl;
  s->fillHCPFeatures( base_prop, base_rela, hcp );
  
  //cout << "adding weights" << endl;
  for ( i=0; i<=hc_feat_index; i++ )
  {
    if ( hcFeatureOn( hc_feats[i], hcp ) )
    {
      return_value += hc_feats[i]->weight;
    }
  }
  return return_value;
}

bool featureOn( Feature * f, 
        bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
        bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK] )
{
  int j;
  
  //int type, card1, card2;
  //type = card1 = card2 = -1;
  
  for ( j=0; j<=f->bp_conj_index; j++ )
  {
    //type = f->bp_conj[j][0];
    //card1 = f->bp_conj[j][1];
    if ( ! base_prop[ f->bp_conj[j][0] ][ f->bp_conj[j][1] ] )
    {
      return false;
    }
  }
  for ( j=0; j<=f->br_conj_index; j++ )
  {
    //type = f->br_conj[j][0];
    //card1 = f->br_conj[j][1];
    //card2 = f->br_conj[j][2];
    if ( ! base_rela[ f->br_conj[j][0] ][ f->br_conj[j][1] ][ f->br_conj[j][2] ] )
    {
      return false;
    }
  }
  return true;
}

bool hcFeatureOn( HCFeature * f, bool hcp[NUM_HCP_FEATURES] )
{
  int i;
  
  // unless you are doing feature generation
  // base_features_index will be 0 (a base
  // feature corresponds to an index into
  // the 595 hand-coded features)
  for ( i=0; i<=f->base_features_index; i++ )
  {
    // also, unless you are doing feature generation
    // hcp[f->base_feathures[i] will always be hcp[i]
    if ( ! hcp[ f->base_features[i] ] )
    {
      return false;
    }
  }
  return true;
}

bool isLegalConjunction( Feature * f1, Feature * f2, bool const_rel[NUM_CONST_RELATIONS][FULL_DECK][FULL_DECK] )
{
  if ( f1->shareVariable( f2 ) )
  {
    return true;
  }
  else if ( f1->relatedByStaticRelation( f2, const_rel ) )
  {
    return true;
  }
  
  return false;
}

bool isCardUsed( int cardIndex )
{
  return ( cardIndex % 13 < SUIT_SIZE );
}

double entropy( int numPos, int numNeg )
{
  double p = ( (double)numPos / ( numPos+numNeg ) );
  double n = ( (double)numNeg / ( numPos+numNeg ) );
  
  if ( numPos == 0 || numNeg == 0 )
    return 0;
  
  return -p*log2( p ) - n*log2( n );
}

bool checkDifferences( Feature * base[BEAM_SIZE], int share_limit )
{
  int i, j;
  
  for ( i=0; i<BEAM_SIZE; i++ )
  {
    for ( j=0; j<BEAM_SIZE; j++ )
    {
      if ( i != j )
      {
        if ( ! base[i]->sufficientlyDifferent( base[j], share_limit ) )
        {
          return false;
        }
      }
    }
  }
  return true;
}

double squared_error( int size, int x[10], int y[10] )
{

  //double x[SIZE] = {1.0, 2.0, 3.0, 4.0, 4.0, 8.0, 8.0, 6.0, 10.0, 3.0};
  //double y[SIZE] = {0.9, 1.3, 4.3, 3.9, 6.0, 7.8, 8.3, 5.6, 10.8, 1.6};
    //double x[SIZE] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 8.0, 8.0};
  //double y[SIZE] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 7.0};
              
  double s_x, s_y, s_xx, s_xy;
  double beta, alpha;

  double c_x, c_y;

  int i;

  s_x = 0.0;
  s_y = 0.0;
  s_xy = 0.0;
  s_xx = 0.0;
                              
  for ( i=0; i<size; i++ )
  {
    s_x += x[i];
     s_y += y[i];
    s_xx += x[i]*x[i];
        s_xy += x[i]*y[i];
  }
                              
  alpha = 0.0;
  beta = 0.0;
                                     
  beta = ( ( size * s_xy ) - ( s_x * s_y ) ) / ( ( size * s_xx ) - ( s_x * s_x ) );
  alpha = ( s_y - ( beta * s_x ) ) / size;
                                    
  c_x = s_x / size;
  c_y = s_y / size;

  cout << "center = (" << c_x << "," << c_y << ")" << endl << endl;

  cout << "alpha = " << alpha << endl;
  cout << "beta = " << beta << endl;

  return 0;
}

// given a feature identifier, and values for card1, and card2, return the 
// index of the correct HCP Feature
int getHCPFeatureIndex( int feat_index, int card1, int card2 )
{  
  int card1_suit = card1 / SUIT_BREAK;      // value 0 .. 3
  int card2_suit = card2 / SUIT_BREAK;      // value 0 .. 3
  int card1_rank = card1 % SUIT_BREAK;      // value 0 .. 12
  int card2_rank = card2 % SUIT_BREAK;      // value 0 .. 12
  int feature_offset, card1_offset, card2_offset;
  //   0 - 311 On Suit Blocked
  // 312 - 399 Off Suit Blocked
  // 400 - 412 Group In Foundation
  // 413 - 438 Similar Card Face Down
  // 439 - 490 In Foundation
  // 491 - 542 In Stock Playable
  // 543 - 594 In Tableau Face Up
  
  switch ( feat_index ) 
  {
    case 0:
      // the first card is the blocked card (i.e. card1 = 2D, card2 = AD)
      if ( card2_suit != card1_suit || card1_rank <= card2_rank )
      {
        cout << "ERORR in getHCPFeatureIndex 0" << endl;
        exit( 0 );
      }
      feature_offset = 0;
      card1_offset = getOSBIndex( card1_rank ) + ( 78 * card1_suit );
      card2_offset = card2_rank;
      break;
    case 1:
      if ( ( card1_suit % 2 == card2_suit % 2 ) || card1_rank != ( card2_rank -1 ) )
      {
        cout << "ERORR in getHCPFeatureIndex 1" << endl;
        exit( 0 );
      }
      feature_offset = 312;
      card1_offset = ( card1_rank - 1 ) * 2 + ( 22 * card1_suit );
      card2_offset = card2 / ( FULL_DECK / 2 );
      break;
    case 2:
      feature_offset = 400;
      card1_offset = card1_rank;
      card2_offset = 0;
      break;
    case 3:
      feature_offset = 413;
      card1_offset = ( card1_rank * 2 ) + ( card1_suit % 2 );
      card2_offset = 0;
      break;
    case 4:
      feature_offset = 439;
      card1_offset = card1;
      card2_offset = 0;
      break;
    case 5:
      feature_offset = 491;
      card1_offset = card1;
      card2_offset = 0;
      break;
    case 6:
      feature_offset = 543;
      card1_offset = card1;
      card2_offset = 0;
      break;
    default:
      cout << "ERORR in getHCPFeatureIndex 7" << endl;
      exit( 0 );
  }
    return feature_offset + card1_offset + card2_offset;
}

int getOSBIndex( int card_mod )
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

// given a feature index, fill the values for feature, card1, and card2 accordingly
void HCPGetFeature( int HCP_index, int & feature, int & card1, int & card2 )
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
    card1 = HCP_index - 413;
    //feature_mod = HCP_index - 413;
    //card1_mod = feature_mod % 2;
    //card1 = ( feature_mod / 2 ) + ( card1_mod * SUIT_BREAK ) ;
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
    feature_mod = HCP_index - 543
      ;
    card1 = feature_mod;
    card2 = -1;
  }
}

int getOSBCard( int index )
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

// --------
//
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

/*
 * while we aren't at a dead end or a win,
 * build a UCT Tree to decide which action is best
 */
bool uctPolicy( State * s[MAX_SEARCH_DEPTH], UCTNode * uctRoot, MTRand * my_rand_gen, State * t_state, State * t_state2, int update_method, int default_policy, double uct_weights[NUM_UCT_WEIGHTS] )
{
  int base_depth = 0;
  int next_action;
  int repeat_count = 0;
  int i;

  //cout << " int uctPolicy" << endl;
  s[0]->copyInto( t_state );
  for ( base_depth = 0; base_depth<MAX_SEARCH_DEPTH-1; base_depth++ ){

    if ( s[base_depth]->win() ) return true;
    if ( s[base_depth]->numFaceDownCards() == 0 ){
      if ( greedyFinish ( s, base_depth ) ) {
        //cout << " greedy finish win:" << endl;
        return true;
      }
    }
    if ( s[base_depth]->valid_actions_index == -1 ) {
      //s[base_depth]->prettyPrint();
      //cout << "  no valid actions" << endl;
      return false;
    }
    if ( base_depth >= MAX_SEARCH_DEPTH-1 ) return 0;

    //cout << " building uctTree" << endl;
    buildUctTree( s, base_depth, uctRoot, my_rand_gen, t_state2, update_method, default_policy, uct_weights );
    //cout << " built tree" << endl;
    
    //s[base_depth]->prettyPrint();
    //cout << " choices are based on :" << endl;
    //cout << s[base_depth]->my_uct_node->num_visits << " visits to this node" << endl;
    //for ( i=0; i<=s[base_depth]->valid_actions_index; i++ ){
    //  if ( s[base_depth]->valid_actions[i]->available ){
    //  cout << " action " << i << " visits = " << s[base_depth]->my_uct_node->action_visits[i];
    //    cout << " value = " << s[base_depth]->my_uct_node->action_values[i] << endl;
    //  }
    //}

    next_action = makeUCTPolicyDecision( s[base_depth] );
    if ( next_action == -1 ) {
      //cout << "  no available actions" << endl;
      return false;
    }
    //cout << " made root decision to take action " << next_action << " based on the state: " << endl;
    //s[base_depth]->prettyPrint();

    t_state->copyInto( s[base_depth] );
    //cout << " taking transition in uctPolicy with action " << next_action << " from the state " << endl;
    //s[base_depth]->prettyPrint();
    s[base_depth]->transition( s[base_depth+1], next_action );
    s[base_depth+1]->copyInto( t_state );
    
    //cout << " transitioned at root base depth " << base_depth << endl;
    //if ( base_depth > 60 ) s[base_depth+1]->prettyPrint();

    repeat_count = 0;
    for ( i=0; i<base_depth; i++ ) if ( s[base_depth+1]->isFaceUpSame( s[i] ) ) repeat_count++;
    if ( repeat_count > 2 ) {
      //s[base_depth+1]->prettyPrint();
      //cout << "  excessive repetition" << endl;
      return false;
    }
  }
  return false;
}


/*
 * while we arent at a dead end or a win,
 * choose a random action to decide which action is best
 */
bool randomPolicy( State * s[MAX_SEARCH_DEPTH], MTRand * my_rand_gen, State * t_state )
{

}

/*
 * while we aren't at a dead end or a win,
 * build a UCT Tree to decide which action is best
 */
bool hOptUctPolicy( State * s[MAX_SEARCH_DEPTH], UCTNode * uctRoot, MTRand * my_rand_gen, State * t_state, State * t_state2,
    int update_method, int default_policy, double uct_weights[NUM_UCT_WEIGHTS] )
{
  int base_depth = 0;
  int repeat_count = 0;
  int i,j;
  double action_values[MAX_NUM_ACTIONS];
  double best_action_value;
  int best_action;

  //cout << " in hOptUctPolicy() " << endl;

  s[0]->copyInto( t_state );
  for ( base_depth = 0; base_depth<MAX_SEARCH_DEPTH-1; base_depth++ ){

    best_action_value = INT_MIN;
    best_action = -1;

    if ( s[base_depth]->win() ) return true;
    if ( s[base_depth]->numFaceDownCards() == 0 ){
      if ( greedyFinish ( s, base_depth ) ) {
        return true;
      }
    }
    if ( s[base_depth]->valid_actions_index == -1 ) {
      //cout << "  no valid actions" << endl;
      return false;
    }

    //cout << endl << endl << " At the base: "  << endl;
    //s[base_depth]->prettyPrint();

    for ( i=0; i<MAX_NUM_ACTIONS; i++ ){ action_values[i] = 0; } for ( i=0; i<HOPT_UCT_TRIALS; i++ ){
      t_state->copyInto( s[base_depth] );

      if (print) cout << " for action " << base_depth << " building tree " << i << endl;

      buildUctTree( s, base_depth, uctRoot, my_rand_gen, t_state2, update_method, default_policy, uct_weights );
      if (print) cout << " out of buildUctTree" << endl;
      //cout << endl << endl;
      
      for ( j=0; j<=s[base_depth]->valid_actions_index; j++ ){
        if ( s[base_depth]->valid_actions[j]->available ) action_values[j]+=s[base_depth]->my_uct_node->action_values[j];
      }
    }

    for ( i=0; i<=s[base_depth]->valid_actions_index; i++ ){
      if ( s[base_depth]->valid_actions[i]->available ){
        if ( action_values[i] > best_action_value ){
          best_action = i;
          best_action_value = action_values[i];
        }
      }
    }
    //for ( j=0; j<=s[base_depth]->valid_actions_index; j++ ) cout << " valid action " << j << " valued at " << action_values[j] << endl;
    //cout << " best action at depth " << base_depth << " = " << best_action << endl; 
    
    if ( best_action == -1 )  return false; 

    t_state->copyInto( s[base_depth] );
    s[base_depth]->transition( s[base_depth+1], best_action );
    s[base_depth+1]->copyInto( t_state );
  }
  uctRoot->cleanUp();
  return false;
}

int buildUctTree( State * s[MAX_SEARCH_DEPTH], int base_depth, UCTNode * uctRoot, MTRand * my_rand_gen, State * t_state, 
    int update_method, int default_policy, double uct_weights[NUM_UCT_WEIGHTS] )
{
  int uct_nodes_index = -1;
  int depth_offset;
  bool win;
  double reward;
  int rollout_count = 0;

  int i;
  if (print) cout << " in buildUctTree at base depth " << base_depth << endl;
  uctRoot->init();
  
  // it's important that the true identity of the face-down cards be hidden from the decicion making process
  // to ensure that we preserve that hidden property, we first shuffle all face-down cards before each rollout
  //s[base_depth]->shuffleFaceDownCards( my_rand_gen );
  
  s[base_depth]->my_uct_node = uctRoot;
  s[base_depth]->fillCompact( s[base_depth]->my_uct_node->c_state );

  // while we haven't reached our limit (tree nodes expanded or rollouts performed)
  while ( sparseUctRollOut( s, uct_nodes_index, my_rand_gen, base_depth, depth_offset, win, t_state, default_policy, uct_weights ) ){

    if ( ++rollout_count > MAX_UCT_ROLLOUTS ) return 0; 
    if ( base_depth >= MAX_SEARCH_DEPTH-1 ) return 0;

    if (print) cout << " rollout count = " << rollout_count << " have used " << uct_nodes_index << " so far " << endl;

    reward = ( win ? 1.0 : 0.0 );
    if ( update_method == AVG_REW_UCT_UPDATE ){
      updateUCTTrace( s, base_depth, depth_offset, reward );
    } else if ( update_method == E_MAX_UCT_UPDATE ){
      expectimaxUpdateUCTTrace( s, base_depth, depth_offset, reward );
    } else {
      cerr << " ERROR 1 in buildUctTRee(): Unknown update method " << endl;
      exit( 1 );
    }

    if (print) cout << " out of uct update at depth " << base_depth << endl;
    //for ( i=0; i<=s[base_depth]->valid_actions_index; i++ ){
      //cout << "   value for action " << i << " is " << s[base_depth]->my_uct_node->action_values[i] << endl;
    //}
    s[base_depth]->shuffleFaceDownCards( my_rand_gen );
  }
  // when done (either reached the goal or a dead end) update all nodes in the trajectory
  // look at the value of all of the possible action choices and return the one that maximizes the likelihood of winning
  return 0;
}

/*
 * sparseUctRollout(...)
 */
bool sparseUctRollOut( State * s[MAX_SEARCH_DEPTH], int &uct_nodes_index, MTRand * my_rand_gen, 
    int base_depth, int &depth_offset, bool &win, State * t_state, 
    int default_policy, double uct_weights[NUM_UCT_WEIGHTS] )
{
  depth_offset = 0;
  bool end_of_search;
  int repeated_node;
  int i, next_action;
  int random_child, child_index, repeat_child;
  State * t_state_local = new State();
  
  win = false;

  if (print) cout << " in sparseUctRollOut with base_depth + depth_offset = " << base_depth+depth_offset << endl;

  end_of_search = ( s[base_depth+depth_offset]->valid_actions_index == -1 || uct_nodes_index >= MAX_UCT_NODES-1 );
  while ( !end_of_search ){

    //s[base_depth+depth_offset]->prettyPrint();
    next_action = makeSparseUCTDecision( s, base_depth+depth_offset, my_rand_gen, t_state, default_policy, uct_weights );
    if (print) cout << " choosing action " << next_action << endl;

    if ( next_action == -1 ) {
      if (print) cout << " Ending due to no valid actions" << endl;
      win = false;
      end_of_search = true;
      break;
    }

    // we need to check to see if all of the slots have been taken for this action
    // and if so, then we choose randomly from one of those slots
    // if not, then re-shuffle the face-down cards and take a new transition
    if ( s[base_depth+depth_offset]->my_uct_node->canAddChild( next_action ) ){

      s[base_depth+depth_offset]->shuffleFaceDownCards( my_rand_gen );
      s[base_depth+depth_offset]->transition( t_state_local, next_action );
      t_state_local->fillUCTCompact( t_state_local->my_cs );

      repeat_child = s[base_depth+depth_offset]->my_uct_node->repeatFaceUpChild( next_action, t_state_local->my_cs );

      if ( repeat_child == -1 ){
        // we haven't yet seen this result for this action 
        if ( ! s[base_depth+depth_offset]->my_uct_node->addChild( next_action ) ) {
          cerr << "ERROR in SparseRollOut(): cannot add a child we thought we could " << endl;
          exit ( 0 );
        }
        uct_nodes_index++;

        s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], next_action );
        s[base_depth+depth_offset+1]->fillUCTCompact( s[base_depth+depth_offset+1]->my_cs );

        // we have already allocated the memory for this UCTNode.  Fill that node with the correct data
        child_index = s[base_depth+depth_offset]->my_uct_node->numChildren(next_action)-1;
        s[base_depth+depth_offset+1]->my_uct_node = s[base_depth+depth_offset]->my_uct_node->children[next_action][child_index];
        s[base_depth+depth_offset+1]->fillCompact( s[base_depth+depth_offset+1]->my_uct_node->c_state );

        if(print) cout << " new child for " << (unsigned long int)s[base_depth+depth_offset]->my_uct_node << " and action " ;
        if(print) cout << next_action << " and index " << child_index << " is " << (unsigned long int)s[base_depth+depth_offset+1]->my_uct_node << endl;
        t_state->fillFromCompact( s[base_depth+depth_offset+1]->my_uct_node->c_state );
        //if(print) t_state->prettyPrint();

      } else {
        if (print) cout << " would have re-added child[" << repeat_child << "] at base_depth = " << base_depth;
        if (print) cout << ", depth_offset = " << depth_offset << ", next_action = " << next_action << endl;
        // we already saw this result for this action (in children[next_action][repeat_child] )
        s[base_depth+depth_offset+1]->fillFromCompact( s[base_depth+depth_offset]->my_uct_node->children[next_action][repeat_child]->c_state );
        s[base_depth+depth_offset+1]->my_uct_node = s[base_depth+depth_offset]->my_uct_node->children[next_action][repeat_child];
      }

    } else {
      if(print) cout << (unsigned long int)s[base_depth+depth_offset]->my_uct_node << " failed to add a child for action " << next_action << endl;
      // we already have at least SPARSE_BRANCHING_FACTOR unique children for this action, so we'll 
      // take one of the branches we've already explored (we've reached our limit)
      
      // randomly pick one of the existing children ( random_child = [0..SPARSE_BRANCHING_FACTOR-1])
      random_child = my_rand_gen->randInt() % SPARSE_BRANCHING_FACTOR;
      if(print) cout << " next action = " << next_action << ", and random_child = " << random_child << endl;
      if(print) cout << " chosen child = " << (unsigned long int)s[base_depth+depth_offset]->my_uct_node->children[next_action][random_child] << endl;
      s[base_depth+depth_offset+1]->fillFromCompact( s[base_depth+depth_offset]->my_uct_node->children[next_action][random_child]->c_state );
      s[base_depth+depth_offset+1]->my_uct_node = s[base_depth+depth_offset]->my_uct_node->children[next_action][random_child];
      if(print) cout << " filled from compact" << endl;
    } 
    depth_offset++;

    if (print) cout << " base_depth = " << base_depth << " and depth_offset = " << depth_offset << " with win = " << win << endl;

    if( s[base_depth+depth_offset]->win() ){
      //cout << " Ending due to win" << endl;
      end_of_search = win = true;
    } else if ( s[base_depth+depth_offset]->numFaceDownCards() == 0 ) {
      if(print) cout << " no more face down cards" << endl;
      if ( greedyFinish( s, base_depth+depth_offset ) ) {
        if(print) cout << " Ending due to greedy win at base_depth " << base_depth << " and depth offset = " << depth_offset << endl;
        end_of_search = win = true;
      } 
    } else if ( s[base_depth+depth_offset]->valid_actions_index == -1 ){
      if(print) cout << " Ending due to lack of valid actions" << endl;
      end_of_search = true;
      win = false;
    } 
    if (print) cout << " win = " << win << " and end_of_search = " << end_of_search << endl;
    if ( !win && ((base_depth+depth_offset) >= (MAX_SEARCH_DEPTH-1) ) ){
      if(print) cout << " ending due to maxxed out search depth" << endl;
      end_of_search = true;
      win = false;
    }
    //else {
    //  for ( i=base_depth+depth_offset-1; i>=base_depth; i-- ){
    //    if ( s[depth_offset]->isFaceUpSame( s[i] ) ) {
    //      cout << " ending due to repeated state" << endl;
    //      end_of_search = true;
    //      win = false;
    //      break;
    //    }
    //  }
    //}
  }
  //if ( base_depth == 44 && s[base_depth]->my_uct_node->size() > 3060 ){
    //cout << " turning print on" << endl;
    //print = true;
  //}
  delete t_state_local;
  if (print) cout << " return with a tree size of " << s[base_depth]->my_uct_node->size() << endl;
  return ( s[base_depth]->my_uct_node->size() < MAX_UCT_NODES ) ;
}


void updateUCTTrace( State * s[MAX_SEARCH_DEPTH], int base_depth, int depth_offset, double reward )
{
  int i;
  int action_taken;
  //cout << "updating with base depth = " << base_depth << " and depth offset = " << depth_offset << endl;
  for ( i=base_depth; i<base_depth+depth_offset; i++ ){
    //cout << " updateUCTTrace() updating state " << i << " which took action " << s[i]->uct_best_action << endl;
    s[i]->my_uct_node->num_visits++;
    action_taken = s[i]->uct_best_action;
    //cout << " action taken = " << action_taken << endl;
    s[i]->my_uct_node->action_visits[ action_taken ]++;
    //if ( i == 0 ) cout << " action_values = " << s[i]->my_uct_node->action_values[action_taken] << endl;
    s[i]->my_uct_node->action_values[ action_taken ] += 
      ( 1/((double)s[i]->my_uct_node->action_visits[action_taken]) )*( reward - s[i]->my_uct_node->action_values[action_taken]);
    //if ( i == 0 ){
    //  cout << " action_visits = " << s[i]->my_uct_node->action_visits[action_taken] << endl;
    //  cout << " reward = " << reward << endl;
    //  cout << " new value for action " << action_taken << " = " << s[i]->my_uct_node->action_values[ action_taken ] << endl;
    //}
  }
  //cout << " updated " << base_depth+depth_offset << endl;
  s[base_depth+depth_offset]->my_uct_node->num_visits++;
}

/*
 * unlike with a regular UCT update, the expectimax update calculates the value of an action with the value of the state that it reached
 * (rather than the value of the reward that it received)
 * also - with expectimax trees, states have values as well, which are calculated as the value of the max action at that state
 */
void expectimaxUpdateUCTTrace( State * s[MAX_SEARCH_DEPTH], int base_depth, int depth_offset, double reward )
{
  int i, j;
  int action_taken;

  //cout << " in expectimaxUpdate, reward = " << reward << endl;

  // going backwards - this is the last state visited,
  // but there were no actions for this final visit (leaf node)
  s[base_depth+depth_offset]->my_uct_node->num_visits++;
  s[base_depth+depth_offset]->my_uct_node->expectimax_value = reward;

  // for each of the actions from s[i] in our trajectory, update the uct nodes based on the values in s[i+1]
  for ( i=base_depth+depth_offset-1; i>=base_depth; i-- ){
    //cout << endl << " expectimaxUpdateUCTTrace() updating state " << i << " which took action " << s[i]->uct_best_action << endl;

    // we are going to update the value of the actions taken as well as the value of the state
    // actions first
    s[i]->my_uct_node->num_visits++;
    action_taken = s[i]->uct_best_action;

    s[i]->my_uct_node->action_visits[ action_taken ]++;

    //cout << "  s[ " << i << "]->my_uct_node->action_visits[" << action_taken << "] = " << s[i]->my_uct_node->action_visits[ action_taken ] << endl;
    //cout << "  s[ " << i << "]->my_uct_node->action_values[" << action_taken << "] = " << s[i]->my_uct_node->action_values[ action_taken ] << endl;
    //cout << "  s[ " << i+1 << "]->my_uct_node->expectimax_value = " << s[i+1]->my_uct_node->expectimax_value << endl;

    s[i]->my_uct_node->action_values[ action_taken ]+=
      ( 1/(double)(s[i]->my_uct_node->action_visits[action_taken]) )*( s[i+1]->my_uct_node->expectimax_value - (double)s[i]->my_uct_node->action_values[action_taken] );
    //cout << " new action value = " << s[i]->my_uct_node->action_values[ action_taken ] << endl;

    // I should be able to just compare the value of the new action with the current state value - none of the other action values should change
    // then update state values
    s[i]->my_uct_node->expectimax_value = (double)s[i]->my_uct_node->action_values[0];
    //cout << "[" << s[i]->my_uct_node->action_values[0] << ", " << getUCTValue(s[i]->my_uct_node, 0)  << "] ";
    for ( j=1; j<=s[i]->valid_actions_index; j++ ){
      //cout << "[" << s[i]->my_uct_node->action_values[j] << ", " << getUCTValue(s[i]->my_uct_node, j)  << "] ";
      if ( s[i]->my_uct_node->action_values[j] > s[i]->my_uct_node->expectimax_value ) s[i]->my_uct_node->expectimax_value = (double)s[i]->my_uct_node->action_values[j];
    }
    //cout << " new expectimax value for s["<< i << "] = " << s[i]->my_uct_node->expectimax_value << endl;
  }
  //cout << " new value for action " << s[base_depth]->uct_best_action << " at base depth of " << base_depth << " is " << s[base_depth]->my_uct_node->action_values[s[base_depth]->uct_best_action] << endl;
}


int makeUCTPolicyDecision( State * cur_state )
{
  //cout << "making decision" << endl;
  int i;

  // all uct values should be greater than (or equal to ) 0
  double best_action_value = INT_MIN;
  int best_action = -1;

  for ( i=0; i<=cur_state->valid_actions_index; i++ ){
    if ( cur_state->valid_actions[i]->available ){
      if ( cur_state->my_uct_node->action_values[i] > best_action_value ){
        best_action = i;
        best_action_value = cur_state->my_uct_node->action_values[i];
      }
    }
  }

  cur_state->uct_best_action = best_action;
  return best_action;
}

/* makeSparseUCTDecision
 * for each action, we allow up to (SPARSE_BRANCHING_FACTOR) paths
 */
int makeSparseUCTDecision( State * s[MAX_SEARCH_DEPTH], int depth, MTRand * my_rand_gen, State * t_state, int default_policy, double uct_weights[NUM_UCT_WEIGHTS] )
{
  int i, j;
  int search_floor, num_deck_cards, num_face_down_cards;
  double uct_values[MAX_NUM_ACTIONS];
  bool non_repeat_actions[MAX_NUM_ACTIONS];

  // all uct values should be greater than (or equal to ) 0
  double best_uct_value = -1;
  int best_action = -1;

  //cout << " in makeSparseUCTDecision" << endl;

  // set a floor where we can start checking for repeated states
  // (# of deck cards and # of face down cards must
  //  monotonically increase through deck play)
  num_deck_cards = s[depth]->numDeckCards();
  num_face_down_cards = s[depth]->numFaceDownCards();
  search_floor = 0;
  for ( j=depth-1; j>=0; j-- ){
    if ( s[j]->numDeckCards() != num_deck_cards || s[j]->numFaceDownCards() != num_face_down_cards ){
      //cout << " setting search floor to " << j+1 << endl;
      search_floor = j+1;
      break;
    }
  }

  for ( i=0; i<MAX_NUM_ACTIONS; i++ ) non_repeat_actions[i] = true;

  for ( i=0; i<=s[depth]->valid_actions_index; i++ ) {
    if ( s[depth]->valid_actions[i]->available ){
      s[depth]->transition( t_state, i );
      for ( j=0; j<depth; j++ ){
        if ( t_state->isFaceUpSame( s[j] ) ){
          non_repeat_actions[i] = false;
        }
      }
    }
  }

  if ( s[depth]->my_uct_node->all_actions_tried ){
    for ( i=0;i<=s[depth]->valid_actions_index; i++ ){ 
      // don't make calculations for actions that aren't actually available
      if ( s[depth]->valid_actions[i]->available && non_repeat_actions[i] ){
        // calculate UCT confidence interval (based on Hoeffding's inequality)
        uct_values[i] = getUCTValue( s[depth]->my_uct_node, i );
        if ( uct_values[i] > best_uct_value ){
          best_uct_value = uct_values[i];
          best_action = i;
        }
      }
    }
    //if ( depth == 0 ) cout << " choosing action " << best_action << endl;
  } else {
    // choose randomly from available unchosen actions
    if ( default_policy == RANDOM_POLICY_DEFAULT )
      best_action = randomUnchosenActionIndex( s[depth], my_rand_gen, non_repeat_actions );
    else if ( default_policy == PRIORITY_POLICY_DEFAULT )
      best_action = firstUnchosenActionIndex( s, depth, non_repeat_actions );
    else if ( default_policy == LEARNED_POLICY_DEFAULT )
      best_action = greedyUnchosenActionIndex( s, depth, uct_weights, non_repeat_actions );

    /* //test code
    for ( i=0; i<=s[depth]->valid_actions_index; i++ ){
      if ( s[depth]->valid_actions[i]->available && s[depth]->my_uct_node->action_values[i] == 1.0 ) {
        best_action = i;
        //cout << " forced to take action " << i << " at depth " << depth << endl;
      }
    }
    */ //end test code
    
    //cout << " UCTDecision() random choosing action " << best_action << endl;
    //best_action = firstAvailableAction( s[depth] );
    //best_action = sampleUniformlyUpTo( s[depth], MIN_SAMPLE_SIZE );
    
    // check to see if we've now tried all possible actions 
    s[depth]->my_uct_node->all_actions_tried = true;
    for ( i=0; i<=s[depth]->valid_actions_index; i++ ) {
      // we actually only care about the actions that are available
      if ( s[depth]->valid_actions[i]->available && s[depth]->my_uct_node->action_visits[i] == 0 ){
        s[depth]->my_uct_node->all_actions_tried = false;
      }
    }
  }

  s[depth]->uct_best_action = best_action;
  return best_action;
}

double getUCTValue( UCTNode * uctNode, int action )
{
  // prevent division by zero (just in case)
  // should this return 1.0 instead?  
  // If we haven't yet taken some action (hence the zero) then we'll want to take it now
  //cout << " in getUCTValue" << endl;
  if ( uctNode->action_visits[action] == 0 || uctNode->num_visits == 0 ) return 0;
  return uctNode->action_values[action] + UCT_CONST * (sqrt( log( (double)uctNode->num_visits) / uctNode->action_visits[action] ) );
  //return uctNode->action_values[action];
}

bool greedyFinish ( State * s[MAX_SEARCH_DEPTH], int base_depth )
{
  int i, first_action;
  int depth_offset = 0;
  bool end_search = false;

  //cout << " greedy finish pre process at base_depth + depth_offset = " << base_depth+depth_offset << endl;
  if ( base_depth+depth_offset >= ( MAX_SEARCH_DEPTH-1 ) ) {
    //cout << "greedyFinish search exhausted, falsing out" << endl;
    return false;
  }
  while ( true ){

    //cout << " in the loop at base_depth + depth_offset = " << base_depth+depth_offset << endl;
    if ( base_depth+depth_offset >= MAX_SEARCH_DEPTH-1 ) return false;
    if ( s[base_depth+depth_offset]->win() ) return true;

    first_action = -1;
    for ( i=0; i<=s[base_depth+depth_offset]->valid_actions_index; i++ ){
      if ( s[base_depth+depth_offset]->valid_actions[i]->available ){
        first_action = i;
        break;
      }
    }
    if ( first_action == -1 ) return false;

    //cout << " greedy finish transition at base_depth + depth_offset = " << base_depth+depth_offset << endl;
    s[base_depth+depth_offset]->transition( s[base_depth+depth_offset+1], first_action );
    //cout << " after transition" << endl;

    for ( i=0; i<base_depth+depth_offset; i++ ){
      if ( s[base_depth+depth_offset+1]->isFaceUpSame( s[i] ) ) return false;
    }
    depth_offset++;
  }
}

int firstAvailableAction( State *s )
{
  int i;
  int first_action = -1;

  for ( i=0; i<=s->valid_actions_index; i++ ){
    first_action = -1;
    if ( s->valid_actions[i]->available ){
      first_action = i;
      break;
    }
  }
  return first_action;
}

double uctUpper( int state_visited, int action_taken, double score, double c )
{
  return score + c * ( sqrt( log( (double)state_visited ) / action_taken ) );
}

double wilsonUpper( int trials, int wins, double z_score )
{
  double base, interval, denom;
  double z2 = z_score * z_score;
  double p_est = wins/(double)trials;

  if ( wins == 0 ) return 0;

  base = p_est + ( z2 /( 2 * trials ) );
  interval = z_score * sqrt( ( p_est * (1-p_est) / trials ) + ( z2 / ( 4 * trials * trials  ) ) );
  denom = 1 + ( z2 / trials  );

  return ( base + interval ) / denom;
}

int chooseUpper( int num_samples[MAX_NUM_ACTIONS], int num_wins[MAX_NUM_ACTIONS], bool valid_actions[MAX_NUM_ACTIONS], double z_score )
{
  // we want to give each action a fighting chance - 
  // sample each valid action 10 time before we start selecting by upper bounds
  // if all of the samples we have sampled lead to failures, sample uniformly
  // use the Wilson interval because it's more stable under small trials and extreme probabilities
  
  int i;
  double best_action_value;
  int best_action_index;
  double temp;
  bool all_zeros = true;

  //cout << " choosing Upper from the following actions: ";
  for ( i=0; i<MAX_NUM_ACTIONS; i++ ){
    if ( valid_actions[i] ){
      //cout << i << "[" << num_wins[i] << "/" << num_samples[i]<< "]  ";
    }
  }
  //cout << endl;

  best_action_value = (double)INT_MAX;
  for ( i=0; i<MAX_NUM_ACTIONS; i++ ){
    if ( valid_actions[i] && num_samples[i] < best_action_value ){
      best_action_value = (double)num_samples[i];
      best_action_index = i;
    }
    if ( num_wins[i] > 0 ) all_zeros = false;
  }
  if ( best_action_value == (double)INT_MAX ) return -1;
  else if ( best_action_value < MIN_SAMPLE_SIZE || all_zeros ) return best_action_index;

  // ----------
  // just testing what happens when we sample uniformly - discard eventually
  // return best_action_index;
  // ----------

  // all actions have been sampled at least MIN_SAMPLE_SIZE times *and*
  //   there is at least one action that has reported winning a game
  // so now calculate the upper bound of the estimated probability of winning
  // using the Wilson Score Test
  
  best_action_value = (double)INT_MIN;
  for ( i=0; i<MAX_NUM_ACTIONS; i++ ){
    if ( valid_actions[i] ){
      temp = wilsonUpper( num_samples[i], num_wins[i], z_score );
      if ( temp > best_action_value ) {
        best_action_value = temp;
        best_action_index = i;
      }
    }
  }
  if ( best_action_value == INT_MIN ){
    cerr << "ERROR IN chooseUpper() " << endl;
    exit ( 0 );
  }

  return best_action_index;
}

/*void default_uct_feat_weights( double uct_feat_weights[NUM_UCT_FEATURES] )
{
  double dummy[NUM_UCT_FEATURES] = {
    // card in foundation [diam, club, hear, spad]
    0.431, 0.609, 0.920, 0.645, 0.689, 0.606, 0.660, 0.850, 0.912, 0.585, 0.074, 0.871, 0.368, 
    0.484, 0.152, 0.900, 0.438, 0.680, 0.644, 0.216, 0.178, 0.724, 0.253, 0.952, 0.691, 0.696, 
    0.387, 0.930, 0.045, 0.268, 0.241, 0.376, 0.227, 0.141, 0.078, 0.905, 0.168, 0.645, 0.081, 
    0.378, 0.896, 0.275, 0.965, 0.187, 0.821, 0.945, 0.257, 0.696, 0.261, 0.523, 0.516, 0.294, 
    // in deck, cannot be moved [diam, club, hear, spad]
    0.132, 0.001, 0.001, 0.001, 0.002, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.002, 
    0.185, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.002, 0.002, 0.001, 0.001, 0.003, 
    0.088, 0.001, 0.002, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.003, 
    0.080, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.002, 0.003, 
    // in deck, can be moved to found [diam, club, hear, spad]
    0.130, 0.000, 0.956, 0.251, 0.818, 0.508, 0.609, 0.844, 0.844, 0.763, 0.260, 0.222, 0.930, 
    0.183, 0.000, 0.108, 0.497, 0.746, 0.953, 0.591, 0.604, 0.373, 0.243, 0.401, 0.607, 0.203, 
    0.087, 0.000, 0.928, 0.027, 0.779, 0.086, 0.498, 0.891, 0.693, 0.224, 0.331, 0.170, 0.134, 
    0.078, 0.000, 0.762, 0.030, 0.931, 0.370, 0.748, 0.197, 0.304, 0.045, 0.497, 0.394, 0.455, 
    // in deck, can be moved to tab [diam, club, hear, spad]
    0.000, -0.001, -0.001, -0.001, -0.001, -0.001, -0.001, -0.001, -0.001, -0.001, -0.001, -0.001, 0.080, 
    0.000, -0.001, -0.001, -0.001, -0.000, -0.000, -0.001, -0.000, -0.001, -0.001, -0.001, -0.001, 0.122, 
    0.000, -0.001, -0.001, -0.001, -0.000, -0.001, -0.001, -0.001, -0.001, -0.001, -0.001, -0.001, 0.143, 
    0.000, 0.000, -0.001, -0.001, -0.001, -0.001, -0.001, -0.001, -0.001, -0.001, -0.001, -0.001, 0.123, 
    // face up in tableau [diam, club, hear, spad]
    0.428, 0.298, 0.297, 0.298, 0.298, 0.298, 0.298, 0.298, 0.298, 0.298, 0.298, 0.298, 0.299, 
    0.480, 0.298, 0.297, 0.298, 0.297, 0.297, 0.297, 0.298, 0.298, 0.298, 0.297, 0.297, 0.299, 
    0.383, 0.298, 0.298, 0.297, 0.297, 0.298, 0.298, 0.297, 0.298, 0.297, 0.298, 0.298, 0.299, 
    0.375, 0.298, 0.298, 0.297, 0.298, 0.298, 0.298, 0.297, 0.298, 0.298, 0.298, 0.297, 0.299, 
    // face down in tableau [diam, club, hear, spad]
    -0.005, -0.136, -0.136, -0.136, -0.135, -0.136, -0.136, -0.135, -0.136, -0.136, -0.136, -0.136, -0.135, 
    0.048,  -0.136, -0.136, -0.136, -0.136, -0.135, -0.136, -0.136, -0.136, -0.136, -0.135, -0.136, -0.135, 
    -0.049, -0.136, -0.136, -0.136, -0.136, -0.136, -0.135, -0.136, -0.135, -0.136, -0.136, -0.136, -0.134, 
    -0.056, -0.136, -0.136, -0.135, -0.136, -0.136, -0.135, -0.136, -0.136, -0.136, -0.136, -0.136, -0.134, 
    // face down in tableau, reveal on this move [diam, club, hear, spad]
    0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.013, 0.014, 0.014, 0.014, 0.013, 
    0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 
    0.015, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.013, 0.014, 0.014, 
    0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 0.014, 
    // max depth of face down tableau [0,1,2,3,4,5,6]
    0.282, 0.356, 0.610, 0.103, 0.847, -0.129, -0.129
  };
  for ( int i=0; i<NUM_UCT_FEATURES; i++ ) uct_feat_weights[i] = dummy[i];
}

void td_learned_weights( double uct_feat_weights[NUM_UCT_FEATURES] )
{
  double dummy[NUM_UCT_FEATURES] = {
    // card in foundation [diam, club, hear, spad]
    6.67962, 1.5499, 3.77195, 3.25515, 2.45916, 2.01496, 2.59506, 3.6786, 4.10655, 4.58847, 4.07903, 4.13538, 5.85109, 
    6.24941, 3.88115, 0.673097, 1.39994, 2.12362, 1.64214, 2.95587, 4.38351, 3.52946, 3.2286, 3.76692, 3.80546, 5.44268, 
    6.63906, 2.449, 2.0717, 2.8455, 3.32748, 3.5514, 1.83414, 2.77578, 4.21348, 4.03034, 4.14564, 3.92239, 5.00681, 
    9.10757, 3.41361, 3.58779, 4.03365, 2.68867, 3.2337, 3.73028, 3.42918, 2.37558, 2.81141, 3.60724, 4.03594, 5.31522, 
    // in deck, cannot be moved [diam, club, hear, spad]
    6.23168, 1.73637, 2.36577, 1.67138, 0.256782, 2.48458, 1.56786, 1.27607, 1.05003, 0.127782, 1.53714, 1.14964, 0.00164405, 
    6.60742, 0.0406831, 0.786003, 2.16624, -0.392623, 2.43897, 1.70271, 1.39033, 1.20672, 1.55344, 0.646827, 1.24934, 0.257185, 
    7.06878, 1.69812, 0.420256, 0.597691, 1.75748, 2.22899, 2.75062, 1.40855, 0.46675, 0.989102, 0.775792, 0.668339, 0.0459098, 
    5.64042, 2.00658, 1.98192, 2.48141, 1.74948, 0.301006, 1.27702, 0.651157, 0.840093, -0.94564, 2.44111, 1.2274, -0.53223, 
    // in deck, can be moved to found [diam, club, hear, spad]
    5.71969, 1.90398, 0.670752, 1.27227, 1.4732, 1.14, 0.743325, 0.232688, 0.330159, 0.353547, 0.207017, 0.119392, 0.0495541, 
    7.21397, 0.51341, -0.0464818, 1.2647, 0.605153, 0.741579, 0.759767, 0.0971598, 0.348844, 0.170813, 0.136713, 0.125628, 0.0271473, 
    6.37192, 1.40995, 0.851796, 0.743931, 0.880693, 0.868987, 0.273303, 0.422247, 0.203891, 0.357608, 0.181396, 0.0482653, 0.0209333, 
    6.28844, 0.90164, 1.49672, 0.823326, 0.702746, 0.908141, 0.270338, 0.0593586, 0.479509, 0.182949, 0.226113, 0.173364, 0.0218814, 
    // in deck, can be moved to tab [diam, club, hear, spad]
    -0.136088, 0.489509, 1.2346, -0.0560297, 0.626806, 0.590032, -0.233705, 0.79654, 1.53419, 0.315703, 1.95022, 1.27244, 1.56798, 
    -0.448563, 1.27364, 0.215134, 0.918676, -0.544894, 0.904694, 0.145276, 0.025396, 0.371602, 0.552575, 0.424012, 0.506919, 1.174, 
    0.587531, 1.06262, 0.247539, 0.504836, 0.758374, 0.663886, 0.509917, 1.10462, 1.62925, 0.962704, 0.42579, 0.760894, -0.0957418, 
    0.227133, 1.09154, 0.353117, -0.348098, -0.279158, 0.94504, 1.38905, 1.1105, 0.443978, -0.709104, 0.74586, 0.62489, 0.575911, 
    // face up in tableau [diam, club, hear, spad]
    7.77746, 2.40804, 0.145171, -0.652998, 0.510241, 0.858618, 0.292737, 2.61431, 1.78142, 1.72836, 2.29632, 0.287458, 2.18746, 
    6.79311, 2.33167, 1.89481, 1.87056, 1.77167, -1.31815, -1.25575, 1.12062, 2.23011, 2.2564, 2.4656, 1.66837, 3.87606, 
    7.2548, 2.39374, 1.53885, 0.520042, -0.389512, 0.960828, 0.86194, -0.207993, 1.64556, 2.09095, 0.325536, 2.92197, 1.50476, 
    7.22549, 2.15287, 0.486726, -0.281247, 0.0955987, 2.28277, 1.48039, 2.58975, 0.476083, 2.9245, 1.87021, 1.86403, 1.95089, 
    // face down in tableau [diam, club, hear, spad]
    5.28405, 1.07762, -1.01538, 1.57109, 1.73888, 1.15372, 2.08073, 1.1957, -0.776806, 1.16417, 1.06517, 0.180324, 2.70298, 
    4.82859, 1.03336, 3.78206, 0.0669362, 0.956495, 0.670316, -0.0209925, 2.06797, 2.40638, 0.352079, -0.413964, 1.97291, 0.225642, 
    4.35794, -0.765687, 1.74978, 2.75073, 0.354909, 0.695255, 1.62379, 2.0138, -1.10082, 2.09454, 1.37159, 1.13716, 1.22171, 
    3.43059, -2.1097, 1.70788, 0.764278, 1.41219, 3.19249, 1.23345, 0.290058, 0.868862, 1.88699, 0.940065, 3.89323, 3.17257, 
    // face down in tableau, reveal on this move [diam, club, hear, spad]
    0.290384, -0.307266, -1.13969, -0.00603737, 0.178147, -0.3559, 0.598397, -0.525486, 0.439533, 0.66177, 0.833808, -0.543404, 0.557766, 
    0.64263, 0.752574, 0.991568, -0.371369, 0.0413065, -0.612441, -0.516381, 0.29682, 0.852486, -0.225849, -0.0590539, -0.644985, 0.295569, 
    1.06241, 0.157177, 0.126501, 0.954981, 0.332799, -1.15613, -0.0993922, 0.190375, 0.0783946, -0.531286, 0.111682, 0.163673, 1.54558, 
    -0.252721, -0.330695, 1.15728, -0.285975, 0.152448, 0.914821, -0.0137195, -0.637013, 0.37519, 0.680643, -0.0849193, 0.933684, 0.853368, 
    // max depth of face down tableau [0,1,2,3,4,5,6]
    15.0484, 12.8772, 9.83157, 1.3545, -1.13176, -3.97897, -2.30842 
  };
  for ( int i=0; i<NUM_UCT_FEATURES; i++ ) uct_feat_weights[i] = dummy[i];
}

void td17_learned_weights( double uct_feat_weights[NUM_UCT_FEATURES] )
{
  double dummy[NUM_UCT_FEATURES] = {
    // card in foundation [diam, club, hear, spad]
    31.0994, 34.4466, 32.7116, 28.7321, 42.7465, 45.6948, 42.4919, 41.079, 46.7915, 54.5881, 57.253, 59.1321, 50.412, 
    31.0994, 34.4466, 32.7116, 28.7321, 42.7465, 45.6948, 42.4919, 41.079, 46.7915, 54.5881, 57.253, 59.1321, 50.412, 
    31.0994, 34.4466, 32.7116, 28.7321, 42.7465, 45.6948, 42.4919, 41.079, 46.7915, 54.5881, 57.253, 59.1321, 50.412, 
    31.0994, 34.4466, 32.7116, 28.7321, 42.7465, 45.6948, 42.4919, 41.079, 46.7915, 54.5881, 57.253, 59.1321, 50.412, 
    // in deck, cannot be moved [diam, club, hear, spad]
    20.3446, 5.74359, 13.9225, 23.7797, 7.6928, 9.13541, 3.82145, 4.8188, 0.887027, 7.28829, 3.89254, 4.82997, -3.61864, 
    20.3446, 5.74359, 13.9225, 23.7797, 7.6928, 9.13541, 3.82145, 4.8188, 0.887027, 7.28829, 3.89254, 4.82997, -3.61864, 
    20.3446, 5.74359, 13.9225, 23.7797, 7.6928, 9.13541, 3.82145, 4.8188, 0.887027, 7.28829, 3.89254, 4.82997, -3.61864, 
    20.3446, 5.74359, 13.9225, 23.7797, 7.6928, 9.13541, 3.82145, 4.8188, 0.887027, 7.28829, 3.89254, 4.82997, -3.61864, 
    // in deck, can be moved to found [diam, club, hear, spad]
    21.8226, 9.69226, 0.232534, 5.04967, 0.342184, 0.968125, -0.227137, -1.52092, -1.01622, -0.0605571, 0.173662, 0.0683212, 0.20297, 
    21.8226, 9.69226, 0.232534, 5.04967, 0.342184, 0.968125, -0.227137, -1.52092, -1.01622, -0.0605571, 0.173662, 0.0683212, 0.20297, 
    21.8226, 9.69226, 0.232534, 5.04967, 0.342184, 0.968125, -0.227137, -1.52092, -1.01622, -0.0605571, 0.173662, 0.0683212, 0.20297, 
    21.8226, 9.69226, 0.232534, 5.04967, 0.342184, 0.968125, -0.227137, -1.52092, -1.01622, -0.0605571, 0.173662, 0.0683212, 0.20297, 
    // in deck, can be moved to tab [diam, club, hear, spad]
    -6.8928, -1.42068, 2.48678, -0.684217, 7.3839, 5.50992, 4.07445, 1.21209, -4.89841, -8.35618, 0.38218, 0.974838, -7.22808, 
    -6.8928, -1.42068, 2.48678, -0.684217, 7.3839, 5.50992, 4.07445, 1.21209, -4.89841, -8.35618, 0.38218, 0.974838, -7.22808, 
    -6.8928, -1.42068, 2.48678, -0.684217, 7.3839, 5.50992, 4.07445, 1.21209, -4.89841, -8.35618, 0.38218, 0.974838, -7.22808, 
    -6.8928, -1.42068, 2.48678, -0.684217, 7.3839, 5.50992, 4.07445, 1.21209, -4.89841, -8.35618, 0.38218, 0.974838, -7.22808, 
    // face up in tableau [diam, club, hear, spad]
    18.965, 4.98672, 6.28953, 3.79578, 13.3858, 8.59226, 10.4755, 16.2048, 11.1658, 32.5985, 30.0609, 30.658, 7.75504, 
    18.965, 4.98672, 6.28953, 3.79578, 13.3858, 8.59226, 10.4755, 16.2048, 11.1658, 32.5985, 30.0609, 30.658, 7.75504, 
    18.965, 4.98672, 6.28953, 3.79578, 13.3858, 8.59226, 10.4755, 16.2048, 11.1658, 32.5985, 30.0609, 30.658, 7.75504, 
    18.965, 4.98672, 6.28953, 3.79578, 13.3858, 8.59226, 10.4755, 16.2048, 11.1658, 32.5985, 30.0609, 30.658, 7.75504, 
    // face down in tableau [diam, club, hear, spad]
    17.6451, 34.29, 12.5318, 4.1137, 9.92524, 10.4468, 11.8862, 6.46547, 18.5553, 25.9999, 14.1758, 20.0008, 25.4006, 
    17.6451, 34.29, 12.5318, 4.1137, 9.92524, 10.4468, 11.8862, 6.46547, 18.5553, 25.9999, 14.1758, 20.0008, 25.4006, 
    17.6451, 34.29, 12.5318, 4.1137, 9.92524, 10.4468, 11.8862, 6.46547, 18.5553, 25.9999, 14.1758, 20.0008, 25.4006, 
    17.6451, 34.29, 12.5318, 4.1137, 9.92524, 10.4468, 11.8862, 6.46547, 18.5553, 25.9999, 14.1758, 20.0008, 25.4006, 
    // face down in tableau, reveal on this move [diam, club, hear, spad]
    0.837542, 10.2503, -0.226694, -6.01589, 11.4506, -2.82863, -0.321889, 3.79692, 11.9135, 5.19659, -0.71923, 7.77778, 12.2772, 
    0.837542, 10.2503, -0.226694, -6.01589, 11.4506, -2.82863, -0.321889, 3.79692, 11.9135, 5.19659, -0.71923, 7.77778, 12.2772, 
    0.837542, 10.2503, -0.226694, -6.01589, 11.4506, -2.82863, -0.321889, 3.79692, 11.9135, 5.19659, -0.71923, 7.77778, 12.2772, 
    0.837542, 10.2503, -0.226694, -6.01589, 11.4506, -2.82863, -0.321889, 3.79692, 11.9135, 5.19659, -0.71923, 7.77778, 12.2772, 
    // max depth of face down tableau [0,1,2,3,4,5,6]
    21.7513, 21.1423, 18.7634, -2.76381, -7.44494, -15.5304, -8.44869
  };
  for ( int i=0; i<NUM_UCT_FEATURES; i++ ) uct_feat_weights[i] = dummy[i];
}
*/



void zero_uct_feat_weights( double uct_feat_weights[NUM_UCT_WEIGHTS] )
{
  double dummy[NUM_UCT_WEIGHTS] = {
    // card in foundation [diam, club, hear, spad]
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    // in deck, cannot be moved [diam, club, hear, spad]
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    // in deck, can be moved to found [diam, club, hear, spad]
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    // in deck, can be moved to tab [diam, club, hear, spad]
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    // face up in tableau [diam, club, hear, spad]
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    // face down in tableau [diam, club, hear, spad]
    0,0,0,0,0,0,0,0,0,0,0,0,0,
    // max depth of face down tableau [0,1,2,3,4,5,6]
    0,0,0,0,0,0,0
  };
  for ( int i=0; i<NUM_UCT_WEIGHTS; i++ ) uct_feat_weights[i] = dummy[i];
}

void random_uct_feat_weights( double uct_feat_weights[NUM_UCT_WEIGHTS], MTRand * my_rand_gen )
{
  for ( int i=0; i<NUM_UCT_WEIGHTS; i++ ){
    uct_feat_weights[i] = ( (double)( my_rand_gen->randInt() % 999 ) + 1) / (double)1000;   // random weight [0.001 .. 0.999]
  }
}

void fill_state_features( State * s, double uct_features[NUM_UCT_FEATURES] )
{
  int i, j, feature_offset;
  int suit_card;
  int last_face_down, lastest_face_down;
  double num_face_down_cards;
  bool suit_card_in_foundation;
  bool ace_card;
  bool is_king_card;
  bool in_deck;
  int build_c_stack[2];
  int build_c_loc[2];
  bool deck_face_up[FULL_DECK];
  bool deck_face_down[FULL_DECK];
  bool build_c_in_tabl[2];
  bool build_c_is_top_tabl[2];
  bool empty_tabl;
  bool reveal_action;
  feature_offset = 0;

  for ( i=0; i<NUM_UCT_FEATURES; i++ ) uct_features[i] = 0.0;

  // 6 types of features (52 cards each) + max depth of face down cards in tableau stacks
  
  //  card in Foundation stack [52] uct_features[0..51]
  //  fill in for each card that is in a foundation stack 
  for ( i=0; i<FULL_DECK; i++ ){
    if ( s->cards[i]->stack_id >= STK_DIAM && s->cards[i]->stack_id <= STK_SPAD ) uct_features[feature_offset + i] = 1.0;
  }
  feature_offset += FULL_DECK;
  
  // card in deck, cannot be moved [52] uct_features[52..103]
  // card in deck, can be moved to Foundation [52] uct_features[104..155]
  // card in deck, can be moved to Tableau [52] uct_features[156..207]
  //  (these are all things we could see, so use the future state)
  for ( i=0; i<FULL_DECK; i++ ){
    in_deck = s->cards[i]->stack_id == STK_DECK;
    if ( in_deck  && ( !s->cards[i]->deck_playable ) ) uct_features[feature_offset + i] = 1.0;
  }
  feature_offset += FULL_DECK;
  
  // card in deck, can be moved to Foundation [52] uct_features[104..155]
  for ( i=0; i<FULL_DECK; i++ ){
    in_deck = s->cards[i]->stack_id == STK_DECK;
    ace_card = ( i % SUIT_SIZE == 0 );
    if ( !ace_card ){
      suit_card = s->cards[i]->suit_card;
      suit_card_in_foundation = ( s->cards[suit_card]->stack_id >= STK_DIAM && s->cards[suit_card]->stack_id <= STK_SPAD );
    }

    if ( in_deck && s->cards[i]->deck_playable && ( ace_card || suit_card_in_foundation ) ) uct_features[feature_offset + i] = 1.0;
  }
  feature_offset += FULL_DECK;
  
  // card in deck, can be moved to Tableau [52] uct_features[156..207]
  // check to make sure the cards we are moving to were already face up in the original state (no cheating, please)
  for ( i=0; i<FULL_DECK; i++ ){
    is_king_card = ( i % SUIT_SIZE == 12 );
    empty_tabl = (   s->first_empty[0] == 0 || s->first_empty[1] == 0 || s->first_empty[2] == 0 || s->first_empty[3] == 0 ||
        s->first_empty[4] == 0 || s->first_empty[5] == 0 || s->first_empty[6] == 0 );

    if ( ! is_king_card ){
      build_c_stack[0] = s->cards[s->cards[i]->build_cards[0]]->stack_id;
      build_c_stack[1] = s->cards[s->cards[i]->build_cards[1]]->stack_id;

      build_c_in_tabl[0] = ( build_c_stack[0] >= STK_B1 && build_c_stack[0] <= STK_B7 );
      build_c_in_tabl[1] = ( build_c_stack[1] >= STK_B1 && build_c_stack[1] <= STK_B7 );

      build_c_loc[0] = s->cards[s->cards[i]->build_cards[0]]->loc_in_stack;
      build_c_loc[1] = s->cards[s->cards[i]->build_cards[1]]->loc_in_stack;

      build_c_is_top_tabl[0] = ( ( s->first_empty[ build_c_stack[0] ]-1 == build_c_loc[0] ) && build_c_in_tabl[0] );
      build_c_is_top_tabl[1] = ( ( s->first_empty[ build_c_stack[1] ]-1 == build_c_loc[1] ) && build_c_in_tabl[1] );
    }

    in_deck = s->cards[i]->stack_id == STK_DECK;
    if (   in_deck  && 
      s->cards[i]->deck_playable && 
      ( ( is_king_card && empty_tabl ) || 
        ( !is_king_card && ( build_c_is_top_tabl[0] || build_c_is_top_tabl[1] ) ) ) ) uct_features[feature_offset + i] = 1.0;
  }
  feature_offset += FULL_DECK;
  
  num_face_down_cards = 0;
  for ( i=0; i<s->valid_actions_index; i++ ) if ( s->valid_actions[i]->available && s->valid_actions[i]->reveal ) reveal_action = true;
  for ( i=0; i<FULL_DECK; i++ ){
    deck_face_up[i] = s->cards[i]->stack_id >= STK_B1 && s->cards[i]->stack_id <= STK_B7 && s->cards[i]->face_up;
    deck_face_down[i] = s->cards[i]->stack_id >= STK_B1 && s->cards[i]->stack_id <= STK_B7 && !s->cards[i]->face_up;
    if ( deck_face_down[i] ) num_face_down_cards += 1.0;
  }

  // card face up in Tableau [52] uct_features[208..259]
  for ( i=0; i<FULL_DECK; i++ ){
    if ( deck_face_up[i] ) uct_features[feature_offset + i] = 1.0;
    else if ( num_face_down_cards > 0 && deck_face_down[i] && reveal_action ) uct_features[feature_offset + i] = 1.0 / num_face_down_cards;
  }
  feature_offset += FULL_DECK;
  
  // card face down in Tableau [52] uct_features[260..311]
  for ( i=0; i<FULL_DECK; i++ ){
    if ( deck_face_down[i] ){
      if ( num_face_down_cards > 0.0 && reveal_action ) uct_features[feature_offset + i] = ( num_face_down_cards - 1) / num_face_down_cards;
      else uct_features[feature_offset + i] = 1.0;
    }
  }
  feature_offset += FULL_DECK;
  
  // (ARCHAIC)
  // card face down in Tableau (reveal on this action) [52] uct_features[312..363]
  //  check to see if an action is available that will reveal a face down card, and if so,
  //  fill in cards that are currently face down in Tableau
  //reveal_action = false;
  //for ( i=0; i<FULL_DECK; i++ ){
    //uct_features[feature_offset + i] = uct_features[feature_offset - FULL_DECK + i] && reveal_action;
  //}
  //feature_offset += FULL_DECK;

  // max depth of face-down cards [7] uct_features[364..370]
  lastest_face_down = 0;
  for ( i=STK_B1; i<=STK_B7; i++) {
    last_face_down = 0;
    for ( j=0; j<s->first_empty[i]; j++ ) if ( !s->cards[s->all_stacks[i][j]]->face_up ) last_face_down = j+1;

    if ( last_face_down > lastest_face_down ) lastest_face_down = last_face_down;
  }
  // set the max to true, remembering that the others were initialized to false in the beginning
  uct_features[feature_offset + lastest_face_down] = 1.0;
}
void fill_uct_features( State * s, int action_index, double uct_features[NUM_UCT_FEATURES] )
{
  State * t_state = new State();
  int i, j, feature_offset;
  int suit_card;
  int last_face_down, lastest_face_down;
  bool suit_card_in_foundation;
  bool ace_card;
  bool is_king_card;
  bool in_deck;
  int build_c_stack[2];
  int build_c_loc[2];
  double num_face_down_cards = 0.0;
  bool build_c_in_tabl[2];
  bool build_c_is_top_tabl[2];
  bool empty_tabl;
  bool face_up_in_result[FULL_DECK];
  bool face_down_in_original[FULL_DECK];
  feature_offset = 0;

  for ( i=0; i<NUM_UCT_FEATURES; i++ ) uct_features[i] = 0.0;

  // we transition into a dummy state because the calculations for the effects of taking this action are 
  // already coded in this function - we aren't going to do any peeking that we shouldn't be doing 
  // (i.e. looking at cards that will be revealed)
  s->transition( t_state, action_index );

  // 7 types of features (52 cards each) + max depth of face down cards in tableau stacks
  
  // card in Foundation stack [52] uct_features[0..51]
  //  from the future state, fill in for each card that is in a foundation stack 
  //  (this is something we could see, so use the future state)
  for ( i=0; i<FULL_DECK; i++ ){
    if ( t_state->cards[i]->stack_id >= STK_DIAM && t_state->cards[i]->stack_id <= STK_SPAD ) uct_features[feature_offset + i] = 1.0;
  }
  feature_offset += FULL_DECK;
  
  // card in deck, cannot be moved [52] uct_features[52..103]
  // card in deck, can be moved to Foundation [52] uct_features[104..155]
  // card in deck, can be moved to Tableau [52] uct_features[156..207]
  //  (these are all things we could see, so use the future state)
  for ( i=0; i<FULL_DECK; i++ ){
    in_deck = t_state->cards[i]->stack_id == STK_DECK;
    if ( in_deck  && ( !t_state->cards[i]->deck_playable ) ) uct_features[feature_offset + i] = 1.0;
  }
  feature_offset += FULL_DECK;
  
  // card in deck, can be moved to Foundation [52] uct_features[104..155]
  for ( i=0; i<FULL_DECK; i++ ){
    in_deck = t_state->cards[i]->stack_id == STK_DECK;
    ace_card = ( i % SUIT_SIZE == 0 );
    if ( !ace_card ){
      suit_card = t_state->cards[i]->suit_card;
      suit_card_in_foundation = ( t_state->cards[suit_card]->stack_id >= STK_DIAM && t_state->cards[suit_card]->stack_id <= STK_SPAD );
    }

    if ( in_deck  && t_state->cards[i]->deck_playable && ( ace_card || suit_card_in_foundation ) ) uct_features[feature_offset + i] = 1.0;
  }
  feature_offset += FULL_DECK;
  
  // card in deck, can be moved to Tableau [52] uct_features[156..207]
  // check to make sure the cards we are moving to were already face up in the original state (no cheating, please)
  for ( i=0; i<FULL_DECK; i++ ){
    is_king_card = ( i % SUIT_SIZE == 12 );
    empty_tabl = (   t_state->first_empty[0] == 0 || t_state->first_empty[1] == 0 || t_state->first_empty[2] == 0 || t_state->first_empty[3] == 0 ||
        t_state->first_empty[4] == 0 || t_state->first_empty[5] == 0 || t_state->first_empty[6] == 0 );

    if ( ! is_king_card ){
      build_c_stack[0] = t_state->cards[t_state->cards[i]->build_cards[0]]->stack_id;
      build_c_stack[1] = t_state->cards[t_state->cards[i]->build_cards[1]]->stack_id;

      build_c_in_tabl[0] = ( build_c_stack[0] >= STK_B1 && build_c_stack[0] <= STK_B7 );
      build_c_in_tabl[1] = ( build_c_stack[1] >= STK_B1 && build_c_stack[1] <= STK_B7 );

      build_c_loc[0] = t_state->cards[t_state->cards[i]->build_cards[0]]->loc_in_stack;
      build_c_loc[1] = t_state->cards[t_state->cards[i]->build_cards[1]]->loc_in_stack;

      build_c_is_top_tabl[0] = ( ( t_state->first_empty[ build_c_stack[0] ]-1 == build_c_loc[0] ) && build_c_in_tabl[0] );
      build_c_is_top_tabl[1] = ( ( t_state->first_empty[ build_c_stack[1] ]-1 == build_c_loc[1] ) && build_c_in_tabl[1] );
    }

    in_deck = t_state->cards[i]->stack_id == STK_DECK;
    if (   in_deck  && 
      t_state->cards[i]->deck_playable && 
      ( ( is_king_card && empty_tabl ) || 
        ( !is_king_card && ( build_c_is_top_tabl[0] || build_c_is_top_tabl[1] ) ) ) )
    uct_features[feature_offset + i] = 1.0;
  }
  feature_offset += FULL_DECK;
  
  num_face_down_cards = 0;
  for ( i=0; i<FULL_DECK; i++ ){
    face_up_in_result[i] = ( t_state->cards[i]->stack_id >= STK_B1 && t_state->cards[i]->stack_id <= STK_B7 && t_state->cards[i]->face_up );
    face_down_in_original[i] = ( s->cards[i]->stack_id >= STK_B1 && s->cards[i]->stack_id <= STK_B7 && ( !s->cards[i]->face_up ) );
    if ( face_down_in_original[i] ) num_face_down_cards += 1.0;
  }
  // card face up in Tableau [52] uct_features[208..259]
  // fill in the probability that each card will be face up in the tableau following the action
  for ( i=0; i<FULL_DECK; i++ ){

    if ( face_up_in_result[i]  && ( !face_down_in_original[i] ) ) uct_features[feature_offset + i] = 1.0;
    else if ( num_face_down_cards > 0.0 && face_down_in_original[i] && s->valid_actions[action_index]->reveal ) uct_features[feature_offset + i] = (1.0 / num_face_down_cards );
  }
  feature_offset += FULL_DECK;
  
  
  // card face down in Tableau [52] uct_features[260..311]
  // fill in the probability that each card will be face down following the action
  for ( i=0; i<FULL_DECK; i++ ){
    if ( face_down_in_original[i] ){
      if ( num_face_down_cards > 0.0 && s->valid_actions[action_index]->reveal ) uct_features[feature_offset + i] = ( num_face_down_cards - 1) / num_face_down_cards;
      else uct_features[feature_offset + i] = 1.0;
    }
  }
  feature_offset += FULL_DECK;
  
  //(ARCHAIC)
  // card face down in Tableau (reveal on this action) [52] uct_features[312..363]
  //  check to see if a face-down card will be revealed on this move, and if so,
  //  fill in cards that are currently face down in Tableau
  //for ( i=0; i<FULL_DECK; i++ ){
    //uct_features[feature_offset + i] = uct_features[feature_offset - FULL_DECK + i] && s->valid_actions[action_index]->reveal;
  //}
  //feature_offset += FULL_DECK;

  // max depth of face-down cards [7] uct_features[364..370]
  lastest_face_down = 0;
  for ( i=STK_B1; i<=STK_B7; i++) {
    last_face_down = 0;
    for ( j=0; j<t_state->first_empty[i]; j++ ){
      if ( !t_state->cards[t_state->all_stacks[i][j]]->face_up ) last_face_down = j+1;
    }
    if ( last_face_down > lastest_face_down ) lastest_face_down = last_face_down;
  }
  // set the max to true, remembering that the others were initialized to 0.0 in the beginning
  uct_features[feature_offset + lastest_face_down] = 1.0;
  
  delete t_state;
}

void interpret_uct_features( double uct_features[NUM_UCT_FEATURES] )
{
  int i, feature_index, count, count2;

  for ( i=0, count=0, feature_index=0; i<FULL_DECK; i++, feature_index++ ) if ( uct_features[feature_index] == 1.0 ) count++;
  cout << count << " cards in the foundation stacks " << endl;

  for ( i=0, count=0; i<FULL_DECK; i++, feature_index++ ) if ( uct_features[feature_index] == 1.0 ) count++;
  cout << count << " cards in the deck that cannot be moved " << endl;

  for ( i=0, count=0; i<FULL_DECK; i++, feature_index++ ) if ( uct_features[feature_index] == 1.0 ) count++;
  cout << count << " cards in the deck that can be moved to the foundation" << endl;

  for ( i=0, count=0; i<FULL_DECK; i++, feature_index++ ) if ( uct_features[feature_index] == 1.0  ) count++;
  cout << count << " cards in the deck that can be moved to the tableau" << endl;

  for ( i=0, count=0, count2=0; i<FULL_DECK; i++, feature_index++ ) {
    if ( uct_features[feature_index] == 1.0  ) count++;
    if ( uct_features[feature_index] > 0.0 && uct_features[feature_index] < 1.0 ) count2++;
  }
  cout << count << " cards face up in the tableau" << endl;
  cout << count2 << " cards with a chance of being face up after the action " << endl;

  for ( i=0, count=0, count2=0; i<FULL_DECK; i++, feature_index++ ) {
    if ( uct_features[feature_index] == 1.0  ) count++;
    if ( uct_features[feature_index] > 0.0 && uct_features[feature_index] < 1.0 ) count2++;
  }
  cout << count << " cards face down in the tableau" << endl;
  cout << count2 << " cards with a chance of being face down after the action " << endl;


  for ( count=0; feature_index<NUM_UCT_FEATURES; feature_index++, count++ )
    if ( uct_features[feature_index] == 1.0 ) cout << " deepest face-down run depth of " << count << endl;
}

void train_td_lambda( State * s[MAX_SEARCH_DEPTH], double uct_features[NUM_UCT_FEATURES], double uct_weights[NUM_UCT_WEIGHTS], MTRand * my_rand_gen, double learning_rate )
{
  int base_depth = 0;
  int best_action;
  //double e_trace[NUM_UCT_FEATURES];
  double e_trace[NUM_UCT_WEIGHTS];
  double action_value, best_action_value, reward, delta;
  bool repeated_state;
  bool actions_available;
  double highest_weight_value;
  double summed_feature_weight;
  int i;

  for ( i=0; i<NUM_UCT_WEIGHTS; i++ ) e_trace[i] = 0.0;

  actions_available = true;
  while( !s[base_depth]->win() && actions_available && base_depth < MAX_SEARCH_DEPTH-1 ){
    actions_available = false;
    // find the best action according to our current policy
    for ( i=0; i<=s[base_depth]->valid_actions_index; i++ ){
      if ( s[base_depth]->valid_actions[i]->available ){
        action_value = uct_evaluate( s, base_depth, i, uct_features, uct_weights, repeated_state );
        if ( !repeated_state ){
          if ( !actions_available || ( action_value > best_action_value ) ){
            best_action_value = action_value;
            best_action = i;
          }
          actions_available = true;
        }
      }
    }  

    // take selected action
    if ( actions_available ){
      s[base_depth]->transition( s[base_depth+1], best_action );

      if ( s[base_depth+1]->win() ) reward = POS_REWARD;
      else if ( s[base_depth+1]->valid_actions_index == -1 ) reward = NEG_REWARD;
      else reward = STEP_PENALTY;

      delta = reward + ( GAMMA * state_value( s[base_depth+1], uct_weights ) ) - ( state_value( s[base_depth], uct_weights ) );

      fill_state_features( s[base_depth], uct_features );

      highest_weight_value = INT_MIN;
      //for ( i=0; i<NUM_UCT_FEATURES; i++ ) {
        //e_trace[i] = (GAMMA * LAMBDA * e_trace[i] ) + uct_features[i];
        //uct_weights[i] = uct_weights[i] + ( learning_rate * delta * e_trace[i] );
        //if ( uct_weights[i] > highest_weight_value ) highest_weight_value = uct_weights[i];
      //}
      for ( i=0; i<NUM_UCT_WEIGHTS; i++ ){
        summed_feature_weight =   uct_features[weight2feat_index(i,0)] +  uct_features[weight2feat_index(i,1)] +  
                uct_features[weight2feat_index(i,2)] +  uct_features[weight2feat_index(i,3)];
        e_trace[i] = (GAMMA * LAMBDA * e_trace[i] ) + summed_feature_weight;
        uct_weights[i] = uct_weights[i] + ( learning_rate * delta * e_trace[i] );
        if ( uct_weights[i] > highest_weight_value ) highest_weight_value = uct_weights[i];
      }

      // keep weights down to a reasonable size
      while ( highest_weight_value > 100 ){
        //cout << " dividing all weights by 10 because highest weight = " << highest_weight_value<< endl;
        highest_weight_value = INT_MIN;
        for ( i=0; i<NUM_UCT_WEIGHTS; i++ ) {
          uct_weights[i] = ( uct_weights[i] / 10.0 );
          if ( uct_weights[i] > highest_weight_value ) highest_weight_value = uct_weights[i];
        }
      }
    }
    base_depth++;
  }
  //cout << " out train_td_lambda " << endl;
}

/*
void train_epsilon_greedy( State * s[MAX_SEARCH_DEPTH], double uct_features[NUM_UCT_FEATURES], double uct_weights[NUM_UCT_WEIGHTS], MTRand * my_rand_gen )
{
  int base_depth = 0;
  int best_action;
  double action_value, best_action_value;
  double e_trace[NUM_UCT_FEATURES];
  double delta, reward;
  bool repeated_state;
  bool actions_available;
  int i;

  for ( i=0; i<NUM_UCT_FEATURES; i++ ) e_trace[i] = 0.0;

  actions_available = true;
  while( !s[base_depth]->win() && actions_available && base_depth < MAX_SEARCH_DEPTH-1 ){

    actions_available = false;
    best_action_value = INT_MIN;

    // with a chance of (1 - epsilon), choose an action based on the current weights
    if ( ( ( my_rand_gen->randInt() % 1000 ) / (double)1000 ) > EPSILON ){
      for ( i=0; i<=s[base_depth]->valid_actions_index; i++ ){
        if ( s[base_depth]->valid_actions[i]->available ){
          action_value = uct_evaluate( s, base_depth, i, uct_features, uct_weights, repeated_state );
          if ( !repeated_state ){
            if ( !actions_available || ( action_value > best_action_value ) ){
              best_action_value = action_value;
              best_action = i;
            }
            actions_available = true;
          }
        }
      }  
      // decay eligibility trace
      for ( i=0; i<NUM_UCT_FEATURES; i++ ) e_trace[i] = GAMMA * LAMBDA * e_trace[i];
    } else {
      // choose randomly
      best_action = randomActionIndex( s, base_depth, my_rand_gen );
      if ( best_action != -1 ) {
        actions_available = true;
        best_action_value = uct_evaluate( s, base_depth, best_action, uct_features, uct_weights, repeated_state );
      }

      // set eligibility tract to zero
      for ( i=0; i<NUM_UCT_FEATURES; i++ ) e_trace[i] = 0;
    }

    // take selected action
    if ( actions_available ){
      // update eligibility trace with current features
      fill_uct_features( s[base_depth], best_action, uct_features );
      for ( i=0; i<NUM_UCT_FEATURES; i++ ) if ( uct_features[i] ) e_trace[i] += 1.0;

      s[base_depth]->transition( s[base_depth+1], best_action );
      base_depth++;

      reward = ( s[base_depth]->win() ? 100.0 : 0.0 );
    } else reward = 0.0;

    if ( !actions_available ) best_action_value = -1.0;

    delta = reward - best_action_value;

    best_action_value = INT_MIN;
    actions_available = false;
    for ( i=0; i<=s[base_depth]->valid_actions_index; i++ ){
      if ( s[base_depth]->valid_actions[i]->available ){
        action_value = uct_evaluate( s, base_depth, i, uct_features, uct_weights, repeated_state );
        if ( !repeated_state ){
          if ( !actions_available || ( action_value > best_action_value ) ){
            best_action_value = action_value;
            best_action = i;
          }
        }
      }
    }  
    if ( ! actions_available ) best_action_value = -1.0;
    delta = delta + ( GAMMA + best_action_value );

    for ( i=0; i<NUM_UCT_FEATURES; i++ ) uct_weights[i] = uct_weights[i] + ( LEARNING_RATE * delta * e_trace[i] );
  }
}
*/

double uct_evaluate( State * s[MAX_SEARCH_DEPTH], int base_depth, int action_choice, double uct_features[NUM_UCT_FEATURES], double uct_weights[NUM_UCT_WEIGHTS], bool &repeated_state )
{
  int i;
  double value;

  fill_uct_features( s[base_depth], action_choice, uct_features );
  value = uct_weighted_sum( uct_features, uct_weights );

  // in this case, it's OK to check for repeats with the entire state, and not just the face-up cards (why?)
  // because the only instance we would violate our "can't see into the future" problem would be when we  reveal a new card
  // and if we reveal a new card, it can't possibly repeat a state already in the trajectory
  s[base_depth]->transition( s[base_depth+1], action_choice );
  repeated_state = false;
  for ( i=base_depth-1; i>=0 && repeated_state==false; i-- ){
    if ( s[base_depth+1]->isSame( s[i] ) ) repeated_state = true;
  }

  return value;
}

double uct_weighted_sum( double uct_features[NUM_UCT_FEATURES], double uct_weights[NUM_UCT_WEIGHTS] )
{
  double sum;
  int i, weight_index;

  sum = 0;
  for ( i=0; i<NUM_UCT_FEATURES; i++ ){
    weight_index = feat2weight_index( i );
    sum += uct_features[i] * uct_weights[weight_index];
  }
  return sum;

}

bool follow_greedy_policy( State * s[MAX_SEARCH_DEPTH], double uct_weights[NUM_UCT_WEIGHTS] )
{
  bool actions_available = true;
  double uct_features[NUM_UCT_FEATURES];
  bool repeated_state;
  int base_depth = 0;
  double action_value, best_action_value;
  int best_action;
  int i;

  while( !s[base_depth]->win() && actions_available && base_depth < MAX_SEARCH_DEPTH-1 ){

    actions_available = false;
    best_action_value = INT_MIN;

    // choose an action based on the feature weights
    for ( i=0; i<=s[base_depth]->valid_actions_index; i++ ){
      if ( s[base_depth]->valid_actions[i]->available ){
        action_value = uct_evaluate( s, base_depth, i, uct_features, uct_weights, repeated_state );
        if ( !repeated_state ){
          if ( !actions_available || ( action_value > best_action_value ) ){
            best_action_value = action_value;
            best_action = i;
          }
          actions_available = true;
        }
      }
    }  

    // take selected action
    if ( actions_available ){
      s[base_depth]->transition( s[base_depth+1], best_action );
      base_depth++;

      if ( s[base_depth]->win() ) return true;
    } else return false;
  }
  return false;
}

bool follow_random_policy( State * s[MAX_SEARCH_DEPTH], MTRand * my_rand_gen )
{
  int base_depth = 0;
  int action_taken;

  while ( base_depth < MAX_SEARCH_DEPTH-1 ){

    action_taken = takeRandomAction( s, base_depth, my_rand_gen );

    if ( action_taken == -1 ) return false;
    if ( s[++base_depth]->win() ) return true;
  }
  return false;
}

bool follow_first_action_policy( State * s[MAX_SEARCH_DEPTH] )
{
  int base_depth = 0;
  int action_taken;

  while ( base_depth < MAX_SEARCH_DEPTH-1 ){
    
    action_taken = firstActionIndex( s, base_depth );

    if ( action_taken == -1 ) return false;
    if ( s[++base_depth]->win() ) return true;
  }

  return false;

}

double state_value( State * s, double feat_weights[NUM_UCT_WEIGHTS] )
{
  double state_features[NUM_UCT_FEATURES];
  fill_state_features( s, state_features );
  return uct_weighted_sum( state_features, feat_weights );

}

int feat2weight_index( int feat_index )
{
  int feat_type, rank;
  if ( feat_index > 311 ) return 78 - 312 + feat_index;
  else {
    feat_type = feat_index / FULL_DECK;
    rank = feat_index % SUIT_BREAK;
  }
  return ( feat_type * SUIT_BREAK ) + rank;
}

int weight2feat_index( int weight_index, int suit_index )
{
  int feat_type, rank;
  if ( weight_index > 77 ) return 312 - 78 + weight_index;
  else {
    feat_type = weight_index / SUIT_BREAK;
    rank = weight_index % SUIT_BREAK;
  }
  return ( feat_type * FULL_DECK ) + ( suit_index * SUIT_BREAK ) + rank;
}

