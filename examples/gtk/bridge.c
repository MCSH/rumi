#include <gtk/gtk.h>

GApplication* gtk_application_to_gapplication(GtkApplication *app) {
  return G_APPLICATION(app);
}

GtkWindow* gtk_widget_to_window(GtkWidget *widget) {
  return (GtkWindow*)GTK_WINDOW(widget);
}
