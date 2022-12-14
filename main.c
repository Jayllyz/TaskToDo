/*
Author: Simon & Antony
Date: 24-11-2022
Description: Main file of our Todo list software
*/

#include "functions.c"
#include "functions.h"
#include "settings/bdd.c"
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    system("clear"); //provisoire c'est jsute pour automatiquement clear le terminal
    system("sudo service postgresql start"); //provisoire
    //Init
    gtk_init(&argc, &argv);
    struct data user;
    if (readOneConfigValue("init") == 0) {
        PGconn *conn = connectBdd();
        if (conn == NULL) {
            return EXIT_FAILURE;
        }
        createTables(conn);
        PQfinish(conn);
    }

    user.builder = gtk_builder_new();
    gtk_builder_add_from_file(user.builder, "data/window_main.glade", NULL);

    //Datas
    user.window = GTK_WIDGET(gtk_builder_get_object(user.builder, "window_main"));
    user.addProjects = GTK_BUTTON(gtk_builder_get_object(user.builder, "add_projects"));
    user.refresh = GTK_BUTTON(gtk_builder_get_object(user.builder, "refresh")); //refresh labeltext
    user.boxC = GTK_BOX(gtk_builder_get_object(user.builder, "boxC"));
    user.boxV = GTK_BOX(gtk_builder_get_object(user.builder, "boxV"));
    user.i = 0;

    user.inputEntry = GTK_WIDGET(gtk_builder_get_object(user.builder, "inputEntry"));
    user.outputLabel = GTK_LABEL(gtk_builder_get_object(user.builder, "outputLabel"));

    //signals

    gtk_entry_set_max_length(GTK_ENTRY(user.inputEntry), 20); //limit char input

    g_signal_connect(user.addProjects, "clicked", G_CALLBACK(click_projects), &user);
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
