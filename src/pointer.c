#include <gtk/gtk.h>

#include "ui.h"
#include "pointer.h"

Pointer pointer_data;

gboolean pointer_switch_callback(GtkSwitch *self, gboolean state, gpointer user_data) {
    pointer_data.activated = state;
    gtk_switch_set_state (self, pointer_data.activated);
    return TRUE;
}

void pointer_motion_event(GtkEventController *gesture, gdouble x, gdouble y, gpointer user_data) {
    pointer_data.x = x;
    pointer_data.y = y;
    gtk_widget_queue_draw(current_page_drawing_area);
    // g_print(" x = %f, y = %f\n", x, y);
}
