#include <pybind11/pybind11.h>

#include "./pybind/gamehandler_pybind.hxx"

/*
	c++ -O3 -Wall -shared -std=c++11 -fPIC $(python3 -m pybind11 --includes) pybind.cpp -o turncoat$(python3-config --extension-suffix)
*/

PYBIND11_MODULE(turncoat, m) {
    m.doc() = "Turncoat game engine package by Arcanite bound from C++ using Pybind11"; // optional module docstring
    gamehandler_pybind(m);
}
