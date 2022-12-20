#include "functions.h"
#include <gtk/gtk.h>
#include <stdio.h>

void changeTaskStatus(GtkWidget *taskStatus, gpointer data)
{
    if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Non completé") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskStatus), "En cours");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "En cours") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskStatus), "Completé");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Completé") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskStatus), "Abandonné");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Abandonné") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskStatus), "Non completé");
    }
}

void changeTaskPriority(GtkWidget *taskPriority, gpointer data)
{
    if (strcmp(gtk_button_get_label(GTK_BUTTON(taskPriority)), "Mineure") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskPriority), "Normale");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskPriority)), "Normale") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskPriority), "Importante");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskPriority)), "Importante") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskPriority), "Urgente");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskPriority)), "Urgente") == 0) {
        gtk_button_set_label(GTK_BUTTON(taskPriority), "Mineure");
    }
}

void editTaskWindow(GtkWidget *taskEdit, gpointer data)
{
    struct data *user = data;

    GtkWidget *parent = gtk_widget_get_parent(taskEdit);
    GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
    GtkWidget *taskWidget = g_list_nth_data(children, 2);
    const gchar *taskName = gtk_label_get_text(GTK_LABEL(taskWidget));
    user->inEditing = taskWidget;

    GtkWidget *editWindow = gtk_dialog_new_with_buttons(taskName, NULL, GTK_DIALOG_MODAL, "Confirmer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *descriptionLabel = gtk_label_new("Description de la tâche");
    user->descriptionEntry = gtk_entry_new();
    gtk_widget_set_size_request(user->descriptionEntry, 200, 50);

    GtkWidget *priorityButton = gtk_button_new();
    int priority = selectPriority(user->conn, taskName);

    if (priority == 0) {
        gtk_button_set_label(GTK_BUTTON(priorityButton), "Mineure");
    }
    else if (priority == 1) {
        gtk_button_set_label(GTK_BUTTON(priorityButton), "Normale");
    }
    else if (priority == 2) {
        gtk_button_set_label(GTK_BUTTON(priorityButton), "Importante");
    }
    else if (priority == 3) {
        gtk_button_set_label(GTK_BUTTON(priorityButton), "Urgente");
    }
    else {
        gtk_button_set_label(GTK_BUTTON(priorityButton), "Erreur");
    }
    g_signal_connect(priorityButton, "clicked", G_CALLBACK(changeTaskPriority), user->descriptionEntry);

    gtk_entry_set_text(GTK_ENTRY(user->descriptionEntry), selectDescription(user->conn, taskName));

    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), descriptionLabel);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), user->descriptionEntry);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), priorityButton);
    gtk_widget_show_all(editWindow);

    g_signal_connect(editWindow, "response", G_CALLBACK(editTaskDB), user);

    gtk_widget_set_tooltip_text(taskWidget, selectDescription(user->conn, taskName));
}

void editTaskDB(GtkDialog *window, gint clicked, gpointer data)
{
    struct data *user = data;

    if (clicked == GTK_RESPONSE_OK) {
        GtkWidget *input = GTK_WIDGET(user->descriptionEntry);
        const gchar *text = gtk_entry_get_text(GTK_ENTRY(input));
        const gchar *taskName = gtk_window_get_title(GTK_WINDOW(window));
        int queryResult;

        GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(user->descriptionEntry));
        GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
        GtkWidget *priorityButton = g_list_nth_data(children, 2);
        const gchar *setPriority = gtk_button_get_label(GTK_BUTTON(priorityButton));

        queryResult = updateDescription(user->conn, text, taskName);
        if (queryResult != 0) {
            g_print("Erreur de la modification de la base");
            return;
        }

        if (strcmp(setPriority, "Mineure") == 0) {
            queryResult = updatePriority(user->conn, 0, taskName);
        }
        else if (strcmp(setPriority, "Normale") == 0) {
            queryResult = updatePriority(user->conn, 1, taskName);
        }
        else if (strcmp(setPriority, "Importante") == 0) {
            queryResult = updatePriority(user->conn, 2, taskName);
        }
        else if (strcmp(setPriority, "Urgente") == 0) {
            queryResult = updatePriority(user->conn, 3, taskName);
        }

        if (queryResult != 0) {
            g_print("Erreur de la modification de la base");
            return;
        }

        gtk_widget_set_tooltip_text(user->inEditing, selectDescription(user->conn, taskName));
    }
    gtk_widget_destroy(GTK_WIDGET(window));
}

