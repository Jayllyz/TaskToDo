#include "functions.h"
#include <curl/curl.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void openApp(GtkWidget *button, struct Data *data)
{
    gtk_widget_show(data->tools.window);
    gtk_widget_hide(data->home.windowHome);

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char *day = malloc(sizeof(char) * 3);
    char *month = malloc(sizeof(char) * 3);
    snprintf(day, 3, "%02u", local_time->tm_mday);
    snprintf(month, 3, "%02u", local_time->tm_mon + 1);
    int newConnect = 0;
    int newMonth = 0;

    if (readOneConfigValue("last connect day") != local_time->tm_mday)
        newConnect = 1;
    if (readOneConfigValue("last connect month") != local_time->tm_mon + 1) {
        newConnect = 1;
        newMonth = 1;
    }
    if (readOneConfigValue("last connect year") != local_time->tm_year + 1900)
        newConnect = 1;

    //Max project and tasks
    for (int i = 0; i < data->state.maxTaskTotal; i++) {
        data->tools.task[i] = gtk_label_new("");
        data->state.taskNumber[i] = i;
    }
    for (int i = 0; i < data->state.maxProject; i++) {
        data->state.projectNumber[i] = i;
    }

    //Projects
    int queryResult = allProject(data->conn);
    if (queryResult == -1)
        g_print("Error: can't collect all projects\n");

    for (int i = 0; i < queryResult; i++) {
        addProject(GTK_WIDGET(data->tools.addProject), GTK_RESPONSE_OK, data, i);
    }
    data->state.repopulatedProject = 1;

    //Tasks
    queryResult = allTask(data->conn);
    if (queryResult == -1)
        g_print("Error: can't collect all tasks\n");

    for (int i = 0; i < queryResult; i++) {
        int taskToAdd = selectTaskId(data->conn, i);
        data->state.taskNumber[taskToAdd] = -1;
        char *project = selectProjectName(data->conn, taskToAdd);
        addTasks(GTK_WIDGET(data->tools.addTask), data, taskToAdd, project);
        addImportantTask(data, taskToAdd);
        addMinorTask(data, taskToAdd);
        addLateTask(data, taskToAdd);
        addPlannedTask(data, taskToAdd);
    }
    data->state.repopulatedTask = 1;

    //Update finance data
    if (newConnect == 1) {
        updateExpense(data->conn, 2, 0);
        if (newMonth == 1)
            updateExpense(data->conn, 3, 0);
    }
    updateFinance(data);

    //Warning message one time per day
    if (newConnect == 1) {

        newConnectUpdate(day, month, local_time->tm_year + 1900, data);

        gchar *message = warningMessage(data);
        if (strcmp(message, "empty") != 0) {
            GtkDialog *dialog = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(data->tools.window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", message));
            gtk_dialog_run(dialog);
            gtk_widget_destroy(GTK_WIDGET(dialog));
        }
        free(message);
    }
}

void clearData(GtkWidget *button, struct Data *data)
{
    PGresult *res = PQexec(data->conn, "DELETE FROM task");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        g_print("Error: delete task failed\n");
        return;
    }
    PQclear(res);
    res = PQexec(data->conn,
        "DELETE FROM project WHERE Name != 'Tâches' AND Name != 'En retard' AND Name != 'Importantes/Urgentes' AND Name != 'Mineures' AND Name != 'Prévues' AND Name != "
        "'Finance'");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        g_print("Error: delete projects failed\n");
        return;
    }
    PQclear(res);
    res = PQexec(data->conn, "UPDATE finance SET Value = 0");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        g_print("Error: delete finance failed\n");
        return;
    }
    PQclear(res);

    g_print("Data cleared!\n");
}

void checkEol(struct Data *data, const char *filename)
{
    FILE *fp;
    char *line = malloc(1000 * sizeof(char));
    if ((fp = fopen(filename, "r")) == NULL) {
        g_print("Error: unable to open file %s\n", filename);
        free(line);
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strchr(line, '\r') != NULL) {
            data->state.crlf = 1;
            break;
        }
        if (strchr(line, '\n') != NULL)
            data->state.crlf = 0;
    }
    free(line);
    fclose(fp);
}

void changeTaskStatus(GtkWidget *taskStatus, struct Data *data)
{
    GtkWidget *parent = gtk_widget_get_parent(taskStatus);
    GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
    GtkWidget *idButton = g_list_nth_data(children, 5);
    int id = atoi(gtk_button_get_label(GTK_BUTTON(idButton)));

    if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Non complété") == 0) {
        int queryResult = updateStatus(data->conn, 1, id);
        if (queryResult == -1) {
            g_print("Error: update status failed\n");
        }
        gtk_button_set_label(GTK_BUTTON(taskStatus), "En cours");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "En cours") == 0) {
        int queryResult = updateStatus(data->conn, 2, id);
        if (queryResult == -1)
            g_print("Error: update status failed\n");

        gtk_button_set_label(GTK_BUTTON(taskStatus), "Complété");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Complété") == 0) {
        int queryResult = updateStatus(data->conn, 3, id);
        if (queryResult == -1)
            g_print("Error: update status failed\n");

        gtk_button_set_label(GTK_BUTTON(taskStatus), "Abandonné");
    }
    else if (strcmp(gtk_button_get_label(GTK_BUTTON(taskStatus)), "Abandonné") == 0) {
        int queryResult = updateStatus(data->conn, 0, id);
        if (queryResult == -1)
            g_print("Error: update status failed\n");

        gtk_button_set_label(GTK_BUTTON(taskStatus), "Non complété");
    }

    //Recherche de tâches du groupe de dépendance
    size_t size = strlen(selectProjectName(data->conn, id)) + 1;
    char *projectName = malloc(size);
    snprintf(projectName, size, "%s", selectProjectName(data->conn, id));

    int dependGroup = selectDependGroup(data->conn, id);
    if (dependGroup != -1) {
        int amountOfDependance = AllDependGroup(data->conn, id, dependGroup);
        for (int i = 0; i < amountOfDependance; i++) {
            int dependanceId = selectIdFromDependGroup(data->conn, i, dependGroup, projectName);
            scanForIdForUpdate(data, dependanceId);
        }
    }
    scanForIdForUpdate(data, id);
}

void changeTaskPriority(GtkWidget *taskPriority, struct Data *data)
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

void editTaskWindow(GtkWidget *taskEdit, struct Data *data)
{
    GtkWidget *parent = gtk_widget_get_parent(taskEdit);
    GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
    GtkWidget *taskWidget = g_list_nth_data(children, 2);
    const gchar *taskName = gtk_label_get_text(GTK_LABEL(taskWidget));
    data->tools.inEditing = taskWidget;

    GtkWidget *idWidget = g_list_nth_data(children, 5);
    int id = atoi(gtk_button_get_label(GTK_BUTTON(idWidget)));
    data->state.inEditingId = id;

    GtkWidget *editWindow = gtk_dialog_new_with_buttons(taskName, NULL, GTK_DIALOG_MODAL, "Confirmer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *descriptionLabel = gtk_label_new("Description de la tâche");
    data->tools.descriptionEntry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(data->tools.descriptionEntry), 100);
    gtk_widget_set_size_request(data->tools.descriptionEntry, 200, 50);

    GtkWidget *dependLabel = gtk_label_new("Groupe de tâches");
    data->tools.dependEntry = gtk_entry_new();
    gtk_widget_set_size_request(data->tools.dependEntry, 200, 50);
    gtk_entry_set_max_length(GTK_ENTRY(data->tools.dependEntry), 5);

    GtkWidget *priorityButton = gtk_button_new();
    int priority = selectPriority(data->conn, id);

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
    g_signal_connect(priorityButton, "clicked", G_CALLBACK(changeTaskPriority), data->tools.descriptionEntry);

    gtk_entry_set_text(GTK_ENTRY(data->tools.descriptionEntry), selectDescription(data->conn, id));

    int groupNumber = selectDependGroup(data->conn, id);
    char *interDepend = malloc(sizeof(char) * 3);
    snprintf(interDepend, 3, "%d", selectDependGroup(data->conn, id));
    const gchar *dependGroup = interDepend;
    if (groupNumber == -1) {
        gtk_entry_set_text(GTK_ENTRY(data->tools.dependEntry), "");
    }
    else {
        gtk_entry_set_text(GTK_ENTRY(data->tools.dependEntry), dependGroup);
    }

    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), descriptionLabel);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), data->tools.descriptionEntry);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), priorityButton);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), dependLabel);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(editWindow))), data->tools.dependEntry);
    gtk_widget_show_all(editWindow);

    g_signal_connect(editWindow, "response", G_CALLBACK(editTaskDB), data);

    gtk_widget_set_tooltip_text(taskWidget, selectDescription(data->conn, id));
}

