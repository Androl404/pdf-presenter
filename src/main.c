#include <gtk/gtk.h>
#include <poppler.h>

static PopplerDocument *document = NULL;

static void draw_function(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
    if (!document) return;

    PopplerPage *page = poppler_document_get_page(document, 0); // First page
    if (!page) return;

    // Get page dimensions
    double page_width, page_height;
    poppler_page_get_size(page, &page_width, &page_height);

    // Calculate scale to fit the page to the window while maintaining aspect ratio
    double scale_x = width / page_width;
    double scale_y = height / page_height;
    double scale = scale_x < scale_y ? scale_x : scale_y;

    // Clear background
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // Center the page
    cairo_translate(cr, (width - page_width * scale) / 2, (height - page_height * scale) / 2);

    // Apply scaling
    cairo_scale(cr, scale, scale);

    // Render the page
    poppler_page_render(page, cr);

    g_object_unref(page);
}

static void on_activate(GtkApplication *app, gpointer user_data) {
    // Create a new window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "PDF Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create drawing area
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_window_set_child(GTK_WINDOW(window), drawing_area);

    // Set drawing function
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_function, NULL, NULL);

    // Show window
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        g_print("Usage: %s <PDF file>\n", argv[0]);
        return 1;
    }

    // Initialize GTK
    GtkApplication *app = gtk_application_new("com.example.pdfviewer", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

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

    // Run application
    int status = g_application_run(G_APPLICATION(app), 1, argv);

    // Clean up
    g_object_unref(document);
    g_object_unref(app);

    return status;
}
