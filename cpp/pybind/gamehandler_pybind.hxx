#pragma once
#ifndef GAMEHANDLER_PYBIND_HXX
#define GAMEHANDLER_PYBIND_HXX

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include <functional>

#include "../constants/default_game.hxx"

#include "../src/gamehandler.hxx"
#include "../src/order.hxx"

namespace py = pybind11;

using DefaultGameHandlerType = Turncoat::GameHandler<
  	default_nb_hexagons,
  	default_nb_factions,
  	default_nb_per_faction,
  	default_nb_hands,
  	default_nb_per_hand,
  	default_nb_starting_units
>;

using DefaultOrderType = Turncoat::Order<default_nb_hexagons, default_nb_factions, default_nb_per_faction>;
using DefaultGameViewType = Turncoat::GameView<default_nb_hexagons, default_nb_factions>;

void gamehandler_pybind(py::module &m) {
	py::class_<DefaultGameHandlerType>(m, "DefaultGameHandler")
	.def(py::init<
		const std::array<std::array<bool, default_nb_hexagons>, default_nb_hexagons> *,
		const std::array<uint8_t, default_nb_factions> *,
		const std::unordered_set<uint8_t> *,
		std::mt19937 *,
		const std::array<
			std::function<
				DefaultOrderType(const DefaultGameViewType)
			>,
			default_nb_hands
		>,
		const std::array<
			std::function<
				uint8_t(const std::array<uint8_t, default_nb_factions>&)
			>,
			default_nb_hands
		>
	>())
	.def("run_turn", &DefaultGameHandlerType::run_turn)
	.def("run_all_turns", &DefaultGameHandlerType::run_all_turns)
	.def("get_winner", &DefaultGameHandlerType::get_winner);
}

#endif // GAMEHANDLER_PYBIND_HXX
