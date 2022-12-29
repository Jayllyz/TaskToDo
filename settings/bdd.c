#include "../functions.h"
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>

void bddExist(PGconn *conn, PGresult *res)
{
    fprintf(stderr, "%s\n", PQerrorMessage(conn));

    PQclear(res);
    PQfinish(conn);

    exit(1);
}

PGconn *connectBdd()
{
    PGconn *conn = PQconnectdb("user=projet password=Respons11 dbname=projet-todolist");
    if (PQstatus(conn) == CONNECTION_BAD) {

        return NULL;
    }
    return conn;
}

int createTables(PGconn *conn)
{

    PGresult *res;

    res = PQexec(conn, "SET TIME ZONE 'Europe/Paris'");

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
    }

    PQclear(res);

    res = PQexec(conn,
        "CREATE TABLE IF NOT EXISTS Project(Name VARCHAR(20) PRIMARY KEY, Description VARCHAR(100), Priority INT, Date TIMESTAMPTZ DEFAULT NOW(), Deadline TIMESTAMPTZ, "
        "Color VARCHAR(20))");

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
    }

    PQclear(res);

    res = PQexec(conn,
        "CREATE TABLE IF NOT EXISTS Task(Id SERIAL PRIMARY KEY, Name VARCHAR(20), Description VARCHAR(100), Priority INT, Date TIMESTAMPTZ DEFAULT NOW(), Deadline "
        "TIMESTAMPTZ, "
        "Status INT NOT NULL DEFAULT 0,DependGroup INT NOT NULL DEFAULT 0, ProjectName "
        "VARCHAR(20), "
        "FOREIGN KEY (ProjectName) REFERENCES Project(Name) ON DELETE CASCADE)");

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
    }

    PQclear(res);

    res = PQexec(conn,
        "INSERT INTO Project (Name, Description, Priority, Date, Deadline, Color) VALUES ('Tâches', 'placeholder', 0, 'now()', 'now()', 'black'), "
        "('Importantes/Urgentes', "
        "'placeholder', 0, 'now()', 'now()', 'red'), ('Mineures', 'placeholder', 0, 'now()', 'now()', 'blue'), ('En retard', 'placeholder', 0, 'now()', 'now()', "
        "'green'), ('Prévues', 'placeholder', 0, 'now()', 'now()', 'grey') , ('Finance', 'placeholder', 0, 'now()', 'now()', 'orange')");

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
    }

    PQclear(res);

    res = PQexec(conn, "CREATE TABLE IF NOT EXISTS Finance(Name VARCHAR(25) PRIMARY KEY, Value INT)");

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
    }

    res = PQexec(
        conn, "INSERT INTO Finance (Name, Value) VALUES ('Dépenses journalières', 0), ('Dépenses mensuelles', 0), ('Plafond journalier', 0), ('Plafond mensuel', 0)");

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
    }

    FILE *file = fopen("settings/config.txt", "r+");
    char *line = NULL;
    size_t len = 0;
    while ((getline(&line, &len, file)) != -1) {
        if (strstr(line, "init value") != NULL) {
            fseek(file, -3, SEEK_CUR); // -1 + '\n'
            fprintf(file, "1");
            break;
        }
    }
    fclose(file);
    return 0;
}

int insertTask(PGconn *conn, int id, char *name, char *description, int priority, char *deadline, int status, const gchar *projectName)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "INSERT INTO Task ( Id, Name, Description, Priority, Deadline, Status, ProjectName) VALUES ( %d,'%s', '%s', %d, '%s', %d, '%s')", id, name,
        description, priority, deadline, status, projectName);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int insertProject(PGconn *conn, char *name, char *description, int priority, char *deadline, char *color)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(
        query, "INSERT INTO Project ( Name, Description, Priority, Deadline, Color) VALUES ('%s', '%s', %d, '%s', '%s')", name, description, priority, deadline, color);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int projectExist(PGconn *conn, const gchar *ProjectName)
{
    int size = strlen("SELECT Name FROM project WHERE Name = ''") + strlen(ProjectName) + 1;
    char *query = malloc(size * sizeof(char));
    sprintf(query, "SELECT Name FROM project WHERE Name = '%s'", ProjectName);
    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error:  projectExist failed");
        return -1;
    }
    int nbTuples = PQntuples(res);
    for (int i = 0; i < nbTuples; i++) {
        if (strcmp(PQgetvalue(res, i, 0), ProjectName) == 0)
            return 1;
    }
    return 0;
}

int deleteTaskDB(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "DELETE FROM Task WHERE id='%d'", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int deleteProjectDB(PGconn *conn, const gchar *name)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "DELETE FROM Project WHERE name='%s'", name);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int deleteAllTaskFromProject(PGconn *conn, const gchar *name)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "DELETE FROM Task WHERE ProjectName='%s'", name);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int allTask(PGconn *conn)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * strlen("SELECT * FROM Task"));
    sprintf(query, "SELECT * FROM Task");
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get all task");
        return -1;
    }

    int amountOfTask = PQntuples(res);
    free(query);
    PQclear(res);
    return amountOfTask;
}

