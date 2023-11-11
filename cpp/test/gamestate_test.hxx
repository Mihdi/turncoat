#pragma once
#ifndef GAMESTATE_TEST_HXX
#define GAMESTATE_TEST_HXX

#include <array>
#include <unordered_set>
#include <cstdint>
#include <numeric>
#include <cstring>

#include "test.hxx"
#include "../src/gamestate.hxx"

#include <iostream>

using namespace Turncoat;

using DefaultGameStateType = GameState<
  	default_nb_hexagons,
  	default_nb_factions,
  	default_nb_per_faction,
  	default_nb_hands,
  	default_nb_per_hand,
  	default_nb_starting_units
  >;


#ifndef INITIALIZE_DEFAULT_GAMESTATE
#define INITIALIZE_DEFAULT_GAMESTATE(ConstToken, VarName, Seed)								\
																						\
std::mt19937 random_generator = std::mt19937(Seed);							\
																						\
std::array<std::array<bool, default_nb_hexagons>, default_nb_hexagons> adjacency_graph;	\
fill_default_adjacency_graph(adjacency_graph);											\
																						\
const std::unordered_set<uint8_t> unreachable_hexagons{7};								\
const std::array<uint8_t, default_nb_factions> factions_starting_point = {{2, 6, 11}};	\
																						\
 ConstToken auto VarName = DefaultGameStateType(										\
  	&adjacency_graph,																	\
  	&factions_starting_point,															\
  	&unreachable_hexagons,																\
  	&random_generator																	\
  );
#endif //INITIALIZE_DEFAULT_GAMESTATE

/*
 * this is enough because rn there is no mutable pointer in GameState apart from the random generator
 * and i don't care about it changing for now
 * however remember to change this if you add mutable pointers in gamestate (which kinda goes against the spirit, but eh)
*/
bool is_unchanged_after_call(void *obj, const size_t obj_size, std::function<void(void *)> call)
{
	void *sealed_obj_copy = ::malloc(obj_size);
	::memcpy(sealed_obj_copy, obj, obj_size); // note that this also copies the padding, which allows for the comparison here

	call(obj);

	const bool out = 0 == ::memcmp(obj, sealed_obj_copy, obj_size);

	::free(sealed_obj_copy);

	return out;
}

void test_gamestate_constructor(void)
{
	INITIALIZE_DEFAULT_GAMESTATE(const, tested, time(nullptr))

	// sanity checks
	assert(tested.successive_negociation_counter == 0);
	assert(tested.adjacency_graph == &adjacency_graph);
	assert(tested.random_generator == &random_generator);
	assert(tested.unreachable_hexagons == &unreachable_hexagons);
	assert(tested.attack_zone.size() == default_nb_factions);
	assert(tested.rally_zone.size() == default_nb_factions);

	// checking attack and rally zones are at 0 by default
	for (auto faction_i = 0; faction_i < default_nb_factions; ++faction_i)
	{
		assert(tested.attack_zone[faction_i] == 0);
		assert(tested.rally_zone[faction_i] == 0);
	}

	// checking correct amount of units in bag
	const auto expected_number_in_bag = (
		default_nb_per_faction * default_nb_factions
		- default_nb_starting_units * default_nb_hexagons 
		- default_nb_hands * default_nb_per_hand
		+ unreachable_hexagons.size() * default_nb_starting_units
	);
	assert(tested.total_in_bag() == expected_number_in_bag);

	// checking correct number of units is initialized in the faction starting points
	for (auto faction_i = 0; faction_i < default_nb_factions; ++faction_i)
	{
		assert(tested.hexagons[factions_starting_point[faction_i]][faction_i] == default_nb_starting_units);
	}

	// checking correct number of units is intiialized in all reachable hexagons
	for (auto hexagon_i = 0; hexagon_i < default_nb_hexagons; ++hexagon_i)
	{
		if(!tested.is_hexagon_reachable(hexagon_i))
		{
			continue;
		}

		uint16_t count = 0;
		for (auto faction_i = 0; faction_i < default_nb_factions; ++faction_i)
		{
			count += tested.hexagons[hexagon_i][faction_i];
		}
		assert(count == default_nb_starting_units);
	}

	// checking that the total number in hand is correct
	for (auto hand_i = 0; hand_i < default_nb_hands; ++hand_i)
	{
		uint16_t count = 0;
		for (auto faction_i = 0; faction_i < default_nb_factions; ++faction_i)
		{
			count += tested.hands[hand_i][faction_i];
		}
		assert(count == default_nb_per_hand);
	}
}

