#ifndef HEADER_DEBUG
#define HEADER_DEBUG

#include <iostream> // print testing
#include <vector> // vector data
#include <array> // array data

#define DEBUG if(1)

namespace{
	template <typename T, std::size_t U>
	std::ostream &operator<<(std::ostream &os, std::array<T, U> const &data){
		os << "{";
		for(T d : data) os << " " << d;
		os << " }";
		return os;
	}
	template <typename T>
	std::ostream &operator<<(std::ostream &os, std::vector<T> const &data){
		if(data.size() < 1) return os;
		os << "[" << data[0];
		for(int d = 1; d < data.size(); d++) os << ", " << data[d];
		os << "]";
		return os;
	}
}

template <typename T> inline void debug(std::string const &str, std::vector<std::vector<T>> const &data){
	DEBUG{
		std::cerr << str << ": [";
		for(std::vector<T> const &d : data) std::cerr << "\n" << d;
		std::cerr << "]\n";
	}
}

template <typename T, std::size_t U> inline void debug(std::string const &str, std::vector<T> const &data){
	DEBUG{
		std::cerr << str << ": [";
		for(int d = 0; d < data.size(); d += U){
			std::array<T, U> bunch;
			for(int b = 0; b < U; b++) bunch[b] = data[d + b];
			std::cerr << " " << bunch;
		}
		std::cerr << " ]\n";
	}
}

template <typename T> inline void debug(std::string const &str, T data){
	DEBUG std::cerr << str << ": " << data << "\n";
}

template <typename T> inline void debug(T data){
	DEBUG std::cerr << data << "\n";
}

#endif