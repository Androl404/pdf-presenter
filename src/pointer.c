#include <gtk/gtk.h>

#include "ui.h"
#include "pointer.h"

Pointer pointer_data;

gboolean pointer_switch_callback(GtkSwitch *self, gboolean state, [[gnu::unused]]gpointer user_data) {
    pointer_data.activated = state;
    gtk_switch_set_state (self, pointer_data.activated);
    return TRUE;
}

void pointer_motion_event([[gnu::unused]]GtkEventController *gesture, gdouble x, gdouble y, [[gnu::unused]]gpointer user_data) {
    pointer_data.mouse_x = x;
    pointer_data.mouse_y = y;
    gtk_widget_queue_draw(current_page_drawing_area);
    if (data_presentation.in_presentation)
        gtk_widget_queue_draw(presentation_drawing_area);
}

void calculate_pointer_positions(gdouble offset_x, gdouble offset_y, gdouble page_width, gdouble page_height) {
    pointer_data.current_page_x = pointer_data.mouse_x / ((double)gtk_widget_get_size(current_page_drawing_area, GTK_ORIENTATION_HORIZONTAL) - (offset_x * 2.0)) * page_width - offset_x / 2.0;
    pointer_data.current_page_y = pointer_data.mouse_y / ((double)gtk_widget_get_size(current_page_drawing_area, GTK_ORIENTATION_VERTICAL) - (offset_y * 2.0)) * page_height - offset_y / 2.0;
    if (data_presentation.in_presentation) {
        pointer_data.presentation_page_x = pointer_data.mouse_x / ((double)gtk_widget_get_size(presentation_drawing_area, GTK_ORIENTATION_HORIZONTAL) - (offset_x * 2.0)) * page_width - offset_x / 2.0;
        pointer_data.presentation_page_y = pointer_data.mouse_y / ((double)gtk_widget_get_size(presentation_drawing_area, GTK_ORIENTATION_VERTICAL) - (offset_y * 2.0)) * page_height - offset_y / 2.0;
    }
}
