/*
Author: Simon & Antony
Date: 24-11-2022
Description: Main file of our Todo list software
*/

#include "functions.c"
#include "functions.h"
#include "settings/bdd.c"
#include <curl/curl.h>
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
    if (data.conn == NULL) {
        g_print("Error: can't connect to database");
        return EXIT_FAILURE;
    }
    if (readOneConfigValue("init") == 0)
        createTables(data.conn);

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    int newConnect = 0;

    if (readOneConfigValue("last connect day") != local_time->tm_mday)
        newConnect = 1;
    if (readOneConfigValue("last connect month") != local_time->tm_mon + 1)
        newConnect = 1;
    if (readOneConfigValue("last connect year") != local_time->tm_year + 1900)
        newConnect = 1;

    data.tools.builder = gtk_builder_new();
    gtk_builder_add_from_file(data.tools.builder, "data/window_main.glade", NULL);

    //Datas
    data.state.maxTaskTotal = 50;
    data.state.maxTaskPerProject = 6;
    data.state.maxProject = 3;
    data.tools.window = GTK_WIDGET(gtk_builder_get_object(data.tools.builder, "window_main"));
    data.tools.addTask = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "addTask"));
    data.tools.addProject = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "addProject"));
    data.tools.boxV = GTK_BOX(gtk_builder_get_object(data.tools.builder, "boxV"));
    data.state.i = 0;
    data.tools.inputEntry = GTK_WIDGET(gtk_builder_get_object(data.tools.builder, "inputEntry"));
    data.tools.notebook = GTK_NOTEBOOK(gtk_builder_get_object(data.tools.builder, "project_notebook"));
    data.tools.calendar = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "calendar"));
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

    int queryResult = allProject(data.conn);
    if (queryResult == -1)
        g_print("Error: can't collect all projects");

    for (int i = 0; i < queryResult; i++) {
        addProject(GTK_WIDGET(data.tools.addProject), GTK_RESPONSE_OK, &data, i);
    }
    data.state.repopulatedProject = 1;

    queryResult = allTask(data.conn);
    if (queryResult == -1) {
        g_print("Error: can't collect all tasks");
    }
    for (int i = 0; i < queryResult; i++) {
        int taskToAdd = selectTaskId(data.conn, i);
        data.state.taskNumber[taskToAdd] = -1;
        char *project = selectProjectName(data.conn, taskToAdd);
        addTasks(GTK_WIDGET(data.tools.addTask), &data, taskToAdd, project);
        addImportantTask(&data, taskToAdd);
        addMinorTask(&data, taskToAdd);
        addLateTask(&data, taskToAdd);
        addPlannedTask(&data, taskToAdd);
    }
    data.state.repopulatedTask = 1;

    gtk_notebook_set_current_page(data.tools.notebook, 0);

    g_signal_connect(data.tools.addTask, "clicked", G_CALLBACK(addTasks), &data);
    g_signal_connect(data.tools.addProject, "clicked", G_CALLBACK(addProjectWindow), &data);
    g_signal_connect(data.tools.calendar, "clicked", G_CALLBACK(calendarDialog), &data);

    gtk_builder_connect_signals(data.tools.builder, NULL);

    g_object_unref(data.tools.builder);

    gtk_widget_show(data.tools.window);

    if (newConnect == 1) {
        newConnectUpdate(local_time->tm_mday, local_time->tm_mon + 1, local_time->tm_year + 1900);
        gchar *message = malloc(sizeof(char) * strlen("Vous avez ??? tâches urgentes à réaliser et ??? tâches en retard"));
        message = warningMessage(&data);
        if (strcmp(message, "empty") != 0) {
            GtkDialog *dialog = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(data.tools.window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", message));
            gtk_dialog_run(dialog);
            gtk_widget_destroy(GTK_WIDGET(dialog));
        }
        free(message);
    }

    gtk_main();

    return 0;
}

// called when window is closed
void on_window_main_destroy()
{
    gtk_main_quit();
}
