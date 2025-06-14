#include <gtk/gtk.h>
#include <poppler.h>
// #include <stdlib.h>

#include "pdf.h"
#include "ui.h"

PopplerDocument *document = NULL;
char absolute_PDF_path[PATH_MAX + 1];
PDF_data pdf_data = {0};

int main(int argc, char **argv) {
    if (argc >= 2) {
        load_PDF_file(argv[1]);
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
