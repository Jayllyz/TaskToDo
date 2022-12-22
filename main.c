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
    system("clear"); //provisoire
    system("sudo service postgresql start"); //provisoire
    //Init
    gtk_init(&argc, &argv);
    struct data data;
    data.conn = connectBdd();
    if (readOneConfigValue("init") == 0) {
        data.conn = connectBdd();
        if (data.conn == NULL) {
            return EXIT_FAILURE;
        }
        createTables(data.conn);
        PQfinish(data.conn);
    }

    data.tools.builder = gtk_builder_new();
    gtk_builder_add_from_file(data.tools.builder, "data/window_main.glade", NULL);

    //Datas
    data.state.maxTaskTotal = 50;
    data.state.maxTaskPerProject = 6;
    data.state.maxProject = 3;
    data.state.unusedTaskSpace = data.state.maxTaskPerProject;
    data.tools.window = GTK_WIDGET(gtk_builder_get_object(data.tools.builder, "window_main"));
    data.tools.addTask = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "addTask"));
    data.tools.addProject = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "addProject"));
    data.tools.boxV = GTK_BOX(gtk_builder_get_object(data.tools.builder, "boxV"));
    data.state.i = 0;
    data.tools.inputEntry = GTK_WIDGET(gtk_builder_get_object(data.tools.builder, "inputEntry"));
    data.tools.notebook = GTK_NOTEBOOK(gtk_builder_get_object(data.tools.builder, "project_notebook"));
    data.state.repopulatedTask = 0;
    data.state.repopulatedProject = 0;
    data.state.projectCount = 0;
    for (int i = 0; i < data.state.maxTaskTotal; i++) {
        data.tools.task[i] = gtk_label_new("");
        data.state.taskNumber[i] = i;
    }
    for (int i = 0; i < data.state.maxProject; i++) {
        data.state.projectNumber[i] = i;
    }

    //signals
    gtk_entry_set_max_length(GTK_ENTRY(data.tools.inputEntry), 35); //limit char input

    int queryResult = allTask(data.conn);
    if (queryResult == -1) {
        g_print("Error: can't collect all tasks");
    }
    for (int i = 0; i < queryResult; i++) {
        addTasks(GTK_WIDGET(data.tools.addTask), &data, i);
    }
    data.state.repopulatedTask = 1;

    queryResult = allProject(data.conn);
    if (queryResult == -1) {
        g_print("Error: can't collect all projects");
    }
    for (int i = 0; i < queryResult; i++) {
        addProject(GTK_WIDGET(data.tools.addProject), GTK_RESPONSE_OK, &data, i);
    }
    data.state.repopulatedProject = 1;

    g_signal_connect(data.tools.addTask, "clicked", G_CALLBACK(addTasks), &data);
    g_signal_connect(data.tools.addProject, "clicked", G_CALLBACK(addProjectWindow), &data);

    gtk_builder_connect_signals(data.tools.builder, NULL);

    g_object_unref(data.tools.builder);

    gtk_widget_show(data.tools.window);
    gtk_main();

    return 0;
}

// called when window is closed
void on_window_main_destroy()
{
    gtk_main_quit();
}
