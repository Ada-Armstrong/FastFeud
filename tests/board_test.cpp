#include <board.h>
#include <gtest/gtest.h>


TEST(BoardBasicTests, Setup)
{
	Board b;
	EXPECT_EQ(b.turn_count, 0) << "Turn count is initialized to " << (int)b.turn_count;

	Team t = BLACK;
	EXPECT_EQ(b.get_passes(t), 0) << "Skips for " << t << " is initialized to " << (int)b.get_passes(t);
	t = WHITE;
	EXPECT_EQ(b.get_passes(t), 0) << "Skips for " << t << " is initialized to " << (int)b.get_passes(t);
}

TEST(BoardLoadingTests, DNE)
{
	Board b;

	std::string file_name = "board_state_dne.txt"; 
	EXPECT_EQ(b.load_file(file_name), false);
}

TEST(BoardLoadingTests, Exists)
{
	Board b;

	std::string file_name = "test_positions/simple1.txt"; 
	ASSERT_EQ(b.load_file(file_name), true);

	EXPECT_EQ(b.state, ACTION);
	EXPECT_EQ(b.to_play, WHITE);
	EXPECT_EQ(b.turn_count, 42);

	Team t = BLACK;
	EXPECT_EQ(b.get_passes(t), 2) << "Skips for " << t << " is initialized to " << (int)b.get_passes(t);
	t = WHITE;
	EXPECT_EQ(b.get_passes(t), 1) << "Skips for " << t << " is initialized to " << (int)b.get_passes(t);

	auto info = b.tile_info();
	ASSERT_EQ(info.size(), 16);

	piece_stats p = info[0];
	EXPECT_EQ(p.hp, 2);
	EXPECT_EQ(p.max_hp, 4);
	EXPECT_EQ(p.team, BLACK);
	EXPECT_EQ(p.type, KING);
	EXPECT_EQ(p.active, true);

	p = info[4];
	EXPECT_EQ(p.hp, 3);
	EXPECT_EQ(p.max_hp, 3);
	EXPECT_EQ(p.team, BLACK);
	EXPECT_EQ(p.type, WIZARD);
	EXPECT_EQ(p.active, true);

	p = info[12];
	EXPECT_EQ(p.hp, 1);
	EXPECT_EQ(p.max_hp, 3);
	EXPECT_EQ(p.team, WHITE);
	EXPECT_EQ(p.type, KNIGHT);
	EXPECT_EQ(p.active, true);

	p = info[13];
	EXPECT_EQ(p.hp, 3);
	EXPECT_EQ(p.max_hp, 4);
	EXPECT_EQ(p.team, WHITE);
	EXPECT_EQ(p.type, KING);
	EXPECT_EQ(p.active, true);

	for (int i = 0; i < info.size(); ++i) {
		if (i == 0 || i == 4 || i == 12 || i == 13) {
			continue;
		}
		p = info[i];
		EXPECT_EQ(p.team, NONE);
		EXPECT_EQ(p.type, EMPTY);
	}
}
