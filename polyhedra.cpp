#include "polyhedra.hpp"
#include "utils/debug.hpp"

// auxiliary functions

namespace{
	std::array<int, 2> sortEdge(std::array<int, 2> e){
		return e[0] > e[1] ? std::array<int, 2>{e[1], e[0]} : e;
	}
	std::array<int, 2> sortEdge(std::array<int, 2> e, bool &isSwitched){
		std::array<int, 2> out = e;
		isSwitched = false;
		if(out[0] > out[1]){
			std::swap(out[0], out[1]);
			isSwitched = true;
		}
		return out;
	}
	bool swapSubstring(std::string &str, const char *substr, const char *altstr){
		int index;
		if((index = str.find(substr, 0)) != std::string::npos){
			str.replace(index, strlen(substr), altstr);
			return true;
		}
		return false;
	}
	std::array<float, 3> getVertexLerp(std::array<float, 3> v0, std::array<float, 3> v1, float w){
		std::array<float, 3> out;
		for(int d = 0; d < 3; d++) out[d] = v0[d] * w + v1[d] * (1.f - w);
		debug("lerp", out);
		return out;
	}
	std::array<float, 3> getVertexMidpoint(std::array<float, 3> v0, std::array<float, 3> v1){
		std::array<float, 3> out;
		for(int d = 0; d < 3; d++) out[d] = (v0[d] + v1[d]) / 2;
		debug("midpoint", out);
		return out;
	}
	void addFaceCentreVertices(std::vector<std::array<float, 3>> &newVertices, std::vector<std::vector<int>> faces, std::vector<std::array<float, 3>> vertices){
		for(std::vector<int> face : faces){
			std::array<float, 3> midPoint{0, 0, 0};
			for(int f : face){
				for(int d = 0; d < 3; d++) midPoint[d] += vertices[f][d];
			}
			for(int d = 0; d < 3; d++) midPoint[d] /= face.size();
			newVertices.push_back(midPoint);
		}
		debug("face centre", vertices);
	}
	template <typename T>
	void orderLoopLinks(std::vector<std::array<int, 2>> &links, std::vector<T> &order, bool flipDirection){
		bool hasOrder = !order.empty();
		int prev = 0;
		int next = 1;
		if(flipDirection) std::swap(prev, next);
		for(int l = 0; l < links.size(); l++){
			if(links[l][next] == links[l + 1][prev]) continue;
			for(int r = l + 2; r < links.size(); r++){
				if(links[l][next] == links[r][prev]){
					std::swap(links[l + 1], links[r]);
					if(hasOrder) std::swap(order[l + 1], order[r]);
					break;
				}
			}
		}
		debug("sorted links", links);
	}
	void orderLoopLinks(std::vector<std::array<int, 2>> &links, bool flipDirection){
		std::vector<int> temp{};
		orderLoopLinks(links, temp, flipDirection);
	}
	void insertConnectedFaces(std::vector<std::vector<int>> &connectedFaces, std::vector<std::vector<int>> faces){
		for(int f = 0; f < faces.size(); f++){
			for(int v : faces[f]) connectedFaces[v].push_back(f); // insert faces by connected vertex
		}
		debug("connected faces", connectedFaces);
	}
	void insertLoopLinks(std::vector<std::vector<std::array<int, 2>>> &links, std::vector<std::vector<int>> source){
		for(int f = 0; f < source.size(); f++){
			int prev, v, next;
			for(prev = source[f].size() - 2, v = source[f].size() - 1, next = 0; next < source[f].size(); prev = v, v = next++)
				links[source[f][v]].push_back({source[f][prev], source[f][next]}); // insert vertices either side of each connected vertex
		}
		debug("loop links", links);
	}
	void insertEdgeLinks(std::vector<std::vector<std::array<int, 2>>> &connectedEdges, std::vector<std::vector<std::array<int, 2>>> links){
		for(int v = 0; v < links.size(); v++){
			connectedEdges[v].reserve(links[v].size());
			for(std::array<int, 2> link : links[v]) connectedEdges[v].push_back(sortEdge({v, link[0]}));
		}
		debug("connected edges", connectedEdges);
	}
	void addFaceEdges(std::vector<std::array<int, 2>> &edges, std::vector<int> face){
		edges.reserve(face.size());
		int f1, f2;
		for(f1 = face.size() - 1, f2 = 0; f2 < face.size(); f1 = f2++) edges.push_back(sortEdge({face[f1], face[f2]}));
	}
	void addFaceEdges(std::vector<std::array<int, 2>> &edges, std::vector<std::vector<int>> faces){
		for(std::vector<int> face : faces){
			int e0, e1;
			for(e0 = face.size() - 1, e1 = 0; e1 < face.size(); e0 = e1++){
				std::array<int, 2> edge = sortEdge({face[e0], face[e1]});
				if(std::find(edges.begin(), edges.end(), edge) == edges.end()) edges.push_back(edge);
			}
		}
		debug("face edges", edges);
	}
	void addEdgeIndices(std::vector<int> &indices, std::vector<std::array<int, 2>> edges, std::vector<std::array<int, 2>> order){
		indices.reserve(edges.size());
		for(std::array<int, 2> edge : edges) indices.push_back(std::find(order.begin(), order.end(), edge) - order.begin());
		debug("edge indices", indices);
	}
}