int allProject(PGconn *conn)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * strlen("SELECT * FROM Project"));
    sprintf(query, "SELECT * FROM Project");
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get all projects");
        return -1;
    }

    int amountOfProject = PQntuples(res);
    free(query);
    PQclear(res);
    return amountOfProject - 6;
}

int allImportantTask(PGconn *conn)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * strlen("SELECT * FROM Task WHERE priority > 1"));
    sprintf(query, "SELECT * FROM Task WHERE priority > 1");
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get all task");
        return -1;
    }

    int amountOfTask = PQntuples(res);
    free(query);
    PQclear(res);
    return amountOfTask;
}

int allUrgentTask(PGconn *conn)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * strlen("SELECT * FROM Task WHERE priority > 2"));
    sprintf(query, "SELECT * FROM Task WHERE priority > 2");
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get all task");
        return -1;
    }

    int amountOfTask = PQntuples(res);
    free(query);
    PQclear(res);
    return amountOfTask;
}

int allLateTask(PGconn *conn)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * strlen("SELECT * FROM Task WHERE deadline < now()"));
    sprintf(query, "SELECT * FROM Task WHERE deadline < now()");
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get all task");
        return -1;
    }

    int amountOfTask = PQntuples(res);
    free(query);
    PQclear(res);
    return amountOfTask;
}

char *selectTask(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT name FROM Task WHERE id = %d", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        return "Error: Can't get the task";

    char *name = PQgetvalue(res, 0, 0);

    free(query);
    PQclear(res);

    return name;
}

int selectTaskId(PGconn *conn, int row)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT Id FROM Task ORDER BY date LIMIT 1 OFFSET %d", row);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        return -1;

    int id = atoi(PQgetvalue(res, 0, 0));

    free(query);
    PQclear(res);

    return id;
}

char *selectProject(PGconn *conn, int row)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT name FROM Project ORDER BY date LIMIT 1 OFFSET %d", 6 + row);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        return "Error: Can't get the Project";

    char *name = PQgetvalue(res, 0, 0);

    free(query);
    PQclear(res);

    return name;
}

char *selectDescription(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT description FROM Task WHERE id = '%d'", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        return "Error: Can't get the task";

    char *description = PQgetvalue(res, 0, 0);

    free(query);
    PQclear(res);

    return description;
}

int selectPriority(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT priority FROM Task WHERE id = '%d'", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return -1;
    }

    int priority = atoi(PQgetvalue(res, 0, 0));

    free(query);
    PQclear(res);

    return priority;
}

int selectStatus(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT status FROM Task WHERE id = '%d'", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        return -1;

    int status = atoi(PQgetvalue(res, 0, 0));

    free(query);
    PQclear(res);

    return status;
}

char *selectProjectName(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT projectName FROM Task WHERE id = '%d'", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        return "Error: Can't get the task";

    char *projectName = PQgetvalue(res, 0, 0);

    free(query);
    PQclear(res);

    return projectName;
}

char *selectDeadline(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT DATE(deadline) FROM Task WHERE id = '%d'", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        return "Error: Can't get the task";

    char *deadline = PQgetvalue(res, 0, 0);

    free(query);
    PQclear(res);

    return deadline;
}

int updateDescription(PGconn *conn, const gchar *description, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "UPDATE Task SET description = '%s' WHERE id = '%d'", description, id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int updatePriority(PGconn *conn, int priority, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "UPDATE Task SET priority = '%d' WHERE id = '%d'", priority, id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int updateStatus(PGconn *conn, int status, int id, gpointer data)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "UPDATE Task SET status = '%d' WHERE id = '%d'", status, id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);

    char *queryDepend = malloc(sizeof(char) * 1000);
    sprintf(queryDepend, "SELECT DependGroup FROM Task WHERE id ='%d' AND ProjectName = '%s'", id, selectProjectName(conn, id));
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return -1;
    }
    free(queryDepend);

    int dependGroup = atoi(PQgetvalue(res, 0, 0));
    if (dependGroup != 0) { // Si la tâche a un groupe de dépendance on met à jour le status de toutes les tâches du groupe
        char *queryUpdateDepend = malloc(sizeof(char) * 1000);
        sprintf(queryUpdateDepend, "UPDATE Task SET status = '%d' WHERE DependGroup = '%d' AND ProjectName = '%s'", status, dependGroup, selectProjectName(conn, id));
        res = PQexec(conn, query);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            return -1;
        }
        free(queryUpdateDepend);
    }
    selectAllTaskInGroup(conn, dependGroup, data);
    PQclear(res);
    return 0;
}

