#include <gtk/gtk.h>
#include <poppler.h>

#include "pdf.h"
#include "ui.h"

PopplerDocument *document = NULL;
PDF_data pdf_data = {0};

int main(int argc, char **argv) {
    // Defer PDF loading (in the case there is a PDF file from cmd arg)
    defer_pdf_loading(argc, argv);

    // Initialize GTK and run the application
    GtkApplication *app = gtk_application_new("com.github.pdfviewer", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), 1, argv);

    // Clean up
    g_object_unref(document);
    g_object_unref(app);

    return status;
}