void deleteTask(GtkWidget *taskDelete, gpointer data)
{
    struct data *user = data;
    GtkWidget *taskToDelete = gtk_widget_get_parent(taskDelete);
    GList *boxChildren = gtk_container_get_children(GTK_CONTAINER(taskToDelete));
    GtkWidget *numberToFree = g_list_nth_data(boxChildren, 5);
    GtkWidget *labelOfTask = g_list_nth_data(boxChildren, 2);
    g_list_free(boxChildren);

    int numberToChange = atoi(gtk_button_get_label(GTK_BUTTON(numberToFree)));
    user->taskNumber[numberToChange] = numberToChange;

    const gchar *nameOfTask = gtk_label_get_text(GTK_LABEL(labelOfTask));
    int queryResult = deleteTaskDB(user->conn, nameOfTask);

    if (queryResult == -1) {
        g_print("Error: insertTask failed");
        return;
    }

    gtk_widget_destroy(taskToDelete);
    user->task[numberToChange] = gtk_label_new("");
    user->unusedTaskSpace++;
}

void deleteProject(GtkWidget *projectDelete, gpointer data)
{
    struct data *user = data;

    GtkWidget *projectBox = gtk_widget_get_parent(projectDelete);
    GList *boxChildren = gtk_container_get_children(GTK_CONTAINER(projectBox));
    GtkWidget *child = g_list_nth_data(boxChildren, 0);
    GtkWidget *numberToFree = g_list_nth_data(boxChildren, 2);

    int numberToChange = atoi(gtk_button_get_label(GTK_BUTTON(numberToFree)));
    user->projectNumber[numberToChange] = numberToChange;

    gint totalPage = gtk_notebook_get_n_pages(GTK_NOTEBOOK(user->notebook));
    const gchar *nameOfProject = gtk_label_get_text(GTK_LABEL(child));
    int numberToDelete;

    for (int i = 0; i < totalPage; i++) {
        GtkWidget *curPage = gtk_notebook_get_nth_page(GTK_NOTEBOOK(user->notebook), i);
        GtkWidget *boxPage = gtk_notebook_get_tab_label(GTK_NOTEBOOK(user->notebook), curPage);
        if (GTK_IS_BOX(boxPage)) {
            boxChildren = gtk_container_get_children(GTK_CONTAINER(boxPage));
            const gchar *nameToSeek = gtk_label_get_text(g_list_nth_data(boxChildren, 0));
            if (g_strcmp0(nameToSeek, nameOfProject) == 0) {
                numberToDelete = i;
            }
        }
    }

    g_list_free(boxChildren);

    int queryResult = deleteProjectDB(user->conn, nameOfProject);
    if (queryResult == -1) {
        g_print("Error: deleteProject failed");
        return;
    }

    gtk_notebook_remove_page(GTK_NOTEBOOK(user->notebook), numberToDelete);
    gtk_notebook_set_current_page(user->notebook, 0);
    user->projectCount--;
}

