/*
Author: Simon & Antony
Date: 24-11-2022
Description: Main file of our Todo list software
*/

#include "functions.c"
#include "functions.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{

    //Init
    gtk_init(&argc, &argv);
    struct data user;

    user.builder = gtk_builder_new();
    gtk_builder_add_from_file(user.builder, "data/window_main.glade", NULL);

    //Datas
    user.window = GTK_WIDGET(gtk_builder_get_object(user.builder, "window_main"));
    user.label_test = GTK_LABEL(gtk_builder_get_object(user.builder, "test_label"));
    user.labeltext = GTK_LABEL(gtk_builder_get_object(user.builder, "labeltext"));
    user.addProjects = GTK_BUTTON(gtk_builder_get_object(user.builder, "add_projects"));
    user.refresh = GTK_BUTTON(gtk_builder_get_object(user.builder, "refresh")); //refresh labeltext

    //signals
    g_signal_connect_swapped(user.addProjects, "clicked", G_CALLBACK(click_projects), user.label_test);
    g_signal_connect(user.refresh, "clicked", G_CALLBACK(refreshButton), &user);

    gtk_builder_connect_signals(user.builder, NULL);

    g_object_unref(user.builder);

    gtk_widget_show(user.window);
    gtk_main();

    return 0;
}

// called when window is closed
void on_window_main_destroy()
{
    gtk_main_quit();
}
