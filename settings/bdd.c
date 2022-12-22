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
        "Status INT NOT NULL DEFAULT 0, ProjectName "
        "VARCHAR(20), "
        "FOREIGN KEY (ProjectName) REFERENCES Project(Name) ON DELETE CASCADE)");

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
    }

    PQclear(res);

    res = PQexec(conn,
        "INSERT INTO Project (Name, Description, Priority, Date, Deadline, Color) VALUES ('Ma journée', 'placeholder', 0, 'now()', 'now()', 'black'), ('Important', "
        "'placeholder', 0, 'now()', 'now()', 'red'), ('Prévu', 'placeholder', 0, 'now()', 'now()', 'blue'), ('Tâches', 'placeholder', 0, 'now()', 'now()', "
        "'green'), ('Projets', 'placeholder', 0, 'now()', 'now()', 'grey') , ('Finance', 'placeholder', 0, 'now()', 'now()', 'orange')");

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
    }

    PQclear(res);

    FILE *file = fopen("settings/config.txt", "r+");
    char *line = NULL;
    size_t len = 0;
    int i = 0;
    while ((getline(&line, &len, file)) != -1) {
        if (i == 1) {
            fseek(file, -1, SEEK_CUR);
            fprintf(file, "1");
            break;
        }
        ++i;
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

char *selectTask(PGconn *conn, int id)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT name FROM Task WHERE id = %d", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return "Error: Can't get the task";
    }

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
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
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
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT name FROM Project ORDER BY date LIMIT 1 OFFSET %d", 6 + row);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return "Error: Can't get the Project";
    }

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
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return "Error: Can't get the task";
    }

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
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
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
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT projectName FROM Task WHERE id = '%d'", id);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return "Error: Can't get the task";
    }

    char *projectName = PQgetvalue(res, 0, 0);

    free(query);
    PQclear(res);

    return projectName;
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

int updateStatus(PGconn *conn, int status, int id)
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
    PQclear(res);
    return 0;
}