void test_draw_random_from_bag(void)
{
	INITIALIZE_DEFAULT_GAMESTATE(, tested, time(nullptr))

	auto previous = tested.total_in_bag();
	while(true)
	{
		tested.draw_random_from_bag();
		auto current = tested.total_in_bag();

		assert(current == (previous - 1));

		previous = current;

		if(previous == 0)
		{
			break;
		}
	}

	for (auto faction_i = 0; faction_i < default_nb_factions; ++faction_i)
	{
		assert(tested.bag[faction_i] == 0);
	}
}

void test_negociate(void)
{
	INITIALIZE_DEFAULT_GAMESTATE(, tested, time(nullptr))

	tested.bag[0] = 0;
	tested.bag[1] = 0;
	tested.bag[2] = default_nb_per_hand + 1;

	tested.hands[0][0] = default_nb_per_hand;
	tested.hands[0][1] = 0;
	tested.hands[0][2] = 0;

	auto pick_first_faction = [](const std::array<uint8_t, default_nb_factions>&){return 0;};

	while(tested.negociate(0, pick_first_faction));

	assert(tested.successive_negociation_counter >= default_nb_per_hand);

	assert(tested.hands[0][0] == 0);
	assert(tested.bag[0] == default_nb_per_hand);
	assert(tested.bag[1] == 0);
	assert(tested.bag[2] == 1);
}

void test_deploy(void)
{
	INITIALIZE_DEFAULT_GAMESTATE(, tested, time(nullptr))

	const auto hand_idx = 3;
	const auto hexagon_idx = 3;
	const auto faction_idx = 1;

	tested.hands[hand_idx][0] = 0;
	tested.hands[hand_idx][1] = 1;
	tested.hands[hand_idx][2] = 0;

	tested.hexagons[hexagon_idx][0] = 0;
	tested.hexagons[hexagon_idx][1] = 0;
	tested.hexagons[hexagon_idx][2] = 0;

	tested.successive_negociation_counter = 1;

	// success case
	assert(tested.deploy(hand_idx, faction_idx, hexagon_idx));
	assert(tested.hands[hand_idx][faction_idx] == 0);
	assert(tested.hexagons[hexagon_idx][faction_idx] == 1);
	assert(tested.successive_negociation_counter == 0);

	// failure case
	auto assert_no_unit_to_deploy_returns_false = (
		[faction_idx, hand_idx, hexagon_idx]
		(void *raw_p_2tested)
		{
			auto p_2_tested = (DefaultGameStateType *)raw_p_2tested;
			const bool res = p_2_tested->deploy(hand_idx, faction_idx, hexagon_idx);
			assert(!res);
		}
	);
	assert(is_unchanged_after_call(&tested, sizeof(tested), assert_no_unit_to_deploy_returns_false));
}