void editTaskDB(GtkDialog *window, gint clicked, struct Data *data)
{
    if (clicked == GTK_RESPONSE_OK) {
        GtkWidget *input = GTK_WIDGET(data->tools.descriptionEntry);
        const gchar *text = gtk_entry_get_text(GTK_ENTRY(input));
        GtkWidget *dependInput = GTK_WIDGET(data->tools.dependEntry);
        const gchar *dependText = gtk_entry_get_text(GTK_ENTRY(dependInput));
        int queryResult;

        GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(data->tools.descriptionEntry));
        GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
        GtkWidget *priorityButton = g_list_nth_data(children, 2);
        const gchar *setPriority = gtk_button_get_label(GTK_BUTTON(priorityButton));

        queryResult = updateDescription(data->conn, text, data->state.inEditingId);
        if (queryResult != 0) {
            g_print("Erreur lors de la mise à jour de la description\n");
            return;
        }

        if (strcmp(setPriority, "Mineure") == 0) {
            queryResult = updatePriority(data->conn, 0, data->state.inEditingId);
        }
        else if (strcmp(setPriority, "Normale") == 0) {
            queryResult = updatePriority(data->conn, 1, data->state.inEditingId);
        }
        else if (strcmp(setPriority, "Importante") == 0) {
            queryResult = updatePriority(data->conn, 2, data->state.inEditingId);
        }
        else if (strcmp(setPriority, "Urgente") == 0) {
            queryResult = updatePriority(data->conn, 3, data->state.inEditingId);
        }

        if (queryResult != 0) {
            g_print("Erreur lors de la mise à jour de la priorité\n");
            return;
        }

        int onlyDigits = 1;
        for (const gchar *p = dependText; *p != '\0'; p++) {
            if (!g_ascii_isdigit(*p)) {
                onlyDigits = 0;
                break;
            }
        }

        if (strcmp(dependText, "") != 0 && onlyDigits == 1 && atoi(dependText) >= 0) {
            queryResult = updateDependGroup(data->conn, data->state.inEditingId, atoi(dependText));
            if (queryResult != 0) {
                g_print("Erreur de la modification du groupe de dépendance\n");
                return;
            }
            if (AllDependGroup(data->conn, data->state.inEditingId, atoi(dependText)) > 1) {
                queryResult = refreshTaskInGroup(data->conn, data->state.inEditingId, atoi(dependText));
                if (queryResult != 0) {
                    return;
                }
                scanForIdForUpdate(data, data->state.inEditingId);
            }
        }
        else if (strcmp(dependText, "") == 0 && atoi(dependText) >= 0) {
            queryResult = updateDependGroup(data->conn, data->state.inEditingId, -1);
            if (queryResult != 0) {
                g_print("Erreur de la modification du groupe de dépendance\n");
                return;
            }
        }

        addImportantTask(data, data->state.inEditingId);
        addMinorTask(data, data->state.inEditingId);
        gtk_widget_set_tooltip_text(data->tools.inEditing, selectDescription(data->conn, data->state.inEditingId));
    }
    gtk_widget_destroy(GTK_WIDGET(window));
}

void deleteTask(GtkWidget *taskDelete, struct Data *data)
{
    GtkWidget *taskToDelete = gtk_widget_get_parent(taskDelete);
    GList *boxChildren = gtk_container_get_children(GTK_CONTAINER(taskToDelete));
    GtkWidget *numberToFree = g_list_nth_data(boxChildren, 5);
    g_list_free(boxChildren);

    int numberToChange = atoi(gtk_button_get_label(GTK_BUTTON(numberToFree)));
    data->state.taskNumber[numberToChange] = numberToChange;

    int queryResult = deleteTaskDB(data->conn, numberToChange);

    if (queryResult == -1) {
        g_print("Error: deleteTask failed\n");
        return;
    }

    scanForIdToDestroy(data, numberToChange);
    data->tools.task[numberToChange] = gtk_label_new("");
}

void deleteProject(GtkWidget *projectDelete, struct Data *data)
{
    GtkWidget *projectBox = gtk_widget_get_parent(projectDelete);
    GList *boxChildren = gtk_container_get_children(GTK_CONTAINER(projectBox));
    GtkWidget *child = g_list_nth_data(boxChildren, 0);
    GtkWidget *numberToFree = g_list_nth_data(boxChildren, 2);

    int numberToChange = atoi(gtk_button_get_label(GTK_BUTTON(numberToFree)));
    data->state.projectNumber[numberToChange] = numberToChange;

    gint totalPage = gtk_notebook_get_n_pages(GTK_NOTEBOOK(data->tools.notebook));
    const gchar *nameOfProject = gtk_label_get_text(GTK_LABEL(child));
    int numberToDelete = 0;

    for (int i = 0; i < totalPage; i++) {
        GtkWidget *curPage = gtk_notebook_get_nth_page(GTK_NOTEBOOK(data->tools.notebook), i);
        GtkWidget *boxPage = gtk_notebook_get_tab_label(GTK_NOTEBOOK(data->tools.notebook), curPage);
        if (GTK_IS_BOX(boxPage)) {
            boxChildren = gtk_container_get_children(GTK_CONTAINER(boxPage));
            const gchar *nameToSeek = gtk_label_get_text(g_list_nth_data(boxChildren, 0));
            if (g_strcmp0(nameToSeek, nameOfProject) == 0) {
                numberToDelete = i;
            }
        }
    }

    g_list_free(boxChildren);

    int queryResult = deleteAllTaskFromProject(data->conn, nameOfProject);
    if (queryResult == -1) {
        g_print("Error: delete all task from project failed\n");
        return;
    }

    queryResult = deleteProjectDB(data->conn, nameOfProject);
    if (queryResult == -1) {
        g_print("Error: deleteProject failed\n");
        return;
    }

    gtk_notebook_remove_page(GTK_NOTEBOOK(data->tools.notebook), numberToDelete);
    gtk_notebook_set_current_page(data->tools.notebook, 0);
    data->state.projectCount--;
}

