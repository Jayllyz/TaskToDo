#include "functions.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
        if (queryResult == -1)
            g_print("Error: update status failed");

        gtk_button_set_label(GTK_BUTTON(taskStatus), "Completé");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Completé") == 0) {
        int queryResult = updateStatus(dataP->conn, 3, id);
        if (queryResult == -1)
            g_print("Error: update status failed");

        gtk_button_set_label(GTK_BUTTON(taskStatus), "Abandonné");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Abandonné") == 0) {
        int queryResult = updateStatus(dataP->conn, 0, id);
        if (queryResult == -1)
            g_print("Error: update status failed");

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
    struct data *dataP = data;

    GtkWidget *parent = gtk_widget_get_parent(taskEdit);
    GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
    GtkWidget *taskWidget = g_list_nth_data(children, 2);
    const gchar *taskName = gtk_label_get_text(GTK_LABEL(taskWidget));
    dataP->tools.inEditing = taskWidget;

    GtkWidget *idWidget = g_list_nth_data(children, 5);
    int id = atoi(gtk_button_get_label(GTK_BUTTON(idWidget)));
    dataP->state.inEditingId = id;

    GtkWidget *editWindow = gtk_dialog_new_with_buttons(taskName, NULL, GTK_DIALOG_MODAL, "Confirmer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *descriptionLabel = gtk_label_new("Description de la tâche");
    dataP->tools.descriptionEntry = gtk_entry_new();
    gtk_widget_set_size_request(dataP->tools.descriptionEntry, 200, 50);

    GtkWidget *priorityButton = gtk_button_new();
    int priority = selectPriority(dataP->conn, id);

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
    g_signal_connect(priorityButton, "clicked", G_CALLBACK(changeTaskPriority), dataP->tools.descriptionEntry);

    gtk_entry_set_text(GTK_ENTRY(dataP->tools.descriptionEntry), selectDescription(dataP->conn, id));

    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), descriptionLabel);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), dataP->tools.descriptionEntry);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), priorityButton);
    gtk_widget_show_all(editWindow);

    g_signal_connect(editWindow, "response", G_CALLBACK(editTaskDB), dataP);

    gtk_widget_set_tooltip_text(taskWidget, selectDescription(dataP->conn, id));
}

void editTaskDB(GtkDialog *window, gint clicked, gpointer data)
{
    struct data *dataP = data;

    if (clicked == GTK_RESPONSE_OK) {
        GtkWidget *input = GTK_WIDGET(dataP->tools.descriptionEntry);
        const gchar *text = gtk_entry_get_text(GTK_ENTRY(input));
        int queryResult;

        GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(dataP->tools.descriptionEntry));
        GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
        GtkWidget *priorityButton = g_list_nth_data(children, 2);
        const gchar *setPriority = gtk_button_get_label(GTK_BUTTON(priorityButton));

        queryResult = updateDescription(dataP->conn, text, dataP->state.inEditingId);
        if (queryResult != 0) {
            g_print("Erreur de la modification de la base");
            return;
        }

        if (strcmp(setPriority, "Mineure") == 0) {
            queryResult = updatePriority(dataP->conn, 0, dataP->state.inEditingId);
        }
        else if (strcmp(setPriority, "Normale") == 0) {
            queryResult = updatePriority(dataP->conn, 1, dataP->state.inEditingId);
        }
        else if (strcmp(setPriority, "Importante") == 0) {
            queryResult = updatePriority(dataP->conn, 2, dataP->state.inEditingId);
        }
        else if (strcmp(setPriority, "Urgente") == 0) {
            queryResult = updatePriority(dataP->conn, 3, dataP->state.inEditingId);
        }

        if (queryResult != 0) {
            g_print("Erreur de la modification de la base");
            return;
        }

        addImportantTask(dataP, dataP->state.inEditingId);
        addMinorTask(dataP, dataP->state.inEditingId);
        gtk_widget_set_tooltip_text(dataP->tools.inEditing, selectDescription(dataP->conn, dataP->state.inEditingId));
    }
    gtk_widget_destroy(GTK_WIDGET(window));
}

void deleteTask(GtkWidget *taskDelete, gpointer data)
{
    struct data *dataP = data;
    GtkWidget *taskToDelete = gtk_widget_get_parent(taskDelete);
    GList *boxChildren = gtk_container_get_children(GTK_CONTAINER(taskToDelete));
    GtkWidget *numberToFree = g_list_nth_data(boxChildren, 5);
    g_list_free(boxChildren);

    int numberToChange = atoi(gtk_button_get_label(GTK_BUTTON(numberToFree)));
    dataP->state.taskNumber[numberToChange] = numberToChange;

    int queryResult = deleteTaskDB(dataP->conn, numberToChange);

    if (queryResult == -1) {
        g_print("Error: insertTask failed");
        return;
    }

    scanForIdToDestroy(data, numberToChange);
    dataP->tools.task[numberToChange] = gtk_label_new("");
}