void test_rally(void)
{
	INITIALIZE_DEFAULT_GAMESTATE(, tested, time(nullptr))

	const auto hand_idx = 2;
	const auto start_hexagon_idx = 0;
	const auto faction_idx = 1;
	const auto wrong_faction_idx = 0;
	const auto non_adjacent_hexagon_idx = 11;
	const auto end_hexagon_idx = 1;
	const auto nb_units = 2;

	tested.hands[hand_idx][0] = 0;
	tested.hands[hand_idx][1] = 2;
	tested.hands[hand_idx][2] = 0;

	tested.hexagons[start_hexagon_idx][faction_idx] = 3;
	tested.hexagons[non_adjacent_hexagon_idx][faction_idx] = 0;
	tested.hexagons[end_hexagon_idx][faction_idx] = 0;

	tested.successive_negociation_counter = 1;

	auto assert_no_faction_in_hand_returns_false = (
		[hand_idx, wrong_faction_idx, nb_units, start_hexagon_idx, end_hexagon_idx]
		(void *raw_p_2tested)
		{
			auto p_2_tested = (DefaultGameStateType *)raw_p_2tested;
			const bool res = p_2_tested->rally(hand_idx, wrong_faction_idx, nb_units, start_hexagon_idx, end_hexagon_idx);
			assert(!res);
		}
	);
	assert(is_unchanged_after_call(&tested, sizeof(tested), assert_no_faction_in_hand_returns_false));

	auto assert_not_adjacent_returns_false = (
		[hand_idx, faction_idx, nb_units, start_hexagon_idx, non_adjacent_hexagon_idx]
		(void *raw_p_2tested)
		{
			auto p_2_tested = (DefaultGameStateType *)raw_p_2tested;
			const bool res = p_2_tested->rally(hand_idx, faction_idx, nb_units, start_hexagon_idx, non_adjacent_hexagon_idx);
			assert(!res);
		}
	);
	assert(is_unchanged_after_call(&tested, sizeof(tested), assert_not_adjacent_returns_false));

	// succeeds
	auto assert_correct_input_returns_true = (
		[hand_idx, faction_idx, nb_units, start_hexagon_idx, end_hexagon_idx]
		(void *raw_p_2tested)
		{
			auto p_2_tested = (DefaultGameStateType *)raw_p_2tested;
			const bool res = p_2_tested->rally(hand_idx, faction_idx, nb_units, start_hexagon_idx, end_hexagon_idx);
			assert(res);
		}
	);
	assert(!is_unchanged_after_call(&tested, sizeof(tested), assert_correct_input_returns_true));

	assert(tested.rally_zone[faction_idx] == 1);
	assert(tested.hands[hand_idx][faction_idx] == 1);
	assert(tested.hexagons[start_hexagon_idx][faction_idx] == 1);
	assert(	tested.hexagons[end_hexagon_idx][faction_idx] == 2);
	assert(tested.successive_negociation_counter == 0);

	// fails if nb units not available
	auto assert_nb_units_not_available_returns_false = (
		[hand_idx, faction_idx, nb_units, start_hexagon_idx, end_hexagon_idx]
		(void *raw_p_2tested)
		{
			auto p_2_tested = (DefaultGameStateType *)raw_p_2tested;
			const bool res = p_2_tested->rally(hand_idx, faction_idx, nb_units, start_hexagon_idx, end_hexagon_idx);
			assert(!res);
		}
	);
	assert(is_unchanged_after_call(&tested, sizeof(tested), assert_nb_units_not_available_returns_false));

}