void addTasks(GtkWidget *task, struct Data *data, int presentTask, char *presentProjectName)
{
    char *projectName = NULL;
    projectName = malloc((strlen(presentProjectName) + 1) * sizeof(char));
    strcpy(projectName, presentProjectName);

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
    snprintf(deadlineDate, 20, "%u-%u-%u", local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday);

    gchar *getText;
    if (data->state.repopulatedTask == 0) {

        gtk_notebook_set_current_page(data->tools.notebook, 0);

        for (int i = 0; i < allProject(data->conn); i++) {

            GtkWidget *projectPageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(data->tools.notebook), i + 7);
            GtkWidget *projectLabelBox = gtk_notebook_get_tab_label(GTK_NOTEBOOK(data->tools.notebook), projectPageBox);
            GList *projectBoxChildren = gtk_container_get_children(GTK_CONTAINER(projectLabelBox));
            GtkWidget *projectLabel = g_list_nth_data(projectBoxChildren, 0);
            const gchar *projectLabelName = gtk_label_get_label(GTK_LABEL(projectLabel));

            if (strcmp(projectLabelName, projectName) == 0)
                gtk_notebook_set_current_page(data->tools.notebook, i + 7);

            g_list_free(projectBoxChildren);
        }
    }

    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));
    GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(data->tools.notebook), currentPage); //boxV
    GList *children = gtk_container_get_children(GTK_CONTAINER(pageBox));
    int numberOfTask = g_list_length(children) - 3;
    GtkWidget *addProjectBox = g_list_last(children)->data;
    g_list_free(children);

    children = gtk_container_get_children(GTK_CONTAINER(addProjectBox));
    GtkWidget *entry = g_list_last(children)->data;
    g_list_free(children);

    getText = malloc(sizeof(gchar) * strlen(get_text_of_entry(entry)) + 1);
    strcpy(getText, get_text_of_entry(entry));

    if (strcmp(getText, "") == 0 && data->state.repopulatedTask == 1) {
        free(getText);
        free(projectName);
        return;
    }

    if (numberOfTask >= data->state.maxTaskPerProject) {
        free(getText);
        free(projectName);
        return;
    }

    //Attribution de l'id
    if (data->state.repopulatedTask == 1) {
        for (data->state.i = 0; data->state.i < data->state.maxTaskTotal; data->state.i++) {
            if (data->state.taskNumber[data->state.i] != -1) {
                data->state.taskNumber[data->state.i] = -1;
                break;
            }
        }
    }
    else if (data->state.repopulatedTask == 0) {
        data->state.taskNumber[presentTask] = -1;
        data->state.i = presentTask;
    }

    const gchar *name;
    if (currentPage < 5) {
        name = gtk_notebook_get_tab_label_text(data->tools.notebook, pageBox);
    }
    else {
        GtkWidget *projectBox = gtk_notebook_get_tab_label(data->tools.notebook, pageBox);
        GList *projectBoxList = gtk_container_get_children(GTK_CONTAINER(projectBox));
        GtkWidget *projectLabel = g_list_nth_data(projectBoxList, 0);
        name = gtk_label_get_label(GTK_LABEL(projectLabel));
    }

    data->tools.boxTask[data->state.i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pageBox), data->tools.boxTask[data->state.i], FALSE, FALSE, 0);
    gtk_box_reorder_child(GTK_BOX(pageBox), data->tools.boxTask[data->state.i], numberOfTask + 2);

    if (data->state.repopulatedTask == 1) {
        data->tools.taskStatus[data->state.i] = gtk_button_new_with_label("Non complété");
    }
    else if (data->state.repopulatedTask == 0) {
        int queryResult = selectStatus(data->conn, data->state.i);
        gchar *status;
        if (queryResult == 0) {
            status = "Non complété";
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
        data->tools.taskStatus[data->state.i] = gtk_button_new_with_label(status);
    }

    gtk_widget_set_margin_top(data->tools.taskStatus[data->state.i], 10);
    gtk_widget_set_margin_bottom(data->tools.taskStatus[data->state.i], 10);
    gtk_widget_set_size_request(data->tools.taskStatus[data->state.i], 150, -1);
    gtk_box_pack_start(GTK_BOX(data->tools.boxTask[data->state.i]), data->tools.taskStatus[data->state.i], FALSE, FALSE, 0);
    g_signal_connect(data->tools.taskStatus[data->state.i], "clicked", G_CALLBACK(changeTaskStatus), data);

    data->tools.taskSeparator1[data->state.i] = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(data->tools.taskSeparator1[data->state.i], 5, -1);
    gtk_box_pack_start(GTK_BOX(data->tools.boxTask[data->state.i]), data->tools.taskSeparator1[data->state.i], FALSE, FALSE, 0);

    if (data->state.repopulatedTask == 0) {
        gtk_label_set_text(GTK_LABEL(data->tools.task[data->state.i]), selectTask(data->conn, data->state.i));
        gtk_widget_set_tooltip_text(data->tools.task[data->state.i], selectDescription(data->conn, data->state.i));
    }
    else if (data->state.repopulatedTask == 1) {
        gtk_label_set_text(GTK_LABEL(data->tools.task[data->state.i]), getText);
        gtk_widget_set_tooltip_text(data->tools.task[data->state.i], "");
    }
    gtk_box_pack_start(GTK_BOX(data->tools.boxTask[data->state.i]), data->tools.task[data->state.i], TRUE, FALSE, 0);

    data->tools.taskEdit[data->state.i] = gtk_button_new_with_label("Editer");
    gtk_box_pack_start(GTK_BOX(data->tools.boxTask[data->state.i]), data->tools.taskEdit[data->state.i], FALSE, FALSE, 0);
    g_signal_connect(data->tools.taskEdit[data->state.i], "clicked", G_CALLBACK(editTaskWindow), data);

    data->tools.taskDelete[data->state.i] = gtk_button_new_with_label("X");
    gtk_box_pack_start(GTK_BOX(data->tools.boxTask[data->state.i]), data->tools.taskDelete[data->state.i], FALSE, FALSE, 0);
    g_signal_connect(data->tools.taskDelete[data->state.i], "clicked", G_CALLBACK(deleteTask), data);

    char numberToTransfer[3];
    snprintf(numberToTransfer, sizeof(numberToTransfer), "%d", data->state.i);
    GtkWidget *taskNumberMarker = gtk_button_new_with_label(numberToTransfer);
    gtk_box_pack_start(GTK_BOX(data->tools.boxTask[data->state.i]), taskNumberMarker, FALSE, FALSE, 0);

    data->tools.taskSeparator2[data->state.i] = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(data->tools.taskSeparator2[data->state.i], 5, -1);
    gtk_box_pack_start(GTK_BOX(data->tools.boxTask[data->state.i]), data->tools.taskSeparator2[data->state.i], FALSE, FALSE, 0);

    if (data->state.repopulatedTask == 0) {
        char *queryResult = selectDeadline(data->conn, data->state.i);
        data->tools.taskDeadline[data->state.i] = gtk_button_new_with_label(queryResult);
    }
    else if (data->state.repopulatedTask == 1) {
        data->tools.taskDeadline[data->state.i] = gtk_button_new_with_label(deadlineDate);
    }

    gtk_widget_set_margin_top(data->tools.taskDeadline[data->state.i], 10);
    gtk_widget_set_margin_bottom(data->tools.taskDeadline[data->state.i], 10);
    gtk_widget_set_size_request(data->tools.taskDeadline[data->state.i], 150, -1);
    gtk_box_pack_start(GTK_BOX(data->tools.boxTask[data->state.i]), data->tools.taskDeadline[data->state.i], FALSE, FALSE, 0);
    g_signal_connect(data->tools.taskDeadline[data->state.i], "clicked", G_CALLBACK(changeDeadlineWindow), data);

    gtk_widget_show_all(data->tools.boxTask[data->state.i]);
    gtk_widget_hide(taskNumberMarker);

    gtk_entry_set_text(GTK_ENTRY(entry), "");

    if (data->state.repopulatedTask == 1) {
        int queryResult = insertTask(data->conn, data->state.i, getText, "", 1, deadlineDate, 0, -1, name); //insert in db
        if (queryResult == -1)
            g_print("Error: insertTask failed\n");
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
        g_print("Error: config file not found\n");
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
            free(line);
            fclose(file);
            return atoi(&line[i + 1]);
        }
    }
    free(line);
    fclose(file);
    return -1;
}

