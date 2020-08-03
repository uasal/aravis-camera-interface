# add comments with options, make options

CXX=g++
LIBS=`pkg-config --libs --cflags aravis-0.6` \
	-lm -pthread -lgio-2.0 -lgobject-2.0 \
	-lxml2 -lgthread-2.0 -lglib-2.0 -lz -lpng 
DEPS=*.h
CFLAGS=-O0 -g3 -Wall -fmessage-length=0 -MMD -MP

%.o: %.cpp $(DEPS)
	$(CXX) -o $@ $< -c $(CFLAGS) $(LIBS)

hdcamera: main.o hdcamera.o
	$(CXX) -o hdcamera main.o hdcamera.o $(CFLAGS) $(LIBS)
	
clean:
	rm -rf *.o *.d
	rm -rf main
	rm -rf *.png

