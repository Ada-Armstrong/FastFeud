#pragma once

#include <cstdint>
#include <cassert>
#include <vector>
#include <iostream>

#define BOARD_WIDTH 4
#define BOARD_HEIGHT 4
#define BOARD_SIZE (BOARD_WIDTH * BOARD_HEIGHT)

enum Team {
	BLACK = 0,
	WHITE = 1,
	NUM_TEAMS
};

enum Piece {
	KING = 0,
	MEDIC,
	WIZARD,
	ARCHER,
	KNIGHT,
	SHIELD,
	NUM_PIECES
};

enum Turn_T {
	SWAP = 0,
	ACTION,
};

struct piece_stats {
	int_fast8_t hp;
	int_fast8_t max_hp;
	Team team;
	Piece type;
};

struct action {
	int_fast8_t pos; // location of action doer
	int_fast8_t num_trgts; // 1-4
	int_fast8_t trgts[4];
};

/*  |  A |  B |  C |  D |
 * -|----|----|----|----|
 * 1|  0 |  1 |  2 |  3 |
 * -|----|----|----|----|
 * 2|  4 |  5 |  6 |  7 |
 * -|----|----|----|----|
 * 3|  8 |  9 | 10 | 11 |
 * -|----|----|----|----|
 * 4| 12 | 13 | 14 | 15 |
 * -|----|----|----|----|
 */

class Board {
	public:

	Turn_T state;
	Team to_play;
	// number of quarter turns
	int_fast32_t turn_count;

	private:

	int_fast16_t pieces[NUM_TEAMS][NUM_PIECES];

	piece_stats info[BOARD_SIZE];

	int_fast16_t active[NUM_TEAMS];
	int_fast8_t passes[NUM_TEAMS];

	// bitmap for each position representing neighbours of that tile
	int_fast16_t neighbours[BOARD_SIZE];
	// stores a bitmap for each team representing where the pieces are
	int_fast16_t team_bitmaps[NUM_TEAMS];
	// tracks pieces that don't have full hp and that are alive
	int_fast16_t damaged;
	// archer attack lookup table, first index archer pos 0-15, second
	// index the pos of enemy shield 0-15, 16 if no shield
	int_fast16_t archer_attacks[BOARD_SIZE][BOARD_SIZE+1];
	// stores index of k element subsets of an array of length n,
	// first index for array len, second for k element subset
	std::vector<std::vector<int_fast8_t>> n_k_subset[4][4];

	bool inbound(int_fast8_t pos);

	void compute_neighbours();

	void compute_team_bitmaps();

	void compute_archer_attacks();

	void compute_n_k_subsets();

	void generate_swaps_at(
			int_fast8_t pos,
			std::vector<std::pair<int_fast8_t, int_fast8_t>> &swaps,
			int seen[BOARD_SIZE][BOARD_SIZE]);

	void generate_actions_at(int_fast8_t pos, std::vector<action> &actions);

	public:

	Board();

	void update_activity(int_fast8_t pos);

	void update_all_activity();

	void place_piece(Piece type, Team colour, int_fast8_t hp, int_fast8_t max_hp, int_fast8_t pos);

	void swap(int_fast8_t pos1, int_fast8_t pos2);

	void apply_swap(int_fast8_t pos1, int_fast8_t pos2);

	void apply_action(action &a);

	std::vector<std::pair<int_fast8_t, int_fast8_t>> generate_swaps();

	std::vector<action> generate_actions();

	bool isolated(Team t);

	bool king_dead(Team t);

	bool surrendered(Team t);

	friend std::ostream &operator<<(std::ostream &os, const Board &b);
};

// ostream overloads
std::ostream &operator<<(std::ostream &os, const Board &b);

std::ostream &operator<<(std::ostream &os, const Team &t);

std::ostream &operator<<(std::ostream &os, const Piece &p);

std::ostream &operator<<(std::ostream &os, const Turn_T &t);
