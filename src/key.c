#include <gtk/gtk.h>
#include <poppler.h>

#include "main.h"
#include "pdf.h"
#include "ui.h"
#include "key.h"
#include "notes.h"

// Callback function for key release events
gboolean on_key_pressed([[gnu::unused]]GtkEventControllerKey *controller, guint keyval, [[gnu::unused]]guint keycode, GdkModifierType state, gpointer user_data) {
    // Check for specific keys
    switch (keyval) {
    case GDK_KEY_F5:
        if (state & GDK_SHIFT_MASK)
            present_current_action(NULL, NULL, user_data);
        else
            present_first_action(NULL, NULL, user_data);
        return TRUE;
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
    case GDK_KEY_j:
    case GDK_KEY_J:
    case GDK_KEY_h:
    case GDK_KEY_H:
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
    case GDK_KEY_k:
    case GDK_KEY_K:
    case GDK_KEY_l:
    case GDK_KEY_L:
    case GDK_KEY_n:
    case GDK_KEY_N:
    case GDK_KEY_Up:
    case GDK_KEY_Right:
        // g_print("Key 'n' or 'N' was pressed\n");
        next_PDF_page();
        // g_print("%d on %d", pdf_data.current_page, pdf_data.total_pages);
        // Add your logic here
        return TRUE; // Event handled

    case GDK_KEY_g:
        custom_PDF_page(0);
        return TRUE;
    case GDK_KEY_G:
        custom_PDF_page(pdf_data.total_pages - 1);
        return TRUE;

    case GDK_KEY_q:
    case GDK_KEY_Q:
        if (data_presentation.in_presentation) {
            finish_presentation_action(GTK_WINDOW(user_data), user_data);
        }
        else
            exit(0);
        return TRUE;

    case GDK_KEY_o:
    case GDK_KEY_O:
        if ((state & GDK_CONTROL_MASK) && (state & GDK_SHIFT_MASK)) {
            open_notes_action(NULL, NULL, user_data);
        } else if (state & GDK_CONTROL_MASK) {
            open_PDF_action(NULL, NULL, user_data);
        }
        return TRUE;

    case GDK_KEY_minus:
        notes_smaller_action(NULL, NULL, user_data);
        return TRUE;

    case GDK_KEY_plus:
        notes_bigger_action(NULL, NULL, user_data);
        return TRUE;

    default:
        // Let other keys pass through
        return FALSE;
    }
}
