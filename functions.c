#include "functions.h"
#include <gtk/gtk.h>
#include <stdio.h>

void changeTaskStatus(GtkWidget *taskStatus, gpointer data)
{
    if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Non completé") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskStatus), "En cours");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "En cours") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskStatus), "Completé");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Completé") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskStatus), "Abandonné");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Abandonné") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskStatus), "Non completé");
    }
}

void changeTaskPriority(GtkWidget *taskPriority, gpointer data)
{
    if (strcmp(gtk_button_get_label(GTK_BUTTON(taskPriority)), "+") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskPriority), "++");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskPriority)), "++") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskPriority), "!");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskPriority)), "!") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskPriority), "-");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskPriority)), "-") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskPriority), "+");
    }
}

void deleteTask(GtkWidget *taskDelete, gpointer data)
{
    struct data *user = data;
    GtkWidget *taskToDelete = gtk_widget_get_parent(taskDelete);

    gtk_widget_destroy(taskToDelete);

    user->unusedTaskSpace++;
}

void addTasks(GtkWidget *task, gpointer data)
{
    struct data *user = data;
    char *getText;
    getText = malloc(sizeof(char) * strlen(get_text_of_entry(user->inputEntry)) + 1);
    strcpy(getText, get_text_of_entry(user->inputEntry));

    if (strcmp(getText, "") == 0) {
        return;
    }

    if (user->unusedTaskSpace <= 0) {
        return;
    }

    for (user->i = 0; user->i < user->maxTask; user->i++) {
        if (strcmp(gtk_label_get_label(GTK_LABEL(user->task[user->i])), "") == 0) {
            break;
        }
    }

    user->boxTask[user->i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(user->boxV, user->boxTask[user->i], FALSE, FALSE, 0);
    gtk_box_reorder_child(user->boxV, user->boxTask[user->i], user->maxTask - user->unusedTaskSpace + 2);

    user->taskStatus[user->i] = gtk_button_new_with_label("Non completé");
    gtk_widget_set_margin_top(user->taskStatus[user->i], 10);
    gtk_widget_set_margin_bottom(user->taskStatus[user->i], 10);
    gtk_widget_set_size_request(user->taskStatus[user->i], 150, -1);
    gtk_box_pack_start(GTK_BOX(user->boxTask[user->i]), user->taskStatus[user->i], FALSE, FALSE, 0);
    g_signal_connect(user->taskStatus[user->i], "clicked", G_CALLBACK(changeTaskStatus), &user);

    user->taskSeparator[user->i] = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(user->taskSeparator[user->i], 5, -1);
    gtk_box_pack_start(GTK_BOX(user->boxTask[user->i]), user->taskSeparator[user->i], FALSE, FALSE, 0);

    user->task[user->i] = gtk_label_new(getText);
    gtk_box_pack_start(GTK_BOX(user->boxTask[user->i]), user->task[user->i], TRUE, FALSE, 0);

    user->taskPriority[user->i] = gtk_button_new_with_label("+");
    gtk_box_pack_start(GTK_BOX(user->boxTask[user->i]), user->taskPriority[user->i], FALSE, FALSE, 0);
    g_signal_connect(user->taskPriority[user->i], "clicked", G_CALLBACK(changeTaskPriority), &user);

    user->taskDelete[user->i] = gtk_button_new_with_label("X");
    gtk_box_pack_start(GTK_BOX(user->boxTask[user->i]), user->taskDelete[user->i], FALSE, FALSE, 0);
    g_signal_connect(user->taskDelete[user->i], "clicked", G_CALLBACK(deleteTask), &user);

    gtk_widget_show(user->boxTask[user->i]);
    gtk_widget_show(user->taskStatus[user->i]);
    gtk_widget_show(user->taskSeparator[user->i]);
    gtk_widget_show(user->task[user->i]);
    gtk_widget_show(user->taskPriority[user->i]);
    gtk_widget_show(user->taskDelete[user->i]);

    user->unusedTaskSpace--;
    gtk_entry_set_text(GTK_ENTRY(user->inputEntry), "");
}

char *get_text_of_entry(GtkWidget *inputEntry) //recup le contenu d'un "textview"
{
    GtkEntryBuffer *buffer = gtk_entry_get_buffer((GtkEntry *)inputEntry);
    gchar *text;
    text = gtk_entry_buffer_get_text(buffer);
    return text;
}

int readOneConfigValue(char *propName)
{
    if (propName == NULL) {
        printf("Error: propName is null");
        return -1;
    }
    FILE *file = fopen("settings/config.txt", "r");
    if (file == NULL) {
        printf("Error: config file not found");
        return -1;
    }
    char *line = NULL;
    size_t len = 0;
    while ((getline(&line, &len, file)) != -1) {
        if (strstr(line, propName) != NULL) {
            int i = 0;
            while (line[i] != ':') {
                i++;
            }
            return atoi(&line[i + 1]);
        }
    }
    return -1;
}