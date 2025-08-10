typedef struct {
    GString *notes_absolute_path;
    gboolean notes_loaded;
} notes_data;

extern notes_data data_notes;

void open_notes_action(GSimpleAction *action, GVariant *parameter, gpointer user_dat);
void load_notes_file(const char *filepath);
gboolean defer_notes_loading(int argc, char **argv);
void load_defered_notes(void);
