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

TEST(BoardBug, MedicOverheal)
{
	// make sure that the medic cannot heal pieces that are at full hp
	// bug was due to the damaged bit board not being properly updated
	Board b;

	std::string file_name = "test_positions/medic_bug.txt"; 
	ASSERT_EQ(b.load_file(file_name), true);

	ASSERT_EQ(b.state, ACTION);
	ASSERT_EQ(b.to_play, BLACK);

	action a;
	a.pos = 1;
	a.num_trgts = 1;
	a.trgts[0] = 13;
	
	b.apply_action(a);

	ASSERT_EQ(b.state, SWAP);
	ASSERT_EQ(b.to_play, WHITE);

	b.apply_swap(12, 13);

	ASSERT_EQ(b.state, ACTION);
	ASSERT_EQ(b.to_play, WHITE);

	auto actions = b.generate_actions();

	// No actions besides skipping should be avaliable
	ASSERT_EQ(actions.size(), 1);
}

TEST(BoardHashingTests, Simple)
{
	Board b1, b2;

	std::string file_name = "test_positions/medic_bug.txt"; 
	ASSERT_EQ(b1.load_file(file_name), true);

	ASSERT_EQ(b2.load_hash(b1.hash()), true);
	EXPECT_EQ(b1.hash(), b2.hash());

	EXPECT_EQ(b1.state, b2.state);
	EXPECT_EQ(b1.to_play, b2.to_play);
	EXPECT_EQ(b1.turn_count, b2.turn_count);

	auto tiles1 = b1.tile_info();
	auto tiles2 = b2.tile_info();
	for (int i = 0; i < tiles1.size(); ++i) {
		auto &t1 = tiles1[i];
		auto &t2 = tiles2[i];

		EXPECT_EQ(t1.team, t2.team);
		EXPECT_EQ(t1.type, t2.type);
		EXPECT_EQ(t1.hp, t2.hp);
		EXPECT_EQ(t1.max_hp, t2.max_hp);
	}
}
