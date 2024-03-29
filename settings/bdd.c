#include "../functions.h"
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>

void bddExist(PGconn *conn, PGresult *res)
{
    g_print("%s\n", PQerrorMessage(conn));

    PQclear(res);
    PQfinish(conn);

    exit(1);
}

PGconn *connectBdd()
{
    PGconn *conn = PQconnectdb("user=project password=Respons11 dbname=TaskToDo");
    if (PQstatus(conn) == CONNECTION_BAD) {
        bddExist(conn, NULL);
    }

    return conn;
}

int createTables(PGconn *conn, struct Data *data)
{
    PGresult *res;

    res = PQexec(conn, "SET TIME ZONE 'Europe/Paris'");

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        bddExist(conn, res);

    PQclear(res);

    res = PQexec(conn, "CREATE TABLE IF NOT EXISTS Project(Name VARCHAR(20) PRIMARY KEY, Description VARCHAR(100), Priority INT, Date TIMESTAMPTZ DEFAULT NOW())");

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        bddExist(conn, res);

    PQclear(res);

    res = PQexec(conn,
        "CREATE TABLE IF NOT EXISTS Task(Id SERIAL PRIMARY KEY, Name VARCHAR(35), Description VARCHAR(100), Priority INT, Date TIMESTAMPTZ DEFAULT NOW(), Deadline "
        "TIMESTAMPTZ, "
        "Status INT NOT NULL DEFAULT 0,DependGroup INT NOT NULL DEFAULT -1, ProjectName "
        "VARCHAR(20), "
        "FOREIGN KEY (ProjectName) REFERENCES Project(Name) ON DELETE CASCADE)");

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        bddExist(conn, res);

    PQclear(res);

    res = PQexec(conn,
        "INSERT INTO Project (Name) VALUES ('Tâches'), "
        "('Importantes/Urgentes'), ('Mineures'), ('En retard'), "
        "('Prévues') , ('Finance') ON CONFLICT DO NOTHING;");

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        bddExist(conn, res);

    PQclear(res);

    res = PQexec(conn, "CREATE TABLE IF NOT EXISTS Finance(Name VARCHAR(25) PRIMARY KEY, Value INT)");

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        bddExist(conn, res);

    res = PQexec(conn,
        "INSERT INTO Finance (Name, Value) VALUES ('Dépenses journalières', 0), ('Dépenses mensuelles', 0), ('Plafond journalier', 0), ('Plafond mensuel', 0) ON "
        "CONFLICT DO NOTHING;");

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        bddExist(conn, res);

    FILE *file = fopen("settings/config.txt", "r+");
    if (file != NULL) {
        char line[200];
        while (fgets(line, sizeof(line), file)) {
            if (strstr(line, "init db") != NULL) {
                if (data->state.crlf == 1)
                    fseek(file, -3, SEEK_CUR); // -1 + '\r\n'
                else
                    fseek(file, -2, SEEK_CUR); // -1 + '\n'
                fprintf(file, "1");
                break;
            }
        }
        fclose(file);
    }
    else {
        g_print("Impossible d'ouvrir le fichier de configuration");
    }
    return 0;
}

int insertTask(PGconn *conn, int id, char *name, char *description, int priority, char *deadline, int status, int dependGroup, const gchar *projectName)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 200);
    replaceQuote(name);
    replaceQuote(description);
    snprintf(query, 200,
        "INSERT INTO Task ( Id, Name, Description, Priority, Deadline, Status, DependGroup, ProjectName) VALUES ( %d,'%s', '%s', %d, '%s', %d, %d, '%s')", id, name,
        description, priority, deadline, status, dependGroup, projectName);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int insertProject(PGconn *conn, char *name)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 100);
    replaceQuote(name);
    snprintf(query, 100, "INSERT INTO Project ( Name) VALUES ('%s')", name);
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
    replaceQuote((char *)ProjectName);
    int size = strlen("SELECT Name FROM project WHERE Name = ''") + strlen(ProjectName) + 1;
    char *query = malloc(size * sizeof(char));
    snprintf(query, size, "SELECT Name FROM project WHERE Name = '%s'", ProjectName);
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
    char *query = malloc(sizeof(char) * 100);
    snprintf(query, 100, "DELETE FROM Task WHERE id='%d'", id);
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
    char *query = malloc(sizeof(char) * 100);
    snprintf(query, 100, "DELETE FROM Project WHERE name='%s'", name);
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
    char *query = malloc(sizeof(char) * 50);
    snprintf(query, 50, "DELETE FROM Task WHERE ProjectName='%s'", name);
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
    char *query = malloc(sizeof(char) * strlen("SELECT * FROM Task") + 1);
    strcpy(query, "SELECT * FROM Task");
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get all task");
        return -1;
    }

    int amountOfTask = 0;
    if (PQntuples(res) > 0)
        amountOfTask = PQntuples(res);

    free(query);
    PQclear(res);
    return amountOfTask;
}

