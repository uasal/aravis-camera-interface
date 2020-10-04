DEPS=*.h
CFLAGS=-O0 -g3 -Wall -fmessage-length=0 -MMD -MP

%.o: %.cpp $(DEPS)
	$(CXX) -o $@ $< -c $(CFLAGS) $(ARAVIS_DEPS)

hdcamera: main.o hdcamera.o
	$(CXX) -o hdcamera main.o hdcamera.o $(CFLAGS) $(ARAVIS_DEPS)
	
clean:
	rm -rf *.o *.d
	rm -rf main
	rm -rf *.png

