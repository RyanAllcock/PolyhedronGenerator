CXX := g++

BIN := temp/
OUT := deploy/
LIB := lib/
UTIL := utils/
SRC := source/
LINKS := -lopenGL32 -lmingw32 -lSDL2main -lSDL2 -lglew32
OBJECTS := $(BIN)camera.o $(BIN)window.o $(BIN)shader.o $(BIN)polyhedra.o $(BIN)model.o
STATIC := -static
MAIN := $(CXX) -o $(OUT)polyhedra.exe $(OBJECTS) main.cpp $(LINKS)

main: main.cpp $(OBJECTS)
	$(MAIN)

$(BIN)camera.o: $(LIB)camera.cpp $(LIB)camera.hpp $(UTIL)debug.hpp
	$(CXX) -c -o $(BIN)camera.o $(LIB)camera.cpp

$(BIN)window.o: $(LIB)window.cpp $(LIB)window.hpp $(UTIL)debug.hpp
	$(CXX) -c -o $(BIN)window.o $(LIB)window.cpp

$(BIN)shader.o: $(LIB)shader.cpp $(LIB)shader.hpp $(UTIL)debug.hpp
	$(CXX) -c -o $(BIN)shader.o $(LIB)shader.cpp

$(BIN)polyhedra.o: $(SRC)polyhedra.cpp $(SRC)polyhedra.hpp $(UTIL)debug.hpp $(UTIL)maths.hpp debug_polyhedra.cpp
	$(CXX) -c -o $(BIN)polyhedra.o debug_polyhedra.cpp

$(BIN)model.o: $(LIB)model.cpp $(LIB)model.hpp $(UTIL)debug.hpp
	$(CXX) -c -o $(BIN)model.o $(LIB)model.cpp

prepare:
	mkdir $(BIN) $(OUT)

clean:
	cd temp & del /q /s "*.o" & cd .. & $(MAKE) --no-print-directory main