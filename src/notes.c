#include <gtk/gtk.h>
#include <poppler.h>

#include "main.h"
#include "ui.h"
#include "pdf.h"
#include "key.h"
#include "notes.h"

char *notes_to_load = NULL;
notes_data data_notes = {0};

// File open callback for GtkFileDialog
static void file_open_callback(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source_object);
    GtkWindow *window = GTK_WINDOW(user_data);
    GError *error = NULL;

    GFile *file = gtk_file_dialog_open_finish(dialog, res, &error);

    if (file != NULL) {
        char *filename = g_file_get_path(file);
        g_print("Selected file: %s\n", filename);

        // What we actually do with the notes files
        load_notes_file(filename);
        // queue_all_drawing_areas();

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
void open_notes_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GtkWindow *window = GTK_WINDOW(user_data);
    g_print("Open action triggered\n");

    // Create file dialog
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Open Notes File");

    // Create PDF file filter
    GtkFileFilter *filter_md = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_md, "MarkDown Files");
    gtk_file_filter_add_pattern(filter_md, "*.md");

    GtkFileFilter *filter_text = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_text, "Text Files");
    gtk_file_filter_add_pattern(filter_text, "*.txt");

    GtkFileFilter *filter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_all, "All Files");
    gtk_file_filter_add_pattern(filter_all, "*");

    // Create filter list and add the PDF filter
    GListStore *filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    g_list_store_append(filters, filter_md);
    g_list_store_append(filters, filter_text);
    g_list_store_append(filters, filter_all);
    gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));

    // Set default filter to PDF
    gtk_file_dialog_set_default_filter(dialog, filter_md);

    // Open the dialog
    gtk_file_dialog_open(dialog, window, NULL, file_open_callback, window);

    // Clean up references
    g_object_unref(filter_md);
    g_object_unref(filter_text);
    g_object_unref(filter_all);
    g_object_unref(filters);
}

void load_notes_file(const char *filepath) {
    // Load notes document
    GError *error = NULL;
    data_notes.notes_absolute_path = g_string_new(NULL);
    char* absolute_notes_path_temp = malloc((PATH_MAX + 1) * sizeof(char));
    char *uri = g_filename_to_uri(realpath(filepath, absolute_notes_path_temp), NULL, &error);
    data_notes.notes_absolute_path = g_string_append(data_notes.notes_absolute_path, absolute_notes_path_temp);
    free(absolute_notes_path_temp);

    // Verify path of notes
    if (!uri) {
        if (error) {
            g_print("Failed to convert filename to URI: %s\n", error->message);
        } else {
            g_print("Failed to convert filename to URI: the file was probably not found\n");
        }
        g_error_free(error);
        return;
    }

    g_print("Loaded notes file at: %s\n", data_notes.notes_absolute_path->str);
}

gboolean defer_notes_loading(int argc, char **argv) {
    if (argc >= 3) {
        notes_to_load = argv[2];
        return true;
    } else {
        return false;
    }
}

void load_defered_notes(void) {
    if (notes_to_load != NULL)
        load_notes_file(notes_to_load);
}
