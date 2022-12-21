#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <stdio.h>
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

struct GTKTools {
    GtkBuilder *builder;
    GtkWidget *window;
    GtkButton *addTask;
    GtkButton *addProject;
    GtkWidget *taskStatus[10];
    GtkWidget *taskSeparator[10];
    GtkBox *boxV;
    GtkWidget *boxTask[10];
    GtkWidget *task[10];
    GtkWidget *taskPriority[10];
    GtkWidget *taskEdit[10];
    GtkWidget *taskDelete[10];
    GtkWidget *inputEntry;
    GtkLabel *outputLabel;
    GtkNotebook *notebook;
    GtkWidget *descriptionEntry;
    GtkWidget *inEditing;
    GtkWidget *projectNameEntry;
    GtkWidget *pageTitleBox[10];
    GtkWidget *projectTaskBox[10];
};

struct TaskProjectState {
    int taskNumber[10];
    int projectNumber[10];
    int i;
    int maxTask;
    int maxProject;
    int projectCount;
    int unusedTaskSpace;
    int repopulatedTask;
    int repopulatedProject;
};

struct data {
    struct GTKTools tools;
    struct TaskProjectState state;
    PGconn *conn;
};

//function.c
void changeTaskStatus(GtkWidget *taskStatus, gpointer data);
void changeTaskPriority(GtkWidget *taskPriority, gpointer data);
void editTaskWindow(GtkWidget *taskEdit, gpointer data);
void editTaskDB(GtkDialog *window, gint clicked, gpointer entry);
void addTasks(GtkWidget *task, gpointer data, int presentTask);
void deleteTask(GtkWidget *taskDelete, gpointer data);
void deleteProject(GtkWidget *projectDelete, gpointer data);
char *get_text_of_entry(GtkWidget *testEntry);
int readOneConfigValue(char *propName);
int taskExist(PGconn *conn, char *input, const gchar *name);
int projectExist(PGconn *conn, const gchar *name);
void addProjectWindow(GtkWidget *project, gpointer data);
void addProject(GtkWidget *projet, gint clicked, gpointer data, int presentProject);

//bdd.c
PGconn *connectBdd();
int createTables(PGconn *conn);
void bddExist(PGconn *conn, PGresult *res);
int insertTask(PGconn *conn, char *name, char *description, int priority, char *deadline, int status, const gchar *projectName);
int insertProject(PGconn *conn, char *name, char *description, int priority, char *deadline, char *color);
int deleteTaskDB(PGconn *conn, const gchar *name);
int deleteProjectDB(PGconn *conn, const gchar *name);
int allTask(PGconn *conn);
int allProject(PGconn *conn);
char *selectTask(PGconn *conn, int row);
char *selectProject(PGconn *conn, int row);
char *selectDescription(PGconn *conn, const gchar *name);
int selectPriority(PGconn *conn, const gchar *name);
int updateDescription(PGconn *conn, const gchar *description, const gchar *name);
int updatePriority(PGconn *conn, int priority, const gchar *name);

#endif