void addTasks(GtkWidget *task, gpointer data, int presentTask)
{
    struct data *user = data;
    char *getText;
    getText = malloc(sizeof(char) * strlen(get_text_of_entry(user->inputEntry)) + 1);
    strcpy(getText, get_text_of_entry(user->inputEntry));

    if (strcmp(getText, "") == 0 && user->repopulatedTask == 1) {
        return;
    }

    if (user->unusedTaskSpace <= 0) {
        return;
    }

    for (user->i = 0; user->i < user->maxTask; user->i++) {
        if (user->taskNumber[user->i] != -1) {
            user->taskNumber[user->i] = -1;
            break;
        }
    }

    int currentPos = gtk_notebook_get_current_page(user->notebook); //recupere la position de l'onglet actif
    GtkWidget *child = gtk_notebook_get_nth_page(user->notebook, currentPos); //recupere le widget de l'onglet actif
    const gchar *name = gtk_notebook_get_tab_label_text(user->notebook, child); //recupere le nom de l'onglet actif

    if (taskExist(user->conn, getText, name) == 1 && user->repopulatedTask == 1) {
        GtkDialog *dialog = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(user->window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Cette tâche existe déjà"));
        gtk_dialog_run(dialog);
        gtk_widget_destroy(GTK_WIDGET(dialog));
        return;
    }

    user->boxTask[user->i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(user->boxV, user->boxTask[user->i], FALSE, FALSE, 0);
    gtk_box_reorder_child(user->boxV, user->boxTask[user->i], user->maxTask - user->unusedTaskSpace + 2);

    user->taskStatus[user->i] = gtk_button_new_with_label("Non completé");
    gtk_widget_set_margin_top(user->taskStatus[user->i], 10);
    gtk_widget_set_margin_bottom(user->taskStatus[user->i], 10);
    gtk_widget_set_size_request(user->taskStatus[user->i], 150, -1);
    gtk_box_pack_start(GTK_BOX(user->boxTask[user->i]), user->taskStatus[user->i], FALSE, FALSE, 0);
    g_signal_connect(user->taskStatus[user->i], "clicked", G_CALLBACK(changeTaskStatus), user);

    user->taskSeparator[user->i] = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(user->taskSeparator[user->i], 5, -1);
    gtk_box_pack_start(GTK_BOX(user->boxTask[user->i]), user->taskSeparator[user->i], FALSE, FALSE, 0);

    if (user->repopulatedTask == 0) {
        gtk_label_set_text(GTK_LABEL(user->task[user->i]), selectTask(user->conn, presentTask));
        gtk_widget_set_tooltip_text(user->task[user->i], selectDescription(user->conn, selectTask(user->conn, presentTask)));
    }
    else {
        gtk_label_set_text(GTK_LABEL(user->task[user->i]), getText);
        gtk_widget_set_tooltip_text(user->task[user->i], "");
    }
    gtk_box_pack_start(GTK_BOX(user->boxTask[user->i]), user->task[user->i], TRUE, FALSE, 0);

    user->taskEdit[user->i] = gtk_button_new_with_label("Editer");
    gtk_box_pack_start(GTK_BOX(user->boxTask[user->i]), user->taskEdit[user->i], FALSE, FALSE, 0);
    g_signal_connect(user->taskEdit[user->i], "clicked", G_CALLBACK(editTaskWindow), user);

    user->taskDelete[user->i] = gtk_button_new_with_label("X");
    gtk_box_pack_start(GTK_BOX(user->boxTask[user->i]), user->taskDelete[user->i], FALSE, FALSE, 0);
    g_signal_connect(user->taskDelete[user->i], "clicked", G_CALLBACK(deleteTask), user);

    char numberToTransfer[3];
    sprintf(numberToTransfer, "%d", user->i);
    user->taskNumberMarker[user->i] = gtk_button_new_with_label(numberToTransfer);
    gtk_box_pack_start(GTK_BOX(user->boxTask[user->i]), user->taskNumberMarker[user->i], FALSE, FALSE, 0);

    gtk_widget_show_all(user->boxTask[user->i]);
    gtk_widget_hide(user->taskNumberMarker[user->i]);

    gtk_entry_set_text(GTK_ENTRY(user->inputEntry), "");
    user->unusedTaskSpace--;

    if (user->repopulatedTask == 1) {
        int queryResult = insertTask(user->conn, getText, "", 1, "now()", 0, name); //insert in db
        if (queryResult == -1) {
            printf("Error: insertTask failed");
        }
    }
}

char *get_text_of_entry(GtkWidget *inputEntry) //recup le contenu d'un "textview"
{
    GtkEntryBuffer *buffer = gtk_entry_get_buffer((GtkEntry *)inputEntry);
    gchar *text;
    text = gtk_entry_buffer_get_text(buffer);
    return text;
}

int readOneConfigValue(char *propName)
{
    if (propName == NULL) {
        printf("Error: propName is null");
        return -1;
    }
    FILE *file = fopen("settings/config.txt", "r");
    if (file == NULL) {
        printf("Error: config file not found");
        return -1;
    }
    char *line = NULL;
    size_t len = 0;
    while ((getline(&line, &len, file)) != -1) {
        if (strstr(line, propName) != NULL) {
            int i = 0;
            while (line[i] != ':') {
                i++;
            }
            return atoi(&line[i + 1]);
        }
    }
    return -1;
}

int taskExist(PGconn *conn, char *input, const gchar *ProjectName)
{
    int size = strlen("SELECT Name FROM task WHERE ProjectName = ''") + strlen(ProjectName) + 1;
    char *query = malloc(size * sizeof(char));
    sprintf(query, "SELECT Name FROM task WHERE ProjectName = '%s'", ProjectName);
    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        g_print("Error: taskExist failed");
        return -1;
    }
    int nbTuples = PQntuples(res);
    for (int i = 0; i < nbTuples; i++) {
        if (strcmp(PQgetvalue(res, i, 0), input) == 0) {
            return 1; //Une tache avec le meme nom existe deja dans le projet
        }
    }
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
        if (strcmp(PQgetvalue(res, i, 0), ProjectName) == 0) {
            return 1;
        }
    }
    return 0;
}

