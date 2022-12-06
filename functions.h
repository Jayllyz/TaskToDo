#include <gtk/gtk.h>
#include <stdio.h>
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

struct data {
    GtkBuilder *builder;
    GtkWidget *window;
    GtkLabel *label_test;
    GtkButton *addProjects;
    GtkButton *refresh;
    GtkLabel *labeltext;
    GtkWidget *textview;
};

char *get_text_of_textview(GtkWidget *text_view); //recup le contenu d'un "textview"
void click_projects(GtkLabel *label, gpointer data);
void refreshButton(GtkWidget *refresh, gpointer data);

#endif