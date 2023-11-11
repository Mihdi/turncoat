#pragma once
#ifndef GAMESTATE_HXX
#define GAMESTATE_HXX

#include <array>
#include <unordered_set>
#include <functional>
#include <random>
#include <ctime>

#include <iostream>

namespace Turncoat
{
template<
	uint8_t NB_HEXAGONS,
	uint8_t NB_FACTIONS,
	uint8_t NB_PER_FACTION,
	uint8_t NB_HANDS,
	uint8_t NB_INITIAL_PER_HAND,
	uint8_t NB_STARTING_UNITS
>
class GameState
{
private:
protected:
	std::mt19937 *random_generator;

	uint8_t successive_negociation_counter;

	std::array<uint8_t, NB_FACTIONS> attack_zone;
	std::array<uint8_t, NB_FACTIONS> rally_zone;
	std::array<uint8_t, NB_FACTIONS> bag;
	std::array<std::array<uint8_t, NB_FACTIONS>, NB_HANDS> hands;
	std::array<std::array<uint8_t, NB_FACTIONS>, NB_HEXAGONS> hexagons;

	const std::array<std::array<bool, NB_HEXAGONS>, NB_HEXAGONS> *adjacency_graph; // maybe make this a simple array instead of pointer to array
	const std::unordered_set<uint8_t> *unreachable_hexagons;

	uint8_t winning_hand_idx;

	inline uint8_t total_in_bag(void) const
	{
		return std::accumulate(this->bag.begin() , this->bag.end(), 0);
	}

	uint8_t draw_random_from_bag(void)
	{
		const auto alpha = this->random_generator->min();
		const auto beta = this->random_generator->max();
		const auto x = (*(this->random_generator))(); // random variable
		const auto chosen_unit = this->total_in_bag() * (x - alpha) / (beta - alpha);

		auto accumulated_units = 0;
		uint8_t faction_i = 0;
		
		for (; faction_i < NB_FACTIONS; ++faction_i)
		{
			const auto added_units = this->bag[faction_i];
			accumulated_units += added_units;
			if (chosen_unit < accumulated_units)
			{
				break;
			}
		}

		--(this->bag[faction_i]);
		return faction_i;
	}

	inline bool is_hexagon_reachable(const uint8_t hexagon_idx) const
	{
		return (
			std::find(
				this->unreachable_hexagons->begin(),
				this->unreachable_hexagons->end(),
				hexagon_idx
			) == this->unreachable_hexagons->end()
		);
	}

	inline bool are_hexagons_adjacent(const uint8_t first_hexagon_idx, const uint8_t second_hexagon_idx) const
	{
		return (*(this->adjacency_graph))[first_hexagon_idx][second_hexagon_idx];
	}

	// returns arr.size() if tied
	static uint8_t non_tied_max_idx(std::array<uint8_t, NB_FACTIONS> &arr)
	{
		auto biggest_found = 0;
		bool tied = false;

		for (auto i = 1; i < arr.size(); ++i)
		{
			if((arr[i] > arr[biggest_found]))
			{
				biggest_found = i;
				tied = false;
			}
			else
			{
				tied |= arr[i] == arr[biggest_found];
			}
		}

		const auto out = ((tied) * arr.size() + (!tied) * biggest_found);
		return out;
	}

	// returns NB_FACTIONS if no winner
	uint8_t faction_which_has_land_control(void)
	{
		std::array<uint8_t, NB_FACTIONS> nb_land_under_control = {0};
		for (auto hexagon_i = 0; hexagon_i < NB_HEXAGONS; ++hexagon_i)
		{
			if(!this->is_hexagon_reachable(hexagon_i))
			{
				continue;
			}

			const auto controlling_faction = non_tied_max_idx(this->hexagons[hexagon_i]);
			if(controlling_faction != NB_FACTIONS)
			{
				++(nb_land_under_control[controlling_faction]);
			}
		}

		const auto most_controlling = non_tied_max_idx(nb_land_under_control);
		return most_controlling;
	}

	inline uint8_t faction_which_atked_the_most(void)
	{
		return non_tied_max_idx(this->attack_zone);
	}

