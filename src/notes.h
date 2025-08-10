enum file_extension {
    MARKDOWN_FILE,
    TEXT_FILE,
    UNKNOWN_FILE,
};

typedef struct {
    GString *notes_absolute_path;
    enum file_extension extension;
    gboolean notes_loaded;
} notes_data;

extern notes_data data_notes;

void open_notes_action(GSimpleAction *action, GVariant *parameter, gpointer user_dat);
void load_notes_file(const char *filepath);
void get_note_file_extension(void);
gboolean defer_notes_loading(int argc, char **argv);
void load_defered_notes(void);
void load_slide_notes(const size_t slide);
