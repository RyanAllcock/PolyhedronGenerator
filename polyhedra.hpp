#ifndef HEADER_POLYHEDRA
#define HEADER_POLYHEDRA

#include <algorithm> // validate stream characters
#include <cstring> // seek-and-replace complex stream operators
#include <list> // list stream elements
#include <vector> // shape data
#include <cmath> // seed shape generation
#include <iostream> // polyhedron data streaming

#include "../utils/maths.hpp"

#ifndef PI
#define PI 3.141519 // vertex rotation
#endif
#ifndef DEGREES_TO_RADIANS
#define DEGREES_TO_RADIANS (PI / 180.f) // specify angles
#endif

struct Polyhedron{
	std::vector<math::vertex> vertices;
	std::vector<math::edge> edges;
	std::vector<math::face> faces;
	Polyhedron() = default;
	Polyhedron(Polyhedron const &) = default;
	Polyhedron(std::size_t vn, std::size_t en, std::size_t fn);
	Polyhedron(std::vector<math::vertex> const &v, std::vector<math::edge> const &e, std::vector<math::face> const &f);
	Polyhedron(std::vector<math::vertex> const &v, std::vector<math::edge> const &e, std::vector<math::face> const &f, 
		std::size_t vn, std::size_t en, std::size_t fn);
	Polyhedron& operator=(Polyhedron &&other);
	void reserve(std::size_t vn, std::size_t en, std::size_t fn);
};

std::ostream &operator<<(std::ostream &os, Polyhedron const &m);

class PolyhedronFactory{
	
	// operators
	static void tetrahedron(Polyhedron &p);
	static void duality(Polyhedron &p);
	static void rectify(Polyhedron &p);
	static void akisate(Polyhedron &p);
	static void gyrate(Polyhedron &p);
	
	// canonical form
	static void tangentify(std::vector<math::vertex> &vertices, std::vector<math::edge> const &edges);
	static void recenter(std::vector<math::vertex> &vertices, std::vector<math::edge> const &edges);
	static void planarize(std::vector<math::vertex> &vertices, std::vector<math::face> const &faces);
	static void canonicalize(std::vector<math::vertex> &vertices, std::vector<math::edge> const &edges, std::vector<math::face> const &faces, unsigned int iterations);
	
	// usage
public:
	static std::vector<Polyhedron> make(std::string ops);
	static void mutate(Polyhedron &p, char op);
};

#endif