int maxTaskInAllProject(PGconn *conn)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * strlen("SELECT ProjectName, COUNT(*) AS Count FROM Task GROUP BY ProjectName ORDER BY Count DESC LIMIT 1") + 1);
    strcpy(query, "SELECT ProjectName, COUNT(*) AS Count FROM Task GROUP BY ProjectName ORDER BY Count DESC LIMIT 1");
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get max task in all project");
        return -1;
    }
    free(query);
    int maxTask = 0;
    if (PQntuples(res) > 0)
        maxTask = atoi(PQgetvalue(res, 0, 1));

    PQclear(res);
    return maxTask;
}

int allProject(PGconn *conn)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * strlen("SELECT * FROM Project") + 1);
    strcpy(query, "SELECT * FROM Project");
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

int allUrgentTask(PGconn *conn)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * strlen("SELECT * FROM Task WHERE priority > 2") + 1);
    strcpy(query, "SELECT * FROM Task WHERE priority > 2");
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get all urgent task");
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
    char *query = malloc(sizeof(char) * strlen("SELECT * FROM Task WHERE deadline < now()") + 1);
    strcpy(query, "SELECT * FROM Task WHERE deadline < now()");
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get all late task");
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
    char *query = malloc(sizeof(char) * 100);
    snprintf(query, 100, "SELECT name FROM Task WHERE id = %d", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get task name");
        return NULL;
    }

    char *name = PQgetvalue(res, 0, 0);

    free(query);
    PQclear(res);

    return name;
}

int selectTaskId(PGconn *conn, int row)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 100);
    snprintf(query, 100, "SELECT Id FROM Task ORDER BY date LIMIT 1 OFFSET %d", row);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get the Task id");
        return -1;
    }

    int id = atoi(PQgetvalue(res, 0, 0));

    free(query);
    PQclear(res);

    return id;
}

char *selectProject(PGconn *conn, int row)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 100);
    snprintf(query, 100, "SELECT name FROM Project ORDER BY date LIMIT 1 OFFSET %d", 6 + row);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get project name");
        return NULL;
    }

    char *name = PQgetvalue(res, 0, 0);

    free(query);
    PQclear(res);

    return name;
}

char *selectDescription(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 100);
    snprintf(query, 100, "SELECT description FROM Task WHERE id = '%d'", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get task description");
        return NULL;
    }

    char *description = PQgetvalue(res, 0, 0);

    free(query);
    PQclear(res);

    return description;
}

int selectPriority(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 100);
    snprintf(query, 100, "SELECT priority FROM Task WHERE id = '%d'", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get task priority");
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
    char *query = malloc(sizeof(char) * 100);
    snprintf(query, 100, "SELECT status FROM Task WHERE id = '%d'", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get task status");
        return -1;
    }

    int status = atoi(PQgetvalue(res, 0, 0));

    free(query);
    PQclear(res);

    return status;
}

char *selectProjectName(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 100);
    snprintf(query, 100, "SELECT ProjectName FROM Task WHERE id = '%d'", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get the task project name");
        return NULL;
    }

    char *projectName = PQgetvalue(res, 0, 0);
    free(query);
    PQclear(res);

    return projectName;
}

