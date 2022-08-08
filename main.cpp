#include <stdlib.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include "board.h"

#define FF_ERROR_STRING(msg) "\033[31;1m" << msg << "\033[0m"
#define FF_SUCCESS_STRING(msg) "\033[32;1m" << msg << "\033[0m"


const std::string index2rank_file[] = {
	"A1", "B1", "C1", "D1",
	"A2", "B2", "C2", "D2",
	"A3", "B3", "C3", "D3",
	"A4", "B4", "C4", "D4"
};

void setup_board(Board &b)
{
	// black pieces
	b.place_piece(ARCHER, BLACK, 3, 3, 0);
	b.place_piece(KING, BLACK, 4, 4, 1);
	b.place_piece(MEDIC, BLACK, 3, 3, 2);
	b.place_piece(ARCHER, BLACK, 3, 3, 3);
	b.place_piece(KNIGHT, BLACK, 3, 3, 4);
	b.place_piece(SHIELD, BLACK, 3, 4, 5);
	b.place_piece(WIZARD, BLACK, 3, 3, 6);
	b.place_piece(KNIGHT, BLACK, 3, 3, 7);
	// white pieces
	b.place_piece(KNIGHT, WHITE, 3, 3, 8);
	b.place_piece(SHIELD, WHITE, 4, 4, 9);
	b.place_piece(WIZARD, WHITE, 3, 3, 10);
	b.place_piece(KNIGHT, WHITE, 3, 3, 11);
	b.place_piece(ARCHER, WHITE, 3, 3, 12);
	b.place_piece(KING, WHITE, 4, 4, 13);
	b.place_piece(MEDIC, WHITE, 3, 3, 14);
	b.place_piece(ARCHER, WHITE, 3, 3, 15);

	b.update_all_activity();
}

std::vector<int_fast8_t> get_input()
{
	std::string pos;

	std::vector<int_fast8_t> out;
	int_fast8_t loc;

	std::cout << "Move: ";
	std::cout.flush();

	std::getline(std::cin, pos);
	std::stringstream ss(pos);

	while (ss >> pos) {
		if (pos.length() != 2 || !('a' <= pos[0] && pos[0] <= 'd' || 'A' <= pos[0] && pos[0] <= 'D')
				|| !('1' <= pos[1] && pos[1] <= '4')) {
			throw std::invalid_argument("Input formated incorrectly");
		}
		loc = BOARD_WIDTH * (pos[1] - '1') + (tolower(pos[0]) - 'a');
		out.push_back(loc);
	}

	return out;
}

void display_moves(Board &b)
{
	if (b.state == SWAP) {
		auto swaps = b.generate_swaps();
		std::cout << "Swaps:";
		for (auto &swap : swaps) {
			std::cout << " (" << index2rank_file[swap.first] << ", " << index2rank_file[swap.second] << ")";
		}
	} else {
		auto actions = b.generate_actions();
		std::cout << "Actions:";
		for (auto &action : actions) {
			std::cout <<" (" << index2rank_file[action.pos] << ", [";
			for (int i = 0; i < action.num_trgts; ++i) {
				std::cout << index2rank_file[action.trgts[i]];
				if (i != action.num_trgts - 1) {
					std::cout << ", ";
				}
			}
			std::cout << "])";
		}
	}
	std::cout << std::endl;
}

bool is_swap_valid(std::vector<int_fast8_t> swap, std::vector<std::pair<int_fast8_t, int_fast8_t>> valid_swaps)
{
	// order swap the way valid swaps are ordered (from least to greatest)
	if (swap[0] > swap[1]) {
		std::swap(swap[0], swap[1]);
	}
	for (auto &vs : valid_swaps) {
		if (vs.first == swap[0] && vs.second == swap[1]) {
			return true;
		}
	}
	return false;
}

bool is_action_valid(action a, std::vector<action> valid_actions) {
	// order action trgts from least to greatest
	std::sort(a.trgts, a.trgts + a.num_trgts);
	for (auto &va : valid_actions) {
		bool differ = false;
		if (va.pos == a.pos && va.num_trgts == a.num_trgts) {
			for (int i = 0; i < va.num_trgts; ++i) {
				if (va.trgts[i] != a.trgts[i]) {
					differ = true;
					break;
				}	
			}
			if (!differ) {
				return true;
			}
		}
	}
	return false;
}

int main(void)
{
	Board b;
	setup_board(b);
	
	std::pair<Team, Win_Condition> winner_info{NONE, NO_WINNER};
	
	while (1) {
		Team turn = b.to_play;
		std::cout << "Turn: " << turn << "\tState: " << b.state << "\tTurn Count: " << b.turn_count / 4 + 1 << std::endl;
		std::cout << b << std::endl;

		display_moves(b);

		std::vector<int_fast8_t> locations;
		try {
			locations = get_input();
			std::cout << std::endl;
		} catch (const std::invalid_argument &e) {
			std::cout << FF_ERROR_STRING("Bad input recieved...\n") << std::endl;
			continue;
		}

		if (b.state == SWAP) {
			if (locations.size() != 2) {
				std::cout << FF_ERROR_STRING("Need two positions for swap, recieved " << locations.size() << " positions...\n") << std::endl;
				continue;
			} else if (!is_swap_valid(locations, b.generate_swaps())) {
				std::cout << FF_ERROR_STRING("Invalid swap provided, try again...\n") << std::endl;
				continue;
			}

			b.apply_swap(locations[0], locations[1]);
		} else {
			if (locations.size() < 2) {
				std::cout << FF_ERROR_STRING("Need at least two positions for action, recieved " << locations.size() << " positions...\n") << std::endl;
				continue;
			} 

			action a;
			a.pos = locations[0];
			a.num_trgts = locations.size() - 1;

			for (int i = 1; i < locations.size(); ++i) {
				a.trgts[i-1] = locations[i];
			}
			
			if (!is_action_valid(a, b.generate_actions())) {
				std::cout << FF_ERROR_STRING("Invalid action provided, try again...\n") << std::endl;
				continue;
			}

			b.apply_action(a);
		}

		winner_info = b.winner();

		if (winner_info.second != NO_WINNER) {
			std::cout << FF_SUCCESS_STRING(winner_info.first << " wins!\tWin Condition: " << winner_info.second) << std::endl;
			std::cout << b << std::endl;
			break;
		}
	}

	return 0;
}
