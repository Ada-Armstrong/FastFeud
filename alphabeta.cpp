#include "alphabeta.h"


float heuristic(AB_Node *node)
{
	float value = 0;
	Team winner = node->state.winner();

	if (winner == BLACK) {
		value = std::numeric_limits<float>::infinity();
	} else if (winner == WHITE) {
		value = -std::numeric_limits<float>::infinity();
	} else {
		value = 0;
	}

	return value;
}


float alphabeta(AB_Node *node, int depth, float alpha, float beta, Team maximizing)
{
	if (depth <= 0 or node->is_leaf()) {
		node->value = heuristic(node);
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
