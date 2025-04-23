#include <gtk/gtk.h>
#include <poppler.h>

#include "pdf.h"

// This function is called each time the drawing area gets resized
void draw_function(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
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
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5); // To set the background color
    cairo_paint(cr);

    // Center the page
    cairo_translate(cr, (width - page_width * scale) / 2, (height - page_height * scale) / 2);

    // Apply scaling
    cairo_scale(cr, scale, scale);

    // Render the page
    poppler_page_render(page, cr);

    g_object_unref(page);
}
