/*
 *  main.h
 *  solitaire
 *
 *  Created by Ronald Bjarnason on Tue Jan 16 2007.
 *  Copyright (c) 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MAIN_H
#define MAIN_H

//#include <Carbon/Carbon.h>
#include <iostream>
//#include <unistd.h>
#include "state.h"

#define MIN_SAMPLE_SIZE			20

#define TRAINING_SET_SIZE		10
#define TEST_SET_SIZE			1000
#define BEAM_SIZE			12
#define FINAL_TEST_SIZE			250

#define DEPTH_LIMIT			3
#define NO_LIMIT			1000
#define NO_FRONTIER			1000

#define UCT_CONST			1.0

#define AVG_REW_UCT_UPDATE		1
#define	E_MAX_UCT_UPDATE		2
 
#define GAMMA				0.99
#define LAMBDA				0.99
#define EPSILON				0.01
#define LEARNING_RATE			0.01
#define LEARNING_STEP			0.1

#define POS_REWARD			10.0
#define NEG_REWARD			-1.0
#define	STEP_PENALTY			-0.001	

#define Z_90				1.65
#define Z_95				1.96
#define Z_99				2.58

#define E_CONSTANT			2.71828

#define POS_MARGIN			5
#define NEG_MARGIN			-5
#define REWARD				10.0

#define RULE_FREEDOM			1
#define POLICY_SIZE			30
#define PERCEPTRON_LEARNING		true
#define PROGRESS_LEARNING		false
#define USE_AVERAGE_WEIGHT		true

#define POSITIVE_VALUE			1
#define NEGATIVE_VALUE			-1

#define ROLLOUT_TEST_LEVEL		1
#define MAX_SEARCH_DEPTH		500

#define TIME_LIMIT			9
#define CHECK_TIME			false

#define VISITED_STATES			5000
#define REPEATED_CHECK_DEPTH		MAX_SEARCH_DEPTH
#define CHECK_REPEAT			true
#define OUTPUT				false

#define LEARNING_EPOCH_SIZE		1
#define TEST_INTERVAL			50
#define INITIAL_WEIGHT_VALUE		10.0
#define MAX_WEIGHT			10000.0
#define MIN_WEIGHT			0.0

#define MAX_POLICIES			3

#define MAX_FEATURES			20000
#define NUM_UNIT_FEATURES		( NUM_CARDS * NUM_CARDS * NUM_BASE_RELATIONS ) + ( NUM_CARDS * NUM_BASE_PROPOSITIONS )

#define LEVEL_ZERO				0
#define LEVEL_ONE				1
#define LEVEL_TWO				2
#define LEVEL_THREE				3
#define LEVEL_FOUR				4

#define FEATURE_ROLLOUT			0

#define INCLUDE_SIMPLE_PROPOSITIONS		true
#define START_WITH_ALL_PROPOSITIONS		false
#define INCLUDE_SIMPLE_RELATIONS		true
#define START_WITH_ALL_RELATIONS		false
#define START_RANDOM					false
#define NUM_RANDOM_FEATURES				400
#define USE_TOP_K_FEATURES				false
#define TOP_K_FEATURES					50

#define TOTAL_PROPS						572
#define TOTAL_RELS						8112
#define TOTAL_SUM						8684

#define HAND_CODED_POLICY_SIZE			154
#define USE_HAND_CODED				true

#define MAX_ROLLOUT				10
#define TEST_LESSER				false

#define DECAY_LEARNING_RATE			false
#define RANDOM_RESTARTS				false
#define CHECK_NEGATIVE				false

#define ROLLOUT_RESORT_GREEDY			false



class RollOutPolicyInfo
{
public:
	RollOutPolicyInfo()
	{
		int i;
		for ( i=0; i<VISITED_STATES; i++ )
		{
			v[i] = new CompactState();
		}
		v_index = -1;
		for ( i=0; i<NUM_HCP_FEATURES; i++ )
		{
			policy[i] = new HCFeature();
		}
		h_level = -1;
		frontier = NO_FRONTIER;
		search_depth = NO_LIMIT;
	}
	
	~RollOutPolicyInfo()
	{
		int i;
		for ( i=0; i<VISITED_STATES;i++ )
		{
			delete v[i];
		}
		for ( i=0; i<NUM_HCP_FEATURES; i++ )
		{
			delete policy[i];
		}
		
	}
	
	void prep(int h, int d, int f )
	{
		v_index = -1;
		h_level = h;
		search_depth = d;
		frontier = f;
	}
	void prep( int h, int d )
	{
		v_index = -1;
		h_level = h;
		search_depth = d;
		//cout << "initialized heuristic without a specified frontier" << endl;
	}

	void prep( int h )
	{
		v_index = -1;
		h_level = h;
		//cout << "initialized heuristic without limited depth and without specified frontier" << endl;
	}

	void copyInto( RollOutPolicyInfo * newP )
	{
		int i;
		newP->h_level = h_level;
		for ( i=0; i<VISITED_STATES; i++ )
		{
			v[i]->copyInto( newP->v[i] );
		}
		newP->v_index = v_index;
		for ( i=0; i<NUM_HCP_FEATURES; i++ )
		{
			policy[i]->copyInto( newP->policy[i] );
		}
		newP->policy_index = policy_index;
		newP->search_depth = search_depth;
	}

	int id;
	int h_level;
	int search_depth, frontier;
	CompactState * v[VISITED_STATES];
	int v_index;
	HCFeature * policy[NUM_HCP_FEATURES];
	int policy_index;
};

struct rollOutStaticInfo
{
	State * s[MAX_SEARCH_DEPTH];
	double s_vals[MAX_SEARCH_DEPTH];
	int h_vals[MAX_SEARCH_DEPTH];
	int p_vals[MAX_SEARCH_DEPTH];
	time_t start;
	bool check_time;
	int & last_node_index;
	bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK];
	bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK];
	int winning_path[MAX_SEARCH_DEPTH];
	bool note_path;
	int greedy_path[MAX_SEARCH_DEPTH];
	bool note_greedy_path;
};

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
				int which_p, int frontier );
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
				int which_p );
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
				State * t_state, MTRand * my_rand_gen );
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
				State * t_state, MTRand * my_rand_gen );
State * rollOutAnnotated( State * s[MAX_SEARCH_DEPTH], int base_depth, 
				RollOutPolicyInfo * p,
				double s_vals[MAX_SEARCH_DEPTH], int h_vals[MAX_SEARCH_DEPTH], int p_vals[MAX_SEARCH_DEPTH],
				int &last_node_index, 
				bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
				bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK], 
				int winning_path[MAX_SEARCH_DEPTH],
				int winning_search[MAX_SEARCH_DEPTH],
				State * t_state, State * b_state );
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
			  	MTRand * my_rand_gen );
bool rollOutCheckTime( bool check_time, time_t start );
bool rollOutCheckWin( State * s[MAX_SEARCH_DEPTH], int state_index );
bool rollOutCheckWin( State * s[MAX_SEARCH_DEPTH], int state_index, bool note_path, int &last_node_index );
bool rollOutEndCase( State * s[MAX_SEARCH_DEPTH], int state_index, CompactState *v[VISITED_STATES], int &v_index, int h_level );
bool rollOutEndCase( State * s[MAX_SEARCH_DEPTH], int state_index, bool note_path, int &last_node_index, 
					 CompactState * v[VISITED_STATES], int & v_index, int h_level );
int rollOutCheckLoop( State * s[MAX_SEARCH_DEPTH], int state_index, double s_vals[MAX_SEARCH_DEPTH], int h_vals[MAX_SEARCH_DEPTH], int p_vals[MAX_SEARCH_DEPTH] );
int rollOutCheckLoop( State * s[MAX_SEARCH_DEPTH], int state_index, bool note_path, int &last_node_index,
					   double svals[MAX_SEARCH_DEPTH], int h_vals[MAX_SEARCH_DEPTH], int p_vals[MAX_SEARCH_DEPTH] );
bool rollOutEndCase( State * s[MAX_SEARCH_DEPTH], int state_index, bool note_path, int &last_node_index, 
					CompactState * v[VISITED_STATES], int & v_index, int h_level,
					int winning_path[MAX_SEARCH_DEPTH], int losing_path[MAX_SEARCH_DEPTH], int &losing_path_index, int current_path[MAX_SEARCH_DEPTH] );
int rollOutCheckLoop( State * s[MAX_SEARCH_DEPTH], int state_index, bool note_path, int &last_node_index,
					double svals[MAX_SEARCH_DEPTH], int h_vals[MAX_SEARCH_DEPTH], int p_vals[MAX_SEARCH_DEPTH],
					int winning_path[MAX_SEARCH_DEPTH], int losing_path[MAX_SEARCH_DEPTH], int &losing_path_index, int current_path[MAX_SEARCH_DEPTH] );
void printSolution( State * s[MAX_SEARCH_DEPTH], int base_depth );
void storeSolution( State * s[MAX_SEARCH_DEPTH], int base_depth, int winning_path[TRAINING_SET_SIZE][MAX_SEARCH_DEPTH], int training_game );
bool alreadyVisited( CompactState * visited[VISITED_STATES], int &visited_index, State * test_state, int roll_out_level );
int fillTrainingStatesFromFile( string in_file, State ** in_states );
void fillConstRelations( bool c[NUM_CONST_RELATIONS][FULL_DECK][FULL_DECK] );
double stateValue( State * s, 
				   bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
				   bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK], 
				   HCFeature * hc_feats[MAX_FEATURES], int hc_feat_index );
bool featureOn( Feature * f,
				bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
				bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK] );
bool hcFeatureOn( HCFeature * f, bool hcp[NUM_HCP_FEATURES] );
bool isLegalConjunction( Feature * f1, Feature * f2, bool const_rel[NUM_CONST_RELATIONS][FULL_DECK][FULL_DECK] );
bool isCardUsed( int cardIndex );
double entropy( int numPos, int numNeg );
bool checkDifferences( Feature * base[BEAM_SIZE], int share_limit );
void fillHandCodedPolicy( Feature * p[HAND_CODED_POLICY_SIZE] );
int getHCPFeatureIndex( int feat_index, int card1, int card2 );
int getOSBIndex( int card_mod );
void HCPGetFeature( int HCP_index, int & feature, int & card1, int & card2 );
int getOSBCard( int index );
void initHandCodedHCPolicy( HCFeature * hcPolicy[595] );
void initHandCodedHCPolicy2( HCFeature * hcPolicy[595] );
void initEmptyHCPolicy( HCFeature * hcPolicy[595] );
void initRandomPolicy( HCFeature * policy[595], MTRand * my_rand_gen );
void printPolicy( HCFeature *policy[595] );
int greedyActionIndex( State * s, 
		   bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
		   bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK], 
		   HCFeature * hc_feats[MAX_FEATURES], int hc_feat_index );
int greedyActionIndex( State * s[MAX_SEARCH_DEPTH], int base_depth, double uct_weights[NUM_UCT_WEIGHTS] );
int firstActionIndex( State * s[MAX_SEARCH_DEPTH], int search_depth );
int randomActionIndex( State *s[MAX_SEARCH_DEPTH], int search_depth, MTRand * my_rand_gen );
int randomActionIndex( State * s, MTRand * my_rand_gen, bool random_action_index[MAX_NUM_ACTIONS] );
int randomUnchosenActionIndex( State * s, MTRand * my_rand_gen, bool non_repeat_actions[MAX_NUM_ACTIONS] );
int firstUnchosenActionIndex( State * s[MAX_SEARCH_DEPTH], int search_depth, bool non_repeat_actions[MAX_NUM_ACTIONS] );
int greedyUnchosenActionIndex( State * s[MAX_SEARCH_DEPTH], int base_depth, double uct_weights[NUM_UCT_WEIGHTS], bool non_repeat_actions[MAX_NUM_ACTIONS] );
int epsilonTakePolicyAction( State * s[MAX_SEARCH_DEPTH], int search_depth, HCFeature* hc_feats[MAX_FEATURES], int hc_feat_index,
		 bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
		 bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK],
		 MTRand * my_rand_gen );
int takePolicyAction( State * s[MAX_SEARCH_DEPTH], int search_depth, HCFeature* hc_feats[MAX_FEATURES], int hc_feat_index,
		 bool base_prop[NUM_BASE_PROPOSITIONS][FULL_DECK],
		 bool base_rela[NUM_BASE_RELATIONS][FULL_DECK][FULL_DECK] );
int takeRandomAction( State * s[MAX_SEARCH_DEPTH], int search_depth, MTRand * my_rand_gen );

bool uctPolicy( State * s[MAX_SEARCH_DEPTH], UCTNode * uctRoot, MTRand * my_rand_gen, State * t_state, State * t_state2, int update_method, int default_policy, double uct_weights[NUM_UCT_WEIGHTS] );
bool hOptUctPolicy( State * s[MAX_SEARCH_DEPTH], UCTNode * uctRoot, MTRand * my_rand_gen, State * t_state, State * t_state2, 
		int update_method, int default_policy, double uct_weights[NUM_UCT_WEIGHTS] );
int buildUctTree( State * s[MAX_SEARCH_DEPTH], int base_depth, UCTNode * uctRoot, MTRand * my_rand_gen, State * t_state, 
		int update_method, int default_policy, double uct_weights[NUM_UCT_WEIGHTS] );
bool sparseUctRollOut( State * s[MAX_SEARCH_DEPTH], int &uct_nodes_index, MTRand * my_rand_gen, 
		int base_depth, int &depth_offset, bool &win, State * t_state, int default_policy, double uct_weights[NUM_UCT_WEIGHTS] );
int makeSparseUCTDecision( State * s[MAX_SEARCH_DEPTH], int depth, MTRand * my_rand_gen, State * t_state, int default_policy, double uct_weights[NUM_UCT_WEIGHTS] );
void updateUCTTrace( State * s[MAX_SEARCH_DEPTH], int base_depth, int depth_offset, double reward );
void expectimaxUpdateUCTTrace( State * s[MAX_SEARCH_DEPTH], int base_depth, int depth_offset, double reward );
int makeUCTRolloutDecision( State * cur_state, MTRand * my_rand_gen );
int makeUCTPolicyDecision( State * cur_state );
bool uctNodeUnique( UCTNode * uctRoot, int uct_nodes_index, int &repeated_node );
double getUCTValue( UCTNode * uctNode, int action );
bool greedyFinish ( State * s[MAX_SEARCH_DEPTH], int base_depth );
int firstAvailableAction( State *s );

double uctUpper( int state_visited, int action_taken, double score, double c );
double wilsonUpper( int trials, int wins, double z_score );
int chooseUpper( int num_samples[MAX_NUM_ACTIONS], int num_wins[MAX_NUM_ACTIONS], bool valid_actions[MAX_NUM_ACTIONS], double z_score );

//void default_uct_feat_weights( double uct_feat_weights[NUM_UCT_FEATURES] );
//void td_learned_weights( double uct_feat_weights[NUM_UCT_FEATURES] );
//void td17_learned_weights( double uct_feat_weights[NUM_UCT_FEATURES] );
void zero_uct_feat_weights( double uct_feat_weights[NUM_UCT_FEATURES] );
void random_uct_feat_weights( double uct_feat_weights[NUM_UCT_FEATURES], MTRand * my_rand_gen );

void fill_state_features( State * s, double uct_features[NUM_UCT_FEATURES] );
void fill_uct_features( State * s, int action_index, double uct_features[NUM_UCT_FEATURES] );
void interpret_uct_features( double uct_features[NUM_UCT_FEATURES] );
void train_td_lambda( State * s[MAX_SEARCH_DEPTH], double uct_features[NUM_UCT_FEATURES], double uct_weights[NUM_UCT_WEIGHTS], MTRand * my_rand_gen, double learning_rate );
void train_epsilon_greedy( State * s[MAX_SEARCH_DEPTH], double uct_features[NUM_UCT_FEATURES], double uct_weights[NUM_UCT_WEIGHTS], MTRand * my_rand_gen );
double uct_evaluate( State * s[MAX_SEARCH_DEPTH], int base_depth, int action_choice, double uct_features[NUM_UCT_FEATURES], double uct_weights[NUM_UCT_WEIGHTS], bool &repeated_state );
double uct_weighted_sum( double uct_features[NUM_UCT_FEATURES], double uct_weights[NUM_UCT_WEIGHTS] );
bool follow_greedy_policy( State * s[MAX_SEARCH_DEPTH], double uct_weights[NUM_UCT_WEIGHTS] );
bool follow_random_policy( State * s[MAX_SEARCH_DEPTH], MTRand * my_rand_gen );
bool follow_first_action_policy( State * s[MAX_SEARCH_DEPTH] );
double state_value( State * s, double feat_weights[NUM_UCT_WEIGHTS] );
int feat2weight_index( int feat_index );
int weight2feat_index( int weight_index, int suit_index );

#endif