void deleteProject(GtkWidget *projectDelete, gpointer data)
{
    struct data *dataP = data;

    GtkWidget *projectBox = gtk_widget_get_parent(projectDelete);
    GList *boxChildren = gtk_container_get_children(GTK_CONTAINER(projectBox));
    GtkWidget *child = g_list_nth_data(boxChildren, 0);
    GtkWidget *numberToFree = g_list_nth_data(boxChildren, 2);

    int numberToChange = atoi(gtk_button_get_label(GTK_BUTTON(numberToFree)));
    dataP->state.projectNumber[numberToChange] = numberToChange;

    gint totalPage = gtk_notebook_get_n_pages(GTK_NOTEBOOK(dataP->tools.notebook));
    const gchar *nameOfProject = gtk_label_get_text(GTK_LABEL(child));
    int numberToDelete;

    for (int i = 0; i < totalPage; i++) {
        GtkWidget *curPage = gtk_notebook_get_nth_page(GTK_NOTEBOOK(dataP->tools.notebook), i);
        GtkWidget *boxPage = gtk_notebook_get_tab_label(GTK_NOTEBOOK(dataP->tools.notebook), curPage);
        if (GTK_IS_BOX(boxPage)) {
            boxChildren = gtk_container_get_children(GTK_CONTAINER(boxPage));
            const gchar *nameToSeek = gtk_label_get_text(g_list_nth_data(boxChildren, 0));
            if (g_strcmp0(nameToSeek, nameOfProject) == 0) {
                numberToDelete = i;
            }
        }
    }

    g_list_free(boxChildren);

    int queryResult = deleteAllTaskFromProject(dataP->conn, nameOfProject);
    if (queryResult == -1) {
        g_print("Error: delete all task from project failed");
        return;
    }

    queryResult = deleteProjectDB(dataP->conn, nameOfProject);
    if (queryResult == -1) {
        g_print("Error: deleteProject failed");
        return;
    }

    gtk_notebook_remove_page(GTK_NOTEBOOK(dataP->tools.notebook), numberToDelete);
    gtk_notebook_set_current_page(dataP->tools.notebook, 0);
    dataP->state.projectCount--;
}

