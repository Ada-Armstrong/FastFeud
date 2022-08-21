#include "alphabeta.h"


float heuristic(AB_Node *node)
{
	float value = 0, points;
	std::pair<Team, Win_Condition> winner_info = node->state.winner();
	static const float multipliers[NUM_PIECES] = {1.0, 0.9, 0.6, 0.7, 0.7, 0.65};

	if (winner_info.first == BLACK) {
		value = std::numeric_limits<float>::infinity();
	} else if (winner_info.first == WHITE) {
		value = -std::numeric_limits<float>::infinity();
	} else if (winner_info.second == NO_WINNER) {
		value = 0;
		auto tiles = node->state.tile_info();
		for (int pos = 0; pos < BOARD_SIZE; ++pos) {
			auto &tile = tiles[pos];
			if (tile.hp <= 0) {
				continue;
			}
			points = (tile.active ? 1.5 * tile.hp : tile.hp) + node->state.num_friendly_neighbours(pos) * 0.5;
			value += multipliers[tile.type] * points * (tile.team == BLACK ? 1 : -1);
		}
	}

	return value;
}


float alphabeta(AB_Node *node, int depth, float alpha, float beta, Team maximizing)
{
	if (depth <= 0 or node->is_leaf()) {
		node->value = heuristic(node) * (maximizing == BLACK ? 1 : -1);
		return node->value;
	}

	float val;
	node->expand();

	if (node->state.to_play == maximizing) {
		// sort children based on most promising from previous searches, in this case from greatest to smallest value
		std::sort(node->children.begin(), node->children.end(), [](AB_Node *a, AB_Node *b) { return a->value > b->value; });

		val = -std::numeric_limits<float>::infinity();
		for (auto &child : node->children) {
			val = std::max(val, alphabeta(child, depth - 1, alpha, beta, maximizing));
			alpha = std::max(alpha, val);
			if (alpha >= beta)
				break;
		}
		node->value = val;
	} else {
		// sort children based on most promising from previous searches, in this case from smallest to greatest value
		std::sort(node->children.begin(), node->children.end(), [](AB_Node *a, AB_Node *b) { return a->value < b->value; });
		
		val = std::numeric_limits<float>::infinity();
		for (auto &child : node->children) {
			val = std::min(val, alphabeta(child, depth - 1, alpha, beta, maximizing));
			beta = std::min(beta, val);
			if (beta <= alpha)
				break;
		}
		node->value = val;
	}

	return val;
}

int suggest_move(Board &state, int depth)
{
	AB_Node *root = new AB_Node{state};

	// for each empty tile add one to depth
	for (auto &tile : state.tile_info()) {
		depth += tile.hp <= 0;
	}

	// use iterative deepening depth first search
	for (int d = 0; d <= depth; ++d) {
		int ret = alphabeta(root, d, -std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity(), state.to_play);
		// found a winning position or all positions are losing
		if (ret == std::numeric_limits<float>::infinity() || ret == -std::numeric_limits<float>::infinity()) {
			break;
		}
	}

	auto max_child = *std::max_element(root->children.begin(), root->children.end(), 
			[](AB_Node *a, AB_Node *b) { return a->value < b->value; });
	const int index = max_child->move_index;
	delete root;

	return index;
}
