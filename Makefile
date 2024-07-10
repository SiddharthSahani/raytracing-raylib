
CXXFLAGS = -O1
DEFINES  = -D ENABLE_TRACE_LOGGING
INCLUDES = -I . -I external/
LDFLAGS  = -L external/raylib -lraylib -lgdi32 -lwinmm

TARGET = raytracing-raylib.exe

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)


$(TARGET): main.cpp $(OBJECTS)
	g++ -o $(TARGET) $^ $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(LDFLAGS)


%.o: %.cpp
	g++ -o $@ -c $< $(CXXFLAGS) $(DEFINES) $(INCLUDES)


clean:
	rm -f $(wildcard src/*.o)
	rm -f $(TARGET)