void addTasks(GtkWidget *task, gpointer data, int presentTask, char *presentProjectName)
{
    struct data *dataP = data;

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    if (readOneConfigValue("set deadline day") != -1) {
        local_time->tm_mday += readOneConfigValue("set deadline day");
        mktime(local_time);
    }
    else {
        local_time = localtime(&now);
    }

    char deadlineDate[20];
    sprintf(deadlineDate, "%d-%d-%d", local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday);

    gchar *getText;
    char *projectName = malloc(strlen(presentProjectName) + 1 * sizeof(char));
    strcpy(projectName, presentProjectName);
    if (dataP->state.repopulatedTask == 0) {

        gtk_notebook_set_current_page(dataP->tools.notebook, 0);

        for (int i = 0; i < allProject(dataP->conn); i++) {

            GtkWidget *projectPageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(dataP->tools.notebook), i + 6);
            GtkWidget *projectLabelBox = gtk_notebook_get_tab_label(GTK_NOTEBOOK(dataP->tools.notebook), projectPageBox);
            GList *projectBoxChildren = gtk_container_get_children(GTK_CONTAINER(projectLabelBox));
            GtkWidget *projectLabel = g_list_nth_data(projectBoxChildren, 0);
            const gchar *projectLabelName = gtk_label_get_label(GTK_LABEL(projectLabel));

            if (strcmp(projectLabelName, projectName) == 0)
                gtk_notebook_set_current_page(dataP->tools.notebook, i + 6);

            g_list_free(projectBoxChildren);
        }
    }

    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(dataP->tools.notebook));
    GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(dataP->tools.notebook), currentPage); //boxV
    GList *children = gtk_container_get_children(GTK_CONTAINER(pageBox));
    int numberOfTask = g_list_length(children) - 3;
    GtkWidget *addProjectBox = g_list_last(children)->data;
    g_list_free(children);

    children = gtk_container_get_children(GTK_CONTAINER(addProjectBox));
    GtkWidget *entry = g_list_last(children)->data;
    g_list_free(children);

    getText = malloc(sizeof(gchar) * strlen(get_text_of_entry(entry)) + 1);
    strcpy(getText, get_text_of_entry(entry));

    if (strcmp(getText, "") == 0 && dataP->state.repopulatedTask == 1)
        return;

    if (numberOfTask >= dataP->state.maxTaskPerProject)
        return;

    //Attribution de l'id
    if (dataP->state.repopulatedTask == 1) {
        for (dataP->state.i = 0; dataP->state.i < dataP->state.maxTaskTotal; dataP->state.i++) {
            if (dataP->state.taskNumber[dataP->state.i] != -1) {
                dataP->state.taskNumber[dataP->state.i] = -1;
                break;
            }
        }
    }
    else if (dataP->state.repopulatedTask == 0) {
        dataP->state.taskNumber[presentTask] = -1;
        dataP->state.i = presentTask;
    }

    const gchar *name;
    if (currentPage < 5) {
        name = gtk_notebook_get_tab_label_text(dataP->tools.notebook, pageBox);
    }
    else {
        GtkWidget *projectBox = gtk_notebook_get_tab_label(dataP->tools.notebook, pageBox);
        GList *projectBoxList = gtk_container_get_children(GTK_CONTAINER(projectBox));
        GtkWidget *projectLabel = g_list_nth_data(projectBoxList, 0);
        name = gtk_label_get_label(GTK_LABEL(projectLabel));
    }

    dataP->tools.boxTask[dataP->state.i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pageBox), dataP->tools.boxTask[dataP->state.i], FALSE, FALSE, 0);
    gtk_box_reorder_child(GTK_BOX(pageBox), dataP->tools.boxTask[dataP->state.i], numberOfTask + 2);

    if (dataP->state.repopulatedTask == 1) {
        dataP->tools.taskStatus[dataP->state.i] = gtk_button_new_with_label("Non completé");
    }
    else if (dataP->state.repopulatedTask == 0) {
        int queryResult = selectStatus(dataP->conn, dataP->state.i);
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
        dataP->tools.taskStatus[dataP->state.i] = gtk_button_new_with_label(status);
    }

    gtk_widget_set_margin_top(dataP->tools.taskStatus[dataP->state.i], 10);
    gtk_widget_set_margin_bottom(dataP->tools.taskStatus[dataP->state.i], 10);
    gtk_widget_set_size_request(dataP->tools.taskStatus[dataP->state.i], 150, -1);
    gtk_box_pack_start(GTK_BOX(dataP->tools.boxTask[dataP->state.i]), dataP->tools.taskStatus[dataP->state.i], FALSE, FALSE, 0);
    g_signal_connect(dataP->tools.taskStatus[dataP->state.i], "clicked", G_CALLBACK(changeTaskStatus), dataP);

    dataP->tools.taskSeparator1[dataP->state.i] = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(dataP->tools.taskSeparator1[dataP->state.i], 5, -1);
    gtk_box_pack_start(GTK_BOX(dataP->tools.boxTask[dataP->state.i]), dataP->tools.taskSeparator1[dataP->state.i], FALSE, FALSE, 0);

    if (dataP->state.repopulatedTask == 0) {
        gtk_label_set_text(GTK_LABEL(dataP->tools.task[dataP->state.i]), selectTask(dataP->conn, dataP->state.i));
        gtk_widget_set_tooltip_text(dataP->tools.task[dataP->state.i], selectDescription(dataP->conn, dataP->state.i));
    }
    else if (dataP->state.repopulatedTask == 1) {
        gtk_label_set_text(GTK_LABEL(dataP->tools.task[dataP->state.i]), getText);
        gtk_widget_set_tooltip_text(dataP->tools.task[dataP->state.i], "");
    }
    gtk_box_pack_start(GTK_BOX(dataP->tools.boxTask[dataP->state.i]), dataP->tools.task[dataP->state.i], TRUE, FALSE, 0);

    dataP->tools.taskEdit[dataP->state.i] = gtk_button_new_with_label("Editer");
    gtk_box_pack_start(GTK_BOX(dataP->tools.boxTask[dataP->state.i]), dataP->tools.taskEdit[dataP->state.i], FALSE, FALSE, 0);
    g_signal_connect(dataP->tools.taskEdit[dataP->state.i], "clicked", G_CALLBACK(editTaskWindow), dataP);

    dataP->tools.taskDelete[dataP->state.i] = gtk_button_new_with_label("X");
    gtk_box_pack_start(GTK_BOX(dataP->tools.boxTask[dataP->state.i]), dataP->tools.taskDelete[dataP->state.i], FALSE, FALSE, 0);
    g_signal_connect(dataP->tools.taskDelete[dataP->state.i], "clicked", G_CALLBACK(deleteTask), dataP);

    char numberToTransfer[3];
    sprintf(numberToTransfer, "%d", dataP->state.i);
    GtkWidget *taskNumberMarker = gtk_button_new_with_label(numberToTransfer);
    gtk_box_pack_start(GTK_BOX(dataP->tools.boxTask[dataP->state.i]), taskNumberMarker, FALSE, FALSE, 0);

    dataP->tools.taskSeparator2[dataP->state.i] = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(dataP->tools.taskSeparator2[dataP->state.i], 5, -1);
    gtk_box_pack_start(GTK_BOX(dataP->tools.boxTask[dataP->state.i]), dataP->tools.taskSeparator2[dataP->state.i], FALSE, FALSE, 0);

    if (dataP->state.repopulatedTask == 0) {
        char *queryResult = selectDeadline(dataP->conn, dataP->state.i);
        dataP->tools.taskDeadline[dataP->state.i] = gtk_button_new_with_label(queryResult);
    }
    else if (dataP->state.repopulatedTask == 1) {
        dataP->tools.taskDeadline[dataP->state.i] = gtk_button_new_with_label(deadlineDate);
    }

    gtk_widget_set_margin_top(dataP->tools.taskDeadline[dataP->state.i], 10);
    gtk_widget_set_margin_bottom(dataP->tools.taskDeadline[dataP->state.i], 10);
    gtk_widget_set_size_request(dataP->tools.taskDeadline[dataP->state.i], 150, -1);
    gtk_box_pack_start(GTK_BOX(dataP->tools.boxTask[dataP->state.i]), dataP->tools.taskDeadline[dataP->state.i], FALSE, FALSE, 0);
    g_signal_connect(dataP->tools.taskDeadline[dataP->state.i], "clicked", G_CALLBACK(changeDeadlineWindow), dataP);

    gtk_widget_show_all(dataP->tools.boxTask[dataP->state.i]);
    gtk_widget_hide(taskNumberMarker);

    gtk_entry_set_text(GTK_ENTRY(entry), "");

    if (dataP->state.repopulatedTask == 1) {
        int queryResult = insertTask(dataP->conn, dataP->state.i, getText, "", 1, deadlineDate, 0, name); //insert in db
        if (queryResult == -1)
            g_print("Error: insertTask failed");
    }
}

gchar *get_text_of_entry(GtkWidget *inputEntry)
{
    GtkEntryBuffer *buffer = gtk_entry_get_buffer((GtkEntry *)inputEntry);
    gchar *text;
    text = (gchar *)gtk_entry_buffer_get_text(buffer);
    return text;
}

int readOneConfigValue(char *propName)
{
    if (propName == NULL) {
        g_print("Error: propName is null");
        return -1;
    }
    FILE *file = fopen("settings/config.txt", "r");
    if (file == NULL) {
        g_print("Error: config file not found");
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

void addProjectWindow(GtkWidget *project, gpointer data)
{
    struct data *dataP = data;

    if (dataP->state.projectCount >= dataP->state.maxProject) {
        return;
    }

    GtkWidget *addProjectDialog
        = gtk_dialog_new_with_buttons("Nouveau projet", NULL, GTK_DIALOG_MODAL, "Confirmer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *nameLabel = gtk_label_new("Nom du projet");
    dataP->tools.projectNameEntry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(addProjectDialog))), nameLabel);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(addProjectDialog))), dataP->tools.projectNameEntry);
    gtk_widget_show_all(addProjectDialog);

    g_signal_connect(addProjectDialog, "response", G_CALLBACK(addProject), dataP);
}