int updateDeadline(PGconn *conn, int id, gchar *deadline, gpointer data)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "UPDATE Task SET deadline = '%s' WHERE id = '%d'", deadline, id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);

    char *queryDepend = malloc(sizeof(char) * 1000);
    sprintf(queryDepend, "SELECT DependGroup FROM Task WHERE id ='%d' AND ProjectName = '%s'", id, selectProjectName(conn, id));
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return -1;
    }
    free(queryDepend);

    int dependGroup = atoi(PQgetvalue(res, 0, 0));
    if (dependGroup != 0) { // Si la tâche a un groupe de dépendance on met à jour le status de toutes les tâches du groupe
        char *queryUpdateDepend = malloc(sizeof(char) * 1000);
        sprintf(queryUpdateDepend, "UPDATE Task SET deadline = '%s' WHERE DependGroup = '%d' AND ProjectName = '%s'", deadline, dependGroup, selectProjectName(conn, id));
        res = PQexec(conn, query);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            return -1;
        }
        free(queryUpdateDepend);
    }

    selectAllTaskInGroup(conn, dependGroup, data);
    PQclear(res);
    return 0;
}

int updateExpense(PGconn *conn, int typeOfExpense, int amount)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    if (typeOfExpense == 2)
        sprintf(query, "UPDATE Finance SET Value = %d WHERE name = 'Dépenses journalières'", amount);
    else if (typeOfExpense == 3)
        sprintf(query, "UPDATE Finance SET Value = %d WHERE name = 'Dépenses mensuelles'", amount);

    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int updateCap(PGconn *conn, int typeOfCap, int amount)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    if (typeOfCap == 0)
        sprintf(query, "UPDATE Finance SET Value = %d WHERE name = 'Plafond journalier'", amount);
    else if (typeOfCap == 1)
        sprintf(query, "UPDATE Finance SET Value = %d WHERE name = 'Plafond mensuel'", amount);

    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int selectExpense(PGconn *conn, int typeOfExpense)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    if (typeOfExpense == 0)
        sprintf(query, "SELECT value FROM Finance WHERE name = 'Dépenses journalières'");
    else if (typeOfExpense == 1)
        sprintf(query, "SELECT value FROM Finance WHERE name = 'Dépenses mensuelles'");

    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        return -1;

    int value = atoi(PQgetvalue(res, 0, 0));

    free(query);
    PQclear(res);

    return value;
}

int selectCap(PGconn *conn, int typeOfCap)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    if (typeOfCap == 2)
        sprintf(query, "SELECT value FROM Finance WHERE name = 'Plafond journalier'");
    else if (typeOfCap == 3)
        sprintf(query, "SELECT value FROM Finance WHERE name = 'Plafond mensuel'");

    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        return -1;

    int value = atoi(PQgetvalue(res, 0, 0));

    free(query);
    PQclear(res);

    return value;
}

char *selectDependGroup(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT DependGroup FROM Task WHERE id ='%d' AND ProjectName = '%s'", id, selectProjectName(conn, id));
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Erreur lors de la récupération du groupe de dépendance de la tâche");
    }
    free(query);

    char *dependGroup = PQgetvalue(res, 0, 0);
    PQclear(res);
    return dependGroup;
}

int updateDependGroup(PGconn *conn, int id, int dependGroup)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "UPDATE Task SET DependGroup = '%d' WHERE id = '%d' AND ProjectName = '%s'", dependGroup, id, selectProjectName(conn, id));
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int selectAllTaskInGroup(PGconn *conn, int dependGroup, gpointer data)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT id FROM Task WHERE DependGroup = '%d' AND ProjectName = '%s'", dependGroup, selectProjectName(conn, dependGroup));
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Erreur lors de la récupération du groupe de dépendance de la tâche");
    }
    free(query);

    //update de toutes les taches graphiquement
    for (int i = 0; i < PQntuples(res); i++) {
        int id = atoi(PQgetvalue(res, i, 0));
        scanForIdForUpdate(data, id);
    }

    int nbTas = PQntuples(res);
    PQclear(res);
    return nbTas;
}

int refreshTaskInGroup(PGconn *conn, int id, int dependGroup)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT status, deadline FROM Task WHERE DependGroup = '%d' AND ProjectName = '%s' AND id != '%d' ", dependGroup, selectProjectName(conn, id), id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Erreur lors de la récupération du groupe de dépendance de la tâche");
    }
    free(query);

    int status = atoi(PQgetvalue(res, 0, 0));
    char *deadline = PQgetvalue(res, 0, 1);

    char *queryUpdate = malloc(sizeof(char) * 1000);
    sprintf(queryUpdate, "UPDATE Task SET status = '%d', deadline = '%s' WHERE id = '%d' AND ProjectName = '%s'", status, deadline, id, selectProjectName(conn, id));
    res = PQexec(conn, queryUpdate);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(queryUpdate);
    PQclear(res);
    return 0;
}