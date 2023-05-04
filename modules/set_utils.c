#include "set_utils.h"
#include <stdlib.h>

// Επιστρέφει την μοναδική τιμή του set που είναι ισοδύναμη με value,
// ή αν δεν υπάρχει, την μικρότερη τιμή του set που είναι μεγαλύτερη
// από value. Αν δεν υπάρχει καμία τότε επιστρέφει NULL.

Pointer set_find_eq_or_greater(Set set, Pointer value)
{
    Pointer ptr = set_find(set, value);
    if (ptr == NULL)
    {
        set_insert(set, value);
        if (set_next(set, set_find_node(set, value)) != SET_EOF)
            ptr = set_node_value(set, set_next(set, set_find_node(set, value)));
        set_remove(set, value);
    }
    return ptr;
}

// Επιστρέφει την μοναδική τιμή του set που είναι ισοδύναμη με value,
// ή αν δεν υπάρχει, την μεγαλύτερη τιμή του set που είναι μικρότερη
// από value. Αν δεν υπάρχει καμία τότε επιστρέφει NULL.

Pointer set_find_eq_or_smaller(Set set, Pointer value)
{
    Pointer ptr = set_find(set, value);
    if (ptr == NULL)
    {
        set_insert(set, value);
        if (set_previous(set, set_find_node(set, value)) != SET_BOF)
            ptr = set_node_value(set, set_previous(set, set_find_node(set, value)));
        set_remove(set, value);
    }
    return ptr;
}
