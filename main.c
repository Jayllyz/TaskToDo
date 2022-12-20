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
    user.conn = connectBdd();
    if (readOneConfigValue("init") == 0) {
        user.conn = connectBdd();
        if (user.conn == NULL) {
            return EXIT_FAILURE;
        }
        createTables(user.conn);
        PQfinish(user.conn);
    }

    user.builder = gtk_builder_new();
    gtk_builder_add_from_file(user.builder, "data/window_main.glade", NULL);

    //Datas
    user.maxTask = 6;
    user.unusedTaskSpace = user.maxTask;
    user.window = GTK_WIDGET(gtk_builder_get_object(user.builder, "window_main"));
    user.addTask = GTK_BUTTON(gtk_builder_get_object(user.builder, "addTask"));
    user.addProject = GTK_BUTTON(gtk_builder_get_object(user.builder, "addProject"));
    user.boxV = GTK_BOX(gtk_builder_get_object(user.builder, "boxV"));
    user.i = 0;
    user.inputEntry = GTK_WIDGET(gtk_builder_get_object(user.builder, "inputEntry"));
    user.notebook = GTK_NOTEBOOK(gtk_builder_get_object(user.builder, "project_notebook"));
    user.repopulated = 0;
    for (int i = 0; i < user.maxTask; i++) {
        user.task[i] = gtk_label_new("");
        user.taskNumber[i] = i;
    }

    //signals
    gtk_entry_set_max_length(GTK_ENTRY(user.inputEntry), 35); //limit char input

    int queryResult = allTask(user.conn);
    if (queryResult == -1) {
        printf("Error: can't collect all tasks");
    }
    for (int i = 0; i < queryResult; i++) {
        addTasks(GTK_WIDGET(user.addTask), &user, i);
    }
    user.repopulated = 1;

    g_signal_connect(user.addTask, "clicked", G_CALLBACK(addTasks), &user);
    g_signal_connect(user.addProject, "clicked", G_CALLBACK(addProjectWindow), &user);

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
