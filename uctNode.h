/*
 * uctNode.h
 * solitaire
 *
 * Created by Ronald Bjarnason on Mon Mar 2 2009.
 * Copyright (c) 2009.  All rights reserved.
 *
 */
#ifndef UCTNODE_H
#define UCTNODE_H

#include "global.h"
#include "compactState.h"

class UCTNode{
public:
	UCTNode();
	~UCTNode();

	bool canAddChild( int entry_action );
	bool addChild( int entry_action );
	int repeatFaceUpChild( int entry_action, CompactState * cs );
	int numChildren( int action );
	bool isSame( UCTNode * other_node );
	void init();
	void cleanUp();
	int size();

	// our children in the UCT tree
	UCTNode * children[MAX_NUM_ACTIONS][SPARSE_BRANCHING_FACTOR];

	// because we don't store which State this node is associated with, we store a compact version
	CompactState * c_state;

	// the number of times this node has been visited
	int num_visits;

	// the value of visiting each of the actions
	double action_values[MAX_NUM_ACTIONS];

	// the number of times we have visited each action
	int action_visits[MAX_NUM_ACTIONS];

	// true if all actions have been tried
	bool all_actions_tried;

	// which actions are valid from this node
	// based on state->valid_actions->available
	// and actions that repeat previously visited states
	bool valid_actions[MAX_NUM_ACTIONS];

	// if we update the values of actions in the style of an expectimax tree,
	// we need to explicitly store the expectimax value of states
	double expectimax_value;
	
private:
	int last_child_index[MAX_NUM_ACTIONS];
};

#endif
