#include <gtk/gtk.h>
#include <poppler.h>

#include "ui.h"
#include "pdf.h"
#include "key.h"

GtkWidget *current_page_drawing_area;
GtkWidget *next_page_drawing_area;
GtkWidget *PDF_level_bar;
GtkWidget *state_label;
GtkWidget *datetime_label;
GtkWidget *pdf_path_label;

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
    const char* authors[] = {"Andrei ZEUCIANU <benjaminpotron@gmail.com>"};

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
    gtk_window_present(GTK_WINDOW(dialog));
    // gtk_widget_set_visible(dialog, TRUE);

    // g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
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
    sprintf(label_string, "Slide %zu of %zu", pdf_data.current_page + 1, pdf_data.total_pages);
    if (pdf_data.absolute_PDF_path[0] != 0) {
        gtk_label_set_label(GTK_LABEL(state_label), label_string);
    } else {
        gtk_label_set_label(GTK_LABEL(state_label), "Slides counter");
    }
}

gboolean sync_datetime_label(gpointer user_data) {
    GDateTime *time = g_date_time_new_now_local();
    char seconds[34];
    sprintf(seconds, "Local time: %04d/%02d/%02d %02d:%02d:%02d", g_date_time_get_year(time), g_date_time_get_month(time), g_date_time_get_day_of_month(time), g_date_time_get_hour(time), g_date_time_get_minute(time), g_date_time_get_second(time));
    gtk_label_set_label(GTK_LABEL(datetime_label), seconds);
    return TRUE;
}

