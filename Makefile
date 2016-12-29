CPPFILES=$(wildcard *.cc)
CFILES=$(wildcard *.c)
OBJECTS=$(patsubst %.cc, %.o, $(CPPFILES)) $(patsubst %.c, %.o, $(CFILES))

LDFLAGS+=-lm -static
EXECUTABLE=vfatbuse

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $^ $(CPPFLAGS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) -MMD -MP $(CPPFLAGS) -o $@ -c $<

DEPS=$(OBJECTS:.o=.d)
-include $(DEPS)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS) $(DEPS)
