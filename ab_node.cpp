#include "ab_node.h"


AB_Node::AB_Node(Board cpy, int index): move_index{index}, value{0}, state{cpy}
{
}

AB_Node::~AB_Node()
{
	for (auto child : this->children) {
		delete child;
	}
}

bool AB_Node::is_leaf()
{
	return this->state.gameover();
}

void AB_Node::expand()
{
	// check if the node has previously been expanded
	if (this->children.size()) {
		return;
	}
	AB_Node *copy;
	int counter = 0;

	if (this->state.state == SWAP) {
		auto swaps = this->state.generate_swaps();
		for (auto &swap : swaps) {
			copy = new AB_Node(Board(this->state), counter++);
			copy->state.apply_swap(swap.first, swap.second);
			this->children.emplace_back(copy);
		}
	} else {
		auto actions = this->state.generate_actions();
		for (auto &act : actions) {
			copy = new AB_Node(Board(this->state), counter++);
			copy->state.apply_action(act);
			this->children.emplace_back(copy);
		}
	}
}
