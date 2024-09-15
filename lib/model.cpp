#include "model.hpp"
#include "../utils/debug.hpp"

namespace{
	template<typename T, std::size_t N>
	std::vector<T> getSerialData(std::vector<std::array<T, N>> data){
		std::vector<T> out;
		out.reserve(data.size() * N);
		for(std::array<T, N> d : data) out.insert(out.end(), d.begin(), d.end());
		return out;
	}
	template<typename T, std::size_t U>
	void operator+=(std::array<T, U> &a, std::array<T, U> &b){
		for(int i = 0; i < U; i++) a[i] += b[i];
	}
	template<typename T, std::size_t U, typename V>
	void operator/=(std::array<T, U> &array, V divisor){
		for(int i = 0; i < U; i++) array[i] /= divisor;
	}
}

// mesh methods

Mesh::Mesh(std::vector<std::array<float, 3>> const &vs, std::vector<std::array<int, 2>> const &es, std::vector<std::vector<int>> const &fs){
	indexVertices = vs;
	indexEdges = es;
	indexFaces = fs;
}

std::vector<std::array<float, 3>> Mesh::getIndexVertices(){
	return indexVertices;
}

std::vector<std::array<int, 2>> Mesh::getIndexEdges(){
	return indexEdges;
}

std::vector<std::vector<int>> Mesh::getIndexFaces(){
	return indexFaces;
}

std::vector<float> Mesh::getSerialVertices(){
	if(!serialVertices.empty()) return serialVertices;
	serialVertices = getSerialData<float, 3>(getIndexVertices());
	return serialVertices;
}

std::vector<int> Mesh::getSerialEdges(){
	if(!serialEdges.empty()) return serialEdges;
	serialEdges = getSerialData<int, 2>(indexEdges);
	return serialEdges;
}

std::vector<int> Mesh::getTriangularFaces(){
	if(!triangleFaces.empty()) return triangleFaces;
	std::vector<std::vector<int>> faces = getIndexFaces();
	int faceTotal = 0;
	for(std::vector<int> face : faces) faceTotal += face.size() - 2;
	triangleFaces.reserve(faceTotal * 3);
	for(std::vector<int> face : faces){
		int f2, f3;
		for(f2 = 1, f3 = 2; f3 < face.size(); f2 = f3++){
			triangleFaces.push_back(face[0]);
			triangleFaces.push_back(face[f2]);
			triangleFaces.push_back(face[f3]);
		}
	}
	return triangleFaces;
	
}

std::vector<float> Mesh::getFanCentreVertices(){
	if(!faceCentreVertices.empty()) return faceCentreVertices;
	std::vector<std::array<float, 3>> vertices = getIndexVertices();
	std::vector<std::vector<int>> faces = getIndexFaces();
	faceCentreVertices.reserve(faces.size() * 3);
	for(std::vector<int> face : faces){
		std::array<float, 3> midPoint = {0,0,0};
		for(int f : face) midPoint += vertices[f];
		midPoint /= face.size();
		faceCentreVertices.push_back(midPoint[0]);
		faceCentreVertices.push_back(midPoint[1]);
		faceCentreVertices.push_back(midPoint[2]);
	}
	return faceCentreVertices;
}

std::vector<int> Mesh::getFanFaces(){
	if(!fanFaces.empty()) return fanFaces;
	std::size_t verticesTotal = getIndexVertices().size();
	std::vector<std::vector<int>> faces = getIndexFaces();
	int faceTotal = 0;
	for(std::vector<int> face : faces) faceTotal += face.size();
	fanFaces.reserve(faceTotal * 3);
	for(int f = 0; f < faces.size(); f++){
		int c = verticesTotal + f;
		int f2, f3;
		for(f2 = faces[f].size() - 1, f3 = 0; f3 < faces[f].size(); f2 = f3++){
			fanFaces.push_back(c);
			fanFaces.push_back(faces[f][f2]);
			fanFaces.push_back(faces[f][f3]);
		}
	}
	return fanFaces;
}

// mesh overloaded functions

std::ostream &operator<<(std::ostream &os, Mesh &m){
	os << "{\n";
	for(std::array<float, 3> vertex : m.getIndexVertices()) os << "vertex {" << vertex[0] << " " << vertex[1] << " " << vertex[2] << "}\n";
	for(std::array<int, 2> edge : m.getIndexEdges()) os << "edge " << edge[0] << "-" << edge[1] << "\n";
	for(std::vector<int> face : m.getIndexFaces()){
		if(face.empty()) continue;
		os << "face [" << face[0];
		for(int f = 1; f < face.size(); f++) os << " " << face[f];
		os << "]\n";
	}
	os << "}";
	return os;
}