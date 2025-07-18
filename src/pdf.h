typedef struct {
    char absolute_PDF_path[PATH_MAX + 1];
    size_t current_page;
    size_t total_pages;
} PDF_data;

extern PopplerDocument *document;
extern PDF_data pdf_data;
// extern char absolute_PDF_path[PATH_MAX + 1]; // Stores the absolute file path of the PDF file

gboolean defer_pdf_loading(int argc, char **argv);
void load_defered_pdf(void);
void draw_current_page(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data);
void draw_next_page(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data);
void load_PDF_file(const char* path);
void next_PDF_page(void);
void previous_PDF_page(void);
