#include <stdlib.h>
#include <time.h>

#include "ADTSet.h"
#include "ADTMap.h"
#include "state.h"
#include "set_utils.h"

// Οι ολοκληρωμένες πληροφορίες της κατάστασης του παιχνιδιού.
// Ο τύπος State είναι pointer σε αυτό το struct, αλλά το ίδιο το struct
// δεν είναι ορατό στον χρήστη.

struct state
{
    Set objects;          // περιέχει στοιχεία Object (Εμπόδια / Εχθροί / Πύλες)
    Map portal_pairs;     // περιέχει PortaPair (ζευγάρια πυλών, είσοδος/έξοδος)
    Map inv_portal_pairs; // περιέχει PortaPair (ζευγάρια πυλών, έξοδος/είσοδος)

    struct state_info info;
};

// Ζευγάρια πυλών

// typedef struct portal_pair
// {
//     Object entrance; // η πύλη entrance
//     Object exit;     // οδηγεί στην exit
// } * PortalPair;

// Συγκρίνει 2 Pointers

int compare(Pointer a, Pointer b)
{
    // return a - b;
    Object obj1 = a;
    Object obj2 = b;
    float retx = obj1->rect.x - obj2->rect.x;
    if (retx > 0)
    {
        return 1;
    }
    else if (retx < 0)
    {
        return -1;
    }
    else
    {
        float rety = obj1->rect.y - obj2->rect.y;
        if (rety > 0)
        {
            return 1;
        }
        else if (rety < 0)
        {
            return -1;
        }
        else
        {
            return obj1 - obj2;
        }
    }
}

int compare_int(Pointer a, Pointer b)
{
    return a - b;
}

// Δεσμεύει μνήμη για έναν float, αντιγράφει το value εκεί και επιστρέφει pointer
float *create_float(float value)
{
    float *pointer = malloc(sizeof(float)); // δέσμευση μνήμης
    *pointer = value;                       // αντιγραφή του value στον νέο float
    return pointer;
}

int *create_int(int value)
{
    int *pointer = malloc(sizeof(int)); // δέσμευση μνήμης
    *pointer = value;                   // αντιγραφή του value στον νέο int
    return pointer;
}

// Δημιουργεί και επιστρέφει την αρχική κατάσταση του παιχνιδιού

