#include <gtk/gtk.h>
#include <poppler.h>

#include "ui.h"
#include "pdf.h"
#include "key.h"

GtkWidget *current_page_drawing_area;
GtkWidget *next_page_drawing_area;
GtkWidget *PDF_level_bar;
GtkWidget *state_label;

// File open callback for GtkFileDialog
static void file_open_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source_object);
    GtkWindow *window = GTK_WINDOW(user_data);
    GError *error = NULL;

    GFile *file = gtk_file_dialog_open_finish(dialog, res, &error);

    if (file != NULL) {
        char *filename = g_file_get_path(file);
        g_print("Selected file: %s\n", filename);

        // Call your load_pdf function here
        load_PDF_file(filename);
        gtk_widget_queue_draw(current_page_drawing_area);
        gtk_widget_queue_draw(next_page_drawing_area);

        g_free(filename);
        g_object_unref(file);
    } else if (error != NULL) {
        if (!g_error_matches(error, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_CANCELLED)) {
            g_print("Error opening file: %s\n", error->message);
        }
        g_error_free(error);
    }
}

// Add these action callback functions
void open_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GtkWindow *window = GTK_WINDOW(user_data);
    g_print("Open action triggered\n");

    // Create file dialog
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Open PDF File");

    // Create PDF file filter
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "PDF Files");
    gtk_file_filter_add_pattern(filter, "*.pdf");

    // Create filter list and add the PDF filter
    GListStore *filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    g_list_store_append(filters, filter);
    gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));

    // Set default filter to PDF
    gtk_file_dialog_set_default_filter(dialog, filter);

    // Open the dialog
    gtk_file_dialog_open(dialog, window, NULL, file_open_callback, window);

    // Clean up references
    g_object_unref(filter);
    g_object_unref(filters);
}

static void quit_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GtkWindow *window = GTK_WINDOW(user_data);
    gtk_window_close(window);
}

static void about_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GtkWindow *window = GTK_WINDOW(user_data);
    const char* authors[] = {"Andrei ZEUCIANU"};

    GtkWidget *dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "PDF Presenter");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "0.1");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "A simple PDF presenter using GTK4 and Poppler");
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);
    gtk_about_dialog_set_artists(GTK_ABOUT_DIALOG(dialog), authors);
    gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(dialog), GTK_LICENSE_MIT_X11);
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "https://github.com/Androl404/pdf-presenter");
    gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(dialog), "Repository on GitHub");
    // gtk_about_dialog_set_wrap_license(GTK_ABOUT_DIALOG(dialog), TRUE);

    gtk_window_set_transient_for(GTK_WINDOW(dialog), window);
    gtk_widget_set_visible(dialog, TRUE);

    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
}

static GtkWidget* create_menu_bar(GtkWindow *window) {
    // Create action group
    GSimpleActionGroup *action_group = g_simple_action_group_new();

    // Create actions
    GSimpleAction *open_act = g_simple_action_new("open", NULL);
    g_signal_connect(open_act, "activate", G_CALLBACK(open_action), window);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(open_act));

    GSimpleAction *quit_act = g_simple_action_new("quit", NULL);
    g_signal_connect(quit_act, "activate", G_CALLBACK(quit_action), window);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(quit_act));

    GSimpleAction *about_act = g_simple_action_new("about", NULL);
    g_signal_connect(about_act, "activate", G_CALLBACK(about_action), window);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(about_act));

    // Add action group to window
    gtk_widget_insert_action_group(GTK_WIDGET(window), "win", G_ACTION_GROUP(action_group));

    // Create menu model
    GMenu *menu_model = g_menu_new();

    // File menu
    GMenu *file_menu = g_menu_new();
    g_menu_append(file_menu, "_Open", "win.open");
    g_menu_append(file_menu, "_Quit", "win.quit");

    // Help menu
    GMenu *help_menu = g_menu_new();
    g_menu_append(help_menu, "_About", "win.about");

    // Add submenus to main menu
    g_menu_append_submenu(menu_model, "_File", G_MENU_MODEL(file_menu));
    g_menu_append_submenu(menu_model, "_Help", G_MENU_MODEL(help_menu));

    // Create popover menu bar
    GtkWidget *menu_bar = gtk_popover_menu_bar_new_from_model(G_MENU_MODEL(menu_model));

    return menu_bar;
}

void update_slides_label() {
    char label_string[(10 + (2*(pdf_data.total_pages / 10) + 1)) * sizeof(char)];
    sprintf(label_string, "Slide %d of %d", pdf_data.current_page + 1, pdf_data.total_pages);
    if (pdf_data.absolute_PDF_path[0] != 0) {
        gtk_label_set_label(GTK_LABEL(state_label), label_string);
    } else {
        gtk_label_set_label(GTK_LABEL(state_label), "Slides counter");
    }
}

void on_activate(GtkApplication *app, gpointer user_data) {
    // Create a new window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "PDF Presenter");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create drawing area
    current_page_drawing_area = gtk_drawing_area_new();
    next_page_drawing_area = gtk_drawing_area_new();
    GtkWidget *menu_bar = create_menu_bar(GTK_WINDOW(window));
    GtkWidget *grid = gtk_grid_new();

    GtkWidget *center_buttons_box = gtk_center_box_new();
    gtk_widget_set_margin_start(center_buttons_box, 20);
    gtk_widget_set_margin_end(center_buttons_box, 20);
    gtk_widget_set_margin_top(center_buttons_box, 7);
    gtk_widget_set_margin_bottom(center_buttons_box, 7);
    // gtk_box_set_baseline_position(GTK_BOX(buttons_box), GTK_BASELINE_POSITION_CENTER);
    GtkWidget *button_prev = gtk_button_new_with_label("Previous");
    g_signal_connect(button_prev, "clicked", G_CALLBACK(previous_PDF_page), window);
    state_label = gtk_label_new("");
    update_slides_label(); // To set basic label
    GtkWidget *button_next = gtk_button_new_with_label("Next");
    g_signal_connect(button_next, "clicked", G_CALLBACK(next_PDF_page), window);

    gtk_center_box_set_start_widget(GTK_CENTER_BOX(center_buttons_box), button_prev);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(center_buttons_box), state_label);
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(center_buttons_box), button_next);

    // Create PDF level bar
    PDF_level_bar = gtk_level_bar_new();

    gtk_widget_set_hexpand(current_page_drawing_area, TRUE); // Set expansion properties for the drawing area
    gtk_widget_set_vexpand(current_page_drawing_area, TRUE);
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(current_page_drawing_area), 450); // Setting the width of the current drawing area
    gtk_widget_set_hexpand(next_page_drawing_area, TRUE);
    gtk_widget_set_vexpand(next_page_drawing_area, TRUE);

    gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), current_page_drawing_area, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), next_page_drawing_area, 1, 1, 1, 2);
    gtk_grid_attach(GTK_GRID(grid), center_buttons_box, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), PDF_level_bar, 0, 3, 2, 1);

    // gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(PDF_level_bar), GTK_LEVEL_BAR_OFFSET_LOW, 0.10);
    gtk_window_set_child(GTK_WINDOW(window), grid);

    // Set drawing function
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(current_page_drawing_area), draw_current_page, NULL, NULL);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(next_page_drawing_area), draw_next_page, NULL, NULL);

    // Create key event controller
    GtkEventController* key_controller = gtk_event_controller_key_new();

    // Connect the key-released signal
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), NULL);

    // Add the controller to the window
    gtk_widget_add_controller(window, key_controller);

    // Show window
    gtk_window_present(GTK_WINDOW(window));

    // Load PDF from command line arguments
    load_defered_pdf();
}