void on_activate(GtkApplication *app, gpointer user_data) {
    // Create a new window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "PDF Presenter");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create drawing area
    current_page_drawing_area = gtk_drawing_area_new();
    next_page_drawing_area = gtk_drawing_area_new();
    // gtk_widget_set_can_focus(current_page_drawing_area, true);
    // gtk_widget_set_can_focus(next_page_drawing_area, true);
    GtkWidget *menu_bar = create_menu_bar(GTK_WINDOW(window));
    GtkWidget *grid = gtk_grid_new();

    // Create separators
    // GtkWidget* infos_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *vertical_separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);

    // Create current slide label
    // GtkWidget* current_slide_label = gtk_label_new("Current slide");
    // gtk_widget_set_halign(current_slide_label, GTK_ALIGN_START);
    // gtk_widget_set_margin_start(current_slide_label, 6);
    // gtk_widget_set_margin_bottom(current_slide_label, 3);

    // Create next slide label
    GtkWidget *next_slide_label = gtk_label_new("Next slide");
    gtk_widget_set_halign(next_slide_label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(next_slide_label, 6);
    // gtk_widget_set_margin_bottom(next_slide_label, 3);

    // Create notes label
    GtkWidget *notes_slide_label = gtk_label_new("Notes");
    gtk_widget_set_halign( notes_slide_label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(notes_slide_label, 6);

    // Set current slide font
    // PangoAttrList *attrlist = pango_attr_list_new();
    // PangoFontDescription *font_desc = pango_font_description_new();
    // pango_font_description_set_size(font_desc, 30 * PANGO_SCALE);
    // pango_font_description_set_weight(font_desc, PANGO_WEIGHT_BOLD);
    // PangoAttribute *attr = pango_attr_font_desc_new(font_desc);
    // pango_attr_list_insert(attrlist, attr);
    // gtk_label_set_attributes(GTK_LABEL(current_slide_label), attrlist);

    // Set next slide font
    PangoAttrList *attrlist = pango_attr_list_new();
    PangoFontDescription *font_desc = pango_font_description_new();
    pango_font_description_set_size(font_desc, 15 * PANGO_SCALE);
    pango_font_description_set_weight(font_desc, PANGO_WEIGHT_BOLD);
    PangoAttribute *attr = pango_attr_font_desc_new(font_desc);
    pango_attr_list_insert(attrlist, attr);
    gtk_label_set_attributes(GTK_LABEL(next_slide_label), attrlist);
    gtk_label_set_attributes(GTK_LABEL(notes_slide_label), attrlist);

    // Create notes text view
    GtkWidget *notes_text_view = gtk_text_view_new();
    GtkWidget *notes_scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(notes_scrolled_window), notes_text_view);
    // gtk_widget_set_hexpand(notes_scrolled_window, true);

    // Create previous button and callback
    GtkWidget *button_prev = gtk_button_new_with_label("Previous");
    g_signal_connect(button_prev, "clicked", G_CALLBACK(previous_PDF_page), window);

    // Slides label creation and initialization
    state_label = gtk_label_new("");
    update_slides_label(); // To set basic label

    // Create next button and callback
    GtkWidget *button_next = gtk_button_new_with_label("Next");
    g_signal_connect(button_next, "clicked", G_CALLBACK(next_PDF_page), window);

    // Create slides center box
    GtkWidget *slides_buttons_box = gtk_center_box_new();
    gtk_widget_set_margin_start(slides_buttons_box, 15);
    gtk_widget_set_margin_end(slides_buttons_box, 15);
    gtk_widget_set_margin_top(slides_buttons_box, 7);
    gtk_widget_set_margin_bottom(slides_buttons_box, 7);

    // Set widgets for slides center box
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(slides_buttons_box), button_prev);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(slides_buttons_box), state_label);
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(slides_buttons_box), button_next);

    // Set default focus
    gtk_widget_set_focus_child(slides_buttons_box, button_next);

    // Date time label creation & update
    datetime_label = gtk_label_new("");
    sync_datetime_label(window);
    g_timeout_add_seconds(1, sync_datetime_label, NULL);

    // Create infos center box and set margins
    GtkWidget *infos_center_box = gtk_center_box_new();
    gtk_widget_set_margin_start(infos_center_box, 10);
    gtk_widget_set_margin_end(infos_center_box, 10);
    gtk_widget_set_margin_top(infos_center_box, 4);
    gtk_widget_set_margin_bottom(infos_center_box, 4);

    // Initialize PDF path label
    pdf_path_label = gtk_label_new("");
    gtk_widget_set_margin_start(pdf_path_label, 8);
    gtk_widget_set_margin_end(pdf_path_label, 8);

    // Create label for timer
    GtkWidget *time_label = gtk_label_new("Timer : 00:00:00");

    // Set widget in info center box
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(infos_center_box), datetime_label);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(infos_center_box), pdf_path_label);
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(infos_center_box), time_label);

    // Create PDF level bar
    PDF_level_bar = gtk_level_bar_new();

    // Allow drawing areas to expand, or they will be not visible
    gtk_widget_set_hexpand(current_page_drawing_area, TRUE); // Set expansion properties for the drawing area
    gtk_widget_set_vexpand(current_page_drawing_area, TRUE);
    gtk_widget_set_hexpand(next_page_drawing_area, TRUE);
    // gtk_widget_set_vexpand(next_page_drawing_area, TRUE); // Commented to avoid eating the notes space

    // The following is awkward
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(next_page_drawing_area), 300); // Setting the width of the next drawing area

    // Attach widgets to grid
    gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 0, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), infos_center_box, 0, 1, 1, 1);
    // gtk_grid_attach(GTK_GRID(grid), infos_separator, 0, 2, 1, 1);
    // gtk_grid_attach(GTK_GRID(grid), current_slide_label, 0, 3, 1, 2);
    gtk_grid_attach(GTK_GRID(grid), current_page_drawing_area, 0, 2, 1, 2);
    gtk_grid_attach(GTK_GRID(grid), vertical_separator, 1, 1, 1, 3);

    gtk_grid_attach(GTK_GRID(grid), next_slide_label, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), next_page_drawing_area, 2, 2, 1, 1);
    // gtk_grid_attach(GTK_GRID(grid), notes_slide_label, 2, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), notes_scrolled_window, 2, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), slides_buttons_box, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), PDF_level_bar, 0, 5, 3, 1);

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
