#include "functions.h"
#include <gtk/gtk.h>
#include <stdio.h>

void click_projects(GtkLabel *label, gpointer data)
{
    gtk_label_set_text(label, "Projet ajouté");
}

char *get_text_of_textview(GtkWidget *text_view) //recup le contenu d'un "textview"
{
    GtkTextIter start, end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer((GtkTextView *)text_view);
    gchar *text;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    return text;
}

void refreshButton(GtkWidget *refresh, gpointer data)
{
    struct data *user = data;
    char *geText;
    geText = malloc(sizeof(char) * strlen(get_text_of_textview(user->textview)) + 1);
    strcpy(geText, get_text_of_textview(user->textview));
    gtk_label_set_text(user->labeltext, geText);
}