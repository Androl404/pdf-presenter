#include <gtk/gtk.h>
#include <poppler.h>

#include "pdf.h"
#include "ui.h"
#include "key.h"

// Callback function for key release events
gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
    // Check for specific keys
    switch (keyval) {
    case GDK_KEY_space:
        if (state & GDK_SHIFT_MASK)
            previous_PDF_page();
        else
            next_PDF_page();
        return TRUE;
    case GDK_KEY_Return:
        if (state & GDK_SHIFT_MASK)
            previous_PDF_page();
        else
            next_PDF_page();
        return TRUE;
    case GDK_KEY_b:
    case GDK_KEY_B:
    case GDK_KEY_k:
    case GDK_KEY_K:
    case GDK_KEY_p:
    case GDK_KEY_P:
    case GDK_KEY_Down:
    case GDK_KEY_Left:
        // g_print("Key 'p' or 'P' was pressed\n");
        previous_PDF_page();
        // Add your logic here
        return TRUE; // Event handled

    case GDK_KEY_f:
    case GDK_KEY_F:
    case GDK_KEY_j:
    case GDK_KEY_J:
    case GDK_KEY_n:
    case GDK_KEY_N:
    case GDK_KEY_Up:
    case GDK_KEY_Right:
        // g_print("Key 'n' or 'N' was pressed\n");
        next_PDF_page();
        // g_print("%d on %d", pdf_data.current_page, pdf_data.total_pages);
        // Add your logic here
        return TRUE; // Event handled

    case GDK_KEY_q:
    case GDK_KEY_Q:
        exit(0);
        return TRUE;

    case GDK_KEY_o:
        if (state & GDK_CONTROL_MASK) {
            // gtk_widget_activate_action(GTK_WIDGET(user_data), "win.open", NULL);
            open_action(NULL, NULL, user_data);
        }
        return TRUE;

    default:
        // Let other keys pass through
        return FALSE;
    }
}
