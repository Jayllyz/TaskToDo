#include <stdio.h>

void inputString(char *string, int size) {
   if (string) {
      if (size > 0) {
         fgets(string, size, stdin);
         if (string[strlen(string) - 1] == '\n')
            string[strlen(string) - 1] = '\0';
         else
            fflush(stdin);
      } else
         string[0] = '\0';
   } else
      printf("Error: string is NULL");
}