char *selectDeadline(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 100);
    snprintf(query, 100, "SELECT DATE(deadline) FROM Task WHERE id = '%d'", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get the task deadline");
        return NULL;
    }

    char *deadline = PQgetvalue(res, 0, 0);

    free(query);
    PQclear(res);

    return deadline;
}

int updateDescription(PGconn *conn, const gchar *description, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 150);
    replaceQuote((char *)description);
    snprintf(query, 150, "UPDATE Task SET description = '%s' WHERE id = '%d'", description, id);
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
    char *query = malloc(sizeof(char) * 150);
    snprintf(query, 150, "UPDATE Task SET priority = '%d' WHERE id = '%d'", priority, id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int updateStatus(PGconn *conn, int status, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 150);
    snprintf(query, 150, "UPDATE Task SET status = '%d' WHERE id = '%d'", status, id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);

    char *queryDepend = malloc(sizeof(char) * 150);
    snprintf(queryDepend, 150, "SELECT DependGroup FROM Task WHERE id ='%d' AND ProjectName = '%s'", id, selectProjectName(conn, id));
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        bddExist(conn, res);
    }
    free(queryDepend);

    int dependGroup = atoi(PQgetvalue(res, 0, 0));

    if (dependGroup != -1) {
        char *queryUpdateDepend = malloc(sizeof(char) * 150);
        snprintf(
            queryUpdateDepend, 150, "UPDATE Task SET status = '%d' WHERE DependGroup = '%d' AND ProjectName = '%s'", status, dependGroup, selectProjectName(conn, id));
        res = PQexec(conn, query);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            bddExist(conn, res);
        }
        free(queryUpdateDepend);
    }

    PQclear(res);
    return 0;
}

int updateDeadline(PGconn *conn, int id, gchar *deadline)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 150);
    snprintf(query, 150, "UPDATE Task SET deadline = '%s' WHERE id = '%d'", deadline, id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);

    char *queryDepend = malloc(sizeof(char) * 150);
    snprintf(queryDepend, 150, "SELECT DependGroup FROM Task WHERE id ='%d' AND ProjectName = '%s'", id, selectProjectName(conn, id));
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        bddExist(conn, res);
    }
    free(queryDepend);

    int dependGroup = atoi(PQgetvalue(res, 0, 0));

    if (dependGroup != -1) {
        char *queryUpdateDepend = malloc(sizeof(char) * 150);
        snprintf(queryUpdateDepend, 150, "UPDATE Task SET deadline = '%s' WHERE DependGroup = '%d' AND ProjectName = '%s'", deadline, dependGroup,
            selectProjectName(conn, id));
        res = PQexec(conn, query);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            bddExist(conn, res);
        }
        free(queryUpdateDepend);
    }

    PQclear(res);
    return 0;
}

int updateExpense(PGconn *conn, int typeOfExpense, int amount)
{
    PGresult *res;
    size_t size = sizeof(char) * strlen("UPDATE Finance SET Value =  WHERE name = 'Dépenses journalières'") + sizeof(amount) + 1;
    char *query = malloc(size);
    if (typeOfExpense == 2)
        snprintf(query, size, "UPDATE Finance SET Value = %d WHERE name = 'Dépenses journalières'", amount);
    else if (typeOfExpense == 3)
        snprintf(query, size, "UPDATE Finance SET Value = %d WHERE name = 'Dépenses mensuelles'", amount);

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
    size_t size = sizeof(char) * strlen("UPDATE Capital SET Value =  WHERE name = 'Capital journalier'") + sizeof(amount) + 1;
    char *query = malloc(size);

    if (typeOfCap == 0) {
        int monthlyCap = selectCap(conn, 3);
        if (amount > monthlyCap && amount != 0 && monthlyCap != 0) {
            free(query);
            return -2;
        }
    }

    if (typeOfCap == 1) {
        int dailyCap = selectCap(conn, 2);
        if (amount < dailyCap && amount != 0) {
            free(query);
            return -3;
        }
    }

    if (typeOfCap == 0)
        snprintf(query, size, "UPDATE Finance SET Value = %d WHERE name = 'Plafond journalier'", amount);
    else if (typeOfCap == 1)
        snprintf(query, size, "UPDATE Finance SET Value = %d WHERE name = 'Plafond mensuel'", amount);

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
    size_t size = sizeof(char) * strlen("SELECT value FROM Finance WHERE name = 'Dépenses journalières'") + 1;
    char *query = malloc(size);
    if (typeOfExpense == 0)
        snprintf(query, size, "SELECT value FROM Finance WHERE name = 'Dépenses journalières'");
    else if (typeOfExpense == 1)
        snprintf(query, size, "SELECT value FROM Finance WHERE name = 'Dépenses mensuelles'");

    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        bddExist(conn, res);
        return -1;
    }
    int value = atoi(PQgetvalue(res, 0, 0));

    free(query);
    PQclear(res);

    return value;
}

