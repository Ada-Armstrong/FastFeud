#include "board.h"

#ifdef __GNUC__
#	define ffs(x) __builtin_ffs(x)
#endif

bool Board::inbound(int_fast8_t pos)
{
	return 0 <= pos && pos < BOARD_SIZE;
}

void Board::compute_neighbours()
{
	const int_fast8_t dx[] = {-1, 0, 1, 0};
	const int_fast8_t dy[] = {0, 1, 0, -1};
	int_fast8_t x, y, x1, y1, bit;

	for (int_fast8_t i = 0; i < BOARD_SIZE; ++i) {
		x = i % BOARD_WIDTH;
		y = i / BOARD_WIDTH;
		this->neighbours[i] = 0;

		for (int_fast8_t j = 0; j < 4; ++j) {
			x1 = x + dx[j];
			y1 = y + dy[j];
			if (x1 < 0 || BOARD_WIDTH <= x1 || y1 < 0 || BOARD_HEIGHT <= y1)
				continue;
			bit = y1 * BOARD_WIDTH + x1;
			this->neighbours[i] |= 1 << bit;
		}
	}
}

void Board::compute_team_bitmaps()
{
	for (int j = 0; j < NUM_TEAMS; ++j) {
		this->team_bitmaps[j] = 0;
		for (int i = 0; i < NUM_PIECES; ++i) {
			this->team_bitmaps[j] |= this->pieces[j][i];
		}
	}

}

void Board::compute_archer_attacks()
{
	int_fast16_t bb;

	for (int_fast8_t pos = 0; pos < BOARD_SIZE; ++pos) {
		for (int_fast8_t shield_pos = 0; shield_pos < BOARD_SIZE + 1; ++shield_pos) {
			if (pos == shield_pos)
				continue;
			bb = 0;
			for (int i = pos + 1; i > 0 && i / BOARD_WIDTH == pos / BOARD_WIDTH; ++i) {
				bb |= 1 << i;
				if (i == shield_pos)
					break;
			}
			for (int i = pos - 1; i > 0 && i / BOARD_WIDTH == pos / BOARD_WIDTH; --i) {
				bb |= 1 << i;
				if (i == shield_pos)
					break;
			}	
			for (int i = pos + BOARD_WIDTH; i > 0 && i < BOARD_SIZE; i += BOARD_WIDTH) {
				bb |= 1 << i;
				if (i == shield_pos)
					break;
			}
			for (int i = pos - BOARD_WIDTH; i > 0 && i < BOARD_SIZE; i -= BOARD_WIDTH) {
				bb |= 1 << i;
				if (i == shield_pos)
					break;
			}

			this->archer_attacks[pos][shield_pos] = bb;
		}
	}
}

void comb(std::vector<std::vector<int_fast8_t>> &res, std::vector<int_fast8_t> data, int n, int k, int index, int i)
{
	if (index == k) {
		res.push_back(data);
		return;
	}

	if (i >= n) {
		return;
	}

	data[index] = i;
	comb(res, data, n, k, index + 1, i + 1);
	comb(res, data, n, k, index, i + 1);
}

void Board::compute_n_k_subsets()
{
	std::vector<int_fast8_t> data;

	for (int n = 1; n <= 4; ++n) {
		for (int k = 1; k <= 4; ++k) {
			data.resize(k);
			comb(this->n_k_subset[n - 1][ k - 1], data, n, k, 0, 0);
		}
	}
}

void Board::update_activity(int_fast8_t pos)
{
	assert(inbound(pos));

	if (this->info[pos].hp <= 0) {
		return;
	}

	const Team t = this->info[pos].team;
	// will be 1 if active and 0 otherwise
	const int_fast16_t a = (this->team_bitmaps[t] & this->neighbours[pos]) != 0;
	// unset pos bit and set pos bit to be a
	this->active[t] &= ~(1 << pos);
	this->active[t] |= a << pos;
}

void Board::update_all_activity()
{
	for (int_fast8_t pos = 0; pos < BOARD_SIZE; ++pos) {
		update_activity(pos);
	}
}

Board::Board()
{
	for (int i = 0; i < BOARD_SIZE; ++i) {
		this->info[i].hp = 0;
	}

	for (int j = 0; j < NUM_TEAMS; ++j) {
		for (int i = 0; i < NUM_PIECES; ++i) {
			this->pieces[j][i] = 0;
		}
		this->active[j] = 0;
		this->passes[j] = 0;
	}

	this->to_play = BLACK;
	this->state = SWAP;
	this->turn_count = 0;
	this->damaged = 0;

	compute_neighbours();
	compute_archer_attacks();
	compute_n_k_subsets();
}

void Board::place_piece(Piece type, Team colour, int_fast8_t hp, int_fast8_t max_hp, int_fast8_t pos)
{
	assert(type != NUM_PIECES);
	assert(colour != NUM_TEAMS);
	assert(inbound(pos));
	// set the piece on the corresponding bitboard
	this->pieces[colour][type] |= 1 << pos;
	this->info[pos].hp = hp;
	this->info[pos].max_hp = max_hp;
	this->info[pos].team = colour;
	this->info[pos].type = type;

	const int_fast16_t d = hp > 0 && hp < max_hp;
	this->damaged &= d << pos;

	compute_team_bitmaps();
}