void addProjectWindow(GtkWidget *project, struct Data *data)
{
    if (data->state.projectCount >= data->state.maxProject) {
        return;
    }

    GtkWidget *addProjectDialog
        = gtk_dialog_new_with_buttons("Nouveau projet", NULL, GTK_DIALOG_MODAL, "Confirmer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *nameLabel = gtk_label_new("Nom du projet");
    data->tools.projectNameEntry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(data->tools.projectNameEntry), 20);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(addProjectDialog))), nameLabel);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(addProjectDialog))), data->tools.projectNameEntry);
    gtk_widget_show_all(addProjectDialog);

    g_signal_connect(addProjectDialog, "response", G_CALLBACK(addProject), data);
}

void addProject(GtkWidget *projet, gint clicked, struct Data *data, int presentProject)
{
    if (clicked == GTK_RESPONSE_OK) {
        gchar *projectName = NULL;

        if (data->state.repopulatedProject == 1) {
            projectName = get_text_of_entry(data->tools.projectNameEntry);
            if (projectName == NULL) {
                g_print("Error: projectName is null\n");
                return;
            }
            if (projectExist(data->conn, projectName) == 1) {
                gtk_widget_destroy(projet);
                GtkDialog *dialog
                    = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(data->tools.window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Ce projet existe déjà"));
                gtk_dialog_run(dialog);
                gtk_widget_destroy(GTK_WIDGET(dialog));
                return;
            }

            int queryResult = insertProject(data->conn, projectName);
            if (queryResult == -1) {
                g_print("Error: insertProject failed\n");
                return;
            }
        }

        for (data->state.i = 0; data->state.i < data->state.maxProject; data->state.i++) {
            if (data->state.projectNumber[data->state.i] != -1) {
                data->state.projectNumber[data->state.i] = -1;
                break;
            }
        }

        data->tools.pageTitleBox[data->state.i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        GtkWidget *title = gtk_label_new("");
        if (data->state.repopulatedProject == 1) {
            gtk_label_set_text(GTK_LABEL(title), (const gchar *)projectName);
        }
        else if (data->state.repopulatedProject == 0) {
            gtk_label_set_text(GTK_LABEL(title), selectProject(data->conn, presentProject));
        }

        GtkWidget *titleButton = gtk_button_new_with_label("X");
        char numberToTransfer[3];
        snprintf(numberToTransfer, sizeof(numberToTransfer), "%d", data->state.i);
        GtkWidget *projectNumberMarker = gtk_button_new_with_label(numberToTransfer);

        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

        data->tools.projectTaskBox[data->state.i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start(GTK_BOX(box), data->tools.projectTaskBox[data->state.i], FALSE, FALSE, 0);
        GtkWidget *separatorH = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_widget_set_size_request(separatorH, -1, 5);
        gtk_box_pack_start(GTK_BOX(box), separatorH, FALSE, FALSE, 0);

        GtkWidget *status = gtk_label_new("Status");
        gtk_label_set_markup(GTK_LABEL(status), "<b>Status</b>");
        gtk_widget_set_margin_top(status, 10);
        gtk_widget_set_margin_bottom(status, 10);
        gtk_widget_set_size_request(status, 150, -1);
        gtk_box_pack_start(GTK_BOX(data->tools.projectTaskBox[data->state.i]), status, FALSE, FALSE, 0);

        GtkWidget *separatorV1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        gtk_widget_set_size_request(separatorV1, 5, -1);
        gtk_box_pack_start(GTK_BOX(data->tools.projectTaskBox[data->state.i]), separatorV1, FALSE, FALSE, 0);

        GtkWidget *projectTitle = gtk_label_new("");
        if (data->state.repopulatedProject == 1) {
            projectTitle = gtk_label_new(projectName);
        }
        else if (data->state.repopulatedProject == 0) {
            projectTitle = gtk_label_new(selectProject(data->conn, presentProject));
        }

        gtk_box_pack_start(GTK_BOX(data->tools.projectTaskBox[data->state.i]), projectTitle, TRUE, FALSE, 0);

        GtkWidget *separatorV2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        gtk_widget_set_size_request(separatorV2, 5, -1);
        gtk_box_pack_start(GTK_BOX(data->tools.projectTaskBox[data->state.i]), separatorV2, FALSE, FALSE, 0);

        GtkWidget *deadline = gtk_label_new("Date limite");
        gtk_label_set_markup(GTK_LABEL(deadline), "<b>Date limite</b>");
        gtk_widget_set_margin_top(deadline, 10);
        gtk_widget_set_margin_bottom(deadline, 10);
        gtk_widget_set_size_request(deadline, 150, -1);
        gtk_box_pack_start(GTK_BOX(data->tools.projectTaskBox[data->state.i]), deadline, FALSE, FALSE, 0);

        GtkWidget *boxAddTask = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        GtkWidget *calendar = gtk_button_new_with_label("Calendrier");
        GtkWidget *addButton = gtk_button_new_with_label("Ajouter la tâche");
        GtkWidget *inputEntry = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX(boxAddTask), calendar, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(boxAddTask), addButton, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(boxAddTask), inputEntry, TRUE, TRUE, 0);

        gtk_box_pack_start(GTK_BOX(box), boxAddTask, TRUE, TRUE, 0);
        gtk_widget_set_valign(boxAddTask, GTK_ALIGN_END);

        gtk_box_pack_start(GTK_BOX(data->tools.pageTitleBox[data->state.i]), title, TRUE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(data->tools.pageTitleBox[data->state.i]), titleButton, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(data->tools.pageTitleBox[data->state.i]), projectNumberMarker, FALSE, FALSE, 0);

        gtk_widget_show_all(data->tools.projectTaskBox[data->state.i]);
        gtk_widget_show_all(box);
        gtk_widget_show_all(GTK_WIDGET(data->tools.pageTitleBox[data->state.i]));
        gtk_widget_hide(projectNumberMarker);

        gint numberOfPage = gtk_notebook_get_n_pages(GTK_NOTEBOOK(data->tools.notebook));

        g_signal_connect(calendar, "clicked", G_CALLBACK(calendarDialog), data);
        g_signal_connect(addButton, "clicked", G_CALLBACK(addTasks), data);
        g_signal_connect(titleButton, "clicked", G_CALLBACK(deleteProject), data);

        gtk_notebook_insert_page(GTK_NOTEBOOK(data->tools.notebook), box, GTK_WIDGET(data->tools.pageTitleBox[data->state.i]), numberOfPage - 1);
        gtk_widget_show(GTK_WIDGET(data->tools.pageTitleBox[data->state.i]));
        gtk_widget_show(box);
        data->state.projectCount++;
        gtk_notebook_set_current_page(GTK_NOTEBOOK(data->tools.notebook), data->state.projectCount + 6);
    }
    if (data->state.repopulatedProject == 1)
        gtk_widget_destroy(projet);
}

void changeDeadlineWindow(GtkWidget *deadline, struct Data *data)
{
    GtkWidget *changeDeadlineDialog
        = gtk_dialog_new_with_buttons("Changer de date limite", NULL, GTK_DIALOG_MODAL, "Confirmer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *calendar = gtk_calendar_new();

    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(changeDeadlineDialog))), calendar);
    gtk_widget_show_all(changeDeadlineDialog);

    GtkWidget *parent = gtk_widget_get_parent(deadline);
    GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
    GtkWidget *idButton = g_list_nth_data(children, 5);
    int id = atoi(gtk_button_get_label(GTK_BUTTON(idButton)));
    data->state.inEditingId = id;

    g_signal_connect(changeDeadlineDialog, "response", G_CALLBACK(changeDeadline), data);
}

