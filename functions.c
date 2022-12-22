#include "functions.h"
#include <gtk/gtk.h>
#include <stdio.h>

void changeTaskStatus(GtkWidget *taskStatus, gpointer data)
{
    struct data *dataP = data;

    GtkWidget *parent = gtk_widget_get_parent(taskStatus);
    GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
    GtkWidget *idButton = g_list_nth_data(children, 5);
    int id = atoi(gtk_button_get_label(GTK_BUTTON(idButton)));

    if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Non completé") == 0) {
        int queryResult = updateStatus(dataP->conn, 1, id);
        if (queryResult == -1) {
            g_print("Error: update status failed");
        }
        gtk_button_set_label(GTK_BUTTON(taskStatus), "En cours");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "En cours") == 0) {
        int queryResult = updateStatus(dataP->conn, 2, id);
        if (queryResult == -1) {
            g_print("Error: update status failed");
        }
        gtk_button_set_label(GTK_BUTTON(taskStatus), "Completé");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Completé") == 0) {
        int queryResult = updateStatus(dataP->conn, 3, id);
        if (queryResult == -1) {
            g_print("Error: update status failed");
        }
        gtk_button_set_label(GTK_BUTTON(taskStatus), "Abandonné");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Abandonné") == 0) {
        int queryResult = updateStatus(dataP->conn, 0, id);
        if (queryResult == -1) {
            g_print("Error: update status failed");
        }
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
    PGconn *conn = connectBdd();

    GtkWidget *parent = gtk_widget_get_parent(taskEdit);
    GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
    GtkWidget *taskWidget = g_list_nth_data(children, 2);
    const gchar *taskName = gtk_label_get_text(GTK_LABEL(taskWidget));

    GtkWidget *editWindow = gtk_dialog_new_with_buttons(taskName, NULL, GTK_DIALOG_MODAL, "Confirmer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *descriptionLabel = gtk_label_new("Description de la tâche");
    GtkWidget *descriptionEntry = gtk_entry_new();
    gtk_widget_set_size_request(descriptionEntry, 200, 50);

    GtkWidget *priorityButton = gtk_button_new();
    int priority = selectPriority(conn, taskName);

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
    g_signal_connect(priorityButton, "clicked", G_CALLBACK(changeTaskPriority), descriptionEntry);

    gtk_entry_set_text(GTK_ENTRY(descriptionEntry), selectDescription(conn, taskName));

    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), descriptionLabel);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), descriptionEntry);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), priorityButton);
    gtk_widget_show_all(editWindow);

    g_signal_connect(editWindow, "response", G_CALLBACK(editTaskDB), descriptionEntry);
}

