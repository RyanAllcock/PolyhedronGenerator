#include <vector> // argument listing & accessing
#include <tuple> // argument-index pairing
#include <algorithm> // argument searching

class ArgumentReader{
	static std::pair<int, std::string> getPair(const char *id, const char *value, std::vector<std::string> const &ids){
		std::vector<std::string>::const_iterator it = std::find_if(ids.begin(), ids.end(), 
			[id](std::string const &type){ return type == id; });
		if(it == ids.end()) return {-1, ""};
		return {it - ids.begin(), value};
	}
	static std::string getImplicitValue(const char *value, std::vector<std::string> const &ids){
		if(!std::any_of(ids.begin(), ids.end(), [value](std::string const &type){ return type == value; }))
			return std::string(value);
		return std::string("");
	}
public:
	static std::vector<std::string> get(int c, char *v[], std::vector<std::string> const &ids, int implicits){
		std::vector<std::string> arguments(ids.size());
		if(c < 1) return arguments;
		int a = 0;
		while(a < implicits && (arguments[a] = getImplicitValue(v[a], ids)) != "") a++;
		while(a + 1 < c){
			std::pair<int, std::string> arg = getPair(v[a], v[a + 1], ids);
			if(std::get<0>(arg) > -1) arguments[std::get<0>(arg)] = std::get<1>(arg);
			else break;
			a += 2;
		}
		return arguments;
	}
	template <typename T>
	static T match(std::vector<std::pair<std::string, T>> const &types, std::string const &target, T defaultValue){
		typename std::vector<std::pair<std::string, T>>::const_iterator select = std::find_if(types.begin(), types.end(), 
			[target](std::pair<std::string, T> const &option){ return target == std::get<0>(option); });
		if(select == types.end()) return defaultValue;
		return std::get<1>(*select);
	}
};