void addProjectWindow(GtkWidget *project, gpointer data)
{
    struct data *user = data;

    if (user->projectCount >= user->maxProject) {
        return;
    }

    GtkWidget *addProjectDialog
        = gtk_dialog_new_with_buttons("Nouveau projet", NULL, GTK_DIALOG_MODAL, "Confirmer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *nameLabel = gtk_label_new("Nom du projet");
    user->projectNameEntry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(addProjectDialog))), nameLabel);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(addProjectDialog))), user->projectNameEntry);
    gtk_widget_show_all(addProjectDialog);

    g_signal_connect(addProjectDialog, "response", G_CALLBACK(addProject), user);
}

void addProject(GtkWidget *projet, gint clicked, gpointer data, int presentProject)
{
    struct data *user = data;
    if (clicked == GTK_RESPONSE_OK) {
        char *projectName;
        if (user->repopulatedProject == 1) {
            projectName = get_text_of_entry(user->projectNameEntry);
            if (projectExist(user->conn, projectName) == 1 || projectName == NULL) {
                gtk_widget_destroy(projet);
                GtkDialog *dialog
                    = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(user->window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Ce projet existe déjà"));
                gtk_dialog_run(dialog);
                gtk_widget_destroy(GTK_WIDGET(dialog));
                return;
            }

            char *query = malloc((strlen("INSERT INTO project VALUES ('','Placeholder', 0, 'now()', 'now()', 0)") + strlen(projectName) + 1) * sizeof(char));
            sprintf(query, "INSERT INTO project VALUES ('%s','Placeholder', 0, 'now()', 'now()', 0)", projectName);
            PGresult *res = PQexec(user->conn, query);
            if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                g_print("Error: addProject failed");
                return;
            }
            free(query);
            PQclear(res);
        }

        for (user->i = 0; user->i < user->maxProject; user->i++) {
            if (user->projectNumber[user->i] != -1) {
                user->projectNumber[user->i] = -1;
                break;
            }
        }

        user->pageTitleBox[user->i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        GtkWidget *title = gtk_label_new("");
        if (user->repopulatedProject == 1) {
            gtk_label_set_text(GTK_LABEL(title), (const gchar *)projectName);
        }
        else if (user->repopulatedProject == 0) {
            gtk_label_set_text(GTK_LABEL(title), selectProject(user->conn, presentProject));
        }

        GtkWidget *titleButton = gtk_button_new_with_label("X");
        char numberToTransfer[3];
        sprintf(numberToTransfer, "%d", user->i);
        GtkWidget *projectNumberMarker = gtk_button_new_with_label(numberToTransfer);

        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_pack_start(GTK_BOX(user->pageTitleBox[user->i]), title, TRUE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(user->pageTitleBox[user->i]), titleButton, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(user->pageTitleBox[user->i]), projectNumberMarker, FALSE, FALSE, 0);
        gtk_widget_show_all(GTK_WIDGET(user->pageTitleBox[user->i]));
        gtk_widget_hide(projectNumberMarker);

        gint numberOfPage = gtk_notebook_get_n_pages(GTK_NOTEBOOK(user->notebook));

        g_signal_connect(titleButton, "clicked", G_CALLBACK(deleteProject), user);

        gtk_notebook_insert_page(GTK_NOTEBOOK(user->notebook), box, GTK_WIDGET(user->pageTitleBox[user->i]), numberOfPage - 1);
        gtk_widget_show(GTK_WIDGET(user->pageTitleBox[user->i]));
        gtk_widget_show(box);
        user->projectCount++;
    }
    if (user->repopulatedProject == 1) {
        gtk_widget_destroy(projet);
    }
}