CXX=g++
LIB=-Lwfslib/
INC=-Iwfslib/
CXXFLAGS=$(INC) -c -Wall -Werror -std=c++14
LDFLAGS=$(LIB) -lwfs -lboost_system -lboost_filesystem -lboost_program_options -lcryptopp -lstdc++
SOURCES=main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=wfs-file-injector

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) wfslib/libwfs.a
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

wfslib/libwfs.a:
    $(MAKE) -C wfslib/wfslib

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
    $(MAKE) -C wfslib/wfslib clean