void Board::swap(int_fast8_t pos1, int_fast8_t pos2)
{
	assert(inbound(pos1));
	assert(inbound(pos2));

	const int_fast16_t bitmap_pos1 = 1 << pos1;
	const int_fast16_t bitmap_pos2 = 1 << pos2;

	const Team t1 = this->info[pos1].team;
	const Team t2 = this->info[pos2].team;
	const Piece p1 = this->info[pos1].type;
	const Piece p2 = this->info[pos2].type;

	// unset bit at pos1 and set bit at pos2
	this->pieces[t1][p1] &= ~bitmap_pos1;
	this->pieces[t1][p1] |= bitmap_pos2;
	// unset bit at pos2 and set bit at pos1
	this->pieces[t2][p2] &= ~bitmap_pos2;
	this->pieces[t2][p2] |= bitmap_pos1;

	std::swap(this->info[pos1], this->info[pos2]);

	// recompute team_bitmaps since board state has changed
	compute_team_bitmaps();

	// update active bitmap
	int_fast16_t loc2update = this->neighbours[pos1] | this->neighbours[pos2] | bitmap_pos1 | bitmap_pos2;

	while (loc2update) {
		//std::cout << (int)loc2update << std::endl;
		// find first bit location
		int_fast8_t loc = ffs(loc2update) - 1;
		// remove loc
		loc2update &= ~(1 << loc);
		update_activity(loc);
	}

}

void Board::apply_swap(int_fast8_t pos1, int_fast8_t pos2)
{
	assert(this->state == SWAP);

	swap(pos1, pos2);
	this->state = ACTION;
	this->turn_count += 1;
}

void Board::apply_action(action &a)
{
	assert(this->state == ACTION);
	assert(inbound(a.pos));
	for (int i = 0; i < a.num_trgts; ++i) {
		assert(inbound(a.trgts[i]));
	}

	Team other_team = static_cast<Team>(1 - this->info[a.pos].team);
	const Piece p = this->info[a.pos].type;
	int_fast16_t d, update = 0;
	piece_stats *stats;

	switch (p) {
		case KING:
		case ARCHER:
		case KNIGHT:
			for (int i = 0; i < a.num_trgts; ++i) {
				stats = &(this->info[a.trgts[i]]);
				stats->hp -= 1;
				this->damaged &= ~(1 << a.trgts[i]);
				if (stats->hp > 0) {
					this->damaged |= 1 << a.trgts[i];
				} else {
					// piece is dead, remove from pieces, team, and active bitmap
					// add neighbours who are teammates to update bitmap
					this->pieces[stats->team][stats->type] &= ~(1 << a.trgts[i]);
					this->team_bitmaps[other_team] &= ~(1 << a.trgts[i]);
					this->active[other_team] &= ~(1 << a.trgts[i]);
					update |= this->neighbours[a.trgts[i]] & this->team_bitmaps[other_team];
				}
			}
			// update activity
			while (update) {
				int_fast8_t loc = ffs(update) - 1;
				update &= ~(1 << loc);
				update_activity(loc);
			}
			break;
		case MEDIC:
			for (int i = 0; i < a.num_trgts; ++i) {
				stats = &(this->info[a.trgts[i]]);
				stats->hp += 1;
				this->damaged &= ~(1 << a.trgts[i]);
				d = stats->hp < stats->max_hp;
				this->damaged |= d << a.trgts[i];
			}
			break;
		case WIZARD:
			swap(a.pos, a.trgts[0]);
			break;
		case SHIELD:
			assert(0);
	}

	// update game state
	this->state = SWAP;
	this->turn_count += 1;
	this->to_play = other_team;
}

void Board::generate_swaps_at(
		int_fast8_t pos, 
		std::vector<std::pair<int_fast8_t, 
		int_fast8_t>> &swaps, int seen[BOARD_SIZE][BOARD_SIZE])
{
	assert(inbound(pos));

	Team other_team = static_cast<Team>(1 - this->info[pos].team);
	const int_fast16_t all_pieces = this->team_bitmaps[WHITE] | this->team_bitmaps[BLACK];

	int_fast16_t swapable = this->neighbours[pos] & (all_pieces ^ this->pieces[other_team][SHIELD]);
	std::pair<int_fast8_t, int_fast8_t> edge;

	while (swapable) {
		int_fast8_t loc = ffs(swapable) - 1;
		swapable &= ~(1 << loc);

		if (pos > loc) {
			edge.first = loc;
			edge.second = pos;
		} else {
			edge.first = pos;
			edge.second = loc;
		}
		if (!seen[edge.first][edge.second]) {
			swaps.push_back(edge);
			seen[edge.first][edge.second] = 1;
		}
	}
}

