extern GtkWidget *current_page_drawing_area;
extern GtkWidget *next_page_drawing_area;
extern GtkWidget *PDF_level_bar;
extern GtkWidget *state_label;
extern GtkWidget *datetime_label;
extern GtkWidget *pdf_path_label;

void open_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void on_activate(GtkApplication *app, gpointer user_data);
void update_slides_label(void);
gboolean sync_datetime_label(gpointer user_data);