	inline uint8_t faction_which_rallied_the_most(void)
	{
		return non_tied_max_idx(this->rally_zone);
	}

	// returns NB_FACTIONS if no winner
	uint8_t get_winning_faction(void)
	{
		const auto most_land_controlling = this->faction_which_has_land_control();
		if(most_land_controlling != NB_FACTIONS)
		{
			return most_land_controlling;
		}

		const auto most_atking = this->faction_which_atked_the_most();
		if(most_atking != NB_FACTIONS)
		{
			return most_atking;
		}

		const auto most_rallying = this->faction_which_rallied_the_most();
		return most_rallying;
	}


public:
	GameState(
		const std::array<std::array<bool, NB_HEXAGONS>, NB_HEXAGONS> *adjacency_graph,
		const std::array<uint8_t, NB_FACTIONS> *factions_starting_point,
		const std::unordered_set<uint8_t> *unreachable_hexagons,
		std::mt19937 *random_generator
	):
	hands({0}),
	hexagons({0}),
	attack_zone({0}),
	rally_zone({0}),
	bag({0}),
	successive_negociation_counter(0),
	adjacency_graph(adjacency_graph),
	unreachable_hexagons(unreachable_hexagons),
	random_generator(random_generator),
	winning_hand_idx(NB_HANDS)
	{
		for (uint8_t faction_i = 0; faction_i < NB_FACTIONS; ++faction_i)
		{
			this->bag[faction_i] = NB_PER_FACTION - NB_STARTING_UNITS;

			this->attack_zone[faction_i] = 0;
			this->rally_zone[faction_i] = 0;

			const uint8_t faction_starting_point = (*factions_starting_point)[faction_i];
			this->hexagons[faction_starting_point][faction_i] = NB_STARTING_UNITS;
		}

		for (auto hexagon_i = 0; hexagon_i < NB_HEXAGONS; ++hexagon_i)
		{
			if(
				(
					std::find(
						factions_starting_point->begin(),
						factions_starting_point->end(),
						hexagon_i
					) == factions_starting_point->end()
				)
				&&
				(
					this->is_hexagon_reachable(hexagon_i)
				)
			) {
				for (auto starting_units_i = 0; starting_units_i < NB_STARTING_UNITS; ++starting_units_i)
				{
					const auto drawn_faction = this->draw_random_from_bag();
					++(this->hexagons[hexagon_i][drawn_faction]);
				}
			}
		}

		for (auto hand_i = 0; hand_i < NB_HANDS; ++hand_i)
		{
			for (auto initial_i = 0; initial_i < NB_INITIAL_PER_HAND; ++initial_i)
			{
				const auto drawn_faction = this->draw_random_from_bag();
				++(this->hands[hand_i][drawn_faction]);
			}
		}
	}

	uint8_t get_successive_negociation_counter(void) const
	{
		return this->successive_negociation_counter;
	}

	// returns whether it succeeded
	bool negociate(
		const uint8_t hand_idx,
		const std::function<uint8_t(const std::array<uint8_t, NB_FACTIONS>&)> discarded_faction_picker
	)
	{
		++successive_negociation_counter;
		
		const auto drawn_idx = this->draw_random_from_bag();
		++(this->hands[hand_idx][drawn_idx]);

		const auto chosen_faction = discarded_faction_picker(this->hands[hand_idx]);
		if(this->hands[hand_idx][chosen_faction] == 0)
		{
			++(this->bag[drawn_idx]); // putting back in place
			return false;
		}

		--(this->hands[hand_idx][chosen_faction]);
		++(this->bag[chosen_faction]);

		return true;
	}

