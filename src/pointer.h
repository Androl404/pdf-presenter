#ifndef POINTER_H
#define POINTER_H

typedef struct Pointer {
    gboolean activated;
    gdouble x, y;
} Pointer;

extern Pointer pointer_data;

gboolean pointer_switch_callback(GtkSwitch *self, gboolean state, gpointer user_data);
void pointer_motion_event(GtkEventController *gesture, gdouble x, gdouble y, gpointer user_data);

#endif // POINTER_H
