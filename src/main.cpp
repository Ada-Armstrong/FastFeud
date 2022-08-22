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

enum Player {
	HUMAN,
	COMPUTER
};

struct Config {
	Player players[2];
	std::string filename;
	int hint_depth;
	int search_depth;
};

union MoveChoice {
	struct {
		uint_fast8_t first;
		uint_fast8_t second;
	} swap;
	action act;
};


int setup_menu(Config &config)
{
	std::string cmd;
	Config copy{config};

	std::cout << "<< " FF_ACTIVE_STRING("Setup Screen") << " >>\n";
	std::cout << "Enter the game configuration as follows:\n" 
		<< "\t player1\t\tplayer2\t\t\thint depth\tsearch depth\n"
		<< "\t(human or computer)\t(human or computer)\t[0-9]\t\t[0-9]" << std::endl;
	std:: cout << ">>> ";
	std::cout.flush();

	std::getline(std::cin, cmd);
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
	std::stringstream ss(cmd);

	for (int i = 0; i < NUM_TEAMS; ++i) {
		if (!(ss >> cmd)) {
			return 1;
		}
		if (cmd == "human") {
			copy.players[i] = HUMAN;
		} else if (cmd == "computer") {
			copy.players[i] = COMPUTER;
		} else {
			return 1;
		}
	}

	if (!(ss >> cmd)) {
		return 1;
	}
	copy.hint_depth = std::stoi(cmd);

	if (!(ss >> cmd)) {
		return 1;
	}
	copy.search_depth = std::stoi(cmd);

	config = copy;
	std::cout << "Writing new configuration..." << std::endl;
	return 0;
}

