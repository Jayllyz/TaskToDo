#include <curl/curl.h>
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
    GtkWidget *taskSeparator1[50];
    GtkWidget *taskSeparator2[50];
    GtkBox *boxV;
    GtkWidget *boxTask[50];
    GtkWidget *task[50];
    GtkWidget *taskPriority[50];
    GtkWidget *taskEdit[50];
    GtkWidget *taskDelete[50];
    GtkWidget *taskDeadline[50];
    GtkWidget *inputEntry;
    GtkLabel *outputLabel;
    GtkNotebook *notebook;
    GtkWidget *descriptionEntry;
    GtkWidget *inEditing;
    GtkWidget *projectNameEntry;
    GtkWidget *pageTitleBox[10];
    GtkWidget *projectTaskBox[10];
    GtkButton *calendar;
};

struct TaskProjectState {
    int taskNumber[50];
    int projectNumber[10];
    int i;
    int maxTaskPerProject;
    int maxTaskTotal;
    int maxProject;
    int projectCount;
    int repopulatedTask;
    int repopulatedProject;
    int inEditingId;
};

struct Calculator {
    int firstNumber;
    int firstB;
    int secondNumber;
    double result;
    int resultB;
    char operator;

    GtkButton *clear;
    GtkButton *plus;
    GtkButton *minus;
    GtkButton *multiply;
    GtkButton *divide;
    GtkButton *equal;
    GtkButton *zero;
    GtkButton *one;
    GtkButton *two;
    GtkButton *three;
    GtkButton *four;
    GtkButton *five;
    GtkButton *six;
    GtkButton *seven;
    GtkButton *eight;
    GtkButton *nine;
    GtkLabel *txtResult;
};

struct data {
    struct GTKTools tools;
    struct TaskProjectState state;
    struct Calculator calc;
    PGconn *conn;
};

//function.c
void changeTaskStatus(GtkWidget *taskStatus, gpointer data);
void changeTaskPriority(GtkWidget *taskPriority, gpointer data);
void editTaskWindow(GtkWidget *taskEdit, gpointer data);
void editTaskDB(GtkDialog *window, gint clicked, gpointer entry);
void addTasks(GtkWidget *task, gpointer data, int presentTask, char *projectName);
void deleteTask(GtkWidget *taskDelete, gpointer data);
void deleteProject(GtkWidget *projectDelete, gpointer data);
gchar *get_text_of_entry(GtkWidget *testEntry);
int readOneConfigValue(char *propName);
int projectExist(PGconn *conn, const gchar *name);
void addProjectWindow(GtkWidget *project, gpointer data);
void addProject(GtkWidget *projet, gint clicked, gpointer data, int presentProject);
void changeDeadlineWindow(GtkWidget *deadline, gpointer data);
void changeDeadline(GtkWidget *deadline, gint clicked, gpointer data);
void addImportantTask(gpointer data, int id);
void addMinorTask(gpointer data, int id);
void addLateTask(gpointer data, int id);
void addPlannedTask(gpointer data, int id);
void scanForIdToDestroy(gpointer data, int id);
void scanForIdToDestroySpecific(gpointer data, int idToDestroy, guint project);
void scanForIdForUpdate(gpointer data, int idToSeek);
void updateTask(gpointer data, GtkWidget *task, int id);
void curlCalendar();
void calendarDialog(GtkButton *calendar, gpointer data);
gchar *warningMessage(gpointer data);

//bdd.c
PGconn *connectBdd();
int createTables(PGconn *conn);
void bddExist(PGconn *conn, PGresult *res);
int insertTask(PGconn *conn, int id, char *name, char *description, int priority, char *deadline, int status, const gchar *projectName);
int insertProject(PGconn *conn, char *name, char *description, int priority, char *deadline, char *color);
int deleteTaskDB(PGconn *conn, int id);
int deleteProjectDB(PGconn *conn, const gchar *name);
int deleteAllTaskFromProject(PGconn *conn, const gchar *name);
int allTask(PGconn *conn);
int allProject(PGconn *conn);
int allImportantTask(PGconn *conn);
int allUrgentTask(PGconn *conn);
int allLateTask(PGconn *conn);
char *selectTask(PGconn *conn, int row);
int selectTaskId(PGconn *conn, int row);
char *selectProject(PGconn *conn, int row);
char *selectDescription(PGconn *conn, int id);
int selectPriority(PGconn *conn, int id);
int selectStatus(PGconn *conn, int id);
char *selectProjectName(PGconn *conn, int id);
char *selectDeadline(PGconn *conn, int id);
int updateDescription(PGconn *conn, const gchar *description, int id);
int updatePriority(PGconn *conn, int priority, int id);
int updateStatus(PGconn *conn, int status, int id);
int updateDeadline(PGconn *conn, int id, gchar *deadline);
int newConnectUpdate(int day, int month, int year);

//calculator.c
void btnClicked(GtkButton *button, gpointer data);
void addDigit(const char *digit, gpointer data);
void addOperator(const char *operator, gpointer data);
void calculate(gpointer data);
void clearCalc(gpointer data);
void showResult(GtkLabel *outputLabel, gpointer data);

#endif