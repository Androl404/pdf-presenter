#include "glib.h"
typedef struct {
    gboolean in_presentation;
    guint window_presentation_id;
} presentation_data;
extern GtkWidget *current_page_drawing_area;
extern GtkWidget *next_page_drawing_area;
extern GtkWidget *presentation_drawing_area;
extern GtkWidget *PDF_level_bar;
extern GtkWidget *state_label;
extern GtkWidget *datetime_label;
extern GtkWidget *chrono_label;
extern GtkWidget *pdf_path_label;
extern presentation_data data_presentation;
extern GDateTime *presentation_start_time;
extern GtkTextBuffer *notes_text_buffer;

void open_PDF_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void on_activate(GtkApplication *app, gpointer user_data);
void update_slides_label(void);
void present_first_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void present_current_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
gboolean finish_presentation_action(GtkWindow *self, gpointer user_data);
gboolean sync_datetime_label(gpointer user_data);
void update_level_bar(void);