void test_attack(void)
{
	INITIALIZE_DEFAULT_GAMESTATE(, tested, time(nullptr))
	// setup: consts
	const auto hand_idx = 2;
	const auto valid_hexagon_idx = 0;
	const auto invalid_hexagon_idx = 7;

	const auto atking_faction_idx = 0;
	const auto wrong_faction_idx = 1;
	const auto atked_faction_idx = 2;

	const auto nb_atked_units = 2;

	// setup: state
	for (auto faction_i = 0; faction_i < default_nb_factions; ++faction_i)
	{
		tested.hands[hand_idx][faction_i] = 0;
		tested.hexagons[valid_hexagon_idx][faction_i] = 0;
	}
	tested.hands[hand_idx][atking_faction_idx] = 1;

	tested.hexagons[valid_hexagon_idx][atked_faction_idx] = nb_atked_units + 2;
	tested.hexagons[valid_hexagon_idx][atking_faction_idx] = nb_atked_units + 1;
	tested.hexagons[valid_hexagon_idx][wrong_faction_idx] = 0;
	
	tested.bag[atked_faction_idx] = 0;
	
	tested.successive_negociation_counter = 1;

	auto assert_invalid_hex_returns_false = (
		[hand_idx, atking_faction_idx, atked_faction_idx, nb_atked_units, invalid_hexagon_idx]
		(void *raw_p_2tested)
		{
			auto p_2_tested = (DefaultGameStateType *)raw_p_2tested;
			const bool res = p_2_tested->attack(
				hand_idx, atking_faction_idx, atked_faction_idx, nb_atked_units, invalid_hexagon_idx
			);
			assert(!res);
		}
	);
	assert(is_unchanged_after_call(&tested, sizeof(tested), assert_invalid_hex_returns_false));

	auto assert_atking_and_atked_are_same_returns_false = (
		[hand_idx, atking_faction_idx, nb_atked_units, valid_hexagon_idx]
		(void *raw_p_2tested)
		{
			auto p_2_tested = (DefaultGameStateType *)raw_p_2tested;
			const bool res = p_2_tested->attack(
				hand_idx, atking_faction_idx, atking_faction_idx, nb_atked_units, valid_hexagon_idx
			);
			assert(!res);
		}
	);
	assert(is_unchanged_after_call(&tested, sizeof(tested), assert_atking_and_atked_are_same_returns_false));

	auto assert_invalid_target_returns_false = (
		[hand_idx, atking_faction_idx, wrong_faction_idx, nb_atked_units, valid_hexagon_idx]
		(void *raw_p_2tested)
		{
			auto p_2_tested = (DefaultGameStateType *)raw_p_2tested;
			const bool res = p_2_tested->attack(
				hand_idx, atking_faction_idx, wrong_faction_idx, nb_atked_units, valid_hexagon_idx
			);
			assert(!res);
		}
	);
	assert(is_unchanged_after_call(&tested, sizeof(tested), assert_invalid_target_returns_false));

	auto assert_number_of_targets_above_present_atking_units_returns_false = (
		[hand_idx, atking_faction_idx, atked_faction_idx, valid_hexagon_idx]
		(void *raw_p_2tested)
		{
			auto p_2_tested = (DefaultGameStateType *)raw_p_2tested;

			const auto nb_atking_units_present =  p_2_tested->hexagons[valid_hexagon_idx][atking_faction_idx];
			const bool res = p_2_tested->attack(
				hand_idx, atking_faction_idx, atked_faction_idx, nb_atking_units_present + 1, valid_hexagon_idx
			);
			assert(!res);
		}
	);
	assert(is_unchanged_after_call(&tested, sizeof(tested), assert_number_of_targets_above_present_atking_units_returns_false));

	// success case induces change
	auto correct_input_returns_true = (
		[hand_idx, atking_faction_idx, atked_faction_idx, nb_atked_units, valid_hexagon_idx]
		(void *raw_p_2tested){
			auto p_2_tested = (DefaultGameStateType *)raw_p_2tested;
			const bool res = p_2_tested->attack(
				hand_idx, atking_faction_idx, atked_faction_idx, nb_atked_units, valid_hexagon_idx
			);
			assert(res);
		}
	);

	assert(!is_unchanged_after_call(&tested, sizeof(tested), correct_input_returns_true));

	for (auto faction_i = 0; faction_i < default_nb_factions; ++faction_i)
	{
		if(faction_i == atking_faction_idx)
		{
			continue;
		}

		assert(tested.hands[hand_idx][faction_i] == 0);
	}
	assert(tested.hands[hand_idx][atking_faction_idx] == 0);

	assert(tested.attack_zone[atking_faction_idx] == 1);

	assert(tested.hexagons[valid_hexagon_idx][atked_faction_idx] == 2);
	assert(tested.hexagons[valid_hexagon_idx][atking_faction_idx] == nb_atked_units + 1);
	assert(tested.hexagons[valid_hexagon_idx][wrong_faction_idx] == 0);

	assert(tested.bag[atked_faction_idx] == nb_atked_units);

	assert(tested.successive_negociation_counter == 0);

	// failure case
	auto assert_no_atker_color_in_hand_returns_false = (
		[hand_idx, atking_faction_idx, atked_faction_idx, nb_atked_units, valid_hexagon_idx]
		(void *raw_p_2tested)
		{
			auto p_2_tested = (DefaultGameStateType *)raw_p_2tested;

			const auto nb_atking_units_present =  p_2_tested->hexagons[valid_hexagon_idx][atking_faction_idx];
			const bool res = p_2_tested->attack(
				hand_idx, atking_faction_idx, atked_faction_idx, nb_atked_units, valid_hexagon_idx
			);
			assert(!res);
		}
	);
	assert(is_unchanged_after_call(&tested, sizeof(tested), assert_no_atker_color_in_hand_returns_false));
}

void test_get_winning_by_control_hand(void)
{
	INITIALIZE_DEFAULT_GAMESTATE(, tested, time(nullptr))

	const auto expected_winning_faction = 2;
	const auto expected_winning_hand = 1;

	// no hand can have more than the winning one
	tested.hands[expected_winning_hand][expected_winning_faction] = default_nb_per_hand+1;

	// all hexagons contains more than half of the winning faction, making it a clear winner
	for (auto hexagon_i = 0; hexagon_i < default_nb_hexagons; ++hexagon_i)
	{
		tested.hexagons[hexagon_i][expected_winning_faction] += default_nb_starting_units + 1;
	}

	const auto actual_winning_faction = tested.get_winning_faction();
	const auto actual_winning_hand = tested.get_winning_hand();

	assert(expected_winning_faction == actual_winning_faction);
	assert(expected_winning_hand == actual_winning_hand);
}

