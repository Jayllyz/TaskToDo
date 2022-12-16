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
        "CREATE TABLE IF NOT EXISTS Project(Id SERIAL PRIMARY KEY,"
        "Name VARCHAR(20), Description VARCHAR(100), Priority INT, Date TIMESTAMPTZ DEFAULT NOW(), Deadline TIMESTAMPTZ, Color VARCHAR(20) )");

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        bddExist(conn, res);
    }

    PQclear(res);

    res = PQexec(conn,
        "CREATE TABLE IF NOT EXISTS Task(Id SERIAL PRIMARY KEY,"
        "Name VARCHAR(20), Description VARCHAR(100), Priority INT, Date TIMESTAMPTZ DEFAULT NOW(), Deadline TIMESTAMPTZ, Status INT NOT NULL DEFAULT 0, ProjectId "
        "SERIAL, "
        "FOREIGN KEY (ProjectId) REFERENCES Project(id) ON DELETE CASCADE)");

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

int insertTask(PGconn *conn, char *name, char *description, int priority, char *deadline, int status, int projectId)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "INSERT INTO Task ( Name, Description, Priority, Deadline, Status, ProjectId) VALUES ('%s', '%s', %d, '%s', %d, '%d')", name, description, priority,
        deadline, status, projectId);
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

int getProjectId(PGconn *conn, const gchar *name)
{
    PGresult *res;
    char *query = malloc(sizeof(char) * 1000);
    sprintf(query, "SELECT id FROM Project WHERE name = '%s'", name);
    res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        printf("No data retrieved\n");
        PQclear(res);
        return -1;
    }
    int rows = PQntuples(res);

    for (int i = 0; i < rows; i++) {
        int id = atoi(PQgetvalue(res, i, 0));
        free(query);
        PQclear(res);
        return id;
    }
}