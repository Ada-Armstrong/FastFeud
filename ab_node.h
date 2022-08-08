#pragma once

#include "board.h"


struct AB_Node {
	int move_index; // index of move used to get to this node
	float value;
	Board state;
	std::vector<AB_Node *> children;

	AB_Node(Board state, int index = -1);

	~AB_Node();

	bool is_leaf();

	void expand();
};
