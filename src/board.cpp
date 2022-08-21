#include "board.h"

#ifdef __GNUC__
#	define ffs(x) __builtin_ffs(x)
#endif


LookupTables Board::lookup;

/*** LookupTables Implementations ***/

LookupTables::LookupTables()
{
	compute_neighbours();
	compute_archer_attacks();
	compute_n_k_subsets();
}

void LookupTables::compute_neighbours()
{
	const int_fast8_t dx[] = {-1, 0, 1, 0};
	const int_fast8_t dy[] = {0, 1, 0, -1};
	int_fast8_t x, y, x1, y1, bit;

	for (uint_fast8_t i = 0; i < BOARD_SIZE; ++i) {
		x = i % BOARD_WIDTH;
		y = i / BOARD_WIDTH;
		this->neighbours[i] = 0;

		for (uint_fast8_t j = 0; j < 4; ++j) {
			x1 = x + dx[j];
			y1 = y + dy[j];
			if (x1 < 0 || BOARD_WIDTH <= x1 || y1 < 0 || BOARD_HEIGHT <= y1)
				continue;
			bit = y1 * BOARD_WIDTH + x1;
			this->neighbours[i] |= 1 << bit;
		}
	}
}

void LookupTables::compute_archer_attacks()
{
	uint_fast16_t bb;

	for (uint_fast8_t pos = 0; pos < BOARD_SIZE; ++pos) {
		for (uint_fast8_t shield_pos = 0; shield_pos < BOARD_SIZE + 1; ++shield_pos) {
			if (pos == shield_pos)
				continue;
			bb = 0;
			for (int i = pos + 1; i >= 0 && i / BOARD_WIDTH == pos / BOARD_WIDTH; ++i) {
				bb |= 1 << i;
				if (i == shield_pos)
					break;
			}
			for (int i = pos - 1; i >= 0 && i / BOARD_WIDTH == pos / BOARD_WIDTH; --i) {
				bb |= 1 << i;
				if (i == shield_pos)
					break;
			}	
			for (int i = pos + BOARD_WIDTH; i >= 0 && i < BOARD_SIZE; i += BOARD_WIDTH) {
				bb |= 1 << i;
				if (i == shield_pos)
					break;
			}
			for (int i = pos - BOARD_WIDTH; i >= 0 && i < BOARD_SIZE; i -= BOARD_WIDTH) {
				bb |= 1 << i;
				if (i == shield_pos)
					break;
			}

			this->archer_attacks[pos][shield_pos] = bb;
		}
	}
}

static void comb(std::vector<std::vector<uint_fast8_t>> &res, std::vector<uint_fast8_t> data, int n, int k, int index, int i)
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

void LookupTables::compute_n_k_subsets()
{
	std::vector<uint_fast8_t> data;

	for (int n = 1; n <= 4; ++n) {
		for (int k = 1; k <= 4; ++k) {
			data.resize(k);
			comb(this->n_k_subset[n - 1][k - 1], data, n, k, 0, 0);
		}
	}
}

/*** Board Implementations ***/

