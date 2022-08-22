#pragma once

#include <cstdint>
#include <cassert>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#define BOARD_WIDTH 4
#define BOARD_HEIGHT 4
#define BOARD_SIZE (BOARD_WIDTH * BOARD_HEIGHT)

#define uint_fast128_t unsigned __int128

enum Team {
	BLACK = 0,
	WHITE = 1,
	NUM_TEAMS,
	NONE
};

enum Piece {
	KING = 0,
	MEDIC,
	WIZARD,
	ARCHER,
	KNIGHT,
	SHIELD,
	NUM_PIECES,
	EMPTY
};

enum Turn_T {
	SWAP = 0,
	ACTION
};

enum Win_Condition {
	ISOLATED,
	KING_DEAD,
	SURRENDERED,
	NO_WINNER
};

// EMPTY tiles are represented by a struct that has hp <= 0
struct piece_stats {
	uint_fast8_t hp;
	uint_fast8_t max_hp;
	Team team;
	Piece type;
	bool active;
};

struct action {
	uint_fast8_t pos; // location of action doer
	uint_fast8_t num_trgts; // 1-4
	uint_fast8_t trgts[4];
};

class LookupTables {
	public:
	// bitmap for each position representing neighbours of that tile
	uint_fast16_t neighbours[BOARD_SIZE];
	// archer attack lookup table, first index archer pos 0-15, second
	// index the pos of enemy shield 0-15, 16 if no shield
	uint_fast16_t archer_attacks[BOARD_SIZE][BOARD_SIZE+1];
	// stores index of k element subsets of an array of length n,
	// first index for array len, second for k element subset
	std::vector<std::vector<uint_fast8_t>> n_k_subset[4][4];

	private:
	/* Description: Populates the neighbours table. Should be called once.
	 * Args: None
	 */
	void compute_neighbours();
	/* Description: Populates the archer attack table. Should be called once.
	 * Args: None
	 */
	void compute_archer_attacks();
	/* Description: Computes all k element subsets of an array of length n, for k = 1,...,4
	 * 		and n = 1,...,4. Should be called once.
	 * Args: None
	 */
	void compute_n_k_subsets();

	public:

	LookupTables();
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

	static LookupTables lookup;

	uint_fast16_t pieces[NUM_TEAMS][NUM_PIECES];

	piece_stats info[BOARD_SIZE];

	uint_fast16_t active[NUM_TEAMS];
	uint_fast8_t passes[NUM_TEAMS];

	// stores a bitmap for each team representing where the pieces are
	uint_fast16_t team_bitmaps[NUM_TEAMS];
	// tracks pieces that don't have full hp and that are alive
	uint_fast16_t damaged;

	/* Description: returns true if pos is within the board.
	 * Args: pos - the position to check.
	 */
	bool inbound(uint_fast8_t pos);
	/* Description: populates the team_bitmaps bitmap according to the pieces array.
	 * Args: None
	 */
	void compute_team_bitmaps();
	/* Description: generates the valid swaps at pos and adds them to the swaps vector.
	 * Args: pos - the position to generate the swaps for.
	 * 	 swaps - the vector to append the results to.
	 * 	 seen - boolean array used to track duplicate swaps (i.e. [A1, A2] == [A2, A1])
	 */
	void generate_swaps_at(
			uint_fast8_t pos,
			std::vector<std::pair<uint_fast8_t, uint_fast8_t>> &swaps,
			int seen[BOARD_SIZE][BOARD_SIZE]);
	/* Description: generates the valid actions at pos and adds them to the actions vector.
	 * Args: pos - the position to generate the actions for.
	 * 	 actions - the vector to append the results to.
	 */
	void generate_actions_at(uint_fast8_t pos, std::vector<action> &actions);

	public:

	Board();
	/* Description: Loads a game state from the given file. Return true if successful else false.
	 * Args: f - the file to load the state from.
	 */
	bool load_file(std::string &f);
	/* Description: Converts the current state of the board to a unsigned 128 bit integer.
	 */
	uint_fast128_t hash();
	/* Description: Loads the game state from the given unsinged 128 bit integer.
	 * Args: state - the integer to load the game state from.
	 */
	bool load_hash(uint_fast128_t state);
	/* Description: Returns the number of skips in a row Team t has used.
	 * Args: t - the team to check.
	 */
	int get_passes(Team t);
	/* Description: Returns the number of friendlies that are neighbours at pos.
	 * Args: pos - the position to check.
	 */
	int num_friendly_neighbours(uint_fast8_t pos);
	/* Description: Updates the active flags of the piece at pos.
	 * Args: pos - the position to update.
	 */
	void update_activity(uint_fast8_t pos);
	/* Description: Updates all of the active flags of all pieces.
	 * Args: None
	 */
	void update_all_activity();
	/* Description: place the piece with the given stats at pos.
	 * Args: type - the type of the piece to place.
	 * 	 colour - the team that the piece belongs to.
	 * 	 hp - the amount of health points the piece will have.
	 * 	 max_hp - the maximum hp of the piece.
	 * 	 pos - the position to place the piece.
	 */
	void place_piece(Piece type, Team colour, uint_fast8_t hp, uint_fast8_t max_hp, uint_fast8_t pos);
	/* Description: swaps the pieces at pos1 and pos2.
	 * Args: pos1 - position of first piece.
	 * 	 pos2 - position of second piece.
	 */
	void swap(uint_fast8_t pos1, uint_fast8_t pos2);
	/* Description: performs a swap and updates the turn_count and state.
	 * Args: pos1 - position of first piece.
	 * 	 pos2 - position of second piece.
	 */
	void apply_swap(uint_fast8_t pos1, uint_fast8_t pos2);
	/* Description: performs the action and updates the turn_count and state.
	 * Args: a - the action to be performed.
	 */
	void apply_action(action &a);
	/* Description: returns a vector of valid swaps for this board state. 
	 * Args: None
	 */
	std::vector<std::pair<uint_fast8_t, uint_fast8_t>> generate_swaps();
	/* Description: returns a vector of valid actions for this board state.
	 * Args: None
	 */
	std::vector<action> generate_actions();
	/* Description: returns a vector of piece information for each tile.
	 * Args: None
	 */
	std::vector<piece_stats> tile_info();
	/* Description: returns true if Team t is isolated in this state (no active pieces).
	 * Args: t - the team to check.
	 */
	bool isolated(Team t);
	/* Description: returns true if Team t's king is dead in this state.
	 * Args: t - the team to check.
	 */
	bool king_dead(Team t);
	/* Description: returns true if Team t has exceed the maximum number of skips (2). 
	 * Args: t - the team to check.
	 */
	bool surrendered(Team t);
	/* Description: returns true if either of the teams are isolated, king_dead, or surrendered.
	 * Args: None
	 */
	bool gameover();
	/* Description: returns the winner as a pair with the first element being the winning team and
	 * 		the second being the win condition (ISOLATED, KING_DEAD, SURRENDERED). If there
	 * 		is no winner the first element is set to NONE.
	 * Args: None
	 */
	std::pair<Team, Win_Condition> winner();

	friend std::ostream &operator<<(std::ostream &os, const Board &b);
};

// ostream overloads
std::ostream &operator<<(std::ostream &os, const Board &b);

std::ostream &operator<<(std::ostream &os, const Team &t);

std::ostream &operator<<(std::ostream &os, const Piece &p);

std::ostream &operator<<(std::ostream &os, const Turn_T &t);

std::ostream &operator<<(std::ostream &os, const Win_Condition &w);
