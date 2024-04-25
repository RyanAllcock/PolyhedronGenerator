#include "polyhedra.hpp"
#include "utils/debug.hpp"

// auxiliary functions

namespace{
	
	// operator stream
	
	bool swapSubstring(std::string &str, const char *substr, const char *altstr){
		int index;
		if((index = str.find(substr, 0)) != std::string::npos){
			str.replace(index, strlen(substr), altstr);
			return true;
		}
		return false;
	}
	
	// operator vertices
	
	void addFaceCentreVertices(std::vector<math::vertex> &newVertices, std::vector<math::face> const &faces, std::vector<math::vertex> const &vertices){
		for(math::face const &face : faces){
			math::vertex midPoint{0, 0, 0};
			for(int f : face) midPoint += vertices[f];
			midPoint /= (float)face.size();
			newVertices.push_back(midPoint);
		}
		debug("face centre", vertices);
	}
	
	// operator edges
	
	void addFaceEdges(std::vector<math::edge> &edges, math::face const &face){
		edges.reserve(face.size());
		int f1, f2;
		for(f1 = face.size() - 1, f2 = 0; f2 < face.size(); f1 = f2++) edges.push_back(math::sort({face[f1], face[f2]}));
	}
	
	void addUniqueFaceEdges(std::vector<math::edge> &edges, std::vector<math::face> const &faces){
		for(math::face const &face : faces){
			int e0, e1;
			for(e0 = face.size() - 1, e1 = 0; e1 < face.size(); e0 = e1++){
				math::edge edge = math::sort({face[e0], face[e1]});
				if(std::find(edges.begin(), edges.end(), edge) == edges.end()) edges.push_back(edge);
			}
		}
		debug("face edges", edges);
	}
	
	// operator faces
	
	void insertConnectedFaces(std::vector<math::face> &connectedFaces, std::vector<math::face> faces){
		for(int f = 0; f < faces.size(); f++){
			for(int v : faces[f]) connectedFaces[v].push_back(f); // insert faces by connected vertex
		}
		debug("connected faces", connectedFaces);
	}
	
	void addEdgeIndices(math::face &indices, std::vector<math::edge> const &edges, std::vector<math::edge> const &order){
		indices.reserve(edges.size());
		for(math::edge const &edge : edges) indices.push_back(std::find(order.begin(), order.end(), edge) - order.begin());
		debug("edge indices", indices);
	}
	
	// operator links
	
