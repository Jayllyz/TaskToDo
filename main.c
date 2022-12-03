/*
Author: Simon & Antony
Date: 24-11-2022
Description: Main file of our Todo list software
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

void click_projects(GtkLabel *label, gpointer data)
{
    gtk_label_set_text(label, "Projet ajouté");
}

int main(int argc, char *argv[])
{
    GtkBuilder *builder;
    GtkWidget *event_box;
    GtkLabel *label_test;
    GtkButton *addProjects;
    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "data/window_main.glade", NULL);

    //Variables
    GtkWidget *window;
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    label_test = GTK_LABEL(gtk_builder_get_object(builder, "test_label"));
    addProjects = GTK_BUTTON(gtk_builder_get_object(builder, "add_projects"));

    g_signal_connect_swapped(addProjects, "clicked", G_CALLBACK(click_projects), label_test);

    gtk_builder_connect_signals(builder, NULL);

    g_object_unref(builder);

    gtk_widget_show(window);
    gtk_main();

    return 0;
}

// called when window is closed
void on_window_main_destroy()
{
    gtk_main_quit();
}
