#include <string>
#include <sstream>
#include <stdexcept>
#include "board.h"


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
		if (pos.length() != 2 || !('a' <= pos[0] && pos[0] <= 'd') || !('1' <= pos[1] && pos[1] <= '4')) {
			throw std::invalid_argument("Input formated incorrectly");
		}
		loc = BOARD_WIDTH * (pos[1] - '1') + (pos[0] - 'a');
		out.push_back(loc);
	}

	return out;
}

int team_isolated(Board &b)
{
	int out = -1;

	if (b.isolated(BLACK) && b.isolated(WHITE)) {
		out = 2;
		std::cout << "Both teams isolated, its a tie!" << std::endl;
	} else if (b.isolated(BLACK)) {
		out = 0;
		std::cout << "Black isolated, White wins!" << std::endl;
	} else if (b.isolated(WHITE)) {
		out = 1;
		std::cout << "White isolated, Black wins!" << std::endl;
	}

	return out;
}

void display_moves(Board &b)
{
	if (b.state == SWAP) {
		auto swaps = b.generate_swaps();
		std::cout << "Swaps:";
		for (auto &swap : swaps) {
			std::cout << " (" << (int)swap.first << ", " << (int)swap.second << ")";
		}
	} else {
		auto actions = b.generate_actions();
		std::cout << "Actions:";
		for (auto &action : actions) {
			std::cout <<" (" << (int)action.pos << ", [";
			for (int i = 0; i < action.num_trgts; ++i) {
				std::cout << (int)action.trgts[i];
				if (i != action.num_trgts - 1) {
					std::cout << ", ";
				}
			}
			std::cout << "])";
		}
	}
	std::cout << std::endl;
}

int main(void)
{
	Board b;
	setup_board(b);
	bool finished = false;
	
	while (!finished) {
		Team turn = b.to_play;
		std::cout << "Turn: " << turn << "\nState: " << b.state << std::endl;
		std::cout << b << std::endl;

		display_moves(b);

		std::vector<int_fast8_t> locations;
		try {
			locations = get_input();
		} catch (const std::invalid_argument &e) {
			std::cout << "Bad input recieved" << std::endl;
			continue;
		}

		if (b.state == SWAP) {
			if (locations.size() != 2) {
				std::cout << "Need two positions for swap, recieved " << locations.size() << std::endl;
				continue;
			}

			b.apply_swap(locations[0], locations[1]);

			if (team_isolated(b) > -1) {
				finished = true;
				std::cout << b << std::endl;
			}
		} else {
			if (locations.size() < 2) {
				std::cout << "Need at least two positions for action, recieved " << locations.size() << std::endl;
				continue;
			}

			action a;
			a.pos = locations[0];
			a.num_trgts = locations.size() - 1;

			for (int i = 1; i < locations.size(); ++i) {
				a.trgts[i-1] = locations[i];
			}

			b.apply_action(a);

			if (team_isolated(b) > -1) {
				finished = true;
				std::cout << b << std::endl;
			} else if (b.king_dead(b.to_play)) { // since action is applied to_play will be next player
				finished = true;
				std::cout << b.to_play << " king is dead!" << std::endl;
				std::cout << b << std::endl;
			} else if (b.surrendered(turn)) {
				finished = true;
				std::cout << turn << " surrendered!" << std::endl;
				std::cout << b << std::endl;
			}
		}
	}

	return 0;
}
