#include <gtk/gtk.h>
#include <poppler.h>

static PopplerDocument *document = NULL;

gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    if (!document) return FALSE;

    PopplerPage *page = poppler_document_get_page(document, 0); // First page
    if (!page) return FALSE;

    // Set the desired scale (1.0 = 100%)
    double width, height;
    poppler_page_get_size(page, &width, &height);
    cairo_scale(cr, 3.0, 3.0);

    // Render the page to the Cairo context
    poppler_page_render(page, cr);
    g_object_unref(page);

    return FALSE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    if (argc < 2) {
        g_print("Usage: %s <PDF file>\n", argv[0]);
        return 1;
    }

    GError *error = NULL;
    document = poppler_document_new_from_file(g_strdup_printf("file://%s", argv[1]), NULL, &error);
    if (!document) {
        g_print("Failed to open PDF: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "PDF Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), NULL);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    g_object_unref(document);
    return 0;
}