State state_create()
{
    srand(time(0));
    // Δημιουργία του state
    State state = malloc(sizeof(*state));

    // Γενικές πληροφορίες
    state->info.current_portal = 0; // Δεν έχουμε περάσει καμία πύλη
    state->info.wins = 0;           // Δεν έχουμε νίκες ακόμα
    state->info.playing = true;     // Το παιχνίδι ξεκινάει αμέσως
    state->info.paused = false;     // Χωρίς να είναι paused.

    // Πληροφορίες για το χαρακτήρα.
    Object character = state->info.character = malloc(sizeof(*character));
    character->type = CHARACTER;
    character->forward = true;
    character->jumping = false;

    // Ο χαρακτήρας (όπως και όλα τα αντικείμενα) έχουν συντεταγμένες x,y σε ένα
    // καρτεσιανό επίπεδο.
    // - Στο άξονα x το 0 είναι η αρχή στης πίστας και οι συντεταγμένες
    //   μεγαλώνουν προς τα δεξιά.
    // - Στον άξονα y το 0 είναι το "δάπεδο" της πίστας, και οι
    //   συντεταγμένες μεγαλώνουν προς τα _κάτω_.
    // Πέρα από τις συντεταγμένες, αποθηκεύουμε και τις διαστάσεις width,height
    // κάθε αντικειμένου. Τα x,y,width,height ορίζουν ένα παραλληλόγραμμο, οπότε
    // μπορούν να αποθηκευτούν όλα μαζί στο obj->rect τύπου Rectangle (ορίζεται
    // στο include/raylib.h).
    //
    // Προσοχή: τα x,y αναφέρονται στην πάνω-αριστερά γωνία του Rectangle, και
    // τα y μεγαλώνουν προς τα κάτω, οπότε πχ ο χαρακτήρας που έχει height=38,
    // αν θέλουμε να "κάθεται" πάνω στο δάπεδο, θα πρέπει να έχει y=-38.

    character->rect.width = 70;
    character->rect.height = 38;
    character->rect.x = 0;
    character->rect.y = SCREEN_HEIGHT - character->rect.height;

    // Δημιουργία των objects (πύλες / εμπόδια / εχθροί) και προσθήκη στο vector
    // state->objects. Η πίστα περιέχει συνολικά 4*PORTAL_NUM αντικείμενα, από
    // τα οποία τα PORTAL_NUM είναι πύλες, και τα υπόλοια εμπόδια και εχθροί.

    state->objects = set_create(compare, NULL); // Δημιουργία του set

    for (int i = 0; i < 4 * PORTAL_NUM; i++)
    {
        // Δημιουργία του Object και προσθήκη στο vector
        Object obj = malloc(sizeof(*obj));

        // Κάθε 4 αντικείμενα υπάρχει μια πύλη. Τα υπόλοιπα αντικείμενα
        // επιλέγονται τυχαία.

        if (i % 4 == 3)
        {                       // Το 4ο, 8ο, 12ο κλπ αντικείμενο
            obj->type = PORTAL; // είναι πύλη.
            obj->rect.width = 100;
            obj->rect.height = 5;
        }
        else if (rand() % 2 == 0)
        {                         // Για τα υπόλοιπα, με πιθανότητα 50%
            obj->type = OBSTACLE; // επιλέγουμε εμπόδιο.
            obj->rect.width = 10;
            obj->rect.height = 80;
        }
        else
        {
            obj->type = ENEMY; // Και τα υπόλοιπα είναι εχθροί.
            obj->rect.width = 30;
            obj->rect.height = 30;
            obj->forward = false; // Οι εχθροί αρχικά κινούνται προς τα αριστερά.
        }

        // Τα αντικείμενα είναι ομοιόμορφα τοποθετημένα σε απόσταση SPACING
        // μεταξύ τους, και "κάθονται" πάνω στο δάπεδο.

        obj->rect.x = (i + 1) * SPACING;
        obj->rect.y = -obj->rect.height;

        set_insert(state->objects, obj);
    }

    // TODO: αρχικοποίηση του set obj->portal_pairs

    // γέμισμα array με objects τύπου PORTAL
    Object array_portals[PORTAL_NUM];
    int j = 0;
    for (SetNode obj_node = set_first(state->objects); obj_node != SET_EOF; obj_node = set_next(state->objects, obj_node))
    {
        Object port = set_node_value(state->objects, obj_node);
        if (port->type == PORTAL)
        {
            array_portals[j] = port;
            j++;
        }
    }

    //Δημιουργία Set portal_pairs και PortalPair για εισαγωγή
    state->portal_pairs = map_create(compare, NULL, NULL);
    state->inv_portal_pairs = map_create(compare, NULL, NULL);

    Set set_used = set_create(compare_int, free);

    //Γέμισμα portal_pairs
    for (int i = 0; i < PORTAL_NUM; i++)
    {
        int randi = rand() % PORTAL_NUM;
        int *ri = create_int(randi);
        int count = 0;
        bool used = true;

        //όσο η πιθανή exit υπάρχει μέσα στο array με τις χρησιμοποιημένες exit
        //και υπάρχουν αντικείμενα δεξιά του τυχαίου στο array
        //τσέκαρε το επόμενο
        while (used == true && randi < PORTAL_NUM)
        {
            if (set_find(set_used, ri) != NULL)
            {
                break; //Έχει χρησιμοποιηθεί
            }
            else
            {
                used = false;
                randi++;
                count++;
            }
        }
        //όσο η πιθανή exit υπάρχει μέσα στο array με τις χρησιμοποιημένες exit
        //και υπάρχουν αντικείμενα αριστερά του τυχαίου στο array
        //τσέκαρε το επόμενο(από τα αριστερά)
        if (used == true)
        {
            randi = randi - count - 1;
            ri = create_int(randi);
            while (i > 0)
            {
                if (set_find(set_used, ri) != NULL)
                {
                    break; //Έχει χρησιμοποιηθεί
                }
                else
                {
                    used = false;
                }
                randi--;
            }
        }

        //γέμισμα map με ζευγάρια portals
        map_insert(state->portal_pairs, array_portals[i], array_portals[randi]);

        //αποθήκευση αριθμού τυχαίου portal που χρησιμοποιήθηκε
        set_insert(set_used, ri);
    }

    //γέμισμα inverted map
    for (MapNode node = map_first(state->portal_pairs); node != MAP_EOF; node = map_next(state->portal_pairs, node))
    {
        map_insert(state->inv_portal_pairs, map_node_value(state->portal_pairs, node), map_node_key(state->portal_pairs, node));
    }

    set_destroy(set_used);

    return state;
}

