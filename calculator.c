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
    if (dataP->calc.result != 0 && dataP->calc.resultB == 1) {
        dataP->calc.firstNumber = (int)dataP->calc.result > 0 ? (int)dataP->calc.result : (int)(dataP->calc.result - 1) + 1;
        dataP->calc.result = 0;
        dataP->calc.operator= '0';
        dataP->calc.secondNumber = 0;
    }
    if (strcmp(text, "C") == 0) {
        clearCalc(dataP);
        gtk_label_set_text(dataP->calc.txtResult, "");
    }
    else if (strcmp(text, "=") == 0) {
        calculate(dataP);
        gtk_label_set_text(dataP->calc.txtResult, "");
        showResult(dataP->calc.txtResult, dataP);
    }
    else if (strcmp(text, "+") == 0 || strcmp(text, "-") == 0 || strcmp(text, "*") == 0 || strcmp(text, "/") == 0) {
        addOperator(text, dataP);
    }
    else {
        if (dataP->calc.result != 0 || dataP->calc.resultB == 1)
            clearCalc(dataP);

        addDigit(text, dataP);
    }
}

void addDigit(const char *digit, gpointer data)
{
    int temp = atoi(digit);
    int i = 0;
    struct data *dataP = data;
    if (dataP->calc.operator== '0') {

        if (dataP->calc.firstNumber != 0)
            dataP->calc.firstNumber = dataP->calc.firstNumber * 10 + atoi(digit);
        else
            dataP->calc.firstNumber = atoi(digit);

        while (temp > 0) {
            temp /= 10;
            ++i;
        }
        char *first = malloc(i + 1 * sizeof(char));
        sprintf(first, "%d", dataP->calc.firstNumber);
        gtk_label_set_text(dataP->calc.txtResult, first);
    }
    if (dataP->calc.operator!= '0' && dataP->calc.firstNumber != 0) {

        if (dataP->calc.secondNumber != 0) {
            char *label = (char *)gtk_label_get_text(dataP->calc.txtResult);
            char *newLabel = malloc(strlen(label) + strlen(digit) + 1);
            label[strlen(label) - 1] = '\0';
            strcpy(newLabel, label);
            dataP->calc.secondNumber = (dataP->calc.secondNumber * 10) + atoi(digit);
        }
        else
            dataP->calc.secondNumber = atoi(digit);

        char *second = malloc(i + 1 * sizeof(char));
        sprintf(second, "%d", dataP->calc.secondNumber);
        char *label = (char *)gtk_label_get_text(dataP->calc.txtResult);
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
    char *label = (char *)gtk_label_get_text(dataP->calc.txtResult);
    char *newLabel = malloc(strlen(label) + strlen(operatorString) + 1);
    strcpy(newLabel, label);
    strcat(newLabel, operatorString);
    gtk_label_set_text(dataP->calc.txtResult, newLabel);
}

void calculate(gpointer data)
{
    struct data *dataP = data;
    g_print("first: %d\n", dataP->calc.firstNumber);
    g_print("operator: %c\n", dataP->calc.operator);
    g_print("second: %d\n", dataP->calc.secondNumber);
    switch (dataP->calc.operator) {
    case '+':
        dataP->calc.result = dataP->calc.firstNumber + dataP->calc.secondNumber;
        dataP->calc.resultB = 1;
        break;
    case '-':
        dataP->calc.result = dataP->calc.firstNumber - dataP->calc.secondNumber;
        dataP->calc.resultB = 1;
        break;
    case '*':
        dataP->calc.result = dataP->calc.firstNumber * dataP->calc.secondNumber;
        dataP->calc.resultB = 1;
        break;
    case '/':
        dataP->calc.result = (double)dataP->calc.firstNumber / (double)dataP->calc.secondNumber;
        dataP->calc.resultB = 1;
        break;
    default:
        dataP->calc.result = 0;
        break;
    }
}

void clearCalc(gpointer data)
{
    struct data *dataP = data;

    dataP->calc.firstNumber = 0;
    dataP->calc.secondNumber = 0;
    dataP->calc.result = 0;
    dataP->calc.operator= '0';
    dataP->calc.resultB = 1;
}

void showResult(GtkLabel *outputLabel, gpointer data)
{
    struct data *dataP = data;
    double tempResult = dataP->calc.result;
    int tempFirst = dataP->calc.firstNumber;
    int tempSecond = dataP->calc.secondNumber;
    int iR = 0;
    int iF = 0;
    int iS = 0;
    while (tempResult > 0) {
        tempResult /= 10;
        ++iR;
    }
    while (tempFirst > 0) {
        tempFirst /= 10;
        ++iF;
    }
    while (tempSecond > 0) {
        tempSecond /= 10;
        ++iS;
    }
    if (dataP->calc.firstNumber == 0 && dataP->calc.secondNumber == 0 && dataP->calc.result == 0 && dataP->calc.resultB == 0)
        return;
    if (dataP->calc.result != 0 || dataP->calc.resultB == 1) {
        char *result = malloc(iR + 1 * sizeof(char));
        sprintf(result, "%.2lf", dataP->calc.result);
        g_print("result: %s\n", result);
        gtk_label_set_text(outputLabel, result);
    }
    else {
        char *first = malloc(iF + 1 * sizeof(char));
        char operator[2];
        char *second = malloc(iS + 1 * sizeof(char));
        sprintf(first, "%d", dataP->calc.firstNumber);
        gtk_label_set_text(outputLabel, first);
        sprintf(operator, "%c", dataP->calc.operator);
        gtk_label_set_text(outputLabel, operator);
        sprintf(second, "%d", dataP->calc.secondNumber);
        gtk_label_set_text(outputLabel, second);
    }
}
