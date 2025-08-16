#include <gtk/gtk.h>
#include <poppler.h>
#include <stdio.h>

#include "main.h"
#include "ui.h"
#include "pdf.h"
#include "key.h"
#include "notes.h"

GtkWidget *current_page_drawing_area;
GtkWidget *next_page_drawing_area;
GtkWidget *presentation_drawing_area;
GtkWidget *PDF_level_bar;
GtkWidget *state_label;
GtkWidget *datetime_label;
GtkWidget *chrono_label;
GtkWidget *pdf_path_label;
GtkWidget *notes_label;
presentation_data data_presentation = {
    .in_presentation = false,
    .window_presentation_id = 0
};
GDateTime *presentation_start_time;
GtkWidget *notes_display_notebook;

guint presentation_monitor;
guint monitor_to_switch;
gsize notes_font_size;

// File open callback for GtkFileDialog
static void file_open_callback(GObject *source_object, GAsyncResult *res, [[gnu::unused]]gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source_object);
    // GtkWindow *window = GTK_WINDOW(user_data);
    GError *error = NULL;

    GFile *file = gtk_file_dialog_open_finish(dialog, res, &error);

    if (file != NULL) {
        char *filename = g_file_get_path(file);
        g_print("Selected file: %s\n", filename);

        // Call your load_pdf function here
        load_PDF_file(filename);
        queue_all_drawing_areas();

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
void open_PDF_action([[gnu::unused]]GSimpleAction *action, [[gnu::unused]]GVariant *parameter, gpointer user_data) {
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

static void quit_action([[gnu::unused]]GSimpleAction *action, [[gnu::unused]]GVariant *parameter, gpointer user_data) {
    GtkWindow *window = GTK_WINDOW(user_data);
    gtk_window_close(window);
}

static void create_presentation_window([[gnu::unused]]GSimpleAction *action, [[gnu::unused]]GVariant *parameter, gpointer user_data) {
    // Set presentation mode to true
    data_presentation.in_presentation = true;

    // TODO: Allow multiple presentation windows on multiple external monitor
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Presentation Window (should be in full screen)");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    presentation_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_hexpand(presentation_drawing_area, TRUE); // Set expansion properties for the drawing area
    gtk_widget_set_vexpand(presentation_drawing_area, TRUE);

    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(presentation_drawing_area), draw_current_page, NULL, NULL);

    gtk_window_set_child(GTK_WINDOW(window), presentation_drawing_area);

    // TODO: Check if we were at the first page to avoid one iteration of redrawing
    queue_all_drawing_areas(); // Redraw everything in case we go to the first page
    update_level_bar();

    // Add key controller also on that window, so we can use the keybindings while focusing on the presentation window.
    GtkEventController* key_controller = gtk_event_controller_key_new();
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), NULL);
    gtk_widget_add_controller(window, key_controller);

    g_signal_connect(window, "close-request", G_CALLBACK(finish_presentation_action), user_data);

    // Show window
    gtk_window_present(GTK_WINDOW(window));

    data_presentation.window_presentation_id = gtk_application_window_get_id(GTK_APPLICATION_WINDOW(window));

    GdkDisplay* default_display = gdk_display_get_default(); // Get display, more at a higher level, like the window manager
    GListModel *monitors_list = gdk_display_get_monitors(default_display); // Get a list of all of the actuals monitor contained by that display
    if (presentation_monitor < g_list_model_get_n_items(monitors_list)) {
        gtk_window_fullscreen_on_monitor(GTK_WINDOW(window), GDK_MONITOR(g_list_model_get_object(monitors_list, presentation_monitor)));
    } else {
        gtk_window_fullscreen_on_monitor(GTK_WINDOW(window), GDK_MONITOR(g_list_model_get_object(monitors_list, 0)));
    }
}