void changeDeadline(GtkWidget *deadline, gint clicked, struct Data *data)
{
    if (clicked == GTK_RESPONSE_OK) {
        GtkWidget *box = GTK_WIDGET(gtk_dialog_get_content_area(GTK_DIALOG(deadline)));
        GList *child = gtk_container_get_children(GTK_CONTAINER(box));
        GtkWidget *calendar = g_list_nth_data(child, 0);

        guint year, month, day;

        gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
        gchar *changedDeadline = malloc(11 * sizeof(gchar));
        snprintf(changedDeadline, 11, "%u-%u-%u", year, month + 1, day);
        updateDeadline(data->conn, data->state.inEditingId, changedDeadline);
        gtk_button_set_label(GTK_BUTTON(data->tools.taskDeadline[data->state.inEditingId]), changedDeadline);
        addLateTask(data, data->state.inEditingId);

        //Recherche de tâches du groupe de dépendance

        char *projectName = malloc(sizeof(char) * 1000);
        snprintf(projectName, 1000, "%s", selectProjectName(data->conn, data->state.inEditingId));
        int dependGroup = selectDependGroup(data->conn, data->state.inEditingId);
        if (dependGroup != -1) {
            int amountOfDependance = AllDependGroup(data->conn, data->state.inEditingId, dependGroup);

            for (int i = 0; i < amountOfDependance; i++) {
                int dependanceId = selectIdFromDependGroup(data->conn, i, dependGroup, projectName);
                scanForIdForUpdate(data, dependanceId);
            }
        }
        scanForIdForUpdate(data, data->state.inEditingId);
    }
    gtk_widget_destroy(deadline);
}

void addImportantTask(struct Data *data, int id)
{
    gint startingPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));
    gtk_notebook_set_current_page(data->tools.notebook, 1);

    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));
    GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(data->tools.notebook), currentPage); //boxV
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
            updateTask(data, boxTask, id);
            checkTask = 1;
        }
    }

    g_list_free(children);

    int priority = selectPriority(data->conn, id);
    if (priority <= 1) {
        if (checkTask == 1) {
            scanForIdToDestroySpecific(data, id, 1);
        }
        gtk_notebook_set_current_page(data->tools.notebook, startingPage);
        return;
    }

    if (checkTask == 1) //Pour que le GList se free
        return;

    GtkWidget *boxTask = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pageBox), boxTask, FALSE, FALSE, 0);

    int queryResult = selectStatus(data->conn, id);
    gchar *status;
    if (queryResult == 0) {
        status = "Non complété";
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
    else
        status = "Erreur";

    GtkWidget *statusButton = gtk_button_new_with_label(status);

    gtk_widget_set_margin_top(statusButton, 10);
    gtk_widget_set_margin_bottom(statusButton, 10);
    gtk_widget_set_size_request(statusButton, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), statusButton, FALSE, FALSE, 0);
    g_signal_connect(statusButton, "clicked", G_CALLBACK(changeTaskStatus), data);

    GtkWidget *taskSeparator1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator1, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator1, FALSE, FALSE, 0);

    GtkWidget *taskName = gtk_label_new(selectTask(data->conn, id));
    gtk_widget_set_tooltip_text(taskName, selectDescription(data->conn, id));
    gtk_box_pack_start(GTK_BOX(boxTask), taskName, TRUE, FALSE, 0);

    GtkWidget *taskEdit = gtk_button_new_with_label("Editer");
    gtk_box_pack_start(GTK_BOX(boxTask), taskEdit, FALSE, FALSE, 0);
    g_signal_connect(taskEdit, "clicked", G_CALLBACK(editTaskWindow), data);

    GtkWidget *taskDelete = gtk_button_new_with_label("X");
    gtk_box_pack_start(GTK_BOX(boxTask), taskDelete, FALSE, FALSE, 0);
    g_signal_connect(taskDelete, "clicked", G_CALLBACK(deleteTask), data);

    char numberToTransfer[3];
    snprintf(numberToTransfer, sizeof(numberToTransfer), "%d", id);
    GtkWidget *taskNumberMarker = gtk_button_new_with_label(numberToTransfer);
    gtk_box_pack_start(GTK_BOX(boxTask), taskNumberMarker, FALSE, FALSE, 0);

    GtkWidget *taskSeparator2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator2, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator2, FALSE, FALSE, 0);

    char *queryDeadline = selectDeadline(data->conn, id);
    GtkWidget *taskDeadline = gtk_button_new_with_label(queryDeadline);
    gtk_widget_set_margin_top(taskDeadline, 10);
    gtk_widget_set_margin_bottom(taskDeadline, 10);
    gtk_widget_set_size_request(taskDeadline, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskDeadline, FALSE, FALSE, 0);
    g_signal_connect(taskDeadline, "clicked", G_CALLBACK(changeDeadlineWindow), data);

    gtk_widget_show_all(boxTask);
    gtk_widget_hide(taskNumberMarker);

    gtk_notebook_set_current_page(data->tools.notebook, startingPage);
}

void addMinorTask(struct Data *data, int id)
{
    gint startingPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));

    gtk_notebook_set_current_page(data->tools.notebook, 2);

    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));
    GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(data->tools.notebook), currentPage); //boxV
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
            updateTask(data, boxTask, id);
            checkTask = 1;
        }
    }

    g_list_free(children);

    int priority = selectPriority(data->conn, id);
    if (priority > 0) {
        if (checkTask == 1) {
            scanForIdToDestroySpecific(data, id, 2);
        }
        gtk_notebook_set_current_page(data->tools.notebook, startingPage);
        return;
    }

    if (checkTask == 1) //Pour que le GList se free
        return;

    GtkWidget *boxTask = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pageBox), boxTask, FALSE, FALSE, 0);

    int queryResult = selectStatus(data->conn, id);
    gchar *status;
    if (queryResult == 0) {
        status = "Non complété";
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
    else
        status = "Erreur";

    GtkWidget *statusButton = gtk_button_new_with_label(status);

    gtk_widget_set_margin_top(statusButton, 10);
    gtk_widget_set_margin_bottom(statusButton, 10);
    gtk_widget_set_size_request(statusButton, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), statusButton, FALSE, FALSE, 0);
    g_signal_connect(statusButton, "clicked", G_CALLBACK(changeTaskStatus), data);

    GtkWidget *taskSeparator1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator1, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator1, FALSE, FALSE, 0);

    GtkWidget *taskName = gtk_label_new(selectTask(data->conn, id));
    gtk_widget_set_tooltip_text(taskName, selectDescription(data->conn, id));
    gtk_box_pack_start(GTK_BOX(boxTask), taskName, TRUE, FALSE, 0);

    GtkWidget *taskEdit = gtk_button_new_with_label("Editer");
    gtk_box_pack_start(GTK_BOX(boxTask), taskEdit, FALSE, FALSE, 0);
    g_signal_connect(taskEdit, "clicked", G_CALLBACK(editTaskWindow), data);

    GtkWidget *taskDelete = gtk_button_new_with_label("X");
    gtk_box_pack_start(GTK_BOX(boxTask), taskDelete, FALSE, FALSE, 0);
    g_signal_connect(taskDelete, "clicked", G_CALLBACK(deleteTask), data);

    char numberToTransfer[3];
    snprintf(numberToTransfer, sizeof(numberToTransfer), "%d", id);
    GtkWidget *taskNumberMarker = gtk_button_new_with_label(numberToTransfer);
    gtk_box_pack_start(GTK_BOX(boxTask), taskNumberMarker, FALSE, FALSE, 0);

    GtkWidget *taskSeparator2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator2, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator2, FALSE, FALSE, 0);

    char *queryDeadline = selectDeadline(data->conn, id);
    GtkWidget *taskDeadline = gtk_button_new_with_label(queryDeadline);
    gtk_widget_set_margin_top(taskDeadline, 10);
    gtk_widget_set_margin_bottom(taskDeadline, 10);
    gtk_widget_set_size_request(taskDeadline, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskDeadline, FALSE, FALSE, 0);
    g_signal_connect(taskDeadline, "clicked", G_CALLBACK(changeDeadlineWindow), data);

    gtk_widget_show_all(boxTask);
    gtk_widget_hide(taskNumberMarker);

    gtk_notebook_set_current_page(data->tools.notebook, startingPage);
}

