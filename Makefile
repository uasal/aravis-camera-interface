CXX=g++
#INCLUDES=-I/usr/local/include
#LIBPATH=-L/usr/local/lib 

CFLAGS=-O0 -g3 -Wall -fmessage-length=0 -MMD -MP -Wl,--verbose

#%.o: %.cpp
#	$(CXX) -o $@ $< -c $(CFLAGS) 

main: main.cpp
	g++ -o main  main.cpp -laravis-0.6 \
	-L/usr/lib -L/usr/local/lib \
	-I/home/pi/Documents/aravis-0.6.4/src/ \
	-L/home/pi/Documents/aravis-0.6.4/src/.libs/ \
	-I/usr/include/glib-2.0/ \
	-I/usr/lib/x86_64-linux-gnu/glib-2.0/include \
	-laravis-0.6 \
	-lglib-2.0 \
	`pkg-config --libs --cflags glib-2.0` \
	-lm -pthread -lgio-2.0 -lgobject-2.0 -lxml2 -lgthread-2.0 -lglib-2.0 -lz -lpng


