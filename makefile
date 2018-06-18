OBJ = main.o
CXXLIBS = `sdl2-config --libs` -lGL -lfreecamera -lsimpleshader -lblenderimport
CXXFLAGS = -Wall `sdl2-config --cflags` -I../../../include -L../../../lib -O3 -g
perlin: $(OBJ)
	$(CXX) -o perlin $(OBJ) $(CXXLIBS) $(CXXFLAGS)

