#include <gtk/gtk.h>
#include <poppler.h>

#include "main.h"
#include "pdf.h"
#include "ui.h"
#include "notes.h"

PopplerDocument *document = NULL;
PDF_data pdf_data = {0};
GtkApplication* app;

static void command_line_arguments(int argc, char **argv) {
    if (argc > 1) {
        if (!strcmp("-h", argv[1]) || !strcmp("--help", argv[1])) {
            g_print("PDF Presenter\nMaintainer: Andrei Zeucianu\nGitHub repository: https://github.com/Androl404/pdf-presenter\n\nUsage:\n    pdf-presenter [pdf_file.pdf] [notes_file.{txt,md}]\n\nHelp:\n    -h, --help       Show this help message\n    -v, --version    Show this software's version\n");
            exit(0);
        } else if (!strcmp("-v", argv[1]) || !strcmp("--version", argv[1])) {
            g_print("PDF Presenter\nVersion " PDF_PRESENTER_VERSION "\n");
            exit(0);
        }
    }
}

int main(int argc, char **argv) {
    // Treat command line arguments
    command_line_arguments(argc, argv);

    // Defer PDF loading (in the case there is a PDF file from cmd arg)
    defer_pdf_loading(argc, argv);
    defer_notes_loading(argc, argv);

    // Initialize GTK and run the application
    app = gtk_application_new("com.github.pdfviewer", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), 1, argv);

    // Clean up
    g_object_unref(document);
    g_object_unref(app);

    return status;
}
