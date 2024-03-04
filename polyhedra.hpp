#ifndef HEADER_POLYHEDRA
#define HEADER_POLYHEDRA

#include <algorithm> // validate stream characters
#include <cstring> // seek-and-replace complex stream operators
#include <list> // list stream elements
#include <vector> // shape data
#include <cmath> // seed shape generation
#include <array> // vertex & edge data elements

#ifndef PI
#define PI 3.141519 // vertex rotation
#endif
#ifndef DEGREES_TO_RADIANS
#define DEGREES_TO_RADIANS (PI / 180.f) // specify angles
#endif

struct Polyhedron{
	std::vector<std::array<float, 3>> vertices;
	std::vector<std::array<int, 2>> edges;
	std::vector<std::vector<int>> faces;
	Polyhedron() = default;
	Polyhedron(Polyhedron const &) = default;
	Polyhedron(std::size_t vn, std::size_t en, std::size_t fn);
	Polyhedron(std::vector<std::array<float, 3>> const &v, std::vector<std::array<int, 2>> const &e, std::vector<std::vector<int>> const &f);
	Polyhedron(std::vector<std::array<float, 3>> const &v, std::vector<std::array<int, 2>> const &e, std::vector<std::vector<int>> const &f, 
		std::size_t vn, std::size_t en, std::size_t fn);
	Polyhedron& operator=(Polyhedron &&other);
	void reserve(std::size_t vn, std::size_t en, std::size_t fn);
};

std::ostream &operator<<(std::ostream &os, Polyhedron const &m);

class PolyhedronFactory{
	static void tetrahedron(Polyhedron &p);
	static void duality(Polyhedron &p);
	static void rectify(Polyhedron &p);
	static void akisate(Polyhedron &p);
	static void gyrate(Polyhedron &p);
public:
	static std::vector<Polyhedron> make(std::string ops);
};

#endif