void addProject(GtkWidget *projet, gint clicked, gpointer data, int presentProject)
{
    struct data *dataP = data;
    if (clicked == GTK_RESPONSE_OK) {
        gchar *projectName;

        if (dataP->state.repopulatedProject == 1) {
            projectName = get_text_of_entry(dataP->tools.projectNameEntry);
            if (projectExist(dataP->conn, projectName) == 1 || projectName == NULL) {
                gtk_widget_destroy(projet);
                GtkDialog *dialog
                    = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(dataP->tools.window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Ce projet existe déjà"));
                gtk_dialog_run(dialog);
                gtk_widget_destroy(GTK_WIDGET(dialog));
                return;
            }

            char *query = malloc((strlen("INSERT INTO project VALUES ('','Placeholder', 0, 'now()', 'now()', 0)") + strlen(projectName) + 1) * sizeof(char));
            sprintf(query, "INSERT INTO project VALUES ('%s','Placeholder', 0, 'now()', 'now()', 0)", projectName);
            PGresult *res = PQexec(dataP->conn, query);
            if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                g_print("Error: addProject failed");
                return;
            }
            free(query);
            PQclear(res);
        }

        for (dataP->state.i = 0; dataP->state.i < dataP->state.maxProject; dataP->state.i++) {
            if (dataP->state.projectNumber[dataP->state.i] != -1) {
                dataP->state.projectNumber[dataP->state.i] = -1;
                break;
            }
        }

        dataP->tools.pageTitleBox[dataP->state.i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        GtkWidget *title = gtk_label_new("");
        if (dataP->state.repopulatedProject == 1) {
            gtk_label_set_text(GTK_LABEL(title), (const gchar *)projectName);
        }
        else if (dataP->state.repopulatedProject == 0) {
            gtk_label_set_text(GTK_LABEL(title), selectProject(dataP->conn, presentProject));
        }

        GtkWidget *titleButton = gtk_button_new_with_label("X");
        char numberToTransfer[3];
        sprintf(numberToTransfer, "%d", dataP->state.i);
        GtkWidget *projectNumberMarker = gtk_button_new_with_label(numberToTransfer);

        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

        dataP->tools.projectTaskBox[dataP->state.i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start(GTK_BOX(box), dataP->tools.projectTaskBox[dataP->state.i], FALSE, FALSE, 0);
        GtkWidget *separatorH = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_widget_set_size_request(separatorH, -1, 5);
        gtk_box_pack_start(GTK_BOX(box), separatorH, FALSE, FALSE, 0);

        GtkWidget *status = gtk_label_new("Status");
        gtk_widget_set_margin_top(status, 10);
        gtk_widget_set_margin_bottom(status, 10);
        gtk_widget_set_size_request(status, 150, -1);
        gtk_box_pack_start(GTK_BOX(dataP->tools.projectTaskBox[dataP->state.i]), status, FALSE, FALSE, 0);

        GtkWidget *separatorV1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        gtk_widget_set_size_request(separatorV1, 5, -1);
        gtk_box_pack_start(GTK_BOX(dataP->tools.projectTaskBox[dataP->state.i]), separatorV1, FALSE, FALSE, 0);

        GtkWidget *projectTitle;
        if (dataP->state.repopulatedProject == 1) {
            projectTitle = gtk_label_new(projectName);
        }
        else if (dataP->state.repopulatedProject == 0) {
            projectTitle = gtk_label_new(selectProject(dataP->conn, presentProject));
        }
        gtk_box_pack_start(GTK_BOX(dataP->tools.projectTaskBox[dataP->state.i]), projectTitle, TRUE, FALSE, 0);

        GtkWidget *separatorV2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        gtk_widget_set_size_request(separatorV2, 5, -1);
        gtk_box_pack_start(GTK_BOX(dataP->tools.projectTaskBox[dataP->state.i]), separatorV2, FALSE, FALSE, 0);

        GtkWidget *deadline = gtk_label_new("Date limite");
        gtk_widget_set_margin_top(deadline, 10);
        gtk_widget_set_margin_bottom(deadline, 10);
        gtk_widget_set_size_request(deadline, 150, -1);
        gtk_box_pack_start(GTK_BOX(dataP->tools.projectTaskBox[dataP->state.i]), deadline, FALSE, FALSE, 0);

        GtkWidget *boxAddTask = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        GtkWidget *addButton = gtk_button_new_with_label("Ajouter la tâche");
        GtkWidget *inputEntry = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX(boxAddTask), addButton, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(boxAddTask), inputEntry, TRUE, TRUE, 0);

        gtk_box_pack_start(GTK_BOX(box), boxAddTask, TRUE, TRUE, 0);
        gtk_widget_set_valign(boxAddTask, GTK_ALIGN_END);

        gtk_box_pack_start(GTK_BOX(dataP->tools.pageTitleBox[dataP->state.i]), title, TRUE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(dataP->tools.pageTitleBox[dataP->state.i]), titleButton, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(dataP->tools.pageTitleBox[dataP->state.i]), projectNumberMarker, FALSE, FALSE, 0);

        gtk_widget_show_all(dataP->tools.projectTaskBox[dataP->state.i]);
        gtk_widget_show_all(box);
        gtk_widget_show_all(GTK_WIDGET(dataP->tools.pageTitleBox[dataP->state.i]));
        gtk_widget_hide(projectNumberMarker);

        gint numberOfPage = gtk_notebook_get_n_pages(GTK_NOTEBOOK(dataP->tools.notebook));

        g_signal_connect(addButton, "clicked", G_CALLBACK(addTasks), dataP);
        g_signal_connect(titleButton, "clicked", G_CALLBACK(deleteProject), dataP);

        gtk_notebook_insert_page(GTK_NOTEBOOK(dataP->tools.notebook), box, GTK_WIDGET(dataP->tools.pageTitleBox[dataP->state.i]), numberOfPage - 1);
        gtk_widget_show(GTK_WIDGET(dataP->tools.pageTitleBox[dataP->state.i]));
        gtk_widget_show(box);
        dataP->state.projectCount++;
    }
    if (dataP->state.repopulatedProject == 1)
        gtk_widget_destroy(projet);
}

void changeDeadlineWindow(GtkWidget *deadline, gpointer data)
{
    struct data *dataP = data;
    GtkWidget *changeDeadlineDialog
        = gtk_dialog_new_with_buttons("Changer de date limite", NULL, GTK_DIALOG_MODAL, "Confirmer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *calendar = gtk_calendar_new();

    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(changeDeadlineDialog))), calendar);
    gtk_widget_show_all(changeDeadlineDialog);

    GtkWidget *parent = gtk_widget_get_parent(deadline);
    GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
    GtkWidget *idButton = g_list_nth_data(children, 5);
    int id = atoi(gtk_button_get_label(GTK_BUTTON(idButton)));
    dataP->state.inEditingId = id;

    g_signal_connect(changeDeadlineDialog, "response", G_CALLBACK(changeDeadline), dataP);
}

