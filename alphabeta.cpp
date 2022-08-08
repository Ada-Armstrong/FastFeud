#include "alphabeta.h"


float heuristic(AB_Node *node)
{
	float value = 0, points;
	std::pair<Team, Win_Condition> winner_info = node->state.winner();

	if (winner_info.first == BLACK) {
		value = std::numeric_limits<float>::infinity();
	} else if (winner_info.first == WHITE) {
		value = -std::numeric_limits<float>::infinity();
	} else if (winner_info.second == NO_WINNER) {
		value = 0;
		for (auto &tile : node->state.tile_info()) {
			if (tile.hp <= 0) {
				continue;
			}
			points = tile.active ? 2 * tile.hp : tile.hp;
			value += points * (tile.team == BLACK ? 1 : -1);
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
		val = -std::numeric_limits<float>::infinity();
		for (auto &child : node->children) {
			val = std::max(val, alphabeta(child, depth - 1, alpha, beta, maximizing));
			alpha = std::max(alpha, val);
			if (alpha >= beta)
				break;
		}
		node->value = val;
	} else {
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

	alphabeta(root, depth, -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), state.to_play);

	// find child with largest value
	int index = 0;
	float val = -std::numeric_limits<float>::infinity();

	for (auto child : root->children) {
		if (child->value > val) {
			val = child->value;
			index = child->move_index;
		}
	}

	delete root;

	return index;
}