	// returns whether it succeeded
	bool attack(
		const uint8_t hand_idx,
		const uint8_t atking_faction_idx,
		const uint8_t atked_faction_idx,
		const uint8_t nb_atked_units,
		const uint8_t hexagon_idx
	)
	{
		if(
			this->hands[hand_idx][atking_faction_idx] == 0
			|| !this->is_hexagon_reachable(hexagon_idx)
			|| atking_faction_idx == atked_faction_idx
			|| this->hexagons[hexagon_idx][atking_faction_idx] < nb_atked_units
			|| this->hexagons[hexagon_idx][atked_faction_idx] < nb_atked_units
		)
		{
			return false;
		}

		--(this->hands[hand_idx][atking_faction_idx]);
		++(this->attack_zone[atking_faction_idx]);

		this->hexagons[hexagon_idx][atked_faction_idx] -= nb_atked_units;
		this->bag[atked_faction_idx] += nb_atked_units;

		this->successive_negociation_counter = 0;

		return true;
	}

	// returns whether it succeeded
	bool rally(
		const uint8_t hand_idx,
		const uint8_t faction_idx,
		const uint8_t nb_units,
		const uint8_t start_hexagon_idx,
		const uint8_t end_hexagon_idx
	)
	{
		if(
			this->hands[hand_idx][faction_idx] == 0
			|| !this->is_hexagon_reachable(start_hexagon_idx)
			|| !this->is_hexagon_reachable(end_hexagon_idx)
			|| this->hexagons[start_hexagon_idx][faction_idx] < nb_units
			|| !this->are_hexagons_adjacent(start_hexagon_idx, end_hexagon_idx)
		)
		{
			return false;
		}

		this->hexagons[start_hexagon_idx][faction_idx] -= nb_units;
		this->hexagons[end_hexagon_idx][faction_idx] += nb_units;

		--(this->hands[hand_idx][faction_idx]);
		++(this->rally_zone[faction_idx]);

		this->successive_negociation_counter = 0;

		return true;
	}

	// returns whether it succeeded
	bool deploy(
		const uint8_t hand_idx,
		const uint8_t faction_idx,
		const uint8_t hexagon_idx
	){
		if (this->hands[hand_idx][faction_idx] < 1)
		{
			return false;
		}

		--(this->hands[hand_idx][faction_idx]);
		++(this->hexagons[hexagon_idx][faction_idx]);

		this->successive_negociation_counter = 0;

		return true;
	}

	uint8_t get_winning_hand(void)
	{
		if(this->winning_hand_idx != NB_HANDS)
		{
			return this->winning_hand_idx;
		}

		// winner is the one who has the more of the winning faction
		const auto winning_faction = this->get_winning_faction();
		if(winning_faction != NB_FACTIONS)
		{
			this->winning_hand_idx = 0;
			bool tied = false;

			for (auto hand_i = 1; hand_i < NB_HANDS; ++hand_i)
			{
				if(this->hands[hand_i][winning_faction] > this->hands[this->winning_hand_idx][winning_faction])
				{
					this->winning_hand_idx = hand_i;
					tied = false;
				}
				else
				{
					tied |= this->hands[hand_i][winning_faction] == this->hands[this->winning_hand_idx][winning_faction];
				}
			}

			if(!tied)
			{
				return this->winning_hand_idx;
			}
		}

		// else, winning_hand_idx is the one who has less of the losing factions
		auto min_losing_units = 0;
		this->winning_hand_idx = NB_HANDS;
		bool tied = false;

		for (auto hand_i = 0; hand_i < NB_HANDS; ++hand_i)
		{
			auto current_player_losing_units = 0;
			for (auto faction_i = 0; faction_i < NB_FACTIONS; ++faction_i)
			{
				if(faction_i == winning_faction)
				{
					continue;
				}
				current_player_losing_units += this->hands[hand_i][faction_i];
			}
			if(this->winning_hand_idx == NB_HANDS || current_player_losing_units < min_losing_units)
			{
				min_losing_units = current_player_losing_units;
				this->winning_hand_idx = hand_i;
				tied = false;
			}
			else
			{
				tied |= current_player_losing_units == min_losing_units;
			}
		}

		if(!tied)
		{
			return this->winning_hand_idx;
		}

		// in the end decide a random winning_hand_idx
		const uint8_t random_winning_hand_idx = (*(this->random_generator))() % NB_HANDS; 

		this->winning_hand_idx = random_winning_hand_idx;

		return random_winning_hand_idx;
	}

};

} // Turncoat
#endif // GAMESTATE_HXX
