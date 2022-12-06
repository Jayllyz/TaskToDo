#include "functions.h"
#include <gtk/gtk.h>
#include <stdio.h>

void click_projects(GtkLabel *label, gpointer data)
{
    struct data *user = data;

    if (user->i != 6){
        user->buttonAddProject[user->i] = gtk_button_new_with_label ("Projet jouté");
        gtk_widget_show(user->buttonAddProject[user->i]);

        gtk_box_pack_start (user->boxV, user->buttonAddProject[user->i], TRUE, TRUE, 0);
        gtk_box_reorder_child (user->boxV, user->buttonAddProject[user->i], 1);
        user->i++;
    }  
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