void test_get_winning_by_attack_hand(void)
{
	INITIALIZE_DEFAULT_GAMESTATE(, tested, time(nullptr))

	// empty all hexagons
	for (auto hexagon_i = 0; hexagon_i < default_nb_hexagons; ++hexagon_i)
	{
		for (auto faction_i = 0; faction_i < default_nb_factions; ++faction_i)
		{
			tested.hexagons[hexagon_i][faction_i] = 0;
		}
	}

	const auto expected_winning_faction = 1;
	const auto expected_winning_hand = 2;

	// no hand can have more than the winning one
	tested.hands[expected_winning_hand][expected_winning_faction] = default_nb_per_hand+1;

	// make the winning faction have the most in atk zone, but do put some units in other factions
	tested.attack_zone[expected_winning_faction] = 10;
	for (auto faction_i = 0; faction_i < default_nb_factions; ++faction_i)
	{
		if(faction_i == expected_winning_faction)
		{
			continue;
		}

		tested.attack_zone[faction_i] = tested.attack_zone[expected_winning_faction] / (faction_i + 2);
	}

	const auto actual_winning_faction = tested.get_winning_faction();
	const auto actual_winning_hand = tested.get_winning_hand();

	assert(expected_winning_faction == actual_winning_faction);
	assert(expected_winning_hand == actual_winning_hand);
}

void test_get_winning_by_rally_hand(void)
{
	INITIALIZE_DEFAULT_GAMESTATE(, tested, time(nullptr))

	// empty all hexagons
	for (auto hexagon_i = 0; hexagon_i < default_nb_hexagons; ++hexagon_i)
	{
		for (auto faction_i = 0; faction_i < default_nb_factions; ++faction_i)
		{
			tested.hexagons[hexagon_i][faction_i] = 0;
		}
	}

	const auto expected_winning_faction = 1;
	const auto expected_winning_hand = 2;

	// no hand can have more than the winning one
	tested.hands[expected_winning_hand][expected_winning_faction] = default_nb_per_hand+1;

	// make the winning faction have the most in atk zone, but do put some units in other factions
	tested.attack_zone[expected_winning_faction] = 10;
	for (auto faction_i = 0; faction_i < default_nb_factions; ++faction_i)
	{
		if(faction_i == expected_winning_faction)
		{
			continue;
		}

		tested.attack_zone[faction_i] = tested.attack_zone[expected_winning_faction] / (faction_i + 2);
	}

	const auto actual_winning_faction = tested.get_winning_faction();
	const auto actual_winning_hand = tested.get_winning_hand();

	assert(expected_winning_faction == actual_winning_faction);
	assert(expected_winning_hand == actual_winning_hand);
}

void test_get_winning_by_random_hand(void)
{
	const auto seed = 0x84D8483;
	INITIALIZE_DEFAULT_GAMESTATE(, tested, seed)

	const auto expected_winning_hand = 1;

	for (auto faction_i = 0; faction_i < default_nb_factions; ++faction_i)
	{
		// empty all hexagons
		for (auto hexagon_i = 0; hexagon_i < default_nb_hexagons; ++hexagon_i)
		{
			tested.hexagons[hexagon_i][faction_i] = 0;
		}

		// empty all ahnds
		for (auto hand_i = 0; hand_i < default_nb_hands; ++hand_i)
		{
			tested.hands[hand_i][faction_i] = 0;
		}
	}

	const auto actual_winning_faction = (int)tested.get_winning_faction();
	const auto actual_winning_hand = (int)tested.get_winning_hand();

	assert(default_nb_factions == actual_winning_faction);
	assert(expected_winning_hand == actual_winning_hand);
}

void test_get_winning_hand(void)
{
	test_get_winning_by_control_hand();
	test_get_winning_by_attack_hand();
	test_get_winning_by_rally_hand();
	test_get_winning_by_random_hand();
}

void test_gamestate(void)
{
	test_gamestate_constructor();
	test_draw_random_from_bag();
	test_negociate();
	test_deploy();
	test_rally();
	test_attack();
	test_get_winning_hand();
}

#endif // GAMESTATE_TEST_HXX