void addLateTask(struct Data *data, int id)
{
    gint startingPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);

    gtk_notebook_set_current_page(data->tools.notebook, 3);

    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));
    GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(data->tools.notebook), currentPage); //boxV
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
            updateTask(data, boxTask, id);
            checkTask = 1;
        }
    }

    g_list_free(children);

    char *deadline = selectDeadline(data->conn, id);

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
            if (deadlineDay >= local_time->tm_mday)
                late = 0;
        }
    }

    if (late == 0) {
        if (checkTask == 1)
            scanForIdToDestroySpecific(data, id, 3);

        gtk_notebook_set_current_page(data->tools.notebook, startingPage);
        return;
    }

    if (checkTask == 1) //Pour que le GList se free
        return;

    GtkWidget *boxTask = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pageBox), boxTask, FALSE, FALSE, 0);

    int queryResult = selectStatus(data->conn, id);
    gchar *status;
    if (queryResult == 0) {
        status = "Non complété";
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
    else
        status = "Erreur";

    GtkWidget *statusButton = gtk_button_new_with_label(status);

    gtk_widget_set_margin_top(statusButton, 10);
    gtk_widget_set_margin_bottom(statusButton, 10);
    gtk_widget_set_size_request(statusButton, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), statusButton, FALSE, FALSE, 0);
    g_signal_connect(statusButton, "clicked", G_CALLBACK(changeTaskStatus), data);

    GtkWidget *taskSeparator1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator1, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator1, FALSE, FALSE, 0);

    GtkWidget *taskName = gtk_label_new(selectTask(data->conn, id));
    gtk_widget_set_tooltip_text(taskName, selectDescription(data->conn, id));
    gtk_box_pack_start(GTK_BOX(boxTask), taskName, TRUE, FALSE, 0);

    GtkWidget *taskEdit = gtk_button_new_with_label("Editer");
    gtk_box_pack_start(GTK_BOX(boxTask), taskEdit, FALSE, FALSE, 0);
    g_signal_connect(taskEdit, "clicked", G_CALLBACK(editTaskWindow), data);

    GtkWidget *taskDelete = gtk_button_new_with_label("X");
    gtk_box_pack_start(GTK_BOX(boxTask), taskDelete, FALSE, FALSE, 0);
    g_signal_connect(taskDelete, "clicked", G_CALLBACK(deleteTask), data);

    char numberToTransfer[3];
    snprintf(numberToTransfer, sizeof(numberToTransfer), "%d", id);
    GtkWidget *taskNumberMarker = gtk_button_new_with_label(numberToTransfer);
    gtk_box_pack_start(GTK_BOX(boxTask), taskNumberMarker, FALSE, FALSE, 0);

    GtkWidget *taskSeparator2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator2, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator2, FALSE, FALSE, 0);

    char *queryDeadline = selectDeadline(data->conn, id);
    GtkWidget *taskDeadline = gtk_button_new_with_label(queryDeadline);
    gtk_widget_set_margin_top(taskDeadline, 10);
    gtk_widget_set_margin_bottom(taskDeadline, 10);
    gtk_widget_set_size_request(taskDeadline, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskDeadline, FALSE, FALSE, 0);
    g_signal_connect(taskDeadline, "clicked", G_CALLBACK(changeDeadlineWindow), data);

    gtk_widget_show_all(boxTask);
    gtk_widget_hide(taskNumberMarker);

    gtk_notebook_set_current_page(data->tools.notebook, startingPage);
}

void addPlannedTask(struct Data *data, int id)
{
    gint startingPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);

    gtk_notebook_set_current_page(data->tools.notebook, 4);

    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));
    GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(data->tools.notebook), currentPage); //boxV
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
            updateTask(data, boxTask, id);
            checkTask = 1;
        }
    }

    g_list_free(children);

    char *deadline = selectDeadline(data->conn, id);

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
            if (deadlineDay >= local_time->tm_mday)
                late = 0;
        }
    }

    if (late == 1) {
        if (checkTask == 1)
            scanForIdToDestroySpecific(data, id, 4);

        gtk_notebook_set_current_page(data->tools.notebook, startingPage);
        return;
    }

    if (checkTask == 1) //Pour que le GList se free
        return;

    GtkWidget *boxTask = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pageBox), boxTask, FALSE, FALSE, 0);

    int queryResult = selectStatus(data->conn, id);
    gchar *status;
    if (queryResult == 0) {
        status = "Non complété";
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
    else
        status = "Erreur";

    GtkWidget *statusButton = gtk_button_new_with_label(status);

    gtk_widget_set_margin_top(statusButton, 10);
    gtk_widget_set_margin_bottom(statusButton, 10);
    gtk_widget_set_size_request(statusButton, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), statusButton, FALSE, FALSE, 0);
    g_signal_connect(statusButton, "clicked", G_CALLBACK(changeTaskStatus), data);

    GtkWidget *taskSeparator1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator1, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator1, FALSE, FALSE, 0);

    GtkWidget *taskName = gtk_label_new(selectTask(data->conn, id));
    gtk_widget_set_tooltip_text(taskName, selectDescription(data->conn, id));
    gtk_box_pack_start(GTK_BOX(boxTask), taskName, TRUE, FALSE, 0);

    GtkWidget *taskEdit = gtk_button_new_with_label("Editer");
    gtk_box_pack_start(GTK_BOX(boxTask), taskEdit, FALSE, FALSE, 0);
    g_signal_connect(taskEdit, "clicked", G_CALLBACK(editTaskWindow), data);

    GtkWidget *taskDelete = gtk_button_new_with_label("X");
    gtk_box_pack_start(GTK_BOX(boxTask), taskDelete, FALSE, FALSE, 0);
    g_signal_connect(taskDelete, "clicked", G_CALLBACK(deleteTask), data);

    char numberToTransfer[3];
    snprintf(numberToTransfer, sizeof(numberToTransfer), "%d", id);
    GtkWidget *taskNumberMarker = gtk_button_new_with_label(numberToTransfer);
    gtk_box_pack_start(GTK_BOX(boxTask), taskNumberMarker, FALSE, FALSE, 0);

    GtkWidget *taskSeparator2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(taskSeparator2, 5, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskSeparator2, FALSE, FALSE, 0);

    char *queryDeadline = selectDeadline(data->conn, id);
    GtkWidget *taskDeadline = gtk_button_new_with_label(queryDeadline);
    gtk_widget_set_margin_top(taskDeadline, 10);
    gtk_widget_set_margin_bottom(taskDeadline, 10);
    gtk_widget_set_size_request(taskDeadline, 150, -1);
    gtk_box_pack_start(GTK_BOX(boxTask), taskDeadline, FALSE, FALSE, 0);
    g_signal_connect(taskDeadline, "clicked", G_CALLBACK(changeDeadlineWindow), data);

    gtk_widget_show_all(boxTask);
    gtk_widget_hide(taskNumberMarker);

    gtk_notebook_set_current_page(data->tools.notebook, startingPage);
}