int main_menu(Config &config)
{
	std::cout << FF_ACTIVE_STRING("---------------------------------------\n")
		<< "<<<<<<<< " << FF_SUCCESS_STRING("Welcome to Fast Feud!") << " >>>>>>>>\n" 
		<< FF_ACTIVE_STRING("---------------------------------------\n\n")
		<< "Commands: help, rules, setup, start, quit" << std::endl;

	std::string cmd;

	while (1) {
		std::cout << ">> ";
		std::cout.flush();

		std::getline(std::cin, cmd);
		std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
		std::stringstream ss(cmd);

		while (ss >> cmd) {
			if (cmd == "help") {
				std::cout << "<< " FF_ACTIVE_STRING("Help Screen") << " >>\n"
					<< "\tHELP  - Prints out this message.\n"
					<< "\tRULES - Provides a summary of the game rules.\n"
					<< "\tSETUP - Configure game settings.\n"
					<< "\tSTART - Begin playing the game.\n"
					<< "\tQUIT  - Exits the program." << std::endl;
			} else if (cmd == "rules") {
				std::ifstream rules_file{"config/rules.txt"};
				if (rules_file.is_open()) {
					std::cout << "<< " FF_ACTIVE_STRING("Rules Screen") " >>\n" << rules_file.rdbuf();
				} else {
					std::cout << FF_ERROR_STRING("Couldn't find the rules file...") << std::endl;
				}
			} else if (cmd == "setup") {
				if (setup_menu(config)) {
					std::cout << FF_ERROR_STRING("Bad commands provided, setup not updated...") << std::endl;
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

std::vector<uint_fast8_t> get_input()
{
	std::string pos;
	static const std::string skip{"skip"};

	std::vector<uint_fast8_t> out;
	uint_fast8_t loc;

	std::cout << "Move: ";
	std::cout.flush();

	std::getline(std::cin, pos);
	std::transform(pos.begin(), pos.end(), pos.begin(), ::tolower);
	std::stringstream ss(pos);

	while (ss >> pos) {
		if (pos == skip) {
			loc = BOARD_SIZE;
		} else {
			if (pos.length() != 2 || !('a' <= pos[0] && pos[0] <= 'd')
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

	std::cout << "  |      A      |      B      |      C      |      D      |\n"
		<< "--|-------------|-------------|-------------|-------------|\n";
	for (auto &stats : tiles) {
		if (i % BOARD_WIDTH == 0) {
			std::cout << i / BOARD_WIDTH + 1 << " ";
		}
		if (stats.hp > 0) {
			if (stats.active) {
				std::cout << "| " << FF_ACTIVE_STRING(stats.team << " " << stats.type << " "
						<< (int)stats.hp << " " << (int)stats.max_hp) << " ";
			} else {
				std::cout << "| " << FF_INACTIVE_STRING(stats.team << " " << stats.type << " "
						<< (int)stats.hp << " " << (int)stats.max_hp) << " ";
			}
		} else {
			std::cout << "|" << FF_INACTIVE_STRING(" ..... . . . ");
		}
		if (i++ % BOARD_WIDTH == BOARD_WIDTH - 1) {
			std::cout << "|\n"
				<< "--|-------------|-------------|-------------|-------------|\n";
		}
	}
	std::cout << std::endl;
}

std::string pretty_print_swap(std::pair<uint_fast8_t, uint_fast8_t> &swap)
{
	std::string out = " (" + index2rank_file[swap.first] + ", " + index2rank_file[swap.second] +  ")"; 
	return out;
}

std::string pretty_print_action(action &action)
{
	std::string out;

	if (action.pos == BOARD_SIZE) {
		out = " (SKIP)";
	} else {
		out = " (" + index2rank_file[action.pos] + ", [";
		for (int i = 0; i < action.num_trgts; ++i) {
			out += index2rank_file[action.trgts[i]];
			if (i != action.num_trgts - 1) {
				out += ", ";
			}
		}
		out += "])";
	}
	return out;
}

void display_moves(Board &b, int depth)
{
	int index = -1;
	if (depth) {
		index = suggest_move(b, depth);
	}

	if (b.state == SWAP) {
		auto swaps = b.generate_swaps();
		std::cout << "Swaps:";
		for (auto &swap : swaps) {
			std::cout << pretty_print_swap(swap);
		}
		if (depth) {
			auto &swap = swaps[index];
			std::cout << "\nSuggested Move: " << pretty_print_swap(swap);
		}
	} else {
		auto actions = b.generate_actions();
		std::cout << "Actions:";
		for (auto &action : actions) {
			std::cout << pretty_print_action(action);
		}
		if (depth) {
			auto &action = actions[index];
			std::cout << "\nSuggested Move: ";
			std::cout << pretty_print_action(action);
		}
	}
	std::cout << std::endl;
}

bool is_swap_valid(std::vector<uint_fast8_t> swap, std::vector<std::pair<uint_fast8_t, uint_fast8_t>> valid_swaps)
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

int human_get_input(Board &b, MoveChoice &choice)
{
	std::vector<uint_fast8_t> locations;

	try {
		locations = get_input();
		std::cout << std::endl;
	} catch (const std::invalid_argument &e) {
		std::cout << FF_ERROR_STRING("Bad input recieved...\n") << std::endl;
		return 1;
	}

	if (b.state == SWAP) {
		if (locations.size() != 2) {
			std::cout << FF_ERROR_STRING("Need two positions for swap, recieved "
					<< locations.size() << " positions...\n") << std::endl;
			return 1;
		} else if (!is_swap_valid(locations, b.generate_swaps())) {
			std::cout << FF_ERROR_STRING("Invalid swap provided, try again...\n") << std::endl;
			return 1;
		}

		choice.swap.first = locations[0];
		choice.swap.second = locations[1];

	} else {
		if (locations.size() < 2 && (locations.size() != 1 || locations[0] != BOARD_SIZE)) {
			std::cout << FF_ERROR_STRING("Need at least two positions for action, recieved "
					<< locations.size() << " positions...\n") << std::endl;
			return 1;
		} 

		choice.act.pos = locations[0];
		choice.act.num_trgts = locations.size() - 1;

		for (int i = 1; i < locations.size(); ++i) {
			choice.act.trgts[i - 1] = locations[i];
		}
		
		if (!is_action_valid(choice.act, b.generate_actions())) {
			std::cout << FF_ERROR_STRING("Invalid action provided, try again...\n") << std::endl;
			return 1;
		}
	}

	return 0;
}

void play(Board &b, Config &config)
{
	if (!b.load_file(config.filename)) {
		return;
	}
	
	std::pair<Team, Win_Condition> winner_info{NONE, NO_WINNER};
	
	while (1) {
		Team turn = b.to_play;
		std::cout << "  Turn: " << FF_SUCCESS_STRING(turn) << "\tState: " << FF_SUCCESS_STRING(b.state)
			<< "\tTurn Count: " << b.turn_count / 4 + 1 
			<< "\n  Skips Black: " << b.get_passes(BLACK) << "\t\tSkips White: " << b.get_passes(WHITE) 
			<< "\n" << std::endl;

		pretty_print_board(b);
		display_moves(b, config.hint_depth);

		MoveChoice choice;

		if (config.players[turn] == HUMAN) {
			if (human_get_input(b, choice)) {
				continue;
			}
		} else {
			// computer move
			std::cout << "Move: ";
			const int move_index = suggest_move(b, config.search_depth);
			if (b.state == SWAP) {
				auto swaps = b.generate_swaps();
				choice.swap.first = swaps[move_index].first; 
				choice.swap.second = swaps[move_index].second; 
				std::cout << pretty_print_swap(swaps[move_index]);
			} else {
				auto actions = b.generate_actions();
				choice.act = actions[move_index];
				std::cout << pretty_print_action(choice.act);
			}
			std::cout << "\n" << std::endl;
		}

		if (b.state == SWAP) {
			b.apply_swap(choice.swap.first, choice.swap.second);
		} else {
			b.apply_action(choice.act);
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
	Config config{{HUMAN, COMPUTER}, "config/positions/default1.txt", 0, 6};

	while (1) {
		if (main_menu(config)) {
			return 0;
		}

		play(b, config);
	}

	return 0;
}
