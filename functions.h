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
    GtkWidget *taskEdit[10];
    GtkWidget *taskDelete[10];
    int taskNumber[10];
    GtkWidget *taskNumberMarker[10];
    int i;
    int maxTask;
    GtkWidget *inputEntry;
    GtkLabel *outputLabel;
    PGconn *conn;
    GtkNotebook *notebook;
    int unusedTaskSpace;
    int repopulated;
    GtkWidget *descriptionEntry;
    GtkWidget *inEditing;
};

//function.c
void changeTaskStatus(GtkWidget *taskStatus, gpointer data);
void changeTaskPriority(GtkWidget *taskPriority, gpointer data);
void editTaskWindow(GtkWidget *taskEdit, gpointer data);
void editTaskDB(GtkDialog *window, gint clicked, gpointer entry);
void addTasks(GtkWidget *task, gpointer data, int presentTask);
void deleteTask(GtkWidget *taskDelete, gpointer data);
char *get_text_of_entry(GtkWidget *testEntry);
int readOneConfigValue(char *propName);
int taskExist(PGconn *conn, char *input, const gchar *name);

//bdd.c
PGconn *connectBdd();
int createTables(PGconn *conn);
void bddExist(PGconn *conn, PGresult *res);
int insertTask(PGconn *conn, char *name, char *description, int priority, char *deadline, int status, const gchar *projectName);
int insertProject(PGconn *conn, char *name, char *description, int priority, char *deadline, char *color);
int deleteTaskDB(PGconn *conn, const gchar *name);
int allTask(PGconn *conn);
char *selectTask(PGconn *conn, int row);
char *selectDescription(PGconn *conn, const gchar *name);
int selectPriority(PGconn *conn, const gchar *name);
int updateDescription(PGconn *conn, const gchar *description, const gchar *name);
int updatePriority(PGconn *conn, int priority, const gchar *name);

#endif