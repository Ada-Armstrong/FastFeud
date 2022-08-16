#include <stdlib.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include "board.h"
#include "alphabeta.h"

#define FF_ERROR_STRING(msg) "\033[31;1m" << msg << "\033[0m"
#define FF_SUCCESS_STRING(msg) "\033[32;1m" << msg << "\033[0m"
#define FF_ACTIVE_STRING(msg) "\033[33m" << msg << "\033[0m"
#define FF_INACTIVE_STRING(msg) "\033[34m" << msg << "\033[0m"


const std::string index2rank_file[] = {
	"A1", "B1", "C1", "D1",
	"A2", "B2", "C2", "D2",
	"A3", "B3", "C3", "D3",
	"A4", "B4", "C4", "D4"
};

int main_menu(Board &b)
{
	std::cout << "-------------------------------\n"
		<< "<<<< " << FF_SUCCESS_STRING("Welcome to Fast Feud!") << " >>>>\n" 
		<< "-------------------------------\n\n"
		<< "Commands: help, rules, start, quit" << std::endl;

	std::string cmd;

	while (1) {
		std::cout << ">> ";
		std::cout.flush();

		std::getline(std::cin, cmd);
		std::stringstream ss(cmd);

		while (ss >> cmd) {
			if (cmd == "help") {
				std::cout << "<< " FF_ACTIVE_STRING("Help Screen") << " >>\n"
					<< "help - prints out this message.\n"
					<< "rules - provides a summary of the game rules.\n"
					<< "start - begin playing the game.\n"
					<< "quit - exit the program." << std::endl;
			} else if (cmd == "rules") {
				std::ifstream rules_file{"rules.txt"};
				if (rules_file.is_open()) {
					std::cout << "<< " FF_ACTIVE_STRING("Rules Screen") " >>\n" << rules_file.rdbuf();
				} else {
					std::cout << FF_ERROR_STRING("Couldn't find the rules file...") << std::endl;
				}
			} else if (cmd == "start") {
				std::cout << "Starting new game...\n" << std::endl;
				return 0;
			} else if (cmd == "quit") {
				std::cout << "Quiting...\n" << std::endl;
				return 1;
			} else {
				std::cout << "Unknown command..." << std::endl;
			}
		}
	}
	return 0;
}

std::vector<int_fast8_t> get_input()
{
	std::string pos;
	static const std::string skip{"skip"};

	std::vector<int_fast8_t> out;
	int_fast8_t loc;

	std::cout << "Move: ";
	std::cout.flush();

	std::getline(std::cin, pos);
	std::stringstream ss(pos);

	while (ss >> pos) {
		if (pos == skip) {
			loc = -1;
		} else {
			if (pos.length() != 2 || !('a' <= pos[0] && pos[0] <= 'd' || 'A' <= pos[0] && pos[0] <= 'D')
					|| !('1' <= pos[1] && pos[1] <= '4')) {
				throw std::invalid_argument("Input formated incorrectly");
			}
			loc = BOARD_WIDTH * (pos[1] - '1') + (tolower(pos[0]) - 'a');
		}
		out.push_back(loc);
	}

	return out;
}

void pretty_print_board(Board &b)
{
	auto tiles = b.tile_info();
	int i = 0;

	for (auto &stats : tiles) {
		if (stats.hp > 0) {
			if (stats.active) {
				std::cout << "| " << FF_ACTIVE_STRING(stats.team << " " << stats.type << " "
						<< (int)stats.hp << " " << (int)stats.max_hp);
			} else {
				std::cout << "| " << FF_INACTIVE_STRING(stats.team << " " << stats.type << " "
						<< (int)stats.hp << " " << (int)stats.max_hp);
			}
		} else {
			std::cout << "|" << FF_INACTIVE_STRING(" ..... . . .");
		}
		if (i++ % BOARD_WIDTH == BOARD_WIDTH - 1) {
			std::cout << "|\n";
		}
	}


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
			if (action.pos < 0) {
				std::cout << " (SKIP)";
			} else {
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
	}
	std::cout << std::endl;
}

void computer_move_suggestion(Board &b, int depth)
{
	std::cout << "Suggested Move Index: " << suggest_move(b, depth) << std::endl;
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

void play(Board &b, int argc, char *argv[])
{
	int search_depth = 6;
	std::string filename("../positions/default1.txt");
	if (argc == 2) {
		filename = argv[1];
	}
	b.load_file(filename);
	
	std::pair<Team, Win_Condition> winner_info{NONE, NO_WINNER};
	
	while (1) {
		Team turn = b.to_play;
		std::cout << "Turn: " << turn << "\tState: " << b.state << "\tSkips Black: " << b.get_passes(BLACK)
			<< "\tSkips White: " << b.get_passes(WHITE) << "\tTurn Count: " << b.turn_count / 4 + 1 << std::endl;
		pretty_print_board(b);

		display_moves(b);
		computer_move_suggestion(b, search_depth);

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
				std::cout << FF_ERROR_STRING("Need two positions for swap, recieved "
						<< locations.size() << " positions...\n") << std::endl;
				continue;
			} else if (!is_swap_valid(locations, b.generate_swaps())) {
				std::cout << FF_ERROR_STRING("Invalid swap provided, try again...\n") << std::endl;
				continue;
			}

			b.apply_swap(locations[0], locations[1]);
		} else {
			if (locations.size() < 2 && (locations.size() != 1 || locations[0] >= 0)) {
				std::cout << FF_ERROR_STRING("Need at least two positions for action, recieved "
						<< locations.size() << " positions...\n") << std::endl;
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
			std::cout << FF_SUCCESS_STRING(winner_info.first << " wins!\tWin Condition: " << winner_info.second)
				<< std::endl;
			pretty_print_board(b);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	Board b;

	while (1) {
		if (main_menu(b)) {
			return 0;
		}

		play(b, argc, argv);
	}

	return 0;
}