void present_first_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    // Reset chronometer anyways
    presentation_start_time = g_date_time_new_now_local();

    // Go to first page if already in presentation
    if (data_presentation.in_presentation) {
        pdf_data.current_page = 0;
        queue_all_drawing_areas();
        update_level_bar();
        return;
    }

    // Start at PDF first page
    pdf_data.current_page = 0;

    create_presentation_window(action, parameter, user_data);
}

void present_current_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    if (data_presentation.in_presentation)
        return;

    create_presentation_window(action, parameter, user_data);
    presentation_start_time = g_date_time_new_now_local();
 }

gboolean finish_presentation_action([[gnu::unused]]GtkWindow *self, [[gnu::unused]]gpointer user_data) {
    if (data_presentation.in_presentation) {
        data_presentation.in_presentation = false;
        gtk_window_destroy(gtk_application_get_window_by_id(app, data_presentation.window_presentation_id));
    }
    return true;
}

static void end_presentation_action([[gnu::unused]]GSimpleAction *action, [[gnu::unused]]GVariant *parameter, gpointer user_data) {
    finish_presentation_action(gtk_application_get_window_by_id(app, data_presentation.window_presentation_id), user_data);
}

static void notes_bigger_button_action([[gnu::unused]]GtkButton *self, gpointer user_data) {
    notes_bigger_action(NULL, NULL, user_data);
}

static void notes_smaller_button_action([[gnu::unused]]GtkButton *self, gpointer user_data) {
    notes_smaller_action(NULL, NULL, user_data);
}

void notes_bigger_action([[gnu::unused]] GSimpleAction *action, [[gnu::unused]] GVariant *parameter, [[gnu::unused]] gpointer user_data) {
    PangoAttrList *attrlist = pango_attr_list_new();
    PangoFontDescription* font_description_notes = pango_font_description_new();
    pango_font_description_set_size(font_description_notes, (++notes_font_size) * PANGO_SCALE);
    // pango_font_description_set_weight(font_description, PANGO_WEIGHT_BOLD);
    PangoAttribute *attr = pango_attr_font_desc_new(font_description_notes);
    pango_attr_list_insert(attrlist, attr);
    gtk_label_set_attributes(GTK_LABEL(notes_label), attrlist);
}

void notes_smaller_action([[gnu::unused]]GSimpleAction *action, [[gnu::unused]]GVariant *parameter, [[gnu::unused]]gpointer user_data) {
    PangoAttrList *attrlist = pango_attr_list_new();
    PangoFontDescription* font_description_notes = pango_font_description_new();
    pango_font_description_set_size(font_description_notes, (--notes_font_size) * PANGO_SCALE);
    // pango_font_description_set_weight(font_description, PANGO_WEIGHT_BOLD);
    PangoAttribute *attr = pango_attr_font_desc_new(font_description_notes);
    pango_attr_list_insert(attrlist, attr);
    gtk_label_set_attributes(GTK_LABEL(notes_label), attrlist);
}

static gboolean close_all_windows(GtkWindow *self, gpointer user_data) {
    finish_presentation_action(self, user_data);
    exit(0);
}

