/*
 *  csTree.cpp
 *  solitaire
 *
 *  Created by Ronald Bjarnason on Thu Mar 15 2007.
 *  Copyright (c) 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "csTree.h"


CSTree::CSTree()
{
	left = NULL;
	right = NULL;
	
	item = new CompactState();
}

CSTree::~CSTree()
{
	delete item;
}

