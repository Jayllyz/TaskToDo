/*
Author: Simon & Antony 
Date: 24-11-2022
Description: Main file of our Todo list software
*/

#include <gtk/gtk.h>

int main (int argc, char **argv)
{
//--m TEST
int x=30; int y= 34; int z=0;
char buf[256];
z=100*(x+y);
sprintf(buf, "Addition de x+y" "\r" "x+y=" "%d" , z);
//--affichage du résultat dans une fenêtre gtk centrée------ GtkWidget *window;
GtkWidget *label;
GtkWidget* window;
gtk_init (&argc, &argv);
window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
gtk_window_set_title(GTK_WINDOW(window), "Ma fenêtre");
gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (gtk_main_quit ), NULL);
label = gtk_label_new (buf);
gtk_misc_set_alignment( GTK_MISC(label), 0.0, 0.0 );
gtk_container_add (GTK_CONTAINER (window), label);
gtk_widget_show (label);
gtk_widget_show (window);
gtk_main ();
//-------------------------------------------------------
return 0;
}