static void about_action([[gnu::unused]]GSimpleAction *action, [[gnu::unused]]GVariant *parameter, gpointer user_data) {
    GtkWindow *window = GTK_WINDOW(user_data);
    const char* authors[] = {"Andrei ZEUCIANU <benjaminpotron@gmail.com>", NULL};

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
    GSimpleAction *open_PDF_act = g_simple_action_new("openPDF", NULL);
    g_signal_connect(open_PDF_act, "activate", G_CALLBACK(open_PDF_action), window);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(open_PDF_act));

    GSimpleAction *open_notes_act = g_simple_action_new("opennotes", NULL);
    g_signal_connect(open_notes_act, "activate", G_CALLBACK(open_notes_action), window);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(open_notes_act));

    GSimpleAction *quit_act = g_simple_action_new("quit", NULL);
    g_signal_connect(quit_act, "activate", G_CALLBACK(quit_action), window);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(quit_act));

    GSimpleAction *present_first_act = g_simple_action_new("present_first", NULL);
    g_signal_connect(present_first_act, "activate", G_CALLBACK(present_first_action), window);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(present_first_act));

    GSimpleAction *present_current_act = g_simple_action_new("present_current", NULL);
    g_signal_connect(present_current_act, "activate", G_CALLBACK(present_current_action), window);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(present_current_act));

    GSimpleAction *end_presentation_act = g_simple_action_new("end_presentation", NULL);
    g_signal_connect(end_presentation_act, "activate", G_CALLBACK(end_presentation_action), window);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(end_presentation_act));

    GSimpleAction *notes_bigger_act = g_simple_action_new("notes_bigger", NULL);
    g_signal_connect(notes_bigger_act, "activate", G_CALLBACK(notes_bigger_action), window);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(notes_bigger_act));

    GSimpleAction *notes_smaller_act = g_simple_action_new("notes_smaller", NULL);
    g_signal_connect(notes_smaller_act, "activate", G_CALLBACK(notes_smaller_action), window);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(notes_smaller_act));

    GSimpleAction *about_act = g_simple_action_new("about", NULL);
    g_signal_connect(about_act, "activate", G_CALLBACK(about_action), window);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(about_act));

    // Add action group to window
    gtk_widget_insert_action_group(GTK_WIDGET(window), "win", G_ACTION_GROUP(action_group));

    // Create menu model
    GMenu *menu_model = g_menu_new();

    // File menu
    GMenu *file_menu = g_menu_new();
    g_menu_append(file_menu, "_Open PDF file...", "win.openPDF");
    g_menu_append(file_menu, "_Open notes file...", "win.opennotes");
    g_menu_append(file_menu, "_Quit", "win.quit");

    // Present menu
    GMenu *present_menu = g_menu_new();
    g_menu_append(present_menu, "_Start presentation at first slide", "win.present_first");
    g_menu_append(present_menu, "_Start presentation at current slide", "win.present_current");
    g_menu_append(present_menu, "_End presentation", "win.end_presentation");

    // Notes menu
    GMenu *notes_menu = g_menu_new();
    g_menu_append(notes_menu, "_Notes font bigger", "win.notes_bigger");
    g_menu_append(notes_menu, "_Notes font smaller", "win.notes_smaller");

    // Help menu
    GMenu *help_menu = g_menu_new();
    g_menu_append(help_menu, "_About", "win.about");

    // Add submenus to main menu
    g_menu_append_submenu(menu_model, "_File", G_MENU_MODEL(file_menu));
    g_menu_append_submenu(menu_model, "_Present", G_MENU_MODEL(present_menu));
    g_menu_append_submenu(menu_model, "_Notes", G_MENU_MODEL(notes_menu));
    g_menu_append_submenu(menu_model, "_Help", G_MENU_MODEL(help_menu));

    // Create popover menu bar
    GtkWidget *menu_bar = gtk_popover_menu_bar_new_from_model(G_MENU_MODEL(menu_model));

    return menu_bar;
}

void update_slides_label() {
    char label_string[(10 + (2*(pdf_data.total_pages / 10) + 1)) * sizeof(char)];
    sprintf(label_string, "Slide %zu of %zu", pdf_data.current_page + 1, pdf_data.total_pages);
    if (pdf_data.pdf_loaded) {
        gtk_label_set_label(GTK_LABEL(state_label), label_string);
    } else {
        gtk_label_set_label(GTK_LABEL(state_label), "Slides counter");
    }
}

void update_level_bar() {
    gtk_level_bar_set_value(GTK_LEVEL_BAR(PDF_level_bar), (double)pdf_data.current_page + 1);
}

