#include <gtk/gtk.h>
#include <poppler.h>

#include "pdf.h"
#include "ui.h"

PopplerDocument *document = NULL;

int main(int argc, char **argv) {
    if (argc < 2) {
        g_print("Usage: %s <PDF file>\n", argv[0]);
        return 1;
    }

    // Load PDF document
    GError *error = NULL;
    char *uri = g_filename_to_uri(argv[1], NULL, &error);

    if (!uri) {
        g_print("Failed to convert filename to URI: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    document = poppler_document_new_from_file(uri, NULL, &error);
    g_free(uri);

    if (!document) {
        g_print("Failed to open PDF: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    // Initialize GTK and run the application
    GtkApplication *app = gtk_application_new("com.example.pdfviewer", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), 1, argv);

    // Clean up
    g_object_unref(document);
    g_object_unref(app);

    return status;
}