// Επιστρέφει τις βασικές πληροφορίες του παιχνιδιού στην κατάσταση state

StateInfo state_info(State state)
{
    if (state != NULL)
    {
        StateInfo state_info = malloc(sizeof(state_info));

        state_info->current_portal = state->info.current_portal;
        state_info->wins = state->info.wins;
        state_info->playing = state->info.playing;
        state_info->paused = state->info.paused;

        state_info->character = state->info.character;

        return state_info;
    }
    else
    {
        return NULL;
    }
}

// Επιστρέφει ένα set με όλα τα αντικείμενα του παιχνιδιού στην κατάσταση state,
// των οποίων η συντεταγμένη x είναι ανάμεσα στο x_from και x_to.

List state_objects(State state, float x_from, float x_to)
{
    //Δημιουργία list προς επιστροφή
    List list_objs = list_create(NULL);

    //Γέμισμα set με αντικείμενα εντός του διαστήματος [x_from, x_to]
    Pointer p1 = set_find_eq_or_smaller(state->objects, create_float(x_from));
    Pointer p2 = set_find_eq_or_greater(state->objects, create_float(x_to));
    if (p1 != NULL && p2 != NULL)
    {
        for (SetNode obj = set_find_node(state->objects, p1); obj != set_find_node(state->objects, p2); obj = set_next(state->objects, obj))
        {
            if (obj != NULL)
            {
                list_insert_next(list_objs, LIST_EOF, set_node_value(state->objects, obj));
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        return NULL;
    }

    return list_objs;
}

// Ενημερώνει την κατάσταση state του παιχνιδιού μετά την πάροδο 1 frame.
// Το keys περιέχει τα πλήκτρα τα οποία ήταν πατημένα κατά το frame αυτό.

void state_update(State state, KeyState keys)
{
    // Προς υλοποίηση
    if (state->info.playing == true)
    {
        //Κίνηση χαρακτήρα
        if (state->info.paused == false)
        {
            //Κανένα πατημένο πλήκτρο
            if (keys->left != true && keys->right != true)
            {
                //Κινείται δεξιά
                if (state->info.character->forward == true)
                {
                    state->info.character->rect.x = state->info.character->rect.x + 7;
                }
                //Κινείται αριστερά
                else
                {
                    state->info.character->rect.x = state->info.character->rect.x - 7;
                }
            }
            //κινείται δεξια
            else if (state->info.character->forward == true)
            {
                //δεξι κουμπί πατημένο
                if (keys->right)
                {
                    state->info.character->rect.x = state->info.character->rect.x + 12;
                }
                //αριστερό κουμπί πατημένο
                else if (keys->left)
                {
                    state->info.character->forward = false;
                    state->info.character->rect.x = state->info.character->rect.x - 7;
                }
            }
            //κινείται αριστερά
            else if (state->info.character->forward == false)
            {
                //αριστερό κουμπί πατημένο
                if (keys->left)
                {
                    state->info.character->rect.x = state->info.character->rect.x - 12;
                }
                //δεξι κουμπί πατημένο
                else if (keys->right)
                {
                    state->info.character->forward = true;
                    state->info.character->rect.x = state->info.character->rect.x + 7;
                }
            }
            //κίνηση χαρακτήρα όταν πιέζουμε το "πάνω" πλήκτρο
            if (keys->up)
            {
                if (state->info.character->rect.y == SCREEN_HEIGHT - state->info.character->rect.height)
                {
                    state->info.character->jumping = true;
                    state->info.character->rect.y = state->info.character->rect.y - 15;
                }
                else if (state->info.character->rect.y > SCREEN_HEIGHT - state->info.character->rect.height - 220 && state->info.character->jumping == true)
                {
                    state->info.character->rect.y = state->info.character->rect.y - 15;
                }
                else if (state->info.character->rect.y < SCREEN_HEIGHT - state->info.character->rect.height - 220 || state->info.character->jumping == false)
                {
                    state->info.character->jumping = false;
                    state->info.character->rect.y = state->info.character->rect.y + 15;
                }
            }
            else
            {
                if (state->info.character->rect.y < SCREEN_HEIGHT - state->info.character->rect.height)
                {
                    state->info.character->jumping = false;
                    state->info.character->rect.y = state->info.character->rect.y + 15;
                }
                else if (state->info.character->rect.y == SCREEN_HEIGHT - state->info.character->rect.height)
                {
                    state->info.character->jumping = false;
                    state->info.character->rect.y = SCREEN_HEIGHT - state->info.character->rect.height;
                }
            }

            //εισαγωγή των objects σε λίστα to_be_changed
            List to_be_changed = list_create(NULL);
            for (SetNode obj_node = set_first(state->objects); obj_node != SET_EOF; obj_node = set_next(state->objects, obj_node))
            {
                list_insert_next(to_be_changed, LIST_EOF, set_node_value(state->objects, obj_node));
            }

            //αφαίρεση τους από το set προσωρινά
            for (ListNode obj_node = list_first(to_be_changed); obj_node != LIST_EOF; obj_node = list_next(to_be_changed, obj_node))
            {
                set_remove(state->objects, list_node_value(to_be_changed, obj_node));
            }

            //Κίνηση εχθρών 5pixes per frame στην κατεύθυνσή τους
            for (ListNode obj_node = list_first(to_be_changed); obj_node != LIST_EOF; obj_node = list_next(to_be_changed, obj_node))
            {
                Object obj = list_node_value(to_be_changed, obj_node);
                if (obj->type == ENEMY)
                {
                    if (obj->forward == true)
                    {
                        obj->rect.x += 5;
                    }
                    else
                    {
                        obj->rect.x -= 5;
                    }
                }
            }

            //Συγκρούσεις χαρακτήρα
            for (ListNode obj_node = list_first(to_be_changed); obj_node != LIST_EOF; obj_node = list_next(to_be_changed, obj_node))
            {
                int i = 0;
                Object obj = list_node_value(to_be_changed, obj_node);

                //Σύγγρουση χαρακτήρα με εχθρό ή εμπόδιο
                if (obj->type == ENEMY || obj->type == OBSTACLE)
                {
                    if (CheckCollisionRecs(state->info.character->rect, obj->rect))
                    {
                        state->info.playing = false;
                    }
                }
                //Σύγκρουση χαρακτήρα με πόρτα
                else if (obj->type == PORTAL)
                {
                    if (CheckCollisionRecs(state->info.character->rect, obj->rect))
                    {
                        //Κίνηση προς τα δεξιά
                        if (state->info.character->forward)
                        {
                            Object exit = map_find(state->portal_pairs, obj);

                            //Bρίσκω τον αριθμό της πόρτας για να ελέγξω αν ο παίκτης νικάει
                            for (ListNode portal_node = list_first(to_be_changed); portal_node != LIST_EOF; portal_node = list_next(to_be_changed, portal_node))
                            {
                                Object portal = list_node_value(to_be_changed, portal_node);
                                if (portal->type == PORTAL)
                                {
                                    if (portal == exit)
                                    {
                                        state->info.current_portal = i / 4;
                                        break;
                                    }
                                    else
                                    {
                                        i = i + 3;
                                    }
                                }
                            }

                            //Έλεγχος νίκης
                            if (state->info.current_portal == 99)
                            {
                                state->info.wins++;

                                state->info.character->rect.x = 0;
                                state->info.character->forward = true;
                                state->info.character->jumping = false;
                                state->info.character->rect.y = SCREEN_HEIGHT - state->info.character->rect.height;
                                state->info.current_portal = 0;
                            }
                            else
                            {
                                state->info.character->rect.x = exit->rect.x + exit->rect.width + 7;
                            }
                        }
                        //Κίνηση προς τα αριστερά
                        else
                        {
                            // Object entrance = map_find(state->portal_pairs, obj);
                            Object entrance = map_node_value(state->portal_pairs, map_find_node(state->portal_pairs, obj));
                            //Bρίσκω τον αριθμό της πόρτας για να ελέγξω αν ο παίκτης νικάει
                            for (ListNode portal_node = list_first(to_be_changed); portal_node != LIST_EOF; portal_node = list_next(to_be_changed, portal_node))
                            {
                                Object portal = list_node_value(to_be_changed, portal_node);
                                if (portal->type == PORTAL)
                                {
                                    if (portal == entrance)
                                    {
                                        state->info.current_portal = i / 4;
                                        break;
                                    }
                                    else
                                    {
                                        i = i + 3;
                                    }
                                }
                            }

                            //έλεγχος νίκης
                            if (state->info.current_portal == 99)
                            {
                                state->info.wins++;

                                state->info.character->rect.x = 0;
                                state->info.character->forward = true;
                                state->info.character->jumping = false;
                                state->info.character->rect.y = SCREEN_HEIGHT - state->info.character->rect.height;
                                state->info.current_portal = 0;
                            }
                            else
                            {
                                state->info.character->rect.x = entrance->rect.x - entrance->rect.width;
                            }
                        }
                    }
                }

                //Συγκρούσεις εχθρών
                if (obj->type == ENEMY)
                {
                    for (ListNode obj2_node = list_first(to_be_changed); obj2_node != LIST_EOF; obj2_node = list_next(to_be_changed, obj2_node))
                    {
                        Object obj2 = list_node_value(to_be_changed, obj2_node);
                        //Σύγγρουση εχρθού με εμπόδιο και αλλαγή κατεύθυνσης
                        if (obj2->type == OBSTACLE)
                        {
                            if (CheckCollisionRecs(obj->rect, obj2->rect))
                            {
                                obj->forward = (obj->forward) ? false : true;
                            }
                        }
                        //Σύγκουση εχθρου με εχθρού
                        else if (obj2->type == ENEMY)
                        {
                            if (CheckCollisionRecs(obj->rect, obj2->rect))
                            {
                                //αλλαγή κατεύθυνσης πρώτου εχθρού
                                if (obj->forward)
                                {
                                    obj->forward = false;
                                }
                                else
                                {
                                    obj->forward = true;
                                }
                                //αλλαγή κατεύθυνσης δεύτερου εχθρού
                                if (obj2->forward)
                                {
                                    obj2->forward = false;
                                }
                                else
                                {
                                    obj2->forward = true;
                                }
                            }
                        }
                        //Σύγκρουση εχθρού με πύλη
                        else if (obj2->type == PORTAL)
                        {
                            if (CheckCollisionRecs(obj->rect, obj2->rect))
                            {
                                //Κίνηση προς τα δεξιά
                                if (obj->forward)
                                {

                                    Object exit = map_node_value(state->portal_pairs, map_find_node(state->portal_pairs, obj2));
                                    obj->rect.x = exit->rect.x + exit->rect.width;
                                }
                                //Κίνηση προς τα αριστερά
                                else
                                {
                                    Object entrance = map_node_value(state->inv_portal_pairs, map_find_node(state->portal_pairs, obj2));
                                    obj->rect.x = entrance->rect.x + entrance->rect.width;
                                }
                            }
                        }
                    }
                }
            }
            //γέμισμα set με αλλαγμένα αντικείμενα
            //καταστροφη list
            for (ListNode node = list_first(to_be_changed); node != LIST_EOF; node = list_next(to_be_changed, node))
            {
                set_insert(state->objects, list_node_value(to_be_changed, node));
            }
            list_destroy(to_be_changed);
        }

        //Pause παιχνιδιού
        if (keys->p)
        {
            state->info.paused = true;
        }
        if (state->info.paused == true && keys->n)
        {
            state->info.paused = false;
        }
    }
    else
    {
        if (state->info.playing == false && keys->enter)
        {
            state->info.character->rect.x = 0;
            state->info.wins = 0;
            state->info.current_portal = 0;
            state->info.playing = true;
            state->info.paused = false;
        }
    }
}

// Καταστρέφει την κατάσταση state ελευθερώνοντας τη δεσμευμένη μνήμη.
void state_destroy(State state)
{
    // Προς υλοποίηση

    set_destroy(state->objects);

    map_destroy(state->portal_pairs);

    free(state->info.character);

    free(state);
}