gboolean sync_datetime_label([[gnu::unused]]gpointer user_data) {
    GDateTime *time = g_date_time_new_now_local();
    char local_time_str[34], chronometer_str[42];
    gint64 hours = 0, minutes = 0, seconds = 0;
    sprintf(local_time_str, "Local time: %04d/%02d/%02d %02d:%02d:%02d", g_date_time_get_year(time), g_date_time_get_month(time), g_date_time_get_day_of_month(time), g_date_time_get_hour(time), g_date_time_get_minute(time), g_date_time_get_second(time));
    gtk_label_set_label(GTK_LABEL(datetime_label), local_time_str);
    if (data_presentation.in_presentation) {
        GTimeSpan presentation_time_difference = g_date_time_difference(time, presentation_start_time);
        presentation_time_difference /= 1000000; // From microseconds to seconds
        if (presentation_time_difference > 59) {
            minutes = presentation_time_difference / 60;
            seconds = presentation_time_difference % 60;
            if (minutes > 59) {
                hours = minutes / 60;
                minutes = minutes % 60;
            }
        } else {
            seconds = presentation_time_difference;
        }
        sprintf(chronometer_str, "Chronometer: %02ld:%02ld:%02ld", hours, minutes, seconds);
        gtk_label_set_label(GTK_LABEL(chrono_label), chronometer_str);
    }
    return TRUE;
}

static void refresh_display_list([[gnu::unused]]GtkButton *self, gpointer user_data) {
    gtk_notebook_remove_page(GTK_NOTEBOOK(notes_display_notebook), 1);
    GtkWidget* notebook_displays_label = gtk_label_new("Displays");
    gtk_notebook_append_page(GTK_NOTEBOOK(notes_display_notebook), get_diplays_box(user_data), notebook_displays_label);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notes_display_notebook), 1);
}

static void set_default_presentation_monitor() {
    GdkDisplay* default_display = gdk_display_get_default(); // Get display, more at a higher level, like the window manager
    GListModel *monitors_list = gdk_display_get_monitors(default_display); // Get a list of all of the actuals monitor contained by that display
    if (g_list_model_get_n_items(monitors_list) > 1)
        presentation_monitor = 1;
    else
        presentation_monitor = 0;
}

static void set_presentation_monitor(GtkButton *self, gpointer user_data) {
    GdkDisplay* default_display = gdk_display_get_default(); // Get display, more at a higher level, like the window manager
    GListModel *monitors_list = gdk_display_get_monitors(default_display); // Get a list of all of the actuals monitor contained by that display

    if (*(guint *)user_data < g_list_model_get_n_items(monitors_list)) {
        presentation_monitor = *(guint *)user_data;
        refresh_display_list(self, gtk_application_get_window_by_id(app, 0));
    } else {
        set_default_presentation_monitor();
        refresh_display_list(self, gtk_application_get_window_by_id(app, 0));
    }

    if (data_presentation.in_presentation) {
        gtk_window_destroy(gtk_application_get_window_by_id(app, data_presentation.window_presentation_id));
        create_presentation_window(NULL, NULL, gtk_application_get_window_by_id(app, 0));
    }
}