void scanForIdToDestroy(struct Data *data, int idToDestroy)
{
    gint startingPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));
    int numberOfProject = allProject(data->conn) + 7;

    for (int i = 0; i < numberOfProject; i++) {
        if (i != 5 && i != 6) { //le i == 5 c'est la page finance
            gtk_notebook_set_current_page(data->tools.notebook, i);

            gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));
            GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(data->tools.notebook), currentPage);
            GList *children = gtk_container_get_children(GTK_CONTAINER(pageBox));

            int numberOfTask;
            if (i == 1 || i == 2 || i == 3 || i == 4) {
                numberOfTask = g_list_length(children) - 2;
            }
            else {
                numberOfTask = g_list_length(children) - 3;
            }

            for (int j = 2; j < numberOfTask + 2; j++) {
                GtkWidget *boxTask = g_list_nth_data(children, j);
                GList *taskList = gtk_container_get_children(GTK_CONTAINER(boxTask));
                GtkWidget *idButton = g_list_nth_data(taskList, 5);
                g_list_free(taskList);

                int id = atoi(gtk_button_get_label(GTK_BUTTON(idButton)));
                if (id == idToDestroy)
                    gtk_widget_destroy(boxTask);
            }
            g_list_free(children);
        }
    }
    gtk_notebook_set_current_page(data->tools.notebook, startingPage);
}

void scanForIdToDestroySpecific(struct Data *data, int idToDestroy, guint project)
{
    gtk_notebook_set_current_page(data->tools.notebook, project);

    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));
    GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(data->tools.notebook), currentPage);
    GList *children = gtk_container_get_children(GTK_CONTAINER(pageBox));

    int numberOfTask;
    if (project == 1 || project == 2 || project == 3 || project == 4) {
        numberOfTask = g_list_length(children) - 2;
    }
    else {
        numberOfTask = g_list_length(children) - 3;
    }

    for (int j = 2; j < numberOfTask + 2; j++) {
        GtkWidget *boxTask = g_list_nth_data(children, j);
        GList *taskList = gtk_container_get_children(GTK_CONTAINER(boxTask));
        GtkWidget *idButton = g_list_nth_data(taskList, 5);
        g_list_free(taskList);

        int id = atoi(gtk_button_get_label(GTK_BUTTON(idButton)));
        if (id == idToDestroy)
            gtk_widget_destroy(boxTask);
    }
    g_list_free(children);
}

void scanForIdForUpdate(struct Data *data, int idToSeek)
{
    gint startingPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));
    int numberOfProject = allProject(data->conn) + 7;

    for (int i = 0; i < numberOfProject; i++) {
        if (i != 5 && i != 6) { //le i == 5 c'est la page finance et le i == 6 c'est la calculatrice
            gtk_notebook_set_current_page(data->tools.notebook, i);

            gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->tools.notebook));
            GtkWidget *pageBox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(data->tools.notebook), currentPage);
            GList *children = gtk_container_get_children(GTK_CONTAINER(pageBox));

            int numberOfTask;
            if (i == 1 || i == 2 || i == 3 || i == 4) {
                numberOfTask = g_list_length(children) - 2;
            }
            else {
                numberOfTask = g_list_length(children) - 3;
            }

            for (int j = 2; j < numberOfTask + 2; j++) {
                GtkWidget *boxTask = g_list_nth_data(children, j);
                GList *taskList = gtk_container_get_children(GTK_CONTAINER(boxTask));
                GtkWidget *idButton = g_list_nth_data(taskList, 5);
                g_list_free(taskList);

                int id = atoi(gtk_button_get_label(GTK_BUTTON(idButton)));
                if (id == idToSeek)
                    updateTask(data, boxTask, id);
            }
            g_list_free(children);
        }
    }
    gtk_notebook_set_current_page(data->tools.notebook, startingPage);
}

void updateTask(struct Data *data, GtkWidget *task, int id)
{
    GList *listOfWidget = gtk_container_get_children(GTK_CONTAINER(task));

    GtkWidget *taskStatus = g_list_nth_data(listOfWidget, 0);
    int queryResult = selectStatus(data->conn, id);
    gchar *status;
    if (queryResult == 0) {
        status = "Non complété";
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
    else
        status = "Erreur";

    gtk_button_set_label(GTK_BUTTON(taskStatus), status);

    GtkWidget *taskName = g_list_nth_data(listOfWidget, 2);
    gtk_widget_set_tooltip_text(taskName, selectDescription(data->conn, id));

    GtkWidget *taskDeadline = g_list_nth_data(listOfWidget, 7);
    char *deadline = selectDeadline(data->conn, id);
    gtk_button_set_label(GTK_BUTTON(taskDeadline), deadline);
}

void curlCalendar()
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    long unsigned yearInt = tm->tm_year + 1900;
    char year[5];
    snprintf(year, sizeof(year), "%lu", yearInt);
    CURL *curl;
    CURLcode res;
    char *url = malloc(60 * sizeof(char));
    strcpy(url, "https://cdn.vertex42.com/calendars/");
    strcat(url, year);
    strcat(url, "/");
    strcat(url, year);
    strcat(url, "-calendar.png");

    curl = curl_easy_init();
    if (curl) {
        char *outfile = "data/calendar.png";
        FILE *fp;
        fp = fopen(outfile, "wb");
        if (fp != NULL) {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
                g_print("Erreur lors de l'exécution de la requête cURL : %s\n", curl_easy_strerror(res));

            curl_easy_cleanup(curl);
            fclose(fp);
        }
        else
            g_print("Erreur lors de l'ouverture du fichier %s\n", outfile);
    }
}