// polyhedron methods

Polyhedron::Polyhedron(std::size_t vn, std::size_t en, std::size_t fn){
	reserve(vn, en, fn);
}

Polyhedron::Polyhedron(std::vector<std::array<float, 3>> const &v, std::vector<std::array<int, 2>> const &e, std::vector<std::vector<int>> const &f) : 
	vertices(v), edges(e), faces(f) {}

Polyhedron::Polyhedron(std::vector<std::array<float, 3>> const &v, std::vector<std::array<int, 2>> const &e, std::vector<std::vector<int>> const &f, 
	std::size_t vn, std::size_t en, std::size_t fn) : 
	Polyhedron(v, e, f) {
	reserve(vn, en, fn);
}

Polyhedron& Polyhedron::operator=(Polyhedron &&other){
	vertices = std::move(other.vertices);
	edges = std::move(other.edges);
	faces = std::move(other.faces);
	return *this;
}

void Polyhedron::reserve(std::size_t vn, std::size_t en, std::size_t fn){
	vertices.reserve(vn);
	edges.reserve(en);
	faces.reserve(fn);
}

std::ostream &operator<<(std::ostream &os, Polyhedron const &m){
	os << "{\n";
	for(std::array<float, 3> vertex : m.vertices) os << "vertex {" << vertex[0] << " " << vertex[1] << " " << vertex[2] << "}\n";
	for(std::array<int, 2> edge : m.edges) os << "edge " << edge[0] << "-" << edge[1] << "\n";
	for(std::vector<int> face : m.faces){
		if(face.empty()) continue;
		os << "face [" << face[0];
		for(int f = 1; f < face.size(); f++) os << " " << face[f];
		os << "]\n";
	}
	os << "}";
	return os;
}

// factory operator methods

void PolyhedronFactory::tetrahedron(Polyhedron &p){
	float angle = 120.f * DEGREES_TO_RADIANS;
	float xSpin = sin(angle);
	float ySpin = cos(angle);
	float xxSpin = xSpin * xSpin;
	float xySpin = xSpin * ySpin;
	p.vertices = {
		{0, 1, 0}, 
		{0, ySpin, xSpin}, 
		{xxSpin, ySpin, xySpin}, 
		{-xxSpin, ySpin, xySpin}};
	p.edges = {
		{0, 1}, {0, 2}, {0, 3}, 
		{1, 2}, {2, 3}, {1, 3}};
	p.faces = {
		{0, 1, 2}, {0, 2, 3}, {0, 3, 1}, 
		{3, 2, 1}};
}

void PolyhedronFactory::duality(Polyhedron &p){
	Polyhedron pNew({}, {}, std::vector<std::vector<int>>(p.vertices.size()), p.faces.size(), p.edges.size(), p.vertices.size());
	
	// vertices from face centres
	addFaceCentreVertices(pNew.vertices, p.faces, p.vertices);
	
	// face loops around each vertex
	std::vector<std::vector<std::array<int, 2>>> faceLinks(p.vertices.size());
	insertConnectedFaces(pNew.faces, p.faces);
	insertLoopLinks(faceLinks, p.faces);
	for(int v = 0; v < p.vertices.size(); v++) orderLoopLinks(faceLinks[v], pNew.faces[v], true);
	
	// edges between adjacent faces
	addFaceEdges(pNew.edges, pNew.faces);
	
	p = std::move(pNew);
}

