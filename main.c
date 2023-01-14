/*
Author: Simon & Antony
Date: 24-11-2022
Description: Main file of our Todo list software
*/

#include "functions.h"
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    //Init
    gtk_init(&argc, &argv);
    struct Data data;
    data.conn = connectBdd();
    if (data.conn == NULL) {
        g_print("Error: can't connect to database");
        return EXIT_FAILURE;
    }

    data.state.crlf = 0;
    checkEol(&data, "settings/config.txt");
    if (readOneConfigValue("init db") == 0)
        createTables(data.conn, &data);

    //Home window
    data.home.builderHome = gtk_builder_new();
    gtk_builder_add_from_file(data.home.builderHome, "data/window_home.glade", NULL);
    data.home.windowHome = GTK_WIDGET(gtk_builder_get_object(data.home.builderHome, "window_home"));
    data.home.openHome = GTK_BUTTON(gtk_builder_get_object(data.home.builderHome, "openHome"));
    data.home.clearData = GTK_BUTTON(gtk_builder_get_object(data.home.builderHome, "clearData"));
    gtk_widget_show(data.home.windowHome);

    g_signal_connect(data.home.openHome, "clicked", G_CALLBACK(openApp), &data);
    g_signal_connect(data.home.clearData, "clicked", G_CALLBACK(clearData), &data);

    //Main window
    data.tools.builder = gtk_builder_new();
    gtk_builder_add_from_file(data.tools.builder, "data/window_main.glade", NULL);
    data.tools.window = GTK_WIDGET(gtk_builder_get_object(data.tools.builder, "window_main"));

    //Datas
    data.state.maxTaskTotal = readOneConfigValue("maxTaskTotal") > 0 ? readOneConfigValue("maxTaskTotal") : 200;
    data.state.maxTaskPerProject = readOneConfigValue("maxTaskPerProject") > 0 ? readOneConfigValue("maxTaskPerProject") : 15;
    data.state.maxProject = readOneConfigValue("maxProject") > 0 ? readOneConfigValue("maxProject") : 10;
    data.tools.addTask = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "addTask"));
    data.tools.addProject = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "addProject"));
    data.tools.boxV = GTK_BOX(gtk_builder_get_object(data.tools.builder, "boxV"));
    data.state.i = 0;
    data.tools.inputEntry = GTK_WIDGET(gtk_builder_get_object(data.tools.builder, "inputEntry"));
    data.tools.notebook = GTK_NOTEBOOK(gtk_builder_get_object(data.tools.builder, "project_notebook"));
    data.tools.calendar = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "calendar"));
    //Calculator
    data.calc.clear = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "clear"));
    data.calc.plus = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "plus"));
    data.calc.minus = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "minus"));
    data.calc.multiply = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "multiply"));
    data.calc.divide = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "divide"));
    data.calc.equal = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "equal"));
    data.calc.zero = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "zero"));
    data.calc.one = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "one"));
    data.calc.two = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "two"));
    data.calc.three = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "three"));
    data.calc.four = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "four"));
    data.calc.five = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "five"));
    data.calc.six = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "six"));
    data.calc.seven = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "seven"));
    data.calc.eight = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "eight"));
    data.calc.nine = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "nine"));
    data.calc.txtResult = GTK_LABEL(gtk_builder_get_object(data.tools.builder, "txtResult"));
    data.calc.firstNumber = -1;
    data.calc.secondNumber = -1;
    data.calc.result = 0;
    data.calc.resultB = 0;
    data.calc.op = '0';

    //Finances
    data.tools.dailyCap = GTK_LABEL(gtk_builder_get_object(data.tools.builder, "dailyCap"));
    data.tools.monthlyCap = GTK_LABEL(gtk_builder_get_object(data.tools.builder, "monthlyCap"));
    data.tools.dailyExpense = GTK_LABEL(gtk_builder_get_object(data.tools.builder, "dailyExpense"));
    data.tools.monthlyExpense = GTK_LABEL(gtk_builder_get_object(data.tools.builder, "monthlyExpense"));
    data.tools.setDaily = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "setDaily"));
    data.tools.setMonthly = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "setMonthly"));
    data.tools.setExpense = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "setExpense"));
    data.tools.desetExpense = GTK_BUTTON(gtk_builder_get_object(data.tools.builder, "desetExpense"));
    data.tools.dailyCapEntry = GTK_ENTRY(gtk_builder_get_object(data.tools.builder, "dailyCapEntry"));
    data.tools.monthlyCapEntry = GTK_ENTRY(gtk_builder_get_object(data.tools.builder, "monthlyCapEntry"));
    data.tools.expenseEntry = GTK_ENTRY(gtk_builder_get_object(data.tools.builder, "expenseEntry"));
    data.tools.savedEntry = GTK_ENTRY(gtk_builder_get_object(data.tools.builder, "savedEntry"));

    data.state.repopulatedTask = 0;
    data.state.repopulatedProject = 0;
    data.state.projectCount = 0;

    gtk_entry_set_max_length(GTK_ENTRY(data.tools.inputEntry), 35);
    gtk_entry_set_max_length(GTK_ENTRY(data.tools.dailyCapEntry), 10);
    gtk_entry_set_max_length(GTK_ENTRY(data.tools.monthlyCapEntry), 10);
    gtk_entry_set_max_length(GTK_ENTRY(data.tools.expenseEntry), 10);
    gtk_entry_set_max_length(GTK_ENTRY(data.tools.savedEntry), 10);

    gtk_notebook_set_current_page(data.tools.notebook, 0);

    g_signal_connect(data.tools.addTask, "clicked", G_CALLBACK(addTasks), &data);
    g_signal_connect(data.tools.addProject, "clicked", G_CALLBACK(addProjectWindow), &data);
    g_signal_connect(data.tools.calendar, "clicked", G_CALLBACK(calendarDialog), &data);
    //calculator
    g_signal_connect(data.calc.clear, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.plus, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.minus, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.multiply, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.divide, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.equal, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.zero, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.one, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.two, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.three, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.four, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.five, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.six, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.seven, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.eight, "clicked", G_CALLBACK(btnClicked), &data);
    g_signal_connect(data.calc.nine, "clicked", G_CALLBACK(btnClicked), &data);

    g_signal_connect(data.tools.setDaily, "clicked", G_CALLBACK(financeButton), &data);
    g_signal_connect(data.tools.setMonthly, "clicked", G_CALLBACK(financeButton), &data);
    g_signal_connect(data.tools.setExpense, "clicked", G_CALLBACK(financeButton), &data);
    g_signal_connect(data.tools.desetExpense, "clicked", G_CALLBACK(financeButton), &data);

    g_signal_connect(data.home.windowHome, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(data.tools.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(data.tools.builder, NULL);

    g_object_unref(data.tools.builder);

    gtk_main();

    return 0;
}