void editTaskDB(GtkDialog *window, gint clicked, gpointer entry)
{
    if (clicked == GTK_RESPONSE_OK) {
        PGconn *conn = connectBdd();
        GtkWidget *input = GTK_WIDGET(entry);
        const gchar *text = gtk_entry_get_text(GTK_ENTRY(input));
        const gchar *taskName = gtk_window_get_title(GTK_WINDOW(window));
        int queryResult;

        GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(entry));
        GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
        GtkWidget *priorityButton = g_list_nth_data(children, 2);
        const gchar *setPriority = gtk_button_get_label(GTK_BUTTON(priorityButton));

        queryResult = updateDescription(conn, text, taskName);
        if (queryResult != 0) {
            g_print("Erreur de la modification de la base");
        }

        if (strcmp(setPriority, "Mineure") == 0) {
            queryResult = updatePriority(conn, 0, taskName);
        }
        else if (strcmp(setPriority, "Normale") == 0) {
            queryResult = updatePriority(conn, 1, taskName);
        }
        else if (strcmp(setPriority, "Importante") == 0) {
            queryResult = updatePriority(conn, 2, taskName);
        }
        else if (strcmp(setPriority, "Urgente") == 0) {
            queryResult = updatePriority(conn, 3, taskName);
        }

        if (queryResult != 0) {
            g_print("Erreur de la modification de la base");
        }

        gtk_widget_destroy(GTK_WIDGET(window));
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

void addTasks(GtkWidget *task, gpointer data, int presentTask)
{
    struct data *user = data;
    char *getText;
    getText = malloc(sizeof(char) * strlen(get_text_of_entry(user->inputEntry)) + 1);
    strcpy(getText, get_text_of_entry(user->inputEntry));

    if (strcmp(getText, "") == 0 && user->repopulated == 1) {
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

    if (taskExist(user->conn, getText, name) == 1 && user->repopulated == 1) {
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

<<<<<<< Updated upstream
    if (user->repopulated == 0) {
        gtk_label_set_text(GTK_LABEL(user->task[user->i]), selectTask(user->conn, presentTask));
        gtk_widget_set_tooltip_text(user->task[user->i], selectDescription(user->conn, selectTask(user->conn, presentTask)));
=======
    dataP->tools.boxTask[dataP->state.i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pageBox), dataP->tools.boxTask[dataP->state.i], FALSE, FALSE, 0);
    gtk_box_reorder_child(GTK_BOX(pageBox), dataP->tools.boxTask[dataP->state.i], numberOfTask + 2);

    if (dataP->state.repopulatedTask == 1) {
        dataP->tools.taskStatus[dataP->state.i] = gtk_button_new_with_label("Non completé");
    }
    else if (dataP->state.repopulatedTask == 0) {
        int queryResult = selectStatus(dataP->conn, presentTask);
        gchar *status;
        if (queryResult == 0) {
            status = "Non completé";
        }
        else if (queryResult == 1) {
            status = "En cours";
        }
        else if (queryResult == 2) {
            status = "Complété";
        }
        else if (queryResult == 3) {
            status = "Abandonné";
        }
        else {
            status = "Erreur";
        }
        dataP->tools.taskStatus[presentTask] = gtk_button_new_with_label(status);
    }

    gtk_widget_set_margin_top(dataP->tools.taskStatus[dataP->state.i], 10);
    gtk_widget_set_margin_bottom(dataP->tools.taskStatus[dataP->state.i], 10);
    gtk_widget_set_size_request(dataP->tools.taskStatus[dataP->state.i], 150, -1);
    gtk_box_pack_start(GTK_BOX(dataP->tools.boxTask[dataP->state.i]), dataP->tools.taskStatus[dataP->state.i], FALSE, FALSE, 0);
    g_signal_connect(dataP->tools.taskStatus[dataP->state.i], "clicked", G_CALLBACK(changeTaskStatus), dataP);

    dataP->tools.taskSeparator[dataP->state.i] = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(dataP->tools.taskSeparator[dataP->state.i], 5, -1);
    gtk_box_pack_start(GTK_BOX(dataP->tools.boxTask[dataP->state.i]), dataP->tools.taskSeparator[dataP->state.i], FALSE, FALSE, 0);

    if (dataP->state.repopulatedTask == 0) {
        gtk_label_set_text(GTK_LABEL(dataP->tools.task[dataP->state.i]), selectTask(dataP->conn, presentTask));
        gtk_widget_set_tooltip_text(dataP->tools.task[dataP->state.i], selectDescription(dataP->conn, presentTask));
>>>>>>> Stashed changes
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

    gtk_widget_show(user->boxTask[user->i]);
    gtk_widget_show(user->taskStatus[user->i]);
    gtk_widget_show(user->taskSeparator[user->i]);
    gtk_widget_show(user->task[user->i]);
    gtk_widget_show(user->taskEdit[user->i]);
    gtk_widget_show(user->taskDelete[user->i]);

    gtk_entry_set_text(GTK_ENTRY(user->inputEntry), "");
    user->unusedTaskSpace--;

    if (user->repopulated == 1) {
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
        printf("Error: taskExist failed");
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