#include <gtk/gtk.h>
#include <poppler.h>

#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "pdf.h"
#include "ui.h"

char *pdf_to_load = NULL;

// This function verifies if there is a PDF from command line argument
gboolean defer_pdf_loading(int argc, char **argv) {
    if (argc >= 2) {
        pdf_to_load = argv[1];
        return true;
    } else {
        return false;
    }
}

// This function loads the PDF from command line argument
void load_defered_pdf(void) {
    if (pdf_to_load != NULL)
        load_PDF_file(pdf_to_load);
}

void load_PDF_file(const char* path) {
    // Load PDF document
    GError *error = NULL;
    char *uri = g_filename_to_uri(realpath(path, pdf_data.absolute_PDF_path), NULL, &error);

    // Verify path of PDF
    if (!uri) {
        if (error) {
            g_print("Failed to convert filename to URI: %s\n", error->message);
        } else {
            g_print("Failed to convert filename to URI: the file was probably not found\n");
        }
        g_error_free(error);
        return;
    }

    document = poppler_document_new_from_file(uri, NULL, &error);
    pdf_data.total_pages = poppler_document_get_n_pages(document);
    pdf_data.current_page = 0;
    g_free(uri);

    // Set level bar
    gtk_level_bar_set_min_value(GTK_LEVEL_BAR(PDF_level_bar), 0.0);
    gtk_level_bar_set_max_value(GTK_LEVEL_BAR(PDF_level_bar), (double)pdf_data.total_pages);
    gtk_level_bar_set_value(GTK_LEVEL_BAR(PDF_level_bar), 1.0);
    gtk_level_bar_set_mode(GTK_LEVEL_BAR(PDF_level_bar), GTK_LEVEL_BAR_MODE_CONTINUOUS);

    // Verify PDF document
    if (!document) {
        g_print("Failed to open PDF: %s\n", error->message);
        g_error_free(error);
        return;
    }

    // Update slides label
    update_slides_label();

    // Update PDF path label
    gtk_label_set_label(GTK_LABEL(pdf_path_label), pdf_data.absolute_PDF_path);
}

void next_PDF_page(void) {
    // Verify if a PDF file is opened and the PDF page exists
    if (pdf_data.absolute_PDF_path[0] == 0 || !(pdf_data.current_page < pdf_data.total_pages - 1)) {
        gtk_widget_error_bell(current_page_drawing_area);
        return;
    }

    // Actually update PDF pages
    pdf_data.current_page++;
    queue_all_drawing_areas(); // Redraw all drawing areas

    // Update PDF level bar
    update_level_bar();

    // Update slides label
    update_slides_label();
}

void previous_PDF_page(void) {
    // Verify if a PDF file is opened and if the PDF page exists
    if (pdf_data.absolute_PDF_path[0] == 0 || !(pdf_data.current_page > 0)) {
        gtk_widget_error_bell(current_page_drawing_area);
        return;
    }

    // Actually update PDF pages
    pdf_data.current_page--;
    queue_all_drawing_areas(); // Redraw all drawing areas

    // Update PDF level bar
    update_level_bar();

    // Update slides label
    update_slides_label();
}

void queue_all_drawing_areas() {
    gtk_widget_queue_draw(current_page_drawing_area);
    gtk_widget_queue_draw(next_page_drawing_area);
    if (data_presentation.in_presentation)
        gtk_widget_queue_draw(presentation_drawing_area);
}

// This function is called each time the drawing area gets resized
void draw_current_page(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
    // Verify if document exists
    if (!document) return;

    // Create poppler page
    PopplerPage *page = poppler_document_get_page(document, pdf_data.current_page);
    if (!page) return;

    // Get page dimensions
    double page_width, page_height;
    poppler_page_get_size(page, &page_width, &page_height);

    // Calculate scale to fit the page to the window while maintaining aspect ratio
    double scale_x = width / page_width;
    double scale_y = height / page_height;
    double scale = scale_x < scale_y ? scale_x : scale_y;

    // Calculate the scaled page dimensions
    double scaled_width = page_width * scale;
    double scaled_height = page_height * scale;

    // Calculate centering offsets
    double offset_x = (width - scaled_width) / 2;
    double offset_y = (height - scaled_height) / 2;

    // Clear entire background with black
    cairo_set_source_rgb(cr, 0.18, 0.18, 0.18); // Black background
    cairo_paint(cr);

    // Draw white rectangle for the PDF area
    cairo_set_source_rgb(cr, 1, 1, 1); // White for PDF background
    cairo_rectangle(cr, offset_x, offset_y, scaled_width, scaled_height);
    cairo_fill(cr);

    // Save the current state
    cairo_save(cr);

    // Translate to center the page
    cairo_translate(cr, offset_x, offset_y);

    // Apply scaling
    cairo_scale(cr, scale, scale);

    // Render the PDF page
    poppler_page_render(page, cr);

    // Restore the previous state
    cairo_restore(cr);

    g_object_unref(page);

    // Update slides label
    update_slides_label();
}

// This function is called each time the drawing area gets resized
void draw_next_page(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
    if (!document) return;

    if ((pdf_data.current_page + 1) >= pdf_data.total_pages) return;
    PopplerPage *page = poppler_document_get_page(document, pdf_data.current_page + 1);
    if (!page) return;

    // Get page dimensions
    double page_width, page_height;
    poppler_page_get_size(page, &page_width, &page_height);

    // Calculate scale to fit the page to the window while maintaining aspect ratio
    double scale_x = width / page_width;
    double scale_y = height / page_height;
    double scale = scale_x < scale_y ? scale_x : scale_y;

    // Calculate the scaled page dimensions
    double scaled_width = page_width * scale;
    double scaled_height = page_height * scale;

    // Calculate centering offsets
    double offset_x = (width - scaled_width) / 2;
    double offset_y = (height - scaled_height) / 2;

    // Clear entire background with black
    cairo_set_source_rgb(cr, 0.18, 0.18, 0.18); // Black background
    cairo_paint(cr);

    // Draw white rectangle for the PDF area
    cairo_set_source_rgb(cr, 1, 1, 1); // White for PDF background
    cairo_rectangle(cr, offset_x, offset_y, scaled_width, scaled_height);
    cairo_fill(cr);

    // Save the current state
    cairo_save(cr);

    // Center the page
    cairo_translate(cr, (width - page_width * scale) / 2, (height - page_height * scale) / 2);

    // Apply scaling
    cairo_scale(cr, scale, scale);

    // Render the page
    poppler_page_render(page, cr);

    // Restore the previous state
    cairo_restore(cr);

    g_object_unref(page);
}
