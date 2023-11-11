#pragma once
#ifndef TEST_HXX
#define TEST_HXX

#include <assert.h>

#ifndef private
#define private public
#endif // private

#ifndef protected
#define protected public
#endif // protected

#include "../constants/default_game.hxx"

void fill_default_adjacency_graph(
	std::array<std::array<bool, default_nb_hexagons>, default_nb_hexagons>& adjacency_graph
)
{
	for (auto hexagon_ite = adjacency_graph.begin(); hexagon_ite != adjacency_graph.end(); ++hexagon_ite)
	{
		hexagon_ite->fill(false);
	}

	adjacency_graph[0][1] = true;
	adjacency_graph[0][3] = true;
	adjacency_graph[0][4] = true;

	adjacency_graph[1][0] = true;
	adjacency_graph[1][2] = true;
	adjacency_graph[1][4] = true;

	adjacency_graph[2][1] = true;
	adjacency_graph[2][4] = true;
	adjacency_graph[2][5] = true;

	adjacency_graph[3][0] = true;
	adjacency_graph[3][4] = true;
	adjacency_graph[3][6] = true;

	adjacency_graph[4][0] = true;
	adjacency_graph[4][1] = true;
	adjacency_graph[4][2] = true;
	adjacency_graph[4][3] = true;
	adjacency_graph[4][5] = true;

	adjacency_graph[5][2] = true;
	adjacency_graph[5][4] = true;
	adjacency_graph[5][8] = true;

	adjacency_graph[6][3] = true;
	adjacency_graph[6][9] = true;

	adjacency_graph[8][5] = true;
	adjacency_graph[8][9] = true;
	adjacency_graph[8][11] = true;

	adjacency_graph[9][6] = true;
	adjacency_graph[9][8] = true;
	adjacency_graph[9][10] = true;

	adjacency_graph[10][9] = true;
	adjacency_graph[10][11] = true;

	adjacency_graph[11][8] = true;
	adjacency_graph[11][10] = true;
}

#endif // TEST_HXX