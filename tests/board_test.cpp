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
