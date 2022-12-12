#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <stdio.h>
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

struct data {
    GtkBuilder *builder;
    GtkWidget *window;
    GtkButton *addProjects;
    GtkButton *refresh;
    GtkBox *boxC;
    GtkBox *boxV;
    GtkWidget *buttonAddProject[10];
    GtkWidget *windowTest;
    int i;

    GtkWidget *inputEntry;
    GtkLabel *outputLabel;
};

void click_projects(GtkLabel *label, gpointer data);
void refreshButton(GtkWidget *testLabel2, gpointer data);
char *get_text_of_entry(GtkWidget *testEntry);
int readOneConfigValue(char *propName);
PGconn *connectBdd();
int createTables(PGconn *conn);
void bddExist(PGconn *conn, PGresult *res);
#endif