void changeDeadline(GtkWidget *deadline, gint clicked, gpointer data)
{
    struct data *dataP = data;
    if (clicked == GTK_RESPONSE_OK) {
        GtkWidget *box = GTK_WIDGET(gtk_dialog_get_content_area(GTK_DIALOG(deadline)));
        GList *child = gtk_container_get_children(GTK_CONTAINER(box));
        GtkWidget *calendar = g_list_nth_data(child, 0);

        guint year, month, day;

        gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
        gchar *changedDeadline = malloc(11 * sizeof(gchar));
        sprintf(changedDeadline, "%d-%d-%d", year, month + 1, day);
        updateDeadline(dataP->conn, dataP->state.inEditingId, changedDeadline);
        gtk_button_set_label(GTK_BUTTON(dataP->tools.taskDeadline[dataP->state.inEditingId]), changedDeadline);
        addLateTask(dataP, dataP->state.inEditingId);
    }
    gtk_widget_destroy(deadline);
}

void addImportantTask(gpointer data, int id)
{
    struct data *dataP = data;

    gint startingPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(dataP->tools.notebook));

    gtk_notebook_set_current_page(dataP->tools.notebook, 1);

    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(dataP->tools.notebook));
    GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(dataP->tools.notebook), currentPage); //boxV
    GList *children = gtk_container_get_children(GTK_CONTAINER(pageBox));

    int checkTask = 0;
    int numberOfTask = g_list_length(children) - 2;

    for (int j = 2; j < numberOfTask + 2; j++) {
        GtkWidget *boxTask = g_list_nth_data(children, j);
        GList *taskList = gtk_container_get_children(GTK_CONTAINER(boxTask));
        GtkWidget *idButton = g_list_nth_data(taskList, 5);
        g_list_free(taskList);

        int idCheck = atoi(gtk_button_get_label(GTK_BUTTON(idButton)));
        if (idCheck == id) {
            updateTask(dataP, boxTask, id);
            checkTask = 1;
        }
    }

    g_list_free(children);

    int priority = selectPriority(dataP->conn, id);
    if (priority <= 1) {
        if (checkTask == 1) {
            scanForIdToDestroySpecific(dataP, id, 1);
            gtk_notebook_set_current_page(dataP->tools.notebook, startingPage);
        }
        return;
    }

    if (checkTask == 1) { //Pour que le GList se free
        return;
    }

    GtkWidget *boxTask = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pageBox), boxTask, FALSE, FALSE, 0);

    int queryResult = selectStatus(dataP->conn, id);
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
    GtkWidget *statusButton = gtk_button_new_with_label(status);

    gtk_widget_set_margin_top(statusButton, 10);
    gtk_widget_set_margin_bottom(statusButton, 10);
    gtk_widget_set_size_request(statusButton, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), statusButton, FALSE, FALSE, 0);
    g_signal_connect(statusButton, "clicked", G_CALLBACK(changeTaskStatus), dataP);

    GtkWidget *taskSeparator1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator1, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator1, FALSE, FALSE, 0);

    GtkWidget *taskName = gtk_label_new(selectTask(dataP->conn, id));
    gtk_widget_set_tooltip_text(taskName, selectDescription(dataP->conn, id));
    gtk_box_pack_start(GTK_BOX(boxTask), taskName, TRUE, FALSE, 0);

    GtkWidget *taskEdit = gtk_button_new_with_label("Editer");
    gtk_box_pack_start(GTK_BOX(boxTask), taskEdit, FALSE, FALSE, 0);
    g_signal_connect(taskEdit, "clicked", G_CALLBACK(editTaskWindow), dataP);

    GtkWidget *taskDelete = gtk_button_new_with_label("X");
    gtk_box_pack_start(GTK_BOX(boxTask), taskDelete, FALSE, FALSE, 0);
    g_signal_connect(taskDelete, "clicked", G_CALLBACK(deleteTask), dataP);

    char numberToTransfer[3];
    sprintf(numberToTransfer, "%d", id);
    GtkWidget *taskNumberMarker = gtk_button_new_with_label(numberToTransfer);
    gtk_box_pack_start(GTK_BOX(boxTask), taskNumberMarker, FALSE, FALSE, 0);

    GtkWidget *taskSeparator2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator2, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator2, FALSE, FALSE, 0);

    char *queryDeadline = selectDeadline(dataP->conn, id);
    GtkWidget *taskDeadline = gtk_button_new_with_label(queryDeadline);
    gtk_widget_set_margin_top(taskDeadline, 10);
    gtk_widget_set_margin_bottom(taskDeadline, 10);
    gtk_widget_set_size_request(taskDeadline, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskDeadline, FALSE, FALSE, 0);
    g_signal_connect(taskDeadline, "clicked", G_CALLBACK(changeDeadlineWindow), dataP);

    gtk_widget_show_all(boxTask);
    gtk_widget_hide(taskNumberMarker);

    gtk_notebook_set_current_page(dataP->tools.notebook, startingPage);
}

