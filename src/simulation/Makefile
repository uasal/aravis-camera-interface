CXX = g++
DEPS = *.h
LIBS = -lpthread -lrt
CFLAGS = -O0 -g3 -Wall -fmessage-length=0 -MMD -MP -lpthread
OBJ = membuffersimulatortest

%.o: %.cpp $(DEPS)
	$(CXX) -o $@ $< -c $(CFLAGS) $(LIBS)

all: $(OBJ)

$(OBJ): %: %.o membuffersimulator.o
	$(CXX) -o $@ membuffersimulator.o $< $(CFLAGS) $(LIBS)
	
clean:
	rm -rf *.o *.d
	rm -rf $(OBJ)

