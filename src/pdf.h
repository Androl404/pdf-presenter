extern PopplerDocument *document;
extern char absolute_PDF_path[PATH_MAX + 1]; // Stores the absolute file path of the PDF file

void draw_function(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data);
