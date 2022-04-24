#include <limits>
#include "board.h"

struct AB_Node {
	float value;
	Board state;
	std::vector<std::unqiue_ptr<AB_Node>> children;

	bool is_leaf()
	{
		return this->state.gamover();
	}

	void expand()
	{
		std::unique_ptr<Board> copy;

		if (this->state.state == SWAP) {
			auto swaps = this->state.generate_swaps();
			for (auto &swap : swaps) {
				copy = std::unique_ptr<Board>(new Board(this->state));
				copy->apply_swap(swap.first, swap.second);
				this->children.emplace_back(copy);
			}
		} else {
			auto actions = this->state.generate_actions();
			for (auto &act : actions) {
				copy = std::unique_ptr<Board>(new Board(this->state));
				copy->apply_action(act);
				this->children.emplace_back(act);
			}
		}
	}
};

AB_Node alphabeta(AB_Node &node, int depth, float alpha, float beta, Team maximizing);