bool Board::inbound(uint_fast8_t pos)
{
	return 0 <= pos && pos < BOARD_SIZE;
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

void Board::update_activity(uint_fast8_t pos)
{
	assert(inbound(pos));

	if (this->info[pos].hp <= 0) {
		return;
	}

	const Team t = this->info[pos].team;
	// will be 1 if active and 0 otherwise
	const uint_fast16_t a = (this->team_bitmaps[t] & this->lookup.neighbours[pos]) != 0;
	this->info[pos].active = a;
	// unset pos bit and set pos bit to be a
	this->active[t] &= ~(1 << pos);
	this->active[t] |= a << pos;
}

void Board::update_all_activity()
{
	for (uint_fast8_t pos = 0; pos < BOARD_SIZE; ++pos) {
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
}

Team char2team(char c)
{
	Team out = NONE;
	switch (c) {
		case 'b':
			out = BLACK;
			break;
		case 'w':
			out = WHITE;
			break;
		default:
			break;
	}
	return out;
}

Piece char2piece(char c)
{
	Piece out = NUM_PIECES;
	switch (c) {
		case 'a':
			out = ARCHER;
			break;
		case 'n':
			out = KNIGHT;
			break;
		case 'k':
			out = KING;
			break;
		case 'm':
			out = MEDIC;
			break;
		case 'w':
			out = WIZARD;
			break;
		case 's':
			out = SHIELD;
			break;
		default:
			break;
	}
	return out;
}

int piece_max_hp(Piece p)
{
	int out = 0;
	switch (p) {
		case ARCHER:
		case KNIGHT:
		case MEDIC:
		case WIZARD:
			out = 3;
			break;
		case KING:
		case SHIELD:
			out = 4;
			break;
		default:
			break;
	}
	return out;
}

bool Board::load_file(std::string &filename)
{
	std::string line;
	std::ifstream f(filename);
	if (!std::getline(f >> std::ws, line, ';')) {
		std::cerr << "FAILED TO READ FILE " << filename << std::endl;
		return false;
	}
	
	if (line.size() < 3) {
		return false;
	}
	// set state
	switch (line[0]) {
		case 's':
			this->state = SWAP;
			break;
		case 'a':
			this->state = ACTION;
			break;
		default:
			return false;
	}
	// set whose turn it is to play
	this->to_play = char2team(line[1]);
	if (this->to_play == NONE) {
		return false;
	}
	// set the quarter turn count
	this->turn_count = std::stoi(line.substr(2, line.size() - 2));
	if (this->turn_count < 0) {
		return false;
	}
	// set number of passes for black and then white
	for (int t = 0; t != NUM_TEAMS; ++t) {
		if (!std::getline(f >> std::ws, line, ';')) {
			std::cerr << "REACHED EOF " << filename << std::endl;
			return false;
		}
		this->passes[t] = std::stoi(line);
		if (this->passes[t] < 0 || this->passes[t] > 2) {
			return false;
		}
	}
	// set board pieces
	uint_fast8_t i;
	for (i = 0; getline(f >> std::ws, line, ';'); ++i) {
		if (i >= BOARD_SIZE) {
			return false;
		}
		if (line.size() == 1 && line[0] == '.') {
			// empty tile
			this->place_piece(EMPTY, NONE, 0, 0, i);
		} else if (line.size() == 3) {
			Team t = char2team(line[0]);
			Piece p = char2piece(line[1]);
			int hp = std::stoi(line.substr(2, 1));
			int max_hp = piece_max_hp(p);
			if (t == NONE || p == NUM_PIECES || hp <= 0 || hp > max_hp) {
				return false;
			}
			this->place_piece(p, t, hp, max_hp, i);
		} else {
			return false;
		}
	}
	update_all_activity();

	return true;
}

int Board::get_passes(Team t)
{
	assert(t != NUM_TEAMS || t != NONE);
	return this->passes[t];
}

int Board::num_friendly_neighbours(uint_fast8_t pos)
{
	assert(inbound(pos));
	const Team t = this->info[pos].team;
	uint_fast16_t f = this->team_bitmaps[t] & this->lookup.neighbours[pos];
	int count = 0;
	while (f) {
		f = f & (f - 1);
		++count;
	}
	return count;
}

void Board::place_piece(Piece type, Team colour, uint_fast8_t hp, uint_fast8_t max_hp, uint_fast8_t pos)
{
	assert(type != NUM_PIECES);
	assert(colour != NUM_TEAMS);
	assert(inbound(pos));
	// set the piece on the corresponding bitboard
	if (hp > 0) {
		this->pieces[colour][type] |= 1 << pos;
	}

	this->info[pos].hp = hp;
	this->info[pos].max_hp = max_hp;
	this->info[pos].team = colour;
	this->info[pos].type = type;
	this->info[pos].active = false;

	const uint_fast16_t d = hp > 0 && hp < max_hp;
	this->damaged |= d << pos;

	compute_team_bitmaps();
}

void Board::swap(uint_fast8_t pos1, uint_fast8_t pos2)
{
	assert(inbound(pos1));
	assert(inbound(pos2));

	const uint_fast16_t bitmap_pos1 = 1 << pos1;
	const uint_fast16_t bitmap_pos2 = 1 << pos2;

	const Team t1 = this->info[pos1].team;
	const Team t2 = this->info[pos2].team;
	const Piece p1 = this->info[pos1].type;
	const Piece p2 = this->info[pos2].type;

	// if the team and piece type are the same don't need to do anything
	if (t1 != t2 || p1 != p2) {
		// unset bit at pos1 and set bit at pos2
		this->pieces[t1][p1] &= ~bitmap_pos1;
		this->pieces[t1][p1] |= bitmap_pos2;
		// unset bit at pos2 and set bit at pos1
		this->pieces[t2][p2] &= ~bitmap_pos2;
		this->pieces[t2][p2] |= bitmap_pos1;
		// unset activity
		this->active[t1] &= ~bitmap_pos1;
		this->active[t2] &= ~bitmap_pos2;
	}
	// swap damaged flags
	const uint_fast16_t d1 = (this->damaged >> pos1) & 1;
	const uint_fast16_t d2 = (this->damaged >> pos2) & 1;
	this->damaged &= ~bitmap_pos1;
	this->damaged &= ~bitmap_pos2;
	this->damaged |= d1 << pos2;
	this->damaged |= d2 << pos1;

	std::swap(this->info[pos1], this->info[pos2]);

	// recompute team_bitmaps since board state has changed
	compute_team_bitmaps();

	// update active bitmap
	uint_fast16_t loc2update = this->lookup.neighbours[pos1] | this->lookup.neighbours[pos2] | bitmap_pos1 | bitmap_pos2;

	while (loc2update) {
		// find first bit location
		uint_fast8_t loc = ffs(loc2update) - 1;
		// remove loc
		loc2update &= ~(1 << loc);
		update_activity(loc);
	}

}

void Board::apply_swap(uint_fast8_t pos1, uint_fast8_t pos2)
{
	assert(this->state == SWAP);

	swap(pos1, pos2);
	this->state = ACTION;
	this->turn_count += 1;
}

void Board::apply_action(action &a)
{
	assert(this->state == ACTION);

	Team other_team = static_cast<Team>(1 - this->to_play);

	if (a.pos == BOARD_SIZE && a.num_trgts == 0) {
		// skip action
		this->passes[this->to_play] += 1;
	} else {
		assert(inbound(a.pos));
		assert(this->to_play == this->info[a.pos].team);

		for (int i = 0; i < a.num_trgts; ++i) {
			assert(inbound(a.trgts[i]));
		}

		const Piece p = this->info[a.pos].type;
		uint_fast16_t update = 0;
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
						update |= this->lookup.neighbours[a.trgts[i]] & this->team_bitmaps[other_team];
					}
				}
				// update activity
				while (update) {
					uint_fast8_t loc = ffs(update) - 1;
					update &= ~(1 << loc);
					update_activity(loc);
				}
				break;
			case MEDIC:
				for (int i = 0; i < a.num_trgts; ++i) {
					stats = &(this->info[a.trgts[i]]);
					stats->hp += 1;
					if (stats->hp >= stats->max_hp) {
						this->damaged &= ~(1 << a.trgts[i]);
					}
				}
				break;
			case WIZARD:
				swap(a.pos, a.trgts[0]);
				break;
			case SHIELD:
				assert(0);
		}
		this->passes[this->to_play] = 0;
	}
	// update game state
	this->state = SWAP;
	this->turn_count += 1;
	this->to_play = other_team;
}