GtkWidget *get_diplays_box(gpointer user_data) {
    GdkDisplay* default_display = gdk_display_get_default(); // Get display, more at a higher level, like the window manager
    GListModel *monitors_list = gdk_display_get_monitors(default_display); // Get a list of all of the actuals monitor contained by that display
    guint monitor_number = g_list_model_get_n_items(monitors_list);        // Get the number of monitors

    // Print monitors infos
    // g_print("Number of monitor(s): %d\n", monitor_number); // Number of monitors
    // for (guint i = 0; i < monitor_number; i++) {
    //     GdkRectangle geometry = {0};
    //     gdk_monitor_get_geometry(GDK_MONITOR(g_list_model_get_object(monitors_list, i)), &geometry);
    //     g_print("Connector: %s\n", gdk_monitor_get_connector(GDK_MONITOR(g_list_model_get_object(monitors_list, i))));
    //     g_print("Description: %s\n", gdk_monitor_get_description(GDK_MONITOR(g_list_model_get_object(monitors_list, i))));
    //     g_print("Geometry:\n    x = %d;\n    y = %d;\n    width = %d;\n    height = %d;\n", geometry.x, geometry.y, geometry.width, geometry.height);
    //     g_print("Manufacturer: %s\n", gdk_monitor_get_manufacturer(GDK_MONITOR(g_list_model_get_object(monitors_list, i))));
    //     g_print("Model: %s\n", gdk_monitor_get_model(GDK_MONITOR(g_list_model_get_object(monitors_list, i))));
    //     g_print("Refresh rate: %d\n", gdk_monitor_get_refresh_rate(GDK_MONITOR(g_list_model_get_object(monitors_list, i))));
    //     g_print("Scale: %f\n", gdk_monitor_get_scale(GDK_MONITOR(g_list_model_get_object(monitors_list, i))));
    //     g_print("Scale factor: %d\n", gdk_monitor_get_scale_factor(GDK_MONITOR(g_list_model_get_object(monitors_list, i))));
    //     // g_list_model_get_object(monitors_list, i);
    // }

    GtkWidget *frame_monitors[monitor_number];
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    for (guint i = 0; i < monitor_number; i++) {
        GdkRectangle geometry = {0};
        gdk_monitor_get_geometry(GDK_MONITOR(g_list_model_get_object(monitors_list, i)), &geometry);
        const char *monitor_description = gdk_monitor_get_model(GDK_MONITOR(g_list_model_get_object(monitors_list, i)));
        const char *monitor_manufacturer = gdk_monitor_get_manufacturer(GDK_MONITOR(g_list_model_get_object(monitors_list, i)));
        const char *monitor_connector = gdk_monitor_get_connector(GDK_MONITOR(g_list_model_get_object(monitors_list, i)));
        const int refresh_rate = gdk_monitor_get_refresh_rate(GDK_MONITOR(g_list_model_get_object(monitors_list, i)));
        const int x = geometry.x;
        const int y = geometry.y;
        const int width = geometry.width;
        const int height = geometry.height;

        // Construct first line for display
        char *monitor_first_line = malloc(strlen(monitor_description) + strlen(monitor_manufacturer) + strlen(monitor_connector) + 4 + 1);
        strcpy(monitor_first_line, monitor_description);
        strcat(monitor_first_line, ", ");
        strcat(monitor_first_line, monitor_manufacturer);
        strcat(monitor_first_line, ", ");
        strcat(monitor_first_line, monitor_connector);

        // Construct first line for display
        char monitor_final_line[strlen(monitor_first_line) + 42]; // One caracter more
        sprintf(monitor_final_line, "%s\n%dx%d, %dHz at (%d,%d)", monitor_first_line, width, height, refresh_rate/1000, x, y);

        // Setting the frame
        frame_monitors[i] = gtk_frame_new(NULL);
        gtk_widget_set_margin_start(frame_monitors[i], 10);
        gtk_widget_set_margin_end(frame_monitors[i], 10);
        gtk_widget_set_margin_bottom(frame_monitors[i], 2);
        gtk_widget_set_margin_top(frame_monitors[i], 8);

        // Setting the icon
        GIcon *monitor_icon = g_themed_icon_new("video-display");
        GtkWidget *monitor_image = gtk_image_new_from_gicon(monitor_icon);
        gtk_image_set_pixel_size(GTK_IMAGE(monitor_image), 50);

        // Settting the box with the main content
        GtkWidget *horizontal_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_widget_set_margin_start(horizontal_box, 5);
        gtk_widget_set_margin_end(horizontal_box, 5);
        gtk_widget_set_margin_bottom(horizontal_box, 5);
        gtk_widget_set_margin_top(horizontal_box, 5);
        gtk_box_append(GTK_BOX(horizontal_box), monitor_image);
        GtkWidget *final_label = gtk_label_new(monitor_final_line);
        // gtk_label_set_selectable(GTK_LABEL(final_label), true);
        gtk_box_append(GTK_BOX(horizontal_box), final_label);
        gtk_frame_set_child(GTK_FRAME(frame_monitors[i]), horizontal_box);
        gtk_box_append(GTK_BOX(box), frame_monitors[i]);

        GtkWidget *expand_label = gtk_label_new(NULL);
        gtk_widget_set_hexpand(expand_label, true);
        gtk_box_append(GTK_BOX(horizontal_box), expand_label);
        if (i != presentation_monitor) {
            GtkWidget *set_button = gtk_button_new_with_label("Set as presentation display");
            gtk_box_append(GTK_BOX(horizontal_box), set_button);
            monitor_to_switch = i;
            g_signal_connect(set_button, "clicked", G_CALLBACK(set_presentation_monitor), &monitor_to_switch);
        } else {
            GIcon *ok_icon = g_themed_icon_new("object-select-symbolic");
            GtkWidget *ok_image = gtk_image_new_from_gicon(ok_icon);
            gtk_widget_set_margin_end(ok_image, 7);
            gtk_box_append(GTK_BOX(horizontal_box), ok_image);
        }

        // Free string memory
        free(monitor_first_line);
    }
    GtkWidget *button_refresh = gtk_button_new_with_label("Refresh monitors list");
    gtk_widget_set_margin_start(button_refresh, 10);
    gtk_widget_set_margin_end(button_refresh, 10);
    gtk_widget_set_margin_bottom(button_refresh, 7);
    gtk_widget_set_margin_top(button_refresh, 5);
    g_signal_connect(button_refresh, "clicked", G_CALLBACK(refresh_display_list), GTK_WINDOW(user_data));
    gtk_box_append(GTK_BOX(box), button_refresh);

    return box;
}

