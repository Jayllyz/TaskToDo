#include <curl/curl.h>
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <stdio.h>
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

struct Home {
    GtkBuilder *builderHome;
    GtkWidget *windowHome;
    GtkButton *openHome;
    GtkButton *clearData;
};

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
    GtkWidget *dependEntry;
    GtkWidget *inEditing;
    GtkWidget *projectNameEntry;
    GtkWidget *pageTitleBox[10];
    GtkWidget *projectTaskBox[10];
    GtkButton *calendar;
    GtkLabel *dailyCap;
    GtkLabel *monthlyCap;
    GtkLabel *dailyExpense;
    GtkLabel *monthlyExpense;
    GtkEntry *dailyCapEntry;
    GtkButton *setDaily;
    GtkEntry *monthlyCapEntry;
    GtkButton *setMonthly;
    GtkEntry *expenseEntry;
    GtkButton *setExpense;
    GtkEntry *savedEntry;
    GtkButton *desetExpense;
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
    int crlf;
};

struct Calculator {
    int firstNumber;
    int secondNumber;
    double result;
    int resultB;
    char op;

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

struct Data {
    struct GTKTools tools;
    struct TaskProjectState state;
    struct Calculator calc;
    struct Home home;
    PGconn *conn;
};

//function.c
void openApp(GtkWidget *button, struct Data *data);
void clearData(GtkWidget *button, struct Data *data);
void checkEol(struct Data *data, const char *filename);
void changeTaskStatus(GtkWidget *taskStatus, struct Data *data);
void changeTaskPriority(GtkWidget *taskPriority, struct Data *data);
void editTaskWindow(GtkWidget *taskEdit, struct Data *data);
void editTaskDB(GtkDialog *window, gint clicked, struct Data *data);
void addTasks(GtkWidget *task, struct Data *data, int presentTask, char *projectName);
void deleteTask(GtkWidget *taskDelete, struct Data *data);
void deleteProject(GtkWidget *projectDelete, struct Data *data);
gchar *get_text_of_entry(GtkWidget *testEntry);
int readOneConfigValue(char *propName);
int projectExist(PGconn *conn, const gchar *name);
void addProjectWindow(GtkWidget *project, struct Data *data);
void addProject(GtkWidget *projet, gint clicked, struct Data *data, int presentProject);
void changeDeadlineWindow(GtkWidget *deadline, struct Data *data);
void changeDeadline(GtkWidget *deadline, gint clicked, struct Data *data);
void addImportantTask(struct Data *data, int id);
void addMinorTask(struct Data *data, int id);
void addLateTask(struct Data *data, int id);
void addPlannedTask(struct Data *data, int id);
void scanForIdToDestroy(struct Data *data, int id);
void scanForIdToDestroySpecific(struct Data *data, int idToDestroy, guint project);
void scanForIdForUpdate(struct Data *data, int idToSeek);
void updateTask(struct Data *data, GtkWidget *task, int id);
void curlCalendar();
void calendarDialog(GtkButton *calendar, struct Data *data);
gchar *warningMessage(struct Data *data);
int newConnectUpdate(char *day, char *month, int year, struct Data *data);
void financeButton(GtkButton *buttonPressed, struct Data *data);
void updateFinance(struct Data *data);

//bdd.c
PGconn *connectBdd();
int createTables(PGconn *conn, struct Data *data);
void bddExist(PGconn *conn, PGresult *res);
int insertTask(PGconn *conn, int id, char *name, char *description, int priority, char *deadline, int status, int dependGroup, const gchar *projectName);
int insertProject(PGconn *conn, char *name);
int deleteTaskDB(PGconn *conn, int id);
int deleteProjectDB(PGconn *conn, const gchar *name);
int deleteAllTaskFromProject(PGconn *conn, const gchar *name);
int allTask(PGconn *conn);
int allProject(PGconn *conn);
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
int updateExpense(PGconn *conn, int typeOfExpense, int amount);
int updateCap(PGconn *conn, int typeOfCap, int amount);
int selectExpense(PGconn *conn, int typeOfExpense);
int selectCap(PGconn *conn, int typeOfCap);
int updateDependGroup(PGconn *conn, int id, int dependGroup);
int selectDependGroup(PGconn *conn, int id);
int AllDependGroup(PGconn *conn, int id, int dependGroup);
int selectIdFromDependGroup(PGconn *conn, int row, int dependGroup, char *projectName);
int refreshTaskInGroup(PGconn *conn, int id, int dependGroup);

//calculator.c
void btnClicked(GtkButton *button, struct Data *data);
void addDigit(const char *digit, struct Data *data);
void addOperator(const char *op, struct Data *data);
void calculate(struct Data *data);
void clearCalc(struct Data *data);
void showResult(GtkLabel *outputLabel, struct Data *data);

#endif