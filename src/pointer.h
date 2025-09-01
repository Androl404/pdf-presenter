#ifndef POINTER_H
#define POINTER_H

typedef struct Pointer {
    gboolean activated;
    gdouble mouse_x, mouse_y;
    gdouble current_page_x, current_page_y;
    gdouble presentation_page_x, presentation_page_y;
} Pointer;

extern Pointer pointer_data;

gboolean pointer_switch_callback(GtkSwitch *self, gboolean state, gpointer user_data);
void pointer_motion_event(GtkEventController *gesture, gdouble x, gdouble y, gpointer user_data);
void calculate_drawing_area_positions(gdouble offset_x, gdouble offset_y, gdouble page_width, gdouble page_height);

#endif // POINTER_H