void calendarDialog(GtkButton *calendar, struct Data *data)
{
    curlCalendar();
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Calendrier", GTK_WINDOW(data->tools.window), GTK_DIALOG_MODAL, NULL, GTK_RESPONSE_CLOSE, NULL);
    GtkWidget *contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *image = gtk_image_new_from_file("data/calendar.png");
    gtk_container_add(GTK_CONTAINER(contentArea), image);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

gchar *warningMessage(struct Data *data)
{
    int urgent = allUrgentTask(data->conn);
    int late = allLateTask(data->conn);
    gchar *message;
    char number[3];

    message = malloc(sizeof(char) * strlen("Vous avez ??? tâches urgentes à réaliser et ??? tâches en retard")); //Le message le plus long possible

    if (urgent != 0) {
        if (urgent == 1) {
            strcpy(message, "Vous avez 1 tâche urgente à réaliser");
        }
        else {
            strcpy(message, "Vous avez ");
            snprintf(number, sizeof(number), "%d", urgent);
            strcat(message, number);
            strcat(message, " tâches urgentes à réaliser");
        }

        if (late != 0) {
            if (late == 1) {
                strcat(message, " et 1 tâche en retard");
            }
            else {
                strcat(message, " et ");
                snprintf(number, sizeof(number), "%d", late);
                strcat(message, number);
                strcat(message, " tâches en retard");
            }
        }
    }
    else if (late != 0) {
        if (late == 1) {
            strcpy(message, "Vous avez 1 tâche en retard");
        }
        else {
            strcpy(message, "Vous avez ");
            snprintf(number, sizeof(number), "%d", late);
            strcat(message, number);
            strcat(message, " tâches en retard");
        }
    }
    else
        strcpy(message, "empty");

    return message;
}

int newConnectUpdate(char *day, char *month, int year, struct Data *data)
{
    FILE *file = fopen("settings/config.txt", "r+");
    char *line = NULL;
    size_t len = 0;
    char insert[4];
    while ((getline(&line, &len, file)) != -1) {
        if (strstr(line, "last connect day") != NULL) {
            if (data->state.crlf == 1)
                fseek(file, -4, SEEK_CUR);
            else
                fseek(file, -3, SEEK_CUR);
            if (data->state.crlf == 1) {
                if (atoi(day) < 10) {
                    fputc('0', file);
                    fprintf(file, "%d", atoi(day));
                }
                else
                    fprintf(file, "%d", atoi(day));
            }
            else
                fprintf(file, "%s", day);
        }
        if (strstr(line, "last connect month") != NULL) {
            if (data->state.crlf == 1)
                fseek(file, -4, SEEK_CUR);
            else
                fseek(file, -3, SEEK_CUR);

            if (data->state.crlf == 1) {
                if (atoi(month) < 10) {
                    fputc('0', file);
                    fprintf(file, "%d", atoi(month));
                }
                else
                    fprintf(file, "%d", atoi(month));
            }
            else
                fprintf(file, "%s", month);
        }
        if (strstr(line, "last connect year") != NULL) {
            if (data->state.crlf == 1)
                fseek(file, -6, SEEK_CUR);
            else
                fseek(file, -5, SEEK_CUR);

            snprintf(insert, sizeof(insert), "%d", year);
            fprintf(file, "%s", insert);
            break;
        }
    }
    fclose(file);
    return 0;
}

void financeButton(GtkButton *buttonPressed, struct Data *data)
{
    int type = 0;

    GtkWidget *box = gtk_widget_get_parent(GTK_WIDGET(buttonPressed));
    GtkWidget *alignment = gtk_widget_get_parent(box);
    GtkWidget *frame = gtk_widget_get_parent(alignment);
    const gchar *frameLabel = gtk_frame_get_label(GTK_FRAME(frame));

    if (strcmp(frameLabel, "Mettre un plafond journalier") == 0)
        type = 0;
    else if (strcmp(frameLabel, "Mettre un plafond mensuel") == 0)
        type = 1;
    else if (strcmp(frameLabel, "Vos dépenses") == 0)
        type = 2;
    else if (strcmp(frameLabel, "Retirer des dépenses") == 0)
        type = 3;

    const gchar *entry;
    if (type == 0)
        entry = gtk_entry_get_text(GTK_ENTRY(data->tools.dailyCapEntry));
    else if (type == 1)
        entry = gtk_entry_get_text(GTK_ENTRY(data->tools.monthlyCapEntry));
    else if (type == 2)
        entry = gtk_entry_get_text(GTK_ENTRY(data->tools.expenseEntry));
    else if (type == 3)
        entry = gtk_entry_get_text(GTK_ENTRY(data->tools.savedEntry));

    int amount = atoi(entry);
    int onlyDigits = 1;

    for (const gchar *p = entry; *p != '\0'; p++) {
        if (!g_ascii_isdigit(*p)) {
            onlyDigits = 0;
            break;
        }
    }

    gtk_entry_set_text(data->tools.dailyCapEntry, "");

    if (onlyDigits == 0) {
        GtkDialog *dialog
            = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(data->tools.window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Veuillez saisir des chiffres"));
        gtk_dialog_run(dialog);
        gtk_widget_destroy(GTK_WIDGET(dialog));
        return;
    }

    int exceededDaily = 0;
    int exceededMonthly = 0;

    if (type == 0 || type == 1) {
        int query = updateCap(data->conn, type, amount);
        if (query == -3) {
            GtkDialog *dialog = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(data->tools.window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "Vous avez saisit un plafond mensuel inférieur à votre plafond journalier."));
            gtk_dialog_run(dialog);
            gtk_widget_destroy(GTK_WIDGET(dialog));
            return;
        }
        if (query == -2) {
            GtkDialog *dialog = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(data->tools.window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "Vous avez saisit un plafond journalier supérieur à votre plafond mensuel."));
            gtk_dialog_run(dialog);
            gtk_widget_destroy(GTK_WIDGET(dialog));
            return;
        }

        if (type == 0)
            gtk_entry_set_text(GTK_ENTRY(data->tools.dailyCapEntry), "");
        if (type == 1)
            gtk_entry_set_text(GTK_ENTRY(data->tools.monthlyCapEntry), "");
    }
    else if (type == 2) {
        int dailyMoney = selectExpense(data->conn, 0);
        int monthlyMoney = selectExpense(data->conn, 1);
        int dailyAmount = amount + dailyMoney;
        int monthlyAmount = amount + monthlyMoney;
        updateExpense(data->conn, 2, dailyAmount);
        updateExpense(data->conn, 3, monthlyAmount);

        int cap = selectCap(data->conn, 2);
        int currentMoney = selectExpense(data->conn, 0);
        if (currentMoney > cap && cap != 0)
            exceededDaily = 1;

        cap = selectCap(data->conn, 3);
        currentMoney = selectExpense(data->conn, 1);
        if (currentMoney > cap && cap != 0)
            exceededMonthly = 1;

        gtk_entry_set_text(GTK_ENTRY(data->tools.expenseEntry), "");
    }
    else if (type == 3) {
        int dailyMoney = selectExpense(data->conn, 0);
        int monthlyMoney = selectExpense(data->conn, 1);

        if (amount > dailyMoney)
            amount = dailyMoney;

        int dailyAmount = dailyMoney - amount;
        int monthlyAmount = monthlyMoney - amount;

        updateExpense(data->conn, 2, dailyAmount);
        updateExpense(data->conn, 3, monthlyAmount);

        gtk_entry_set_text(GTK_ENTRY(data->tools.savedEntry), "");
    }

    if (exceededDaily == 1 && exceededMonthly == 1) {
        GtkDialog *dialog = GTK_DIALOG(gtk_message_dialog_new(
            GTK_WINDOW(data->tools.window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Attention, vous avez dépassé le plafond journalier et mensuel"));
        gtk_dialog_run(dialog);
        gtk_widget_destroy(GTK_WIDGET(dialog));
    }
    else if (exceededDaily == 1) {
        GtkDialog *dialog = GTK_DIALOG(gtk_message_dialog_new(
            GTK_WINDOW(data->tools.window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Attention, vous avez dépassé le plafond journalier"));
        gtk_dialog_run(dialog);
        gtk_widget_destroy(GTK_WIDGET(dialog));
    }
    else if (exceededMonthly == 1) {
        GtkDialog *dialog = GTK_DIALOG(gtk_message_dialog_new(
            GTK_WINDOW(data->tools.window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Attention, vous avez dépassé le plafond mensuel"));
        gtk_dialog_run(dialog);
        gtk_widget_destroy(GTK_WIDGET(dialog));
    }

    updateFinance(data);
}

void updateFinance(struct Data *data)
{
    char showMoney[25];

    int money = selectExpense(data->conn, 0);
    snprintf(showMoney, sizeof(showMoney), "%d", money);
    strcat(showMoney, " €");
    gtk_label_set_text(data->tools.dailyExpense, showMoney);

    money = selectExpense(data->conn, 1);
    snprintf(showMoney, sizeof(showMoney), "%d", money);
    strcat(showMoney, " €");
    gtk_label_set_text(data->tools.monthlyExpense, showMoney);

    int cap = selectCap(data->conn, 2);
    if (cap != 0) {
        snprintf(showMoney, sizeof(showMoney), "Plafond: %d", cap);
        strcat(showMoney, " €");
        gtk_label_set_text(data->tools.dailyCap, showMoney);
    }
    else
        gtk_label_set_text(data->tools.dailyCap, "Plafond: Aucun");

    cap = selectCap(data->conn, 3);
    if (cap != 0) {
        snprintf(showMoney, sizeof(showMoney), "Plafond: %d", cap);
        strcat(showMoney, " €");
        gtk_label_set_text(data->tools.monthlyCap, showMoney);
    }
    else
        gtk_label_set_text(data->tools.monthlyCap, "Plafond: Aucun");
}