#pragma once

#include "board.h"


struct AB_Node {
	float value;
	Board state;
	std::vector<AB_Node *> children;

	AB_Node(Board state);

	bool is_leaf();

	void expand();
};
