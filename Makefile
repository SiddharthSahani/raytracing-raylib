
CXX = g++
CXXFLAGS = -O1
DEFINES =
CPPFLAGS = -I . -I external/
LDFLAGS = -L external/raylib
LDLIBS = -lraylib -lgdi32 -lwinmm

SRC_DIR = src
BUILD_DIR = build

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))
TARGET = realtime-raytracing.exe


# logging for debug purpose
$(info SOURCES: $(SOURCES))
$(info OBJECTS: $(OBJECTS))


all: $(TARGET)


$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(BUILD_DIR)
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(CPPFLAGS) $(DEFINES) $(INCLUDES)


$(TARGET): $(OBJECTS)
	$(CXX) -o $(TARGET) $^ $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)


clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
