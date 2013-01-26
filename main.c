#include <gtk/gtk.h>
#include "paperwidget.h"

int
main(int argc, char **argv) {
  GtkWidget *window;
  gra_paper_widget *paper;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  gtk_window_set_title(GTK_WINDOW(window), "Paper");

  paper = gra_paper_widget_new(NULL);
  
  gtk_container_add(GTK_CONTAINER(window), paper->widget);
  gtk_widget_show(paper->widget);
  gtk_widget_show(window);

  gtk_main();

  return 0;
}