void addMinorTask(gpointer data, int id)
{
    struct data *dataP = data;

    gint startingPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(dataP->tools.notebook));

    gtk_notebook_set_current_page(dataP->tools.notebook, 2);

    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(dataP->tools.notebook));
    GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(dataP->tools.notebook), currentPage); //boxV
    GList *children = gtk_container_get_children(GTK_CONTAINER(pageBox));

    int checkTask = 0;
    int numberOfTask = g_list_length(children) - 2;

    for (int j = 2; j < numberOfTask + 2; j++) {
        GtkWidget *boxTask = g_list_nth_data(children, j);
        GList *taskList = gtk_container_get_children(GTK_CONTAINER(boxTask));
        GtkWidget *idButton = g_list_nth_data(taskList, 5);
        g_list_free(taskList);

        int idCheck = atoi(gtk_button_get_label(GTK_BUTTON(idButton)));
        if (idCheck == id) {
            updateTask(dataP, boxTask, id);
            checkTask = 1;
        }
    }

    g_list_free(children);

    int priority = selectPriority(dataP->conn, id);
    if (priority > 0) {
        if (checkTask == 1) {
            scanForIdToDestroySpecific(dataP, id, 2);
            gtk_notebook_set_current_page(dataP->tools.notebook, startingPage);
        }
        return;
    }

    if (checkTask == 1) { //Pour que le GList se free
        return;
    }

    GtkWidget *boxTask = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pageBox), boxTask, FALSE, FALSE, 0);

    int queryResult = selectStatus(dataP->conn, id);
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
    GtkWidget *statusButton = gtk_button_new_with_label(status);

    gtk_widget_set_margin_top(statusButton, 10);
    gtk_widget_set_margin_bottom(statusButton, 10);
    gtk_widget_set_size_request(statusButton, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), statusButton, FALSE, FALSE, 0);
    g_signal_connect(statusButton, "clicked", G_CALLBACK(changeTaskStatus), dataP);

    GtkWidget *taskSeparator1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator1, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator1, FALSE, FALSE, 0);

    GtkWidget *taskName = gtk_label_new(selectTask(dataP->conn, id));
    gtk_widget_set_tooltip_text(taskName, selectDescription(dataP->conn, id));
    gtk_box_pack_start(GTK_BOX(boxTask), taskName, TRUE, FALSE, 0);

    GtkWidget *taskEdit = gtk_button_new_with_label("Editer");
    gtk_box_pack_start(GTK_BOX(boxTask), taskEdit, FALSE, FALSE, 0);
    g_signal_connect(taskEdit, "clicked", G_CALLBACK(editTaskWindow), dataP);

    GtkWidget *taskDelete = gtk_button_new_with_label("X");
    gtk_box_pack_start(GTK_BOX(boxTask), taskDelete, FALSE, FALSE, 0);
    g_signal_connect(taskDelete, "clicked", G_CALLBACK(deleteTask), dataP);

    char numberToTransfer[3];
    sprintf(numberToTransfer, "%d", id);
    GtkWidget *taskNumberMarker = gtk_button_new_with_label(numberToTransfer);
    gtk_box_pack_start(GTK_BOX(boxTask), taskNumberMarker, FALSE, FALSE, 0);

    GtkWidget *taskSeparator2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator2, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator2, FALSE, FALSE, 0);

    char *queryDeadline = selectDeadline(dataP->conn, id);
    GtkWidget *taskDeadline = gtk_button_new_with_label(queryDeadline);
    gtk_widget_set_margin_top(taskDeadline, 10);
    gtk_widget_set_margin_bottom(taskDeadline, 10);
    gtk_widget_set_size_request(taskDeadline, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskDeadline, FALSE, FALSE, 0);
    g_signal_connect(taskDeadline, "clicked", G_CALLBACK(changeDeadlineWindow), dataP);

    gtk_widget_show_all(boxTask);
    gtk_widget_hide(taskNumberMarker);

    gtk_notebook_set_current_page(dataP->tools.notebook, startingPage);
}

