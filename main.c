/*
Author: Simon & Antony
Date: 24-11-2022
Description: Main file of our Todo list software
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

    //J'ai declare une variable en global, faut surement pas faire mais je sais pas comment envoyer cette
    //variable en parametre dans la fonction click_projects en appuyant dans le bouton
    //je verrai surement plus tard
    GtkLabel *label_test;

int main(int argc, char *argv[]) {
   GtkBuilder *builder;

   gtk_init(&argc, &argv);

   builder = gtk_builder_new();
   gtk_builder_add_from_file(builder, "data/window_main.glade", NULL);

    //Variables
    
    GtkWidget *window;
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    label_test = GTK_LABEL(gtk_builder_get_object(builder, "test_label"));

   gtk_builder_connect_signals(builder, NULL);

   g_object_unref(builder);

   gtk_widget_show(window);
   gtk_main();

   return 0;
}

// called when window is closed
void on_window_main_destroy() { gtk_main_quit(); }

void click_projects(){
    printf("test");
    gtk_label_set_text(label_test, "Projet ajouté");
}