#include "ab_node.h"


AB_Node::AB_Node(Board cpy): state{cpy}, value{0}
{
}

bool AB_Node::is_leaf()
{
	return this->state.gameover();
}

void AB_Node::expand()
{
	AB_Node *copy;

	if (this->state.state == SWAP) {
		auto swaps = this->state.generate_swaps();
		for (auto &swap : swaps) {
			copy = new AB_Node(Board(this->state));
			copy->state.apply_swap(swap.first, swap.second);
			this->children.emplace_back(copy);
		}
	} else {
		auto actions = this->state.generate_actions();
		for (auto &act : actions) {
			copy = new AB_Node(Board(this->state));
			copy->state.apply_action(act);
			this->children.emplace_back(copy);
		}
	}
}
