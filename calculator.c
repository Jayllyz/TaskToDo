#include "functions.h"
#include <curl/curl.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void btnClicked(GtkButton *button, gpointer data)
{
    struct data *dataP = data;
    const char *text = gtk_button_get_label(button);
    if (strcmp(text, "C") == 0) {
        g_print("test");
        clearCalc(dataP);
        gtk_label_set_text(dataP->calc.txtResult, "");
    }
    else if (strcmp(text, "=") == 0) {
        g_print("ici");
        calculate(dataP);
        gtk_label_set_text(dataP->calc.txtResult, "");
        showResult(dataP->calc.txtResult, dataP);
    }
    else if (strcmp(text, "+") == 0 || strcmp(text, "-") == 0 || strcmp(text, "*") == 0 || strcmp(text, "/") == 0) {
        addOperator(text, dataP);
    }
    else {
        if (dataP->calc.result != 0)
            clearCalc(dataP);
        addDigit(text, dataP);
    }
}

void addDigit(const char *digit, gpointer data)
{
    struct data *dataP = data;
    if (dataP->calc.operator== '0') {
        dataP->calc.firstNumber = atoi(digit);
        char first[100];
        sprintf(first, "%d", dataP->calc.firstNumber);
        gtk_label_set_text(dataP->calc.txtResult, first);
    }
    else {
        dataP->calc.secondNumber = atoi(digit);
        char second[100];
        sprintf(second, "%d", dataP->calc.secondNumber);
        char *label = gtk_label_get_text(dataP->calc.txtResult);
        char *newLabel = malloc(strlen(label) + strlen(second) + 1);
        strcpy(newLabel, label);
        strcat(newLabel, second);
        gtk_label_set_text(dataP->calc.txtResult, newLabel);
    }
}

void addOperator(const char *operator, gpointer data)
{
    struct data *dataP = data;

    dataP->calc.operator=(char) operator[0];
    char operatorString[2];
    sprintf(operatorString, "%c", dataP->calc.operator);
    char *label = gtk_label_get_text(dataP->calc.txtResult);
    char *newLabel = malloc(strlen(label) + strlen(operatorString) + 1);
    strcpy(newLabel, label);
    strcat(newLabel, operatorString);
    gtk_label_set_text(dataP->calc.txtResult, newLabel);
}

void calculate(gpointer data)
{
    struct data *dataP = data;

    switch (dataP->calc.operator) {
    case '+':
        dataP->calc.result = dataP->calc.firstNumber + dataP->calc.secondNumber;
        break;
    case '-':
        dataP->calc.result = dataP->calc.firstNumber - dataP->calc.secondNumber;
        break;
    case '*':
        dataP->calc.result = dataP->calc.firstNumber * dataP->calc.secondNumber;
        break;
    case '/':
        dataP->calc.result = dataP->calc.firstNumber / dataP->calc.secondNumber;
        break;
    }
    g_print("F:%d\n", dataP->calc.firstNumber);
    g_print("O:%c\n", dataP->calc.operator);
    g_print("S:%d\n", dataP->calc.secondNumber);
}

void clearCalc(gpointer data)
{
    struct data *dataP = data;

    dataP->calc.firstNumber = 0;
    dataP->calc.secondNumber = 0;
    dataP->calc.result = 0;
    dataP->calc.operator= '0';
}

void showResult(GtkLabel *outputLabel, gpointer data)
{
    struct data *dataP = data;
    if (dataP->calc.firstNumber == 0 && dataP->calc.secondNumber == 0 && dataP->calc.result == 0)
        return;
    if (dataP->calc.firstNumber != 0 && dataP->calc.secondNumber != 0 && dataP->calc.result != 0) {
        char result[100];
        sprintf(result, "%.2lf", dataP->calc.result);
        gtk_label_set_text(outputLabel, result);
    }
    else {
        char first[100];
        char operator[2];
        char second[100];
        sprintf(first, "%d", dataP->calc.firstNumber);
        gtk_label_set_text(outputLabel, first);
        sprintf(operator, "%c", dataP->calc.operator);
        gtk_label_set_text(outputLabel, operator);
        sprintf(second, "%d", dataP->calc.secondNumber);
        gtk_label_set_text(outputLabel, second);
    }
}
