#include <limits>
#include <algorithm>
#include "ab_node.h"


/* Description: returns the index of a move for the current board state using
 * 		alpha beta pruning with iterative deepening depth first search.
 * Args: state - the board state to return a move for.
 * 	 depth - how deep into the game tree to search.
 */
int suggest_move(Board &state, int depth);
