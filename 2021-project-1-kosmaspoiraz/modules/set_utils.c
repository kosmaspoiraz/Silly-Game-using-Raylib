#include "set_utils.h"
#include <stdlib.h>

// Επιστρέφει την μοναδική τιμή του set που είναι ισοδύναμη με value,
// ή αν δεν υπάρχει,  ττην μικρότερη τιμή του set που είναι μεγαλύτερη
// από value. Αν δεν υπάρχει καμία τότε επιστρέφει NULL.

Pointer set_find_eq_or_greater(Set set, Pointer value)
{
    if (set_find(set, value) != NULL)
    {
        return set_find(set, value);
    }
    else
    {
        set_insert(set, value);
        Pointer to_return = set_node_value(set, set_next(set, set_find_node(set, value)));
        set_remove(set, value);
        return to_return;
    }
}

// Επιστρέφει την μοναδική τιμή του set που είναι ισοδύναμη με value,
// ή αν δεν υπάρχει, την μεγαλύτερη τιμή του set που είναι μικρότερη
// από value. Αν δεν υπάρχει καμία τότε επιστρέφει NULL.

Pointer set_find_eq_or_smaller(Set set, Pointer value)
{
    if (set_find(set, value) != NULL)
    {
        return set_find(set, value);
    }
    else
    {
        set_insert(set, value);
        SetNode node = set_find_node(set, value);
        SetNode pnode = set_previous(set, node);
        if (pnode == NULL)
        {
            set_remove(set, value);
            return NULL;
        }
        Pointer to_return = set_node_value(set, pnode);
        set_remove(set, value);
        return to_return;
    }
}