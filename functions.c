#include "functions.h"
#include <gtk/gtk.h>
#include <stdio.h>

void click_projects(GtkLabel *label, gpointer data)
{
    struct data *user = data;

    if (user->i != 6) { //Nombre de projets ajoutable
        user->buttonAddProject[user->i] = gtk_button_new_with_label("Projet ajouté");
        gtk_widget_show(user->buttonAddProject[user->i]);

        gtk_box_pack_start(user->boxV, user->buttonAddProject[user->i], TRUE, TRUE, 0);
        gtk_box_reorder_child(user->boxV, user->buttonAddProject[user->i], 1);
        user->i++;
    }
}

char *get_text_of_entry(GtkWidget *inputEntry) //recup le contenu d'un "textview"
{
    GtkEntryBuffer *buffer = gtk_entry_get_buffer((GtkEntry *)inputEntry);
    gchar *text;
    text = gtk_entry_buffer_get_text(buffer);
    return text;
}

void refreshButton(GtkWidget *outputLabel, gpointer data)
{
    struct data *user = data;
    char *geText;
    geText = malloc(sizeof(char) * strlen(get_text_of_entry(user->inputEntry)) + 1);
    strcpy(geText, get_text_of_entry(user->inputEntry));
    gtk_label_set_text(user->outputLabel, geText);
}

int readOneConfigValue(char *propName)
{
    FILE *file = fopen("settings/config.txt", "r");
    if (file == NULL) {
        printf("Error: config file not found");
        return 1;
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