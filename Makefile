
CXXFLAGS = -O1
DEFINES  = 
INCLUDES = -I . -I external/
LDFLAGS  = -L external/raylib -lraylib -lgdi32 -lwinmm

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

REALTIME_TARGET = realtime-raytracing.exe


realtime: $(REALTIME_TARGET)


$(REALTIME_TARGET): realtime-raytracing.cpp $(OBJECTS)
	g++ -o $(REALTIME_TARGET) $^ $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(LDFLAGS)


%.o: %.cpp
	g++ -o $@ -c $< $(CXXFLAGS) $(DEFINES) $(INCLUDES)


clean:
	rm -f $(wildcard src/*.o)
	rm -f $(REALTIME_TARGET)