void Board::generate_swaps_at(
		uint_fast8_t pos, 
		std::vector<std::pair<uint_fast8_t, 
		uint_fast8_t>> &swaps, int seen[BOARD_SIZE][BOARD_SIZE])
{
	assert(inbound(pos));

	Team other_team = static_cast<Team>(1 - this->info[pos].team);
	const uint_fast16_t all_pieces = this->team_bitmaps[WHITE] | this->team_bitmaps[BLACK];

	uint_fast16_t swapable = this->lookup.neighbours[pos] & (all_pieces ^ this->pieces[other_team][SHIELD]);
	std::pair<uint_fast8_t, uint_fast8_t> edge;

	while (swapable) {
		uint_fast8_t loc = ffs(swapable) - 1;
		swapable &= ~(1 << loc);

		edge.first = pos;
		edge.second = loc;

		if (pos > loc) {
			std::swap(edge.first, edge.second);
		}
		if (!seen[edge.first][edge.second]) {
			swaps.push_back(edge);
			seen[edge.first][edge.second] = 1;
		}
	}
}

std::vector<std::pair<uint_fast8_t, uint_fast8_t>> Board::generate_swaps()
{
	std::vector<std::pair<uint_fast8_t, uint_fast8_t>> swaps;
	// at most 20 swaps per state
	swaps.reserve(20);

	int seen[BOARD_SIZE][BOARD_SIZE] = {0};

	uint_fast16_t candidates = this->team_bitmaps[this->to_play] & this->active[this->to_play];

	while (candidates) {
		uint_fast8_t loc = ffs(candidates) - 1;
		candidates &= ~(1 << loc);

		generate_swaps_at(loc, swaps, seen);
	}

	return swaps;
}