void on_activate(GtkApplication *app, gpointer user_data) {
    // Create a new window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "PDF Presenter");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create drawing area
    current_page_drawing_area = gtk_drawing_area_new();
    next_page_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_can_focus(current_page_drawing_area, true);
    gtk_widget_set_can_focus(next_page_drawing_area, true);
    GtkWidget *menu_bar = create_menu_bar(GTK_WINDOW(window));
    GtkWidget *grid = gtk_grid_new();

    // Create next slide label
    GtkWidget *next_slide_label = gtk_label_new("Next slide");
    gtk_widget_set_halign(next_slide_label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(next_slide_label, 6);
    // gtk_widget_set_margin_bottom(next_slide_label, 3);

    // Set next slide font
    PangoAttrList *attrlist = pango_attr_list_new();
    PangoFontDescription *font_desc = pango_font_description_new();
    pango_font_description_set_size(font_desc, 15 * PANGO_SCALE);
    pango_font_description_set_weight(font_desc, PANGO_WEIGHT_BOLD);
    PangoAttribute *attr = pango_attr_font_desc_new(font_desc);
    pango_attr_list_insert(attrlist, attr);
    gtk_label_set_attributes(GTK_LABEL(next_slide_label), attrlist);
    // gtk_label_set_attributes(GTK_LABEL(notes_slide_label), attrlist);

    // Create notes text label
    notes_label = gtk_label_new("");
    gtk_label_set_selectable(GTK_LABEL(notes_label), true);
    GtkWidget *notes_scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(notes_scrolled_window), notes_label);
    gtk_widget_set_hexpand(notes_scrolled_window, TRUE);

    gtk_label_set_xalign(GTK_LABEL(notes_label), 0);
    gtk_label_set_yalign(GTK_LABEL(notes_label), 0);
    gtk_widget_set_margin_start(notes_label, 5);
    gtk_widget_set_margin_top(notes_label, 5);
    gtk_label_set_wrap(GTK_LABEL(notes_label), true);
    gtk_label_set_wrap_mode(GTK_LABEL(notes_label), PANGO_WRAP_WORD_CHAR);

    // Set default size font for notes
    notes_font_size = 11;
    PangoAttrList *attrlist_notes = pango_attr_list_new();
    PangoFontDescription* font_description_notes = pango_font_description_new();
    pango_font_description_set_size(font_description_notes, notes_font_size * PANGO_SCALE);
    PangoAttribute *attr_notes = pango_attr_font_desc_new(font_description_notes);
    pango_attr_list_insert(attrlist_notes, attr_notes);
    gtk_label_set_attributes(GTK_LABEL(notes_label), attrlist_notes);
    // gtk_label_set_attributes(GTK_LABEL(notes_slide_label), attrlist);

    // Create previous button and callback
    GtkWidget *button_prev = gtk_button_new_with_label("Previous");
    g_signal_connect(button_prev, "clicked", G_CALLBACK(previous_PDF_page), window);

    // Slides label creation and initialization
    state_label = gtk_label_new("");
    gtk_label_set_selectable(GTK_LABEL(state_label), true);
    gtk_label_set_single_line_mode(GTK_LABEL(state_label), true);
    update_slides_label(); // To set basic label

    // Create next button and callback
    GtkWidget *button_next = gtk_button_new_with_label("Next");
    g_signal_connect(button_next, "clicked", G_CALLBACK(next_PDF_page), window);

    // Create font smaller button and callback
    GtkWidget *button_font_smaller = gtk_button_new_from_icon_name("format-text-direction-rtl-symbolic");
    g_signal_connect(button_font_smaller, "clicked", G_CALLBACK(notes_smaller_button_action), window);

    // Create font bigger and callback
    GtkWidget *button_font_bigger = gtk_button_new_from_icon_name("format-text-direction-ltr-symbolic");
    g_signal_connect(button_font_bigger, "clicked", G_CALLBACK(notes_bigger_button_action), window);

    // Put next, font smaller and font bigger in box
    GtkWidget *end_widget_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_append(GTK_BOX(end_widget_box), button_next);
    gtk_box_append(GTK_BOX(end_widget_box), button_font_smaller);
    gtk_box_append(GTK_BOX(end_widget_box), button_font_bigger);

    // Create slides center box
    GtkWidget *slides_buttons_box = gtk_center_box_new();
    gtk_widget_set_margin_start(slides_buttons_box, 15);
    gtk_widget_set_margin_end(slides_buttons_box, 15);
    gtk_widget_set_margin_top(slides_buttons_box, 7);
    gtk_widget_set_margin_bottom(slides_buttons_box, 7);

    // Set widgets for slides center box
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(slides_buttons_box), button_prev);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(slides_buttons_box), state_label);
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(slides_buttons_box), end_widget_box);

    // Date time label creation & update
    datetime_label = gtk_label_new("");
    gtk_label_set_selectable(GTK_LABEL(datetime_label), true);
    gtk_label_set_single_line_mode(GTK_LABEL(datetime_label), true);
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
    gtk_label_set_selectable(GTK_LABEL(pdf_path_label), true);
    gtk_label_set_single_line_mode(GTK_LABEL(pdf_path_label), true);
    gtk_widget_set_margin_start(pdf_path_label, 8);
    gtk_widget_set_margin_end(pdf_path_label, 8);

    // Create label for timer
    chrono_label = gtk_label_new("Chronometer: 00:00:00");
    gtk_label_set_selectable(GTK_LABEL(chrono_label), true);
    gtk_label_set_single_line_mode(GTK_LABEL(chrono_label), true);

    // Set widget in info center box
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(infos_center_box), datetime_label);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(infos_center_box), pdf_path_label);
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(infos_center_box), chrono_label);

    // Create PDF level bar
    PDF_level_bar = gtk_level_bar_new();

    // Allow drawing areas to expand, or they will be not visible
    gtk_widget_set_hexpand(current_page_drawing_area, TRUE); // Set expansion properties for the drawing area
    gtk_widget_set_vexpand(current_page_drawing_area, TRUE);
    gtk_widget_set_hexpand(next_page_drawing_area, TRUE);
    gtk_widget_set_vexpand(next_page_drawing_area, TRUE);

    // The following is awkward (to set the size of the drawing areas)
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(next_page_drawing_area), 300); // Setting the width of the next drawing area
    // gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(next_page_drawing_area), 400); // Setting the width of the next drawing area
    gtk_widget_set_hexpand(notes_scrolled_window, TRUE);

    // Set defautl presentation monitor at startup
    set_default_presentation_monitor();

    // To set the right paned widget with next slides and notes
    notes_display_notebook = gtk_notebook_new();
    GtkWidget *notebook_notes_label = gtk_label_new("Notes");
    GtkWidget *notebook_displays_label = gtk_label_new("Displays");
    gtk_notebook_append_page(GTK_NOTEBOOK(notes_display_notebook), notes_scrolled_window, notebook_notes_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(notes_display_notebook), get_diplays_box(window), notebook_displays_label);
    GtkWidget *box_PDF_next_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(box_PDF_next_page), next_slide_label);
    gtk_box_append(GTK_BOX(box_PDF_next_page), next_page_drawing_area);
    GtkWidget *frame_next_PDF_page = gtk_frame_new(NULL);
    gtk_frame_set_child(GTK_FRAME(frame_next_PDF_page), box_PDF_next_page);
    GtkWidget *frame_notes = gtk_frame_new(NULL);
    gtk_frame_set_child(GTK_FRAME(frame_notes), notes_display_notebook);
    GtkWidget *next_paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_paned_set_start_child(GTK_PANED(next_paned), frame_next_PDF_page);
    gtk_paned_set_end_child(GTK_PANED(next_paned), frame_notes);
    gtk_paned_set_resize_start_child(GTK_PANED(next_paned), TRUE);
    gtk_paned_set_shrink_start_child(GTK_PANED(next_paned), FALSE);
    gtk_paned_set_resize_end_child(GTK_PANED(next_paned), FALSE);
    gtk_paned_set_shrink_end_child(GTK_PANED(next_paned), FALSE);

    // To set the paned widget with the current and next PDF pages
    GtkWidget* box_current_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *box_next_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(box_current_page), infos_center_box);
    gtk_box_append(GTK_BOX(box_current_page), current_page_drawing_area);
    gtk_box_append(GTK_BOX(box_next_page), next_paned);
    GtkWidget *frame_current_page = gtk_frame_new(NULL);
    gtk_frame_set_child(GTK_FRAME(frame_current_page), box_current_page);
    GtkWidget *frame_next_page = gtk_frame_new(NULL);
    gtk_frame_set_child(GTK_FRAME(frame_next_page), box_next_page);
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_start_child(GTK_PANED(paned), frame_current_page);
    gtk_paned_set_end_child(GTK_PANED(paned), frame_next_page);
    gtk_paned_set_resize_start_child(GTK_PANED(paned), TRUE);
    gtk_paned_set_shrink_start_child(GTK_PANED(paned), FALSE);
    gtk_paned_set_resize_end_child(GTK_PANED(paned), TRUE);
    gtk_paned_set_shrink_end_child(GTK_PANED(paned), FALSE);

    // Attach widgets to grid
    gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), paned, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), slides_buttons_box, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), PDF_level_bar, 0, 3, 1, 1);

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

    // Set default focus, avoiding the text view
    gtk_window_set_focus(GTK_WINDOW(window), button_next);

    // If we close the main window
    g_signal_connect(window, "close-request", G_CALLBACK(close_all_windows), user_data);

    // Load PDF from command line arguments
    load_defered_pdf();

    // Load notes from command line arguments
    load_defered_notes();
}
