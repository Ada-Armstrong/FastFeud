#pragma once

#include "board.h"


struct AB_Node {
	int move_index; // index of move used to get to this node
	float value;
	Board state;
	std::vector<AB_Node *> children;

	AB_Node(Board state, int index = -1);

	~AB_Node();
	/* Description: returns true if the node is a leaf (gameover state).
	 * Args: None
	 */
	bool is_leaf();
	/* Description: populates the children vector with the states that can be reached
	 * 		from the current state.
	 * Args: None
	 */
	void expand();
};
