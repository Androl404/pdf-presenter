extern GtkWidget *current_page_drawing_area;
extern GtkWidget *next_page_drawing_area;
extern GtkWidget *PDF_level_bar;

void open_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void on_activate(GtkApplication *app, gpointer user_data);