void addLateTask(gpointer data, int id)
{
    struct data *dataP = data;

    gint startingPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(dataP->tools.notebook));

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    local_time = localtime(&now);

    gtk_notebook_set_current_page(dataP->tools.notebook, 3);

    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(dataP->tools.notebook));
    GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(dataP->tools.notebook), currentPage); //boxV
    GList *children = gtk_container_get_children(GTK_CONTAINER(pageBox));

    int checkTask = 0;
    int numberOfTask = g_list_length(children) - 2;

    for (int j = 2; j < numberOfTask + 2; j++) {
        GtkWidget *boxTask = g_list_nth_data(children, j);
        GList *taskList = gtk_container_get_children(GTK_CONTAINER(boxTask));
        GtkWidget *idButton = g_list_nth_data(taskList, 5);
        g_list_free(taskList);

        int idCheck = atoi(gtk_button_get_label(GTK_BUTTON(idButton)));
        if (idCheck == id) {
            updateTask(dataP, boxTask, id);
            checkTask = 1;
        }
    }

    g_list_free(children);

    char *deadline = selectDeadline(dataP->conn, id);

    int deadlineYear, deadlineMonth, deadlineDay, late = 1;
    sscanf(deadline, "%d-%d-%d", &deadlineYear, &deadlineMonth, &deadlineDay);

    if (deadlineYear > local_time->tm_year + 1900) {
        late = 0;
    }
    else if (deadlineYear == local_time->tm_year + 1900) {
        if (deadlineMonth > local_time->tm_mon + 1) {
            late = 0;
        }
        else if (deadlineMonth == local_time->tm_mon + 1) {
            if (deadlineDay >= local_time->tm_mday) {
                late = 0;
            }
        }
    }

    if (late == 0) {
        if (checkTask == 1) {
            scanForIdToDestroySpecific(dataP, id, 3);
            gtk_notebook_set_current_page(dataP->tools.notebook, startingPage);
        }
        return;
    }

    if (checkTask == 1) { //Pour que le GList se free
        return;
    }

    GtkWidget *boxTask = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pageBox), boxTask, FALSE, FALSE, 0);

    int queryResult = selectStatus(dataP->conn, id);
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
    GtkWidget *statusButton = gtk_button_new_with_label(status);

    gtk_widget_set_margin_top(statusButton, 10);
    gtk_widget_set_margin_bottom(statusButton, 10);
    gtk_widget_set_size_request(statusButton, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), statusButton, FALSE, FALSE, 0);
    g_signal_connect(statusButton, "clicked", G_CALLBACK(changeTaskStatus), dataP);

    GtkWidget *taskSeparator1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator1, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator1, FALSE, FALSE, 0);

    GtkWidget *taskName = gtk_label_new(selectTask(dataP->conn, id));
    gtk_widget_set_tooltip_text(taskName, selectDescription(dataP->conn, id));
    gtk_box_pack_start(GTK_BOX(boxTask), taskName, TRUE, FALSE, 0);

    GtkWidget *taskEdit = gtk_button_new_with_label("Editer");
    gtk_box_pack_start(GTK_BOX(boxTask), taskEdit, FALSE, FALSE, 0);
    g_signal_connect(taskEdit, "clicked", G_CALLBACK(editTaskWindow), dataP);

    GtkWidget *taskDelete = gtk_button_new_with_label("X");
    gtk_box_pack_start(GTK_BOX(boxTask), taskDelete, FALSE, FALSE, 0);
    g_signal_connect(taskDelete, "clicked", G_CALLBACK(deleteTask), dataP);

    char numberToTransfer[3];
    sprintf(numberToTransfer, "%d", id);
    GtkWidget *taskNumberMarker = gtk_button_new_with_label(numberToTransfer);
    gtk_box_pack_start(GTK_BOX(boxTask), taskNumberMarker, FALSE, FALSE, 0);

    GtkWidget *taskSeparator2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator2, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator2, FALSE, FALSE, 0);

    char *queryDeadline = selectDeadline(dataP->conn, id);
    GtkWidget *taskDeadline = gtk_button_new_with_label(queryDeadline);
    gtk_widget_set_margin_top(taskDeadline, 10);
    gtk_widget_set_margin_bottom(taskDeadline, 10);
    gtk_widget_set_size_request(taskDeadline, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskDeadline, FALSE, FALSE, 0);
    g_signal_connect(taskDeadline, "clicked", G_CALLBACK(changeDeadlineWindow), dataP);

    gtk_widget_show_all(boxTask);
    gtk_widget_hide(taskNumberMarker);

    gtk_notebook_set_current_page(dataP->tools.notebook, startingPage);
}

void addPlannedTask(gpointer data, int id)
{
    struct data *dataP = data;

    gint startingPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(dataP->tools.notebook));

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    local_time = localtime(&now);

    gtk_notebook_set_current_page(dataP->tools.notebook, 4);

    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(dataP->tools.notebook));
    GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(dataP->tools.notebook), currentPage); //boxV
    GList *children = gtk_container_get_children(GTK_CONTAINER(pageBox));

    int checkTask = 0;
    int numberOfTask = g_list_length(children) - 2;

    for (int j = 2; j < numberOfTask + 2; j++) {
        GtkWidget *boxTask = g_list_nth_data(children, j);
        GList *taskList = gtk_container_get_children(GTK_CONTAINER(boxTask));
        GtkWidget *idButton = g_list_nth_data(taskList, 5);
        g_list_free(taskList);

        int idCheck = atoi(gtk_button_get_label(GTK_BUTTON(idButton)));
        if (idCheck == id) {
            updateTask(dataP, boxTask, id);
            checkTask = 1;
        }
    }

    g_list_free(children);

    char *deadline = selectDeadline(dataP->conn, id);

    int deadlineYear, deadlineMonth, deadlineDay, late = 1;
    sscanf(deadline, "%d-%d-%d", &deadlineYear, &deadlineMonth, &deadlineDay);

    if (deadlineYear > local_time->tm_year + 1900) {
        late = 0;
    }
    else if (deadlineYear == local_time->tm_year + 1900) {
        if (deadlineMonth > local_time->tm_mon + 1) {
            late = 0;
        }
        else if (deadlineMonth == local_time->tm_mon + 1) {
            if (deadlineDay >= local_time->tm_mday) {
                late = 0;
            }
        }
    }

    if (late == 1) {
        if (checkTask == 1) {
            scanForIdToDestroySpecific(dataP, id, 4);
            gtk_notebook_set_current_page(dataP->tools.notebook, startingPage);
        }
        return;
    }

    if (checkTask == 1) { //Pour que le GList se free
        return;
    }

    GtkWidget *boxTask = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pageBox), boxTask, FALSE, FALSE, 0);

    int queryResult = selectStatus(dataP->conn, id);
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
    GtkWidget *statusButton = gtk_button_new_with_label(status);

    gtk_widget_set_margin_top(statusButton, 10);
    gtk_widget_set_margin_bottom(statusButton, 10);
    gtk_widget_set_size_request(statusButton, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), statusButton, FALSE, FALSE, 0);
    g_signal_connect(statusButton, "clicked", G_CALLBACK(changeTaskStatus), dataP);

    GtkWidget *taskSeparator1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator1, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator1, FALSE, FALSE, 0);

    GtkWidget *taskName = gtk_label_new(selectTask(dataP->conn, id));
    gtk_widget_set_tooltip_text(taskName, selectDescription(dataP->conn, id));
    gtk_box_pack_start(GTK_BOX(boxTask), taskName, TRUE, FALSE, 0);

    GtkWidget *taskEdit = gtk_button_new_with_label("Editer");
    gtk_box_pack_start(GTK_BOX(boxTask), taskEdit, FALSE, FALSE, 0);
    g_signal_connect(taskEdit, "clicked", G_CALLBACK(editTaskWindow), dataP);

    GtkWidget *taskDelete = gtk_button_new_with_label("X");
    gtk_box_pack_start(GTK_BOX(boxTask), taskDelete, FALSE, FALSE, 0);
    g_signal_connect(taskDelete, "clicked", G_CALLBACK(deleteTask), dataP);

    char numberToTransfer[3];
    sprintf(numberToTransfer, "%d", id);
    GtkWidget *taskNumberMarker = gtk_button_new_with_label(numberToTransfer);
    gtk_box_pack_start(GTK_BOX(boxTask), taskNumberMarker, FALSE, FALSE, 0);

    GtkWidget *taskSeparator2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator2, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator2, FALSE, FALSE, 0);

    char *queryDeadline = selectDeadline(dataP->conn, id);
    GtkWidget *taskDeadline = gtk_button_new_with_label(queryDeadline);
    gtk_widget_set_margin_top(taskDeadline, 10);
    gtk_widget_set_margin_bottom(taskDeadline, 10);
    gtk_widget_set_size_request(taskDeadline, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskDeadline, FALSE, FALSE, 0);
    g_signal_connect(taskDeadline, "clicked", G_CALLBACK(changeDeadlineWindow), dataP);

    gtk_widget_show_all(boxTask);
    gtk_widget_hide(taskNumberMarker);

    gtk_notebook_set_current_page(dataP->tools.notebook, startingPage);
}

