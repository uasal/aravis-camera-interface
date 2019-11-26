// system and aravis includes
#include "glib.h"
#include "arv.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <png.h> // Requires libpng1.2
#include <assert.h>
#include "camera.h"
#include <pthread.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

using namespace std;

Camera camera = Camera();
bool done = false;

gboolean keypressHandler(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	printf("%x\n", event->keyval);
	switch(event->keyval) {
		case GDK_KEY_s: 
			camera.startStream(50);
			break;
		case GDK_KEY_q:
			camera.stopStream();
			break;
		case GDK_KEY_f:
			camera.freeStream();
			break;
		case GDK_KEY_n:
			camera.getSnapshot();
			break;
		case GDK_KEY_d:
			gtk_main_quit();
			break;
		default:
			cout << "excuse me?" << endl;
			break;
	}
	return false;
}

void *saveThread(void *vargp) 
{
	gint minH, maxH, minW, maxW;
	arv_camera_get_height_bounds(camera.getArvInstance(), &minH, &maxH);
	arv_camera_get_width_bounds(camera.getArvInstance(), &minW, &maxW);
	printf("width %d %d height %d %d\n", minW, maxW, minH, maxH);
	camera.configureStream(3.0, maxW/2, maxH/2);
	
	return NULL;
} 

int main(int argc, char *argv[]){
	pthread_t id;
	pthread_create(&id, NULL, saveThread, NULL); 
	printf("middle of main\n");
	pthread_join(id, NULL);
	gtk_init(&argc, &argv);
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(keypressHandler), NULL);
	gtk_widget_show_all(window);
	gtk_main();
	printf("done here\n"); 
	return 0;
}