	template <typename T>
	void orderLoopLinks(std::vector<math::edge> &links, std::vector<T> &order, bool flipDirection){
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
	
	void orderLoopLinks(std::vector<math::edge> &links, bool flipDirection){
		std::vector<int> temp{};
		orderLoopLinks(links, temp, flipDirection);
	}
	
	void insertLoopLinks(std::vector<std::vector<math::edge>> &links, std::vector<math::face> const &source){
		for(int f = 0; f < source.size(); f++){
			int prev, v, next;
			for(prev = source[f].size() - 2, v = source[f].size() - 1, next = 0; next < source[f].size(); prev = v, v = next++)
				links[source[f][v]].push_back({source[f][prev], source[f][next]}); // insert vertices either side of each connected vertex
		}
		debug("loop links", links);
	}
	
	void insertEdgeLinks(std::vector<std::vector<math::edge>> &connectedEdges, std::vector<std::vector<math::edge>> const &links){
		for(int v = 0; v < links.size(); v++){
			connectedEdges[v].reserve(links[v].size());
			for(math::edge link : links[v]) connectedEdges[v].push_back(math::sort({v, link[0]}));
		}
		debug("connected edges", connectedEdges);
	}
	
	// canonical form
	
	#define TOLERANCE 0.00000001
	#define TANGENT 0.5f
	#define PLANAR 0.2f
	#define MAX_ITERATIONS 80
	
	math::vertex getClosestMidpointToOrigin(math::vertex const &p1, math::vertex const &p2){
		if(p1 == p2) return p1;
		math::vertex L = p2 - p1;
		float alpha = -math::dot(L, p1) / math::dot(L, L);
		math::vertex out = p1 + alpha * L;
		return p1 + alpha * L;
	}
	
	math::vertex getApproximateNormal(std::vector<math::vertex> const &vertices, math::face const &face){
		math::vertex normal = {0,0,0};
		int f1,f2,f3;
		for(f1 = face.size() - 2, f2 = face.size() - 1, f3 = 0; f3 < face.size(); f1 = f2, f2 = f3++)
			normal += math::cross(vertices[face[f1]] - vertices[face[f2]], vertices[face[f2]] - vertices[face[f3]]);
		return math::unit(normal);
	}
}

// polyhedron methods

Polyhedron::Polyhedron(std::size_t vn, std::size_t en, std::size_t fn){
	reserve(vn, en, fn);
}

Polyhedron::Polyhedron(std::vector<math::vertex> const &v, std::vector<math::edge> const &e, std::vector<math::face> const &f) : 
	vertices(v), edges(e), faces(f) {}

Polyhedron::Polyhedron(std::vector<math::vertex> const &v, std::vector<math::edge> const &e, std::vector<math::face> const &f, 
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
	for(math::vertex vertex : m.vertices) os << "vertex {" << vertex[0] << " " << vertex[1] << " " << vertex[2] << "}\n";
	for(math::edge edge : m.edges) os << "edge " << edge[0] << "-" << edge[1] << "\n";
	for(math::face face : m.faces){
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
	Polyhedron pNew({}, {}, std::vector<math::face>(p.vertices.size()), p.faces.size(), p.edges.size(), p.vertices.size());
	
	// vertices from face centres
	addFaceCentreVertices(pNew.vertices, p.faces, p.vertices);
	
	// face loops around each vertex
	std::vector<std::vector<math::edge>> faceLinks(p.vertices.size());
	insertConnectedFaces(pNew.faces, p.faces);
	insertLoopLinks(faceLinks, p.faces);
	for(int v = 0; v < p.vertices.size(); v++) orderLoopLinks(faceLinks[v], pNew.faces[v], true);
	
	// edges between adjacent faces
	addUniqueFaceEdges(pNew.edges, pNew.faces);
	
	p = std::move(pNew);
}

void PolyhedronFactory::rectify(Polyhedron &p){
	Polyhedron pNew({}, {}, std::vector<math::face>(p.vertices.size()), p.edges.size(), 2 * p.edges.size(), p.vertices.size() + p.faces.size());
	
	// loops of edges connected by each vertex
	std::vector<std::vector<math::edge>> faceLinks(p.vertices.size());
	insertLoopLinks(faceLinks, p.faces);
	for(int v = 0; v < p.vertices.size(); v++) orderLoopLinks(faceLinks[v], true);
	std::vector<std::vector<math::edge>> connectedEdges(p.vertices.size());
	insertEdgeLinks(connectedEdges, faceLinks);
	
	// vertices from edge midpoints
	for(math::edge edge : p.edges) pNew.vertices.push_back(math::midpoint(p.vertices[edge[0]], p.vertices[edge[1]]));
	
	// faces from connected edge midpoints
	for(int v = 0; v < p.vertices.size(); v++) addEdgeIndices(pNew.faces[v], connectedEdges[v], p.edges);
	
	// faces from face edge midpoints
	for(math::face face : p.faces){
		std::vector<math::edge> faceEdges;
		addFaceEdges(faceEdges, face);
		math::face facePoints;
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
		for(int e0 : p.faces[f]) pNew.edges.push_back(math::sort({c, e0}));
	}
	
	// faces into new centre vertices
	for(int f = 0; f < p.faces.size(); f++){
		int c = p.vertices.size() + f;
		int e0, e1;
		for(e0 = p.faces[f].size() - 1, e1 = 0; e1 < p.faces[f].size(); e0 = e1++) pNew.faces.push_back(math::face{p.faces[f][e0], p.faces[f][e1], c});
	}
	
	p = std::move(pNew);
}

void PolyhedronFactory::gyrate(Polyhedron &p){
	Polyhedron pNew(p.vertices.size() + p.edges.size() * 2 + p.faces.size(), p.edges.size() * 5, p.edges.size() * 2);
	pNew.vertices = p.vertices;
	
	// vertices
	for(math::edge edge : p.edges){ // across edges
		pNew.vertices.push_back(math::lerp(p.vertices[edge[0]], p.vertices[edge[1]], .33333f));
		pNew.vertices.push_back(math::lerp(p.vertices[edge[0]], p.vertices[edge[1]], .66667f));
	}
	addFaceCentreVertices(pNew.vertices, p.faces, p.vertices); // inside faces
	
	// edges
	int ve = p.vertices.size();
	int vf = ve + p.edges.size() * 2;
	for(int e = 0; e < p.edges.size(); e++){ // around faces
		int e0 = p.edges[e][0];
		int e1 = ve + e * 2 + 1; // opposite; backwards winding order
		int e2 = ve + e * 2 + 0;
		int e3 = p.edges[e][1];
		pNew.edges.push_back(math::sort({e0, e1}));
		pNew.edges.push_back(math::sort({e1, e2}));
		pNew.edges.push_back(math::sort({e2, e3}));
	}
	for(int f = 0; f < p.faces.size(); f++){ // inside faces
		int f1, f2;
		for(f1 = p.faces[f].size() - 1, f2 = 0; f2 < p.faces[f].size(); f1 = f2++){
			bool flip;
			math::edge edge = math::sort({p.faces[f][f1], p.faces[f][f2]}, flip);
			int e = std::find(p.edges.begin(), p.edges.end(), edge) - p.edges.begin();
			pNew.edges.push_back(math::sort({ve + e * 2 + 1 - flip, vf + f}));
		}
	}
	
	// faces around edges
	for(int f = 0; f < p.faces.size(); f++){
		int v0, v1, v2;
		for(v0 = p.faces[f].size() - 2, v1 = p.faces[f].size() - 1, v2 = 0; v2 < p.faces[f].size(); v0 = v1, v1 = v2++){
			bool flip1;
			bool flip2;
			math::edge faceEdge1 = math::sort({p.faces[f][v0], p.faces[f][v1]}, flip1);
			math::edge faceEdge2 = math::sort({p.faces[f][v1], p.faces[f][v2]}, flip2);
			int ei1 = std::find(p.edges.begin(), p.edges.end(), faceEdge1) - p.edges.begin();
			int ei2 = std::find(p.edges.begin(), p.edges.end(), faceEdge2) - p.edges.begin();
			int e11 = ve + ei1 * 2 + 1 - flip1;
			int e12 = ve + ei1 * 2 + flip1;
			int e2 = ve + ei2 * 2 + 1 - flip2;
			pNew.faces.push_back(math::face{p.faces[f][v1], e2, vf + f, e11, e12});
		}
	}
	
	p = std::move(pNew);
}

// canonical form methods

void PolyhedronFactory::tangentify(std::vector<math::vertex> &vertices, std::vector<math::edge> const &edges){
	std::vector<math::vertex> const initial(vertices);
	for(math::edge const &edge : edges){
		math::vertex tangent = getClosestMidpointToOrigin(vertices[edge[0]], vertices[edge[1]]);
		math::vertex distance = (float)(TANGENT * (1.f - math::length(tangent))) * tangent;
		vertices[edge[0]] += distance;
		vertices[edge[1]] += distance;
	}
}

void PolyhedronFactory::recenter(std::vector<math::vertex> &vertices, std::vector<math::edge> const &edges){
	math::vertex centroid = {0,0,0};
	for(math::edge const &edge : edges) centroid += getClosestMidpointToOrigin(vertices[edge[0]], vertices[edge[1]]);
	centroid /= (float)edges.size();
	for(math::vertex &vertex : vertices) vertex -= centroid;
}

void PolyhedronFactory::planarize(std::vector<math::vertex> &vertices, std::vector<math::face> const &faces){
	std::vector<math::vertex> const initial(vertices);
	for(math::face const &face : faces){
		math::vertex normal = getApproximateNormal(initial, face);
		math::vertex centroid = {0,0,0};
		for(const int f : face) centroid += initial[f];
		centroid /= (float)face.size();
		if(math::dot(centroid, normal) < 0) normal = -normal;
		for(const int f : face) vertices[f] += PLANAR * math::dot(normal, (centroid - initial[f])) * normal;
	}
}

void PolyhedronFactory::canonicalize(std::vector<math::vertex> &vertices, std::vector<math::edge> const &edges, std::vector<math::face> const &faces, unsigned int iterations){
	int i;
	double maxChange;
	double latestMaxChange = 1000;
	for(i = 0; i < iterations; i++){
		std::vector<math::vertex> previous(vertices);
		tangentify(vertices, edges);
		recenter(vertices, edges);
		planarize(vertices, faces);
		maxChange = 0;
		for(int i = 0; i < vertices.size(); i++) maxChange = math::max(maxChange, math::length(previous[i] - vertices[i]));
		if(latestMaxChange > maxChange){
			latestMaxChange = maxChange;
			debug("canon new lowest change", latestMaxChange);
		}
		if(maxChange < TOLERANCE) break;
	}
	debug("canon form iterations", i);
	debug("canon tolerated change", maxChange);
}

// factory methods

std::vector<Polyhedron> PolyhedronFactory::make(std::string ops){
	std::vector<Polyhedron> polys;
	
	// ensure only valid terms are used
	const std::string opchars("djaknztoegsmbTOCIDc");
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
	const std::string acceptedOps("dakgc");
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
		debug("current operator", *it);
		char op = *it++;
		switch(op){
			case 'T':
				polys.push_back(Polyhedron()); // mesh assumptions: edge pairs are sorted lowest-highest index; faces are in winding order
				tetrahedron(polys.back());
				break;
			default:
				mutate(polys.back(), op);
				break;
		}
	}
	return polys;
}

void PolyhedronFactory::mutate(Polyhedron &p, char op){
	switch(op){
		case 'd':
			duality(p);
			break;
		case 'a':
			rectify(p);
			break;
		case 'k':
			akisate(p);
			break;
		case 'g':
			gyrate(p);
			break;
		case 'c':
			canonicalize(p.vertices, p.edges, p.faces, 10);
			break;
	}
	debug("new polyhedron count", std::vector<std::size_t>{p.vertices.size(), p.edges.size(), p.faces.size()});
}