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
        if (state & 0b1)
            previous_PDF_page();
        else
            next_PDF_page();
        return TRUE;
    case GDK_KEY_p:
    case GDK_KEY_P:
        // g_print("Key 'p' or 'P' was pressed\n");
        previous_PDF_page();
        // Add your logic here
        return TRUE; // Event handled

    case GDK_KEY_n:
    case GDK_KEY_N:
        // g_print("Key 'n' or 'N' was pressed\n");
        next_PDF_page();
        // g_print("%d on %d", pdf_data.current_page, pdf_data.total_pages);
        // Add your logic here
        return TRUE; // Event handled

    default:
        // Let other keys pass through
        return FALSE;
    }
}
