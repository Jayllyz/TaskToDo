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
    //system("sudo service postgresql start"); //provisoire
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
    user.maxTask = 6;
    user.unusedTaskSpace = user.maxTask;
    user.window = GTK_WIDGET(gtk_builder_get_object(user.builder, "window_main"));
    user.addTask = GTK_BUTTON(gtk_builder_get_object(user.builder, "addTask"));
    user.boxV = GTK_BOX(gtk_builder_get_object(user.builder, "boxV"));
    user.i = 0;
    user.inputEntry = GTK_WIDGET(gtk_builder_get_object(user.builder, "inputEntry"));
    for (int i = 0; i < user.maxTask; i++) {
        user.task[i] = gtk_label_new("");
        user.taskNumber[i] = i;
    }

    //signals
    gtk_entry_set_max_length(GTK_ENTRY(user.inputEntry), 35); //limit char input

    g_signal_connect(user.addTask, "clicked", G_CALLBACK(addTasks), &user);
    for (int i = 0; i < user.maxTask; i++) {
        // g_signal_connect(user.taskStatus[i], "clicked", G_CALLBACK(changeTaskStatus), &user);
        // g_signal_connect(user.taskPriority[i], "clicked", G_CALLBACK(changeTaskPriority), &user);
        // g_signal_connect(user.taskDelete[i], "clicked", G_CALLBACK(deleteTask), &user);
    }
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
