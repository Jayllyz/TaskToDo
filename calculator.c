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
        data->calc.op = '0';
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

    while (temp > 0) {
        temp /= 10;
        ++i;
    }
    if (data->calc.op == '0') {

        if (data->calc.firstB == 1)
            data->calc.firstNumber = data->calc.firstNumber * 10 + atoi(digit);
        else
            data->calc.firstNumber = atoi(digit);

        char *first = malloc(i + 1 * sizeof(char));
        data->calc.firstB = 1;
        snprintf(first, i + 1, "%d", data->calc.firstNumber);
        gtk_label_set_text(data->calc.txtResult, first);
        free(first);
    }
    if (data->calc.op != '0' && data->calc.firstNumber != 0) {

        if (data->calc.secondNumber != 0)
            data->calc.secondNumber = (data->calc.secondNumber * 10) + atoi(digit);
        else
            data->calc.secondNumber = atoi(digit);

        char *second = malloc(i + 1 * sizeof(char));
        snprintf(second, i + 1, "%d", data->calc.secondNumber);
        char *label = (char *)gtk_label_get_text(data->calc.txtResult);
        char *newLabel = malloc(strlen(label) + strlen(second) + 10);
        strcpy(newLabel, label);
        strcat(newLabel, second);
        gtk_label_set_text(data->calc.txtResult, newLabel);
        free(second);
        free(newLabel);
    }
}

void addOperator(const char *op, struct Data *data)
{
    data->calc.op = (char)op[0];
    char *operatorString = malloc(2 * sizeof(char));
    snprintf(operatorString, 2, "%c", data->calc.op);
    char *label = (char *)gtk_label_get_text(data->calc.txtResult);
    char *newLabel = malloc(strlen(label) + strlen(operatorString) + 1);
    strcpy(newLabel, label);
    strcat(newLabel, operatorString);
    gtk_label_set_text(data->calc.txtResult, newLabel);
    free(newLabel);
    free(operatorString);
}

void calculate(struct Data *data)
{
    switch (data->calc.op) {
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
    data->calc.op = '0';
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
        snprintf(result, iR + 1, "%.2lf", data->calc.result);
        gtk_label_set_text(outputLabel, result);
        free(result);
    }
    else {
        char *first = malloc(iF + 1 * sizeof(char));
        char *op = malloc(2 * sizeof(char));
        char *second = malloc(iS + 1 * sizeof(char));
        snprintf(first, iF + 1, "%d", data->calc.firstNumber);
        gtk_label_set_text(outputLabel, first);
        snprintf(op, 2, "%c", data->calc.op);
        gtk_label_set_text(outputLabel, op);
        snprintf(second, iS + 1, "%d", data->calc.secondNumber);
        gtk_label_set_text(outputLabel, second);
        free(first);
        free(second);
        free(op);
    }
}
