PROGRAM = bracket
CXXFLAGS=-I/usr/local/include
LDFLAGS=-L/usr/local/lib -lgphoto2 -lgphoto2_port -lm
OBJECTS=bracket.o

.SUFFIXES: .cpp .o

$(PROGRAM): $(OBJECTS)
	$(CXX)  $^ $(LDFLAGS)  -o $(PROGRAM)

.c.o:
	$(CXX) -c $(CXXFLAGS) $<

.PHONY:clean
clean:
	rm -f bracket *.o
