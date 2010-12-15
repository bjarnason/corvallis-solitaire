/*
 *  csTree.h
 *  solitaire
 *
 *  Created by Ronald Bjarnason on Thu Mar 15 2007.
 *  Copyright (c) 2007 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef CSTREE_H
#define CSTREE_H

#include <Carbon/Carbon.h>
#include "compactState.h"


class CSTree{
public:
	
	CSTree();
	~CSTree();
	
	CSTree * left;
	CSTree * right;
	CompactState * item;
};

#endif
