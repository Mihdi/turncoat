#pragma once
#ifndef ORDER_HXX
#define ORDER_HXX

namespace Turncoat
{

enum OrderType
{
	ATTACK = 1,
	DEPLOY = 2,
	NEGOCIATE = 3,
	RALLY = 4
};

template<
	uint8_t NB_HEXAGONS,
	uint8_t NB_FACTIONS,
	uint8_t NB_PER_FACTION
>
struct Order
{
	const OrderType order_type;
	const uint8_t faction_idx; // rally & deploy
	const uint8_t atking_faction_idx; // attack
	const uint8_t atked_faction_idx; // attack
	const uint8_t nb_units; // rally
	const uint8_t nb_atked_units; // attack
	const uint8_t hexagon_idx; // attack, deploy
	const uint8_t start_hexagon_idx; // rally
	const uint8_t end_hexagon_idx; // rally

	// full order constructor, idk why but might as well have it
	Order(
		const OrderType order_type,
		const uint8_t faction_idx,
		const uint8_t atking_faction_idx,
		const uint8_t atked_faction_idx,
		const uint8_t nb_units,
		const uint8_t nb_atked_units,
		const uint8_t hexagon_idx,
		const uint8_t start_hexagon_idx,
		const uint8_t end_hexagon_idx
	):
		order_type(order_type),
		faction_idx(faction_idx),
		atking_faction_idx(atking_faction_idx),
		atked_faction_idx(atked_faction_idx),
		nb_units(nb_units),
		nb_atked_units(nb_atked_units),
		hexagon_idx(hexagon_idx),
		start_hexagon_idx(start_hexagon_idx),
		end_hexagon_idx(end_hexagon_idx)
	{}

	// attack constructor
	Order(
		const uint8_t atking_faction_idx,
		const uint8_t atked_faction_idx,
		const uint8_t nb_atked_units,
		const uint8_t hexagon_idx
	):
		order_type(ATTACK),
		atking_faction_idx(atking_faction_idx),
		atked_faction_idx(atked_faction_idx),
		nb_atked_units(nb_atked_units),
		hexagon_idx(hexagon_idx),
		nb_units(0),
		faction_idx(0),
		start_hexagon_idx(0),
		end_hexagon_idx(0)
	{}

	// deploy constructor
	Order(
		const uint8_t faction_idx,
		const uint8_t hexagon_idx
	):
		order_type(DEPLOY),
		faction_idx(faction_idx),
		hexagon_idx(hexagon_idx),
		atking_faction_idx(0),
		atked_faction_idx(0),
		nb_units(0),
		nb_atked_units(0),
		start_hexagon_idx(0),
		end_hexagon_idx(0)
	{}

	// negociate constructor
	Order(void):
		order_type(NEGOCIATE),
		faction_idx(0),
		atking_faction_idx(0),
		atked_faction_idx(0),
		nb_units(0),
		nb_atked_units(0),
		hexagon_idx(0),
		start_hexagon_idx(0),
		end_hexagon_idx(0)
	{}

	// rally constructor
	Order(
		const OrderType order_type,
		const uint8_t faction_idx,
		const uint8_t nb_units,
		const uint8_t start_hexagon_idx,
		const uint8_t end_hexagon_idx
	):
		order_type(RALLY),
		faction_idx(faction_idx),
		nb_units(nb_units),
		start_hexagon_idx(start_hexagon_idx),
		end_hexagon_idx(end_hexagon_idx),
		atking_faction_idx(0),
		atked_faction_idx(0),
		nb_atked_units(0),
		hexagon_idx(0)
	{}


	static inline bool is_valid_faction_idx(const uint8_t x)
	{
		return x < NB_FACTIONS;
	}

	static inline bool is_valid_unit_nb(const uint8_t x)
	{
		return x < NB_PER_FACTION;
	}

	static inline bool is_valid_hexagon_idx(const uint8_t x)
	{
		return x < NB_HEXAGONS;
	}

	inline bool is_valid(void) const
	{
		switch(this->order_type)
		{
			case ATTACK:
				return (
					Order::is_valid_faction_idx(this->atking_faction_idx)
					&& Order::is_valid_faction_idx(this->atked_faction_idx)
					&& Order::is_valid_unit_nb(this->nb_atked_units)
					&& Order::is_valid_hexagon_idx(this->hexagon_idx)
				);

			case DEPLOY:
				return (
					Order::is_valid_faction_idx(this->faction_idx)
					&& Order::is_valid_hexagon_idx(this->hexagon_idx)
				);

			case NEGOCIATE:
				return true;

			case RALLY:
				return(
					Order::is_valid_faction_idx(this->faction_idx)
					&& Order::is_valid_unit_nb(this->nb_units)
					&& Order::is_valid_hexagon_idx(this->start_hexagon_idx)
					&& Order::is_valid_hexagon_idx(this->end_hexagon_idx)
				);

			default:
				return false;
		}
	}
};

} // Turncoat
#endif // ORDER_HXX