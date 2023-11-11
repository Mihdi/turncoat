#pragma once
#ifndef GAMEVIEW_HXX
#define GAMEVIEW_HXX

namespace Turncoat
{

template<
	uint8_t NB_HEXAGONS,
	uint8_t NB_FACTIONS
>
struct GameView
{
	const std::array<uint8_t, NB_FACTIONS> attack_zone;
	const std::array<uint8_t, NB_FACTIONS> rally_zone;
	const std::array<uint8_t, NB_FACTIONS> hand;
	const std::array<std::array<uint8_t, NB_FACTIONS>, NB_HEXAGONS> hexagons;

	GameView(
		const std::array<uint8_t, NB_FACTIONS> attack_zone,
		const std::array<uint8_t, NB_FACTIONS> rally_zone,
		const std::array<uint8_t, NB_FACTIONS> hand,
		const std::array<std::array<uint8_t, NB_FACTIONS>, NB_HEXAGONS> hexagons
	):
	attack_zone(attack_zone),
	rally_zone(rally_zone),
	hand(hand),
	hexagons(hexagons)
	{}
};

} // Turncoat
#endif // GAMEVIEW_HXX
