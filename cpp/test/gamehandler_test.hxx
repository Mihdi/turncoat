#pragma once
#ifndef GAMEHANDLER_TEST_HXX
#define GAMEHANDLER_TEST_HXX

#include <cstring>

#include "test.hxx"
#include "../src/gamehandler.hxx"

using namespace Turncoat;

using DefaultGameHandlerType = GameHandler<
  	default_nb_hexagons,
  	default_nb_factions,
  	default_nb_per_faction,
  	default_nb_hands,
  	default_nb_per_hand,
  	default_nb_starting_units
  >;
using DefaultGameViewType = GameView<default_nb_hexagons, default_nb_factions>;
using DefaultOrderType = Order<default_nb_hexagons, default_nb_factions, default_nb_per_faction>;

const uint16_t MAX_ALLOWED_GAME_SIZE = sizeof(std::mt19937) + 1024;
const uint16_t MAX_ALLOWED_GAMEHANDLER_SIZE = 512;

#ifndef INITIALIZE_DEFAULT_GAMEHANDLER
#define INITIALIZE_DEFAULT_GAMEHANDLER(ConstToken, VarName) 																				\
	std::mt19937 random_generator = std::mt19937(time(nullptr));																			\
																																																		\
	std::array<std::array<bool, default_nb_hexagons>, default_nb_hexagons> adjacency_graph;						\
	fill_default_adjacency_graph(adjacency_graph);																										\
	const std::unordered_set<uint8_t> unreachable_hexagons{7};																				\
	const std::array<uint8_t, default_nb_factions> factions_starting_point = {{2, 6, 11}};						\
																																																		\
	const std::function< 																																							\
		Order<default_nb_hexagons, default_nb_factions, default_nb_per_faction>(												\
			const GameView<default_nb_hexagons, default_nb_factions>																			\
		)																																																\
	> always_order_negociate = [](const DefaultGameViewType dgvt){return DefaultOrderType();};				\
																																																		\
	std::array<																																												\
		std::function<																																									\
			DefaultOrderType(const DefaultGameViewType)																										\
		>,																																															\
		default_nb_hands																																								\
	> all_players_always_order_negociate = {																													\
		always_order_negociate, always_order_negociate, always_order_negociate, always_order_negociate	\
	};																																																\
																																																		\
	const std::function<																																							\
		uint8_t(const std::array<uint8_t, default_nb_factions>&)																				\
	> discard_first_possible = [](																																		\
		const std::array<uint8_t, default_nb_factions>& hand																						\
	){																																																\
		for (uint8_t faction_i = 0; faction_i < hand.size(); ++faction_i)																\
		{																																																\
			if(hand[faction_i] > 0)																																				\
			{																																															\
				return faction_i;																																						\
			}																																															\
		}																																																\
		return (uint8_t)hand.size();																																		\
	};																																																\
																																																		\
	std::array all_players_always_discard_first_possible = {																					\
		discard_first_possible, discard_first_possible, discard_first_possible, discard_first_possible	\
	};																																																\
																																																		\
	ConstToken auto VarName = DefaultGameHandlerType(																									\
		&adjacency_graph,																																								\
		&factions_starting_point,																																				\
		&unreachable_hexagons,																																					\
		&random_generator,																																							\
		all_players_always_order_negociate,																															\
		all_players_always_discard_first_possible																												\
	);
#endif // INITIALIZE_DEFAULT_GAMEHANDLER


void test_gamehandler_constructor(void)
{
		INITIALIZE_DEFAULT_GAMEHANDLER(, tested)
}

void test_default_memory_usage(void)
{
	INITIALIZE_DEFAULT_GAMEHANDLER(, tested)
	constexpr auto default_gamehandler_size = sizeof(tested);
	constexpr auto default_random_generator_size = sizeof(random_generator);
	constexpr auto adjacency_graph_size = sizeof(adjacency_graph);
	constexpr auto unreachable_hexagons_size = sizeof(unreachable_hexagons);

	std::cerr << "[INFO] default gamehandler memory usage is " << default_gamehandler_size << " bytes." << std::endl;
	std::cerr << "[INFO] default random generator memory usage is " << default_random_generator_size << " bytes." << std::endl;
	std::cerr << "[INFO] default adjacency graph memory usage is " << adjacency_graph_size << " bytes." << std::endl;
	std::cerr << "[INFO] default unreachable hexagons memory usage is " << unreachable_hexagons_size << " bytes." << std::endl;

	constexpr auto total_default_mem_size = (
		default_gamehandler_size
		+ default_random_generator_size
		+ adjacency_graph_size
		+ unreachable_hexagons_size
	);

	std::cerr << "[INFO] total default memory usage is " << total_default_mem_size << " bytes." << std::endl;

	assert(default_gamehandler_size < MAX_ALLOWED_GAMEHANDLER_SIZE);
	assert(total_default_mem_size < MAX_ALLOWED_GAME_SIZE);
}

void test_gamehandler_run_sanity_check(void)
{
	INITIALIZE_DEFAULT_GAMEHANDLER(, tested)

	// sanity check for dopamine
	tested.run_all_turns();
	assert(tested.game_state.successive_negociation_counter == default_nb_hands);
}

void test_gamehandler_get_winner(void)
{
	for (auto i = 0; i < 1000; ++i)
	{
		INITIALIZE_DEFAULT_GAMEHANDLER(, tested)
		assert(tested.get_winner() == tested.game_state.get_winning_hand());
	}
}

void test_gamehandler(void)
{
	test_default_memory_usage();
	test_gamehandler_constructor();
	test_gamehandler_run_sanity_check();
	test_gamehandler_get_winner();
}

#endif // GAMEHANDLER_TEST_HXX
