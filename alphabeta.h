#include <limits>
#include <algorithm>
#include "ab_node.h"

struct Move {
	union data {
		action act;
		std::pair<int_fast16_t, int_fast16_t> swap;
	};
	Turn_T type;
};

int suggest_move(Board &state, int depth);
