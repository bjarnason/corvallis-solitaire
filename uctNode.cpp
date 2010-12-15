/*
 *  compactState.cpp
 *  solitaire
 *
 *  Created by Ronald Bjarnason on Mon Jan 29 2007.
 *  Copyright (c) 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "uctNode.h"
#include "global.h"

using namespace std;

UCTNode::UCTNode()
{
	num_visits = 0;
	all_actions_tried = false;

	for ( int i=0; i<MAX_NUM_ACTIONS; i++ ){
		action_values[i] = 0.0;
		action_visits[i] = 0;
		last_child_index[i] = -1;
	}

	c_state = new CompactState();
}

UCTNode::~UCTNode()
{
	for ( int i=0; i<MAX_NUM_ACTIONS; i++ ){
		for ( int j=0; j<=last_child_index[i]; j++ ) delete children[i][j];
	}
	delete c_state;
}

bool UCTNode::canAddChild( int entry_action )
{
	return ( last_child_index[entry_action]+1 < SPARSE_BRANCHING_FACTOR );
}

bool UCTNode::addChild( int entry_action )
{
	if ( last_child_index[entry_action]+1 >= SPARSE_BRANCHING_FACTOR ) return false;

	children[entry_action][++last_child_index[entry_action]] = new UCTNode();
	//cout << (unsigned long int)this << " : creating a new child for action " << entry_action << endl;

	return true;
}

int UCTNode::repeatFaceUpChild( int entry_action, CompactState * cs )
{
	int i;
	for ( i=0; i<=last_child_index[entry_action]; i++ ){
		if ( children[entry_action][i]->c_state->isFaceUpSame( cs ) ) return i;
	}
	return -1;
}

int UCTNode::numChildren( int action )
{
	return last_child_index[action]+1;
}

bool UCTNode::isSame( UCTNode * other_node )
{
	return c_state->isSame( other_node->c_state );
}

void UCTNode::init()
{
	int i, j;
	num_visits = 0;
	for ( i=0; i<MAX_NUM_ACTIONS; i++ ){
		action_values[i] = 0;
		action_visits[i] = 0;
		for ( j=0; j<=last_child_index[i]; j++ ){
			delete children[i][j];
		}
		last_child_index[i] = -1;
	}
	all_actions_tried = false;
}

void UCTNode::cleanUp()
{
	int i, j;
	num_visits = 0;
	for ( i=0; i<MAX_NUM_ACTIONS; i++ ){
		action_values[i] = 0;
		action_visits[i] = 0;
		for ( j=0; j<=last_child_index[i]; j++ ){
			delete children[i][j];
		}
		last_child_index[i] = -1;
	}
	all_actions_tried = false;
}

int UCTNode::size()
{
	int temp = 0;
	for ( int i=0; i<MAX_NUM_ACTIONS; i++ ) {
		for ( int j=0; j<=last_child_index[i]; j++ ){
			temp += children[i][j]->size();
		}
	}
	return temp+1;
}



