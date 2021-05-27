CXX = g++
LD = g++
CXXFLAGS = -std=c++17 -O3 -I./inc -Ilib/cul/inc -Wall -pedantic -Werror -DMACRO_PLATFORM_LINUX
SOURCES  = $(shell find src | grep '[.]cpp$$')
OBJECTS_DIR = .release-build
OBJECTS = $(addprefix $(OBJECTS_DIR)/,$(SOURCES:%.cpp=%.o))

$(OBJECTS_DIR)/%.o: | $(OBJECTS_DIR)/src
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $@

.PHONY: default
default: $(OBJECTS)
	@echo $(SOURCES)
	ar rvs libasg.a $(OBJECTS)

$(OBJECTS_DIR)/src:
	mkdir -p $(OBJECTS_DIR)/src/sfml
.PHONY: clean
clean:
	rm -rf $(OBJECTS_DIR)

DEMO_OPTIONS = -g -L/usr/lib/ -L$(shell pwd) -L$(shell pwd)/lib/cul -lsfml-system -lsfml-graphics -lsfml-window -lasg -lcommon
.PHONY: demos
demos:
	$(CXX) $(CXXFLAGS) demos/demo.cpp $(DEMO_OPTIONS) -o demos/.demo
	$(CXX) $(CXXFLAGS) demos/spacer-tests.cpp $(DEMO_OPTIONS) -o demos/.spacer_tests
	$(CXX) $(CXXFLAGS) demos/drag-frames.cpp $(DEMO_OPTIONS) -o demos/.drag_frames
