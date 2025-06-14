#include <gtk/gtk.h>
#include <poppler.h>

#include "ui.h"
#include "pdf.h"

void on_activate(GtkApplication *app, gpointer user_data) {
    // Create a new window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "PDF Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget *grid = gtk_grid_new();
    GtkWidget *popovermenubar = gtk_popover_menu_bar_new_from_model(NULL);

    // Create drawing area
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_hexpand(drawing_area, TRUE); // Set expansion properties for the drawing area
    gtk_widget_set_vexpand(drawing_area, TRUE);
    // gtk_grid_attach(GTK_GRID(grid), popovermenubar, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), drawing_area, 0, 0, 1, 1);
    gtk_window_set_child(GTK_WINDOW(window), grid);

    // Set drawing function
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_function, NULL, NULL);

    // Show window
    gtk_window_present(GTK_WINDOW(window));
}
