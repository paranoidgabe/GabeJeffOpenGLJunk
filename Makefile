CXX = g++
CXXFLAGS = -DGLEW_STATIC -Wall --std=c++0x

OBJ_DIR=bin
OBJ_BIN=bin

UNAME := $(shell uname)

ifeq ($(UNAME),Darwin)
LIB_DIR=-L/usr/local/lib -L./lib/osx
INC_DIR=-I/usr/local/include -Iincludes -I.
LDLIBS =-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -lglfw3 -lGLEW
EXECUTABLE=main
else ifeq ($UNAME), Linux)
LIB_DIR=-L/usr/local/lib -L./lib/linux
INC_DIR=-I/usr/local/include:includes:.
LDLIBS = -lglfw -lm -lGLEW -lGL -lGLU -lXrandr -lXi 
EXECUTABLE=main
else

CXX = g++ -m32
CXXFLAGS = -Wall --std=c++0x -DGLFW_BUILD_DLL -DGLEW_STATIC
LIB_DIR=-Llib/Win32
INC_DIR=-Iincludes
LDLIBS =-lm -lglew32 -lopengl32 -lglu32 -lgdi32 -lglfw3
EXECUTABLE=main.exe

endif

SOURCE=main.cpp
OBJECTS=${SOURCE:%.cpp=$(OBJ_DIR)/%.o}

all: math renderer generators $(OBJECTS) $(OBJ_BIN)/$(EXECUTABLE)

$(OBJ_BIN)/$(EXECUTABLE):
	$(CXX) $(CXXFLAGS) $(LD_FLAGS) $(LIB_DIR) -o $@ $(OBJECTS) bin/generators.a bin/math.a bin/renderer.a $(LDLIBS)

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIR) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR) $(EXECUTABLE)/*.o main
	$(MAKE) -C math clean
	$(MAKE) -C renderer clean
	$(MAKE) -C generators clean
	mkdir bin

test:
	cd tests
	make
	cd ..

.PHONY:	math

.PHONY: renderer

.PHONY: generators

renderer:
	$(MAKE) -C renderer

math:
	$(MAKE) -C math

generators:
	$(MAKE) -C generators

run: clean all
	cd $(OBJ_BIN); ./$(EXECUTABLE) 
