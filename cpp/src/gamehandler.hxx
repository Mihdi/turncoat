#pragma once
#ifndef GAMEHANDLER_HXX
#define GAMEHANDLER_HXX

#include <array>
#include <functional>

#include "./gamestate.hxx"
#include "./gameview.hxx"
#include "./order.hxx"
#include "./util.hxx"

namespace Turncoat
{

template<
	uint8_t NB_HEXAGONS,
	uint8_t NB_FACTIONS,
	uint8_t NB_PER_FACTION,
	uint8_t NB_PLAYERS,
	uint8_t NB_INITIAL_PER_HAND,
	uint8_t NB_STARTING_UNITS
>
class GameHandler
{
	using CorrespondingGameViewType = GameView<NB_HEXAGONS, NB_FACTIONS>;
	using CorrespondingOrderType = Order<NB_HEXAGONS, NB_FACTIONS, NB_PER_FACTION>;

protected:
	GameState<NB_HEXAGONS, NB_FACTIONS, NB_PER_FACTION, NB_PLAYERS, NB_INITIAL_PER_HAND, NB_STARTING_UNITS> game_state;
	
	const std::array<
		std::function<
			CorrespondingOrderType(const CorrespondingGameViewType)
		>,
		NB_PLAYERS
	> order_getters;

	const std::array<
		std::function<
			uint8_t(const std::array<uint8_t, NB_FACTIONS>&)
		>,
		NB_PLAYERS
	> discarded_faction_pickers;

	uint8_t current_player_idx;

	CorrespondingGameViewType get_game_view_for_player(uint8_t player_idx) const
	{
		return CorrespondingGameViewType(
			this->game_state.attack_zone,
			this->game_state.rally_zone,
			this->game_state.hands[player_idx],
			this->game_state.hexagons
		);
	}

	inline CorrespondingOrderType get_order_from_player(const uint8_t current_player_idx) const
	{
		const auto current_player_game_view = this->get_game_view_for_player(current_player_idx);

		while(true)
		{
			const auto current_order = this->order_getters[this->current_player_idx](current_player_game_view);

			if(current_order.is_valid())
			{
				return current_order;
			}

			LOG_ERR(
				"[GameHandler.get_order_from_player]\tWarning: was given invalid order by player $"
				<< (int)current_player_idx
			);
		}
	}

public:
	GameHandler(
		const std::array<std::array<bool, NB_HEXAGONS>, NB_HEXAGONS> *adjacency_graph,
		const std::array<uint8_t, NB_FACTIONS> *factions_starting_point,
		const std::unordered_set<uint8_t> *unreachable_hexagons,
		std::mt19937 *random_generator,
		const std::array<
			std::function<
				CorrespondingOrderType(const CorrespondingGameViewType)
			>,
			NB_PLAYERS
		> order_getters,
		const std::array<
			std::function<
				uint8_t(const std::array<uint8_t, NB_FACTIONS>&)
			>,
			NB_PLAYERS
		> discarded_faction_pickers
	):
	game_state(adjacency_graph, factions_starting_point, unreachable_hexagons, random_generator),
	order_getters(order_getters),
	discarded_faction_pickers(discarded_faction_pickers),
	current_player_idx(0)
	{}

	// returns whether order was successfully executed
	bool run_turn(void)
	{
		const auto current_order = get_order_from_player(this->current_player_idx);
		bool order_succeeded = false;
		switch(current_order.order_type)
		{
			case ATTACK:
				order_succeeded = this->game_state.attack(
					this->current_player_idx,
					current_order.atking_faction_idx,
					current_order.atked_faction_idx,
					current_order.nb_atked_units,
					current_order.hexagon_idx
				);
				break;

			case DEPLOY:
				order_succeeded = this->game_state.deploy(
					this->current_player_idx,
					current_order.faction_idx,
					current_order.hexagon_idx
				);
				break;

			case NEGOCIATE:
				order_succeeded = this->game_state.negociate(
					this->current_player_idx,
					this->discarded_faction_pickers[this->current_player_idx]
				);
				break;

			case RALLY:
				order_succeeded = this->game_state.rally(
					this->current_player_idx,
					current_order.faction_idx,
					current_order.nb_units,
					current_order.start_hexagon_idx,
					current_order.end_hexagon_idx
				);
				break;

			default:
				panic("[GameHandler.run_all_turns]\tImpossible state detected: given order of unknown type");
				return false;
		}

		if (!order_succeeded)
		{
			return false;
		}

		// knowledge is knowing the two following lines can me written in one, wisdom is not doing it
		++(this->current_player_idx);
		this->current_player_idx *= (this->current_player_idx < NB_PLAYERS);

		return true;
	}

	void run_all_turns(void)
	{
		while(this->game_state.get_successive_negociation_counter() != NB_PLAYERS)
		{
			if(!this->run_turn())
			{
				LOG_ERR("[GameHandler.run_all_turns]\tOrder failed!");
			}
		}
	}

	uint8_t get_winner(void)
	{
		return this->game_state.get_winning_hand();
	}
};

} // Turncoat
#endif // GAMEHANDLER_HXX