void Board::generate_actions_at(uint_fast8_t pos, std::vector<action> &actions)
{
	assert(inbound(pos));

	Piece p = this->info[pos].type;
	Team t = this->info[pos].team;
	Team other_team = static_cast<Team>(1 - this->info[pos].team);

	int_fast8_t opp_shield;
	uint_fast16_t trgts;
	int k; // max number of targets

	switch (p) {
		case KING:
			trgts = this->lookup.neighbours[pos] & this->team_bitmaps[other_team];
			k = 1;
			break;
		case MEDIC:
			trgts = this->lookup.neighbours[pos] & this->team_bitmaps[t] & this->damaged;
			k = 4;
			break;
		case WIZARD:
			trgts = this->team_bitmaps[t] ^ this->pieces[t][WIZARD];
			k = 1;
			break;
		case ARCHER:
			opp_shield = ffs(this->pieces[other_team][SHIELD]) - 1;
			opp_shield = opp_shield >= 0 ? opp_shield : BOARD_SIZE;
			trgts = this->lookup.archer_attacks[pos][opp_shield] & this->team_bitmaps[other_team];
			k = 1;
			break;
		case KNIGHT:
			trgts = this->lookup.neighbours[pos] & this->team_bitmaps[other_team];
			k = 2;
			break;
		case SHIELD:
			trgts = 0;
			k = 0;
			return;
	}

	std::vector<uint_fast8_t> trgt_pos;

	while (trgts) {
		uint_fast8_t loc = ffs(trgts) - 1;
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
			for (auto &subset: this->lookup.n_k_subset[trgt_pos.size()-1][i-1]) {
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
	actions.reserve(21);

	// pieces on the active team except shield that are active
	uint_fast16_t candidates = (this->team_bitmaps[this->to_play] ^ this->pieces[this->to_play][SHIELD])
		& this->active[this->to_play];

	while (candidates) {
		uint_fast8_t loc = ffs(candidates) - 1;
		candidates &= ~(1 << loc);

		generate_actions_at(loc, actions);
	}

	// skip action
	actions.push_back(action{BOARD_SIZE, 0});

	return actions;
}

std::vector<piece_stats> Board::tile_info()
{
	return std::vector<piece_stats>(this->info, this->info + BOARD_SIZE);
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

bool Board::gameover()
{
	const Team opponent = static_cast<Team>(1 - this->to_play);
	return isolated(this->to_play) || isolated(opponent)
		|| king_dead(this->to_play) || king_dead(opponent)
		|| surrendered(this->to_play) || surrendered(opponent);
}

std::pair<Team, Win_Condition> Board::winner()
{
	std::pair<Team, Win_Condition> out{NONE, NO_WINNER};

	if (!this->gameover()) {
		return out;
	}

	const bool bi = this->isolated(BLACK);
	const bool wi = this->isolated(WHITE);

	if (this->surrendered(BLACK)) {
		out.first = WHITE;
		out.second = SURRENDERED;
	} else if (this->king_dead(BLACK)) {
		out.first = WHITE;
		out.second = KING_DEAD;
	} else if (this->surrendered(WHITE)) {
		out.first = BLACK;
		out.second = SURRENDERED;
	} else if (this->king_dead(WHITE)) {
		out.first = BLACK;
		out.second = KING_DEAD;
	} else if (bi && wi) {
		out.first = NONE;
		out.second = ISOLATED;
	} else if (bi) {
		out.first = WHITE;
		out.second = ISOLATED;
	} else if (wi) {
		out.first = BLACK;
		out.second = ISOLATED;
	}

	return out;
}

std::ostream &operator<<(std::ostream &os, const Board &b)
{
	for (int i = 0; i < BOARD_SIZE; ++i) {
		piece_stats stats = b.info[i];
		if (stats.hp > 0) {
			os << "| " << stats.team << " " << stats.type << " " << (int)stats.hp << " "
				<< (int)stats.max_hp << " " << (stats.active ? "*" : ".");
		} else {
			os << "| ..... . . . .";
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
	} else if (t == WHITE) {
		os << "WHITE";
	} else {
		os << "EMPTY";
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
		case EMPTY:
			out = 'E';
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

std::ostream &operator<<(std::ostream &os, const Win_Condition &w)
{
	if (w == ISOLATED) {
		os << "ISOLATED";
	} else if (w == KING_DEAD) {
		os << "KING DEAD";
	} else if (w == SURRENDERED) {
		os << "SURRENDERED";
	} else {
		os << "NO WINNER";
	}

	return os;
}
