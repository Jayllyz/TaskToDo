#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <stdio.h>
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

struct data {
    GtkBuilder *builder;
    GtkWidget *window;
    GtkButton *addTask;
    GtkWidget *taskStatus[10];
    GtkWidget *taskSeparator[10];
    GtkBox *boxV;
    GtkWidget *boxTask[10];
    GtkWidget *task[10];
    GtkWidget *taskPriority[10];
    GtkWidget *taskDelete[10];
    int taskNumber[10];
    GtkWidget *taskNumberMarker[10];
    int i;
    int maxTask;
    GtkWidget *inputEntry;
    int unusedTaskSpace;
};

void addTasks(GtkWidget *task, gpointer data);
char *get_text_of_entry(GtkWidget *testEntry);
int readOneConfigValue(char *propName);
void changeTaskStatus(GtkWidget *taskStatus, gpointer data);
void changeTaskPriority(GtkWidget *taskPriority, gpointer data);
void deleteTask(GtkWidget *taskDelete, gpointer data);
PGconn *connectBdd();
int createTables(PGconn *conn);
void bddExist(PGconn *conn, PGresult *res);
#endif