void scanForIdToDestroy(gpointer data, int idToDestroy)
{
    struct data *dataP = data;
    gint startingPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(dataP->tools.notebook));
    int numberOfProject = allProject(dataP->conn) + 6;

    for (int i = 0; i < numberOfProject; i++) {
        if (i != 5) { //le i == 5 c'est la page finance
            gtk_notebook_set_current_page(dataP->tools.notebook, i);

            gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(dataP->tools.notebook));
            GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(dataP->tools.notebook), currentPage);
            GList *children = gtk_container_get_children(GTK_CONTAINER(pageBox));

            int numberOfTask;
            if (i == 1 || i == 2 || i == 3 || i == 4)
                numberOfTask = g_list_length(children) - 2;
            else
                numberOfTask = g_list_length(children) - 3;

            for (int j = 2; j < numberOfTask + 2; j++) {
                GtkWidget *boxTask = g_list_nth_data(children, j);
                GList *taskList = gtk_container_get_children(GTK_CONTAINER(boxTask));
                GtkWidget *idButton = g_list_nth_data(taskList, 5);
                g_list_free(taskList);

                int id = atoi(gtk_button_get_label(GTK_BUTTON(idButton)));
                if (id == idToDestroy) {
                    gtk_widget_destroy(boxTask);
                }
            }
            g_list_free(children);
        }
    }
    gtk_notebook_set_current_page(dataP->tools.notebook, startingPage);
}

void scanForIdToDestroySpecific(gpointer data, int idToDestroy, guint project)
{
    struct data *dataP = data;
    int numberOfProject = allProject(dataP->conn) + 6;

    gtk_notebook_set_current_page(dataP->tools.notebook, project);

    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(dataP->tools.notebook));
    GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(dataP->tools.notebook), currentPage);
    GList *children = gtk_container_get_children(GTK_CONTAINER(pageBox));

    int numberOfTask;
    if (project == 1 || project == 2 || project == 3 || project == 4)
        numberOfTask = g_list_length(children) - 2;
    else
        numberOfTask = g_list_length(children) - 3;

    for (int j = 2; j < numberOfTask + 2; j++) {
        GtkWidget *boxTask = g_list_nth_data(children, j);
        GList *taskList = gtk_container_get_children(GTK_CONTAINER(boxTask));
        GtkWidget *idButton = g_list_nth_data(taskList, 5);
        g_list_free(taskList);

        int id = atoi(gtk_button_get_label(GTK_BUTTON(idButton)));
        if (id == idToDestroy) {
            gtk_widget_destroy(boxTask);
        }
    }
    g_list_free(children);
}

void updateTask(gpointer data, GtkWidget *task, int id)
{
    struct data *dataP = data;

    GList *listOfWidget = gtk_container_get_children(GTK_CONTAINER(task));

    GtkWidget *taskStatus = g_list_nth_data(listOfWidget, 0);
    int queryResult = selectStatus(dataP->conn, id);
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
    taskStatus = gtk_button_new_with_label(status);

    GtkWidget *taskName = g_list_nth_data(listOfWidget, 2);
    gtk_label_set_text(GTK_LABEL(taskName), selectTask(dataP->conn, id));
    gtk_widget_set_tooltip_text(taskName, selectDescription(dataP->conn, id));

    GtkWidget *taskDeadline = g_list_nth_data(listOfWidget, 6);
    char *deadline = selectDeadline(dataP->conn, id);
    taskDeadline = gtk_button_new_with_label(deadline);
}