std::vector<std::pair<int_fast8_t, int_fast8_t>> Board::generate_swaps()
{
	std::vector<std::pair<int_fast8_t, int_fast8_t>> swaps;
	// at most 20 swaps per state
	swaps.reserve(20);

	int seen[BOARD_SIZE][BOARD_SIZE] = {0};

	int_fast16_t candidates = this->team_bitmaps[this->to_play] & this->active[this->to_play];

	while (candidates) {
		int_fast8_t loc = ffs(candidates) - 1;
		candidates &= ~(1 << loc);

		generate_swaps_at(loc, swaps, seen);
	}

	return swaps;
}

void Board::generate_actions_at(int_fast8_t pos, std::vector<action> &actions)
{
	assert(inbound(pos));

	Piece p = this->info[pos].type;
	Team t = this->info[pos].team;
	Team other_team = static_cast<Team>(1 - this->info[pos].team);

	int_fast8_t opp_shield;
	int_fast16_t trgts;
	int k; // max number of targets

	switch (p) {
		case KING:
			trgts = this->neighbours[pos] & this->team_bitmaps[other_team];
			k = 1;
			break;
		case MEDIC:
			trgts = this->neighbours[pos] & this->team_bitmaps[t] & this->damaged;
			k = 4;
			break;
		case WIZARD:
			trgts = this->team_bitmaps[t] ^ this->pieces[t][WIZARD];
			k = 1;
			break;
		case ARCHER:
			opp_shield = ffs(this->pieces[other_team][SHIELD]) - 1;
			opp_shield = opp_shield >= 0 ? opp_shield : BOARD_SIZE;
			trgts = this->archer_attacks[pos][opp_shield] & this->team_bitmaps[other_team];
			k = 1;
			break;
		case KNIGHT:
			trgts = this->neighbours[pos] & this->team_bitmaps[other_team];
			k = 2;
			break;
		case SHIELD:
			trgts = 0;
			k = 0;
			return;
	}

	std::vector<int_fast8_t> trgt_pos;

	while (trgts) {
		int_fast8_t loc = ffs(trgts) - 1;
		trgts &= ~(1 << loc);
		trgt_pos.push_back(loc);
	}

	action act;
	act.pos = pos;

	if (k == 1) {
		act.num_trgts = 1;
		for (auto t: trgt_pos) {
			act.trgts[0] = t;
			actions.push_back(act);
		}
	} else {
		for (int i = 1; i <= k; ++i) {
			if (trgt_pos.size() < i)
				break;
			act.num_trgts = i;
			for (auto &subset: this->n_k_subset[trgt_pos.size()-1][i-1]) {
				for (int j = 0; j < i; ++j) {
					act.trgts[j] = trgt_pos[subset[j]];
				}
				actions.push_back(act);
			}
		}
	}
}

std::vector<action> Board::generate_actions()
{
	std::vector<action> actions;
	actions.reserve(20);

	int_fast16_t candidates = (this->team_bitmaps[this->to_play] ^ this->pieces[this->to_play][SHIELD]) & this->active[this->to_play];

	while (candidates) {
		int_fast8_t loc = ffs(candidates) - 1;
		candidates &= ~(1 << loc);

		generate_actions_at(loc, actions);
	}

	return actions;
}

bool Board::isolated(Team t)
{
	assert(t != NUM_TEAMS);

	return this->active[t] == 0;
}

bool Board::king_dead(Team t)
{
	assert(t != NUM_TEAMS);

	return this->pieces[t][KING] == 0;
}

bool Board::surrendered(Team t)
{
	assert(t != NUM_TEAMS);

	return this->passes[t] > 2;
}

std::ostream &operator<<(std::ostream &os, const Board &b)
{
	for (int i = 0; i < BOARD_SIZE; ++i) {
		piece_stats stats = b.info[i];
		if (stats.hp > 0) {
			os << "| " << stats.team << " " << stats.type << " " << (int)stats.hp << " " << (int)stats.max_hp << " ";
		} else {
			os << "| ..... . . . ";
		}
		if (i % BOARD_WIDTH == BOARD_WIDTH - 1) {
			os << "|\n";
		}
	}

	return os;
}

std::ostream &operator<<(std::ostream &os, const Team &t)
{
	assert(t != NUM_TEAMS);

	if (t == BLACK) {
		os << "BLACK";
	} else {
		os << "WHITE";
	}

	return os;
}

std::ostream &operator<<(std::ostream &os, const Piece &p)
{
	assert(p != NUM_PIECES);
	char out;

	switch (p) {
		case KING:
			out = 'K';
			break;
		case MEDIC:
			out = 'M';
			break;
		case WIZARD:
			out = 'W';
			break;
		case ARCHER:
			out = 'A';
			break;
		case KNIGHT:
			out = 'N';
			break;
		case SHIELD:
			out = 'S';
			break;
	}

	os << out;
	return os;
}

std::ostream &operator<<(std::ostream &os, const Turn_T &t)
{
	if (t == SWAP) {
		os << "SWAP";
	} else {
		os << "ACTION";
	}

	return os;
}
