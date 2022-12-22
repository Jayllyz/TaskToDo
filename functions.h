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
    GtkWidget *taskStatus[50];
    GtkWidget *taskSeparator[50];
    GtkBox *boxV;
    GtkWidget *boxTask[50];
    GtkWidget *task[50];
    GtkWidget *taskPriority[50];
    GtkWidget *taskEdit[50];
    GtkWidget *taskDelete[50];
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
    int maxTaskPerProject;
    int maxTaskTotal;
    int maxProject;
    int projectCount;
    int unusedTaskSpace;
    int repopulatedTask;
    int repopulatedProject;
    int inEditingId;
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
gchar *get_text_of_entry(GtkWidget *testEntry);
int readOneConfigValue(char *propName);
int projectExist(PGconn *conn, const gchar *name);
void addProjectWindow(GtkWidget *project, gpointer data);
void addProject(GtkWidget *projet, gint clicked, gpointer data, int presentProject);

//bdd.c
PGconn *connectBdd();
int createTables(PGconn *conn);
void bddExist(PGconn *conn, PGresult *res);
int insertTask(PGconn *conn, int id, char *name, char *description, int priority, char *deadline, int status, const gchar *projectName);
int insertProject(PGconn *conn, char *name, char *description, int priority, char *deadline, char *color);
int deleteTaskDB(PGconn *conn, int id);
int deleteProjectDB(PGconn *conn, const gchar *name);
int allTask(PGconn *conn);
int allProject(PGconn *conn);
char *selectTask(PGconn *conn, int row);
int selectTaskId(PGconn *conn, int row);
char *selectProject(PGconn *conn, int row);
char *selectDescription(PGconn *conn, int id);
int selectPriority(PGconn *conn, int id);
int updateDescription(PGconn *conn, const gchar *description, int id);
int updatePriority(PGconn *conn, int priority, int id);

#endif