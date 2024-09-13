#ifndef HEADER_MATHS
#define HEADER_MATHS

#include <array> // vertex & edge data
#include <vector> // face data

// operation extensions

namespace mathop{
	template <typename T, std::size_t U>
	inline std::array<T,U> operator/(std::array<T,U> const &a, const T b){
		std::array<T,U> out;
		for(int i = 0; i < U; i++) out[i] = a[i] / b;
		return out;
	}
	template <typename T, std::size_t U>
	inline std::array<T,U> operator-(std::array<T,U> const &a, std::array<T,U> const &b){
		std::array<T,U> out;
		for(int i = 0; i < U; i++) out[i] = a[i] - b[i];
		return out;
	}
	template <typename T, std::size_t U>
	inline std::array<T,U> operator+(std::array<T,U> const &a, std::array<T,U> const &b){
		std::array<T,U> out;
		for(int i = 0; i < U; i++) out[i] = a[i] + b[i];
		return out;
	}
	template <typename T, std::size_t U>
	inline std::array<T,U> operator*(T a, std::array<T,U> const &b){
		std::array<T,U> out;
		for(int i = 0; i < U; i++) out[i] = a * b[i];
		return out;
	}
	template <typename T, std::size_t U>
	inline std::array<T,U>& operator+=(std::array<T,U> &a, std::array<T,U> const &b){
		a = a + b;
		return a;
	}
	template <typename T, std::size_t U>
	inline std::array<T,U>& operator/=(std::array<T,U> &a, const T b){
		a = a / b;
		return a;
	}
	template <typename T, std::size_t U>
	inline std::array<T,U>& operator-=(std::array<T,U> &a, std::array<T,U> const &b){
		a = a - b;
		return a;
	}
	template <typename T, std::size_t U>
	inline std::array<T,U> operator-(std::array<T,U> const &a){
		std::array<T,U> out;
		for(int i = 0; i < U; i++) out[i] = -a[i];
		return out;
	}
}

using namespace mathop;

// general functions

namespace math{
	
	// types
	typedef std::array<int,2> edge;
	typedef std::array<float,3> vertex;
	typedef std::vector<int> face;
	typedef std::array<float,16> transform;
	
	// functions
	inline edge sort(edge const &e){
		return e[0] > e[1] ? math::edge{e[1], e[0]} : e;
	}
	inline edge sort(edge const &e, bool &isSwitched){
		edge out = e;
		isSwitched = false;
		if(out[0] > out[1]){
			std::swap(out[0], out[1]);
			isSwitched = true;
		}
		return out;
	}
	inline vertex lerp(vertex const &v0, vertex const &v1, float w){
		return w * v0 + (1.f - w) * v1;
	}
	inline vertex midpoint(vertex const &v0, vertex const &v1){
		return (v0 + v1) / 2.f;
	}
	inline math::vertex cross(math::vertex const &a, math::vertex const &b){
		return math::vertex{a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0]};
	}
	inline float dot(math::vertex const &a, math::vertex const &b){
		float out = 0;
		for(int i = 0; i < 3; i++) out += a[i] * b[i];
		return out;
	}
	inline double length(math::vertex const &v){
		return sqrt(dot(v, v));
	}
	inline math::vertex unit(math::vertex const &v){
		float mag = length(v);
		if(mag == 0) return v;
		return v / mag;
	}
	template <typename T>
	inline T max(T const &a, T const &b){
		if(b > a) return b;
		return a;
	}
	inline int pow(int base, unsigned int exponent){
		int p = base;
		if(exponent == 0) return 1;
		for(int i = 0; i < exponent; i++) p *= base;
		return p;
	}
	inline math::transform rotate(float m, math::vertex const &n){ // source: https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
		math::transform t;
		float rcos = cos(m);
		float rsin = sin(m);
		t[0] = rcos + n[0]*n[0] * (1.f - rcos);
		t[1] = n[0]*n[1] * (1.f - rcos) - n[2] * rsin;
		t[2] = n[0]*n[2] * (1.f - rcos) + n[1] * rsin;
		t[3] = 0;
		t[4] = n[1]*n[0] * (1.f - rcos) + n[2] * rsin;
		t[5] = rcos + n[1]*n[1] * (1.f - rcos);
		t[6] = n[1]*n[2] * (1.f - rcos) - n[0] * rsin;
		t[7] = 0;
		t[8] = n[2]*n[0] * (1.f - rcos) - n[1] * rsin;
		t[9] = n[2]*n[1] * (1.f - rcos) + n[0] * rsin;
		t[10]= rcos + n[2]*n[2] * (1.f - rcos);
		t[11]= 0;
		t[12]= 0;
		t[13]= 0;
		t[14]= 0;
		t[15]= 1;
		return t;
	}
}

#endif