void PolyhedronFactory::rectify(Polyhedron &p){
	Polyhedron pNew({}, {}, std::vector<std::vector<int>>(p.vertices.size()), p.edges.size(), 2 * p.edges.size(), p.vertices.size() + p.faces.size());
	
	// loops of edges connected by each vertex
	std::vector<std::vector<std::array<int, 2>>> faceLinks(p.vertices.size());
	insertLoopLinks(faceLinks, p.faces);
	for(int v = 0; v < p.vertices.size(); v++) orderLoopLinks(faceLinks[v], true);
	std::vector<std::vector<std::array<int, 2>>> connectedEdges(p.vertices.size());
	insertEdgeLinks(connectedEdges, faceLinks);
	
	// vertices from edge midpoints
	for(std::array<int, 2> edge : p.edges) pNew.vertices.push_back(getVertexMidpoint(p.vertices[edge[0]], p.vertices[edge[1]]));
	
	// faces from connected edge midpoints
	for(int v = 0; v < p.vertices.size(); v++) addEdgeIndices(pNew.faces[v], connectedEdges[v], p.edges);
	
	// faces from face edge midpoints
	for(std::vector<int> face : p.faces){
		std::vector<std::array<int, 2>> faceEdges;
		addFaceEdges(faceEdges, face);
		std::vector<int> facePoints;
		addEdgeIndices(facePoints, faceEdges, p.edges);
		pNew.faces.push_back(std::move(facePoints));
	}
	
	// edges between connected face edges
	for(int v = 0; v < p.vertices.size(); v++) addFaceEdges(pNew.edges, pNew.faces[v]);
	
	p = std::move(pNew);
}

void PolyhedronFactory::akisate(Polyhedron &p){
	Polyhedron pNew(p.vertices, p.edges, {}, p.vertices.size() + p.faces.size(), p.edges.size() * 3, p.edges.size() * 2);
	
	// vertices inside faces
	addFaceCentreVertices(pNew.vertices, p.faces, p.vertices);
	
	// edges from face vertices to face centres
	for(int f = 0; f < p.faces.size(); f++){
		int c = p.vertices.size() + f;
		for(int e0 : p.faces[f]) pNew.edges.push_back(sortEdge({c, e0}));
	}
	
	// faces into new centre vertices
	for(int f = 0; f < p.faces.size(); f++){
		int c = p.vertices.size() + f;
		int e0, e1;
		for(e0 = p.faces[f].size() - 1, e1 = 0; e1 < p.faces[f].size(); e0 = e1++) pNew.faces.push_back(std::vector<int>{p.faces[f][e0], p.faces[f][e1], c});
	}
	
	p = std::move(pNew);
}

void PolyhedronFactory::gyrate(Polyhedron &p){
	Polyhedron pNew(p.vertices.size() + p.edges.size() * 2 + p.faces.size(), p.edges.size() * 5, p.edges.size() * 2);
	pNew.vertices = p.vertices;
	
	// vertices
	for(std::array<int, 2> edge : p.edges){ // across edges
		pNew.vertices.push_back(getVertexLerp(p.vertices[edge[0]], p.vertices[edge[1]], .33333f));
		pNew.vertices.push_back(getVertexLerp(p.vertices[edge[0]], p.vertices[edge[1]], .66667f));
	}
	addFaceCentreVertices(pNew.vertices, p.faces, p.vertices); // inside faces
	
	// edges
	int ve = p.vertices.size();
	int vf = ve + p.edges.size() * 2;
	for(int e = 0; e < p.edges.size(); e++){ // around faces
		int e0 = p.edges[e][0];
		int e1 = ve + e * 2 + 0;
		int e2 = ve + e * 2 + 1;
		int e3 = p.edges[e][1];
		pNew.edges.push_back({e0, e1});
		pNew.edges.push_back({e1, e2});
		pNew.edges.push_back({e2, e3});
	}
	for(int f = 0; f < p.faces.size(); f++){ // inside faces
		int f1, f2;
		for(f1 = p.faces[f].size() - 1, f2 = 0; f2 < p.faces[f].size(); f1 = f2++){
			bool flip;
			std::array<int, 2> edge = sortEdge({p.faces[f][f1], p.faces[f][f2]}, flip);
			int e = std::find(p.edges.begin(), p.edges.end(), edge) - p.edges.begin();
			pNew.edges.push_back({ve + e * 2 + flip, vf + f});
		}
	}
	
	// faces around edges
	for(int f = 0; f < p.faces.size(); f++){
		int v0, v1, v2;
		for(v0 = p.faces[f].size() - 2, v1 = p.faces[f].size() - 1, v2 = 0; v2 < p.faces[f].size(); v0 = v1, v1 = v2++){
			bool flip1;
			bool flip2;
			std::array<int, 2> faceEdge1 = sortEdge({p.faces[f][v0], p.faces[f][v1]}, flip1);
			std::array<int, 2> faceEdge2 = sortEdge({p.faces[f][v1], p.faces[f][v2]}, flip2);
			int ei1 = std::find(p.edges.begin(), p.edges.end(), faceEdge1) - p.edges.begin();
			int ei2 = std::find(p.edges.begin(), p.edges.end(), faceEdge2) - p.edges.begin();
			int e11 = ve + ei1 * 2 + 1 - flip1;
			int e12 = ve + ei1 * 2 + flip1;
			int e2 = ve + ei2 * 2 + 1 - flip2;
			pNew.faces.push_back(std::vector<int>{p.faces[f][v1], e2, vf + f, e11, e12});
		}
	}
	
	p = std::move(pNew);
}

// factory methods

std::vector<Polyhedron> PolyhedronFactory::make(std::string ops){
	std::vector<Polyhedron> polys;
	
	// ensure only valid terms are used
	const std::string opchars("djaknztoegsmbTOCID");
	if(!std::all_of(ops.cbegin(), ops.cend(), [opchars](const char op){ return opchars.find(op) != std::string::npos; })){
		std::cout << "Error: operator stream contains non-operator characters\n";
		return polys;
	}
	
	// simplify & substitute-unimplemented operators
	std::list<std::array<const char*, 2>> swaps{
		{"dd", ""}, // cancellation
		{"j", "da"}, // rectification
		{"n", "kd"}, {"z", "dk"}, {"t", "dkd"}, // akisation
		{"o", "daa"}, {"e", "aa"}, // expansion
		{"s", "dgd"}, // snub
		{"m", "kda"}, {"b", "dkda"}, // meta
		{"O", "aT"}, {"C", "jT"}, {"I", "sT"}, {"D", "gT"}}; // platonic solids
	while(std::any_of(swaps.cbegin(), swaps.cend(), [&ops](std::array<const char*, 2> pair){ 
		return swapSubstring(ops, pair[0], pair[1]); }));
	debug("simplified stream", ops);
	
	// validate final stream
	const std::string acceptedSeeds("T");
	const std::string acceptedOps("dakg");
	std::string acceptedStream = acceptedSeeds + acceptedOps;
	if(std::any_of(ops.begin(), ops.end(), [acceptedStream](char op){ return acceptedStream.find(op) == std::string::npos; })){
		debug("Error: operator stream invalid after simplification");
		return polys;
	}
	
	// move to first seed
	std::string::reverse_iterator it = ops.rbegin();
	while(std::find(acceptedSeeds.begin(), acceptedSeeds.end(), *it) == acceptedSeeds.end() && it != ops.rend()) it++;
	
	// operations
	while(it != ops.rend()){
		switch(*it){
			case 'T':
				polys.push_back(Polyhedron()); // mesh assumptions: edge pairs are sorted lowest-highest index; faces are in winding order
				tetrahedron(polys.back());
				break;
			case 'd':
				duality(polys.back());
				break;
			case 'a':
				rectify(polys.back());
				break;
			case 'k':
				akisate(polys.back());
				break;
			case 'g':
				gyrate(polys.back());
				break;
		}
		it++;
	}
	return polys;
}