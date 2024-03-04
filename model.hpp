#ifndef HEADER_MODEL
#define HEADER_MODEL

#include <vector> // data storage
#include <array> // explicit data indexing

class Mesh{
	
	// default data
	std::vector<std::array<float, 3>> indexVertices;
	std::vector<std::array<int, 2>> indexEdges;
	std::vector<std::vector<int>> indexFaces;
	
	// serialised data
	std::vector<float> serialVertices;
	std::vector<int> serialEdges;
	
	// processed data
	std::vector<int> triangleFaces;
	std::vector<float> faceCentreVertices;
	std::vector<int> fanFaces;
	
	// usage
public:
	Mesh(std::vector<std::array<float, 3>> const &vs, std::vector<std::array<int, 2>> const &es, std::vector<std::vector<int>> const &fs);
	std::vector<std::array<float, 3>> getIndexVertices();
	std::vector<std::array<int, 2>> getIndexEdges();
	std::vector<std::vector<int>> getIndexFaces();
	std::vector<float> getSerialVertices();
	std::vector<int> getSerialEdges();
	std::vector<int> getTriangularFaces();
	std::vector<float> getFanCentreVertices();
	std::vector<int> getFanFaces();
};

std::ostream &operator<<(std::ostream &os, Mesh &m);

#endif