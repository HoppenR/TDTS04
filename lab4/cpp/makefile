EXECUTABLE := RouterSimulator

CC := g++
CXXFLAGS += -std=c++17
CXXFLAGS += -Iinclude
CXXFLAGS += -fPIC
CXXFLAGS += -Wall
CXXFLAGS += -Wextra
CXXFLAGS += $(shell pkg-config --cflags Qt5Widgets)
LDFLAGS := $(shell pkg-config --libs Qt5Widgets)

SRCDIR := src
OBJDIR := obj

SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