int selectCap(PGconn *conn, int typeOfCap)
{
    PGresult *res;
    size_t size = sizeof(char) * strlen("SELECT value FROM Finance WHERE name = 'Plafond journalier'") + 1;
    char *query = malloc(size);
    if (typeOfCap == 2)
        snprintf(query, size, "SELECT value FROM Finance WHERE name = 'Plafond journalier'");
    else if (typeOfCap == 3)
        snprintf(query, size, "SELECT value FROM Finance WHERE name = 'Plafond mensuel'");

    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        bddExist(conn, res);
        return -1;
    }

    int value = atoi(PQgetvalue(res, 0, 0));

    free(query);
    PQclear(res);

    return value;
}

int selectDependGroup(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 150);
    snprintf(query, 150, "SELECT DependGroup FROM Task WHERE id ='%d' AND ProjectName = '%s'", id, selectProjectName(conn, id));
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Erreur lors de la récupération du groupe de dépendance de la tâche");
        return -1;
    }
    free(query);

    int dependGroup = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return dependGroup;
}

int updateDependGroup(PGconn *conn, int id, int dependGroup)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 150);
    snprintf(query, 150, "UPDATE Task SET DependGroup = '%d' WHERE id = '%d' AND ProjectName = '%s'", dependGroup, id, selectProjectName(conn, id));
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int AllDependGroup(PGconn *conn, int id, int dependGroup)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 150);
    snprintf(query, 150, "SELECT * FROM Task WHERE DependGroup = '%d' AND ProjectName = '%s'", dependGroup, selectProjectName(conn, id));
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: Can't get all dependances");
        return -1;
    }

    int amountOfDependance = PQntuples(res);
    free(query);
    PQclear(res);
    return amountOfDependance;
}

int selectIdFromDependGroup(PGconn *conn, int row, int dependGroup, char *projectName)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 200);
    snprintf(query, 200, "SELECT id FROM Task WHERE DependGroup = '%d' AND ProjectName = '%s' LIMIT 1 OFFSET %d", dependGroup, projectName, row);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        bddExist(conn, res);
    }

    int id = atoi(PQgetvalue(res, 0, 0));

    free(query);
    PQclear(res);

    return id;
}

int refreshTaskInGroup(PGconn *conn, int id, int dependGroup)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 100);
    snprintf(
        query, 100, "SELECT status, deadline FROM Task WHERE DependGroup = '%d' AND ProjectName = '%s' AND id != '%d' ", dependGroup, selectProjectName(conn, id), id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Erreur lors de la récupération du groupe de dépendance de la tâche");
    }
    free(query);

    int status = atoi(PQgetvalue(res, 0, 0));
    char *deadline = PQgetvalue(res, 0, 1);

    char *queryUpdate = malloc(sizeof(char) * 150);
    snprintf(
        queryUpdate, 150, "UPDATE Task SET status = '%d', deadline = '%s' WHERE id = '%d' AND ProjectName = '%s'", status, deadline, id, selectProjectName(conn, id));
    res = PQexec(conn, queryUpdate);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }

    free(queryUpdate);
    PQclear(res);
    return 0;
}

void replaceQuote(char *str)
{
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == '\'')
            str[i] = '"';
    }
    return;
}