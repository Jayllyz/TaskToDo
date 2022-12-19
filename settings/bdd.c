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
        "CREATE TABLE IF NOT EXISTS Task(Name VARCHAR(20) PRIMARY KEY, Description VARCHAR(100), Priority INT, Date TIMESTAMPTZ DEFAULT NOW(), Deadline TIMESTAMPTZ, "
        "Status INT NOT NULL DEFAULT 0, ProjectName "
        "VARCHAR(20), "
        "FOREIGN KEY (ProjectName) REFERENCES Project(Name) ON DELETE CASCADE)");

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

int insertTask(PGconn *conn, char *name, char *description, int priority, char *deadline, int status, const gchar *projectName)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "INSERT INTO Task ( Name, Description, Priority, Deadline, Status, ProjectName) VALUES ('%s', '%s', %d, '%s', %d, '%s')", name, description, priority,
        deadline, status, projectName);
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

int deleteTaskDB(PGconn *conn, const gchar *name)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "DELETE FROM Task WHERE name='%s'", name);
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
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT * FROM Task");
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        printf("Error: Can't get all task");
        return -1;
    }

    int amountOfTask = PQntuples(res);

    free(query);
    PQclear(res);
    return amountOfTask;
}

char *selectTask(PGconn *conn, int row)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT name FROM Task ORDER BY date LIMIT 1 OFFSET %d", row);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return "Error: Can't get the task";
    }

    char *name = PQgetvalue(res, 0, 0);

    free(query);
    PQclear(res);

    return name;
}

char *selectDescription(PGconn *conn, const gchar *name)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT description FROM Task WHERE name = '%s'", name);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return "Error: Can't get the task";
    }

    char *description = PQgetvalue(res, 0, 0);

    free(query);
    PQclear(res);

    return description;
}

int selectPriority(PGconn *conn, const gchar *name)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT priority FROM Task WHERE name = '%s'", name);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        return -1;
    }

    int priority = atoi(PQgetvalue(res, 0, 0));

    free(query);
    PQclear(res);

    return priority;
}

int updateDescription(PGconn *conn, const gchar *description, const gchar *name)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "UPDATE Task SET description = '%s' WHERE name = '%s'", description, name);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}

int updatePriority(PGconn *conn, int priority, const gchar *name)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "UPDATE Task SET priority = '%d' WHERE name = '%s'", priority, name);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
        return -1;
    }
    free(query);
    PQclear(res);
    return 0;
}