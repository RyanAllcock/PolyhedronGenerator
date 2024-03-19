CXX := g++
CXXFLAGS := -o

BIN := temp/
UTIL := utils/
LINKS := -lopenGL32 -lmingw32 -lSDL2main -lSDL2 -lglew32
OBJECTS := $(BIN)camera.o $(BIN)window.o $(BIN)shader.o $(BIN)polyhedra.o $(BIN)model.o
STATIC := -static
MAIN := $(CXX) $(CXXFLAGS) polyhedra.exe $(OBJECTS) main.cpp $(LINKS)

main: main.cpp $(OBJECTS)
	$(MAIN)

$(BIN)camera.o: camera.cpp camera.hpp $(UTIL)debug.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)camera.o camera.cpp

$(BIN)window.o: window.cpp window.hpp $(UTIL)debug.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)window.o window.cpp

$(BIN)shader.o: shader.cpp shader.hpp $(UTIL)debug.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)shader.o shader.cpp

$(BIN)polyhedra.o: polyhedra.cpp polyhedra.hpp $(UTIL)debug.hpp $(UTIL)maths.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)polyhedra.o polyhedra.cpp

$(BIN)model.o: model.cpp model.hpp $(UTIL)debug.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)model.o model.cpp

prepare:
	mkdir temp

clean:
	cd temp & del /q /s "*.o" & cd .. & $(MAKE) --no-print-directory main