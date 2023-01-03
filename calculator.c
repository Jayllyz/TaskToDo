#include "functions.h"
#include <gtk/gtk.h>
#include <stdio.h>

void btnClicked(GtkButton *button, struct Data *data)
{
    const char *text = gtk_button_get_label(button);
    if (data->calc.resultB == 1) {
        data->calc.firstNumber = (int)data->calc.result > 0 ? (int)data->calc.result : (int)(data->calc.result - 1) + 1;
        data->calc.resultB = 0;
        data->calc.firstB = 1;
        data->calc.operator= '0';
        data->calc.secondNumber = 0;
    }
    if (strcmp(text, "C") == 0) {
        clearCalc(data);
        gtk_label_set_text(data->calc.txtResult, "");
    }
    else if (strcmp(text, "=") == 0) {
        calculate(data);
        gtk_label_set_text(data->calc.txtResult, "");
        showResult(data->calc.txtResult, data);
    }
    else if (strcmp(text, "+") == 0 || strcmp(text, "-") == 0 || strcmp(text, "*") == 0 || strcmp(text, "/") == 0) {
        if (data->calc.firstB == 1)
            addOperator(text, data);
    }
    else {
        if (data->calc.resultB == 1)
            clearCalc(data);

        addDigit(text, data);
    }
}

void addDigit(const char *digit, struct Data *data)
{
    int temp = atoi(digit);
    int i = 0;

    if (data->calc.operator== '0') {

        if (data->calc.firstB == 1)
            data->calc.firstNumber = data->calc.firstNumber * 10 + atoi(digit);
        else
            data->calc.firstNumber = atoi(digit);

        while (temp > 0) {
            temp /= 10;
            ++i;
        }
        char *first = malloc(i + 1 * sizeof(char));
        data->calc.firstB = 1;
        sprintf(first, "%d", data->calc.firstNumber);
        gtk_label_set_text(data->calc.txtResult, first);
    }
    if (data->calc.operator!= '0' && data->calc.firstNumber != 0) {

        if (data->calc.secondNumber != 0) {
            char *label = (char *)gtk_label_get_text(data->calc.txtResult);
            char *newLabel = malloc(strlen(label) + strlen(digit) + 1);
            label[strlen(label) - 1] = '\0';
            strcpy(newLabel, label);
            data->calc.secondNumber = (data->calc.secondNumber * 10) + atoi(digit);
        }
        else
            data->calc.secondNumber = atoi(digit);

        char *second = malloc(i + 1 * sizeof(char));
        sprintf(second, "%d", data->calc.secondNumber);
        char *label = (char *)gtk_label_get_text(data->calc.txtResult);
        char *newLabel = malloc(strlen(label) + strlen(second) + 1);
        strcpy(newLabel, label);
        strcat(newLabel, second);
        gtk_label_set_text(data->calc.txtResult, newLabel);
    }
}

void addOperator(const char *operator, struct Data * data)
{
    data->calc.operator=(char) operator[0];
    char operatorString[2];
    sprintf(operatorString, "%c", data->calc.operator);
    char *label = (char *)gtk_label_get_text(data->calc.txtResult);
    char *newLabel = malloc(strlen(label) + strlen(operatorString) + 1);
    strcpy(newLabel, label);
    strcat(newLabel, operatorString);
    gtk_label_set_text(data->calc.txtResult, newLabel);
}

void calculate(struct Data *data)
{
    switch (data->calc.operator) {
    case '+':
        data->calc.result = data->calc.firstNumber + data->calc.secondNumber;
        data->calc.resultB = 1;
        break;
    case '-':
        data->calc.result = data->calc.firstNumber - data->calc.secondNumber;
        data->calc.resultB = 1;
        break;
    case '*':
        data->calc.result = data->calc.firstNumber * data->calc.secondNumber;
        data->calc.resultB = 1;
        break;
    case '/':
        data->calc.result = (double)data->calc.firstNumber / (double)data->calc.secondNumber;
        data->calc.resultB = 1;
        break;
    default:
        data->calc.result = 0;
        break;
    }
}

void clearCalc(struct Data *data)
{
    data->calc.firstNumber = 0;
    data->calc.firstB = 0;
    data->calc.secondNumber = 0;
    data->calc.result = 0;
    data->calc.operator= '0';
    data->calc.resultB = 0;
}

void showResult(GtkLabel *outputLabel, struct Data *data)
{
    double tempResult = data->calc.result;
    int tempFirst = data->calc.firstNumber;
    int tempSecond = data->calc.secondNumber;
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
    if (data->calc.firstNumber == 0 && data->calc.secondNumber == 0 && data->calc.result == 0 && data->calc.resultB == 0)
        return;
    if (data->calc.result != 0 || data->calc.resultB == 1) {
        char *result = malloc(iR + 1 * sizeof(char));
        sprintf(result, "%.2lf", data->calc.result);
        gtk_label_set_text(outputLabel, result);
    }
    else {
        char *first = malloc(iF + 1 * sizeof(char));
        char operator[2];
        char *second = malloc(iS + 1 * sizeof(char));
        sprintf(first, "%d", data->calc.firstNumber);
        gtk_label_set_text(outputLabel, first);
        sprintf(operator, "%c", data->calc.operator);
        gtk_label_set_text(outputLabel, operator);
        sprintf(second, "%d", data->calc.secondNumber);
        gtk_label_set_text(outputLabel, second);
    }
}
