//////////////////////////////////////////////////////////////////
//
// Test για το state.h module
//
//////////////////////////////////////////////////////////////////

#include "acutest.h" // Απλή βιβλιοθήκη για unit testing

#include "state.h"
#include "set_utils.h"
#include "interface.h"

static int compare_ptrs(Pointer a, Pointer b)
{
	return a - b;
}

// Δεσμεύει μνήμη για έναν int, αντιγράφει το value εκεί και επιστρέφει pointer
static int *create_ints(int value)
{
	int *pointer = malloc(sizeof(int)); // δέσμευση μνήμης
	*pointer = value;					// αντιγραφή του value στον νέο int
	return pointer;
}

void test_state_create()
{

	State state = state_create();
	TEST_ASSERT(state != NULL);

	StateInfo info = state_info(state);
	TEST_ASSERT(info != NULL);

	TEST_ASSERT(info->current_portal == 0);
	TEST_ASSERT(info->wins == 0);
	TEST_ASSERT(info->playing);
	TEST_ASSERT(!info->paused);

	// Προσθέστε επιπλέον ελέγχους
	TEST_ASSERT(info->character->forward == true);
	TEST_ASSERT(info->character->jumping == false);
	TEST_ASSERT(info->character->type == CHARACTER);
	TEST_ASSERT(info->character->rect.height == 38);
	TEST_ASSERT(info->character->rect.width == 70);
	TEST_ASSERT(info->character->rect.x == 0);
	TEST_ASSERT(info->character->rect.y == SCREEN_HEIGHT - info->character->rect.height);

	//Έλεγχοι για state_objects
	List list_obj1 = state_objects(state, 0, 1000);
	TEST_ASSERT(list_obj1 != NULL);
	TEST_ASSERT(list_size(list_obj1) > 0);

	List list_obj2 = state_objects(state, 1000, 2000);
	TEST_ASSERT(list_obj2 != NULL);
	TEST_ASSERT(list_size(list_obj2) > 0);

	list_destroy(list_obj1);
	list_destroy(list_obj2);

	state_destroy(state);
}

void test_state_update()
{
	State state = state_create();
	TEST_ASSERT(state != NULL && state_info(state) != NULL);

	// Πληροφορίες για τα πλήκτρα (αρχικά κανένα δεν είναι πατημένο)
	struct key_state keys = {false, false, false, false, false, false};

	// Χωρίς κανένα πλήκτρο, ο χαρακτήρας μετακινείται 7 pixels μπροστά
	Rectangle old_rect = state_info(state)->character->rect;
	state_update(state, &keys);
	Rectangle new_rect = state_info(state)->character->rect;

	TEST_ASSERT(new_rect.x == old_rect.x + 7 && new_rect.y == old_rect.y);

	// Με πατημένο το δεξί βέλος, ο χαρακτήρας μετακινείται 12 pixes μπροστά
	keys.right = true;
	old_rect = state_info(state)->character->rect;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;

	TEST_ASSERT(new_rect.x == old_rect.x + 12 && new_rect.y == old_rect.y);

	// Προσθέστε επιπλέον ελέγχους

	// Με πατημένο το αριστερό βέλος, ο χαρακτήρας αλλάζει κατεύθυνση και μετακινείται 7 pixes πίσω
	keys.left = true;
	keys.right = false;
	old_rect = state_info(state)->character->rect;
	bool old_forward = state_info(state)->character->forward;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;
	bool new_forward = state_info(state)->character->forward;

	TEST_ASSERT(new_rect.x == old_rect.x - 7 && new_rect.y == old_rect.y && old_forward == true && new_forward == false);

	// Με πατημένο το αριστερό βέλος, ενώ κινείται αριστερά, ο χαρακτήρας  μετακινείται 12 pixes πίσω
	keys.left = true;
	old_rect = state_info(state)->character->rect;
	old_forward = state_info(state)->character->forward;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;
	new_forward = state_info(state)->character->forward;

	TEST_ASSERT(new_rect.x == old_rect.x - 12 && new_rect.y == old_rect.y && old_forward == false && new_forward == false);

	//Αλλαγή κατεύθυνσης απο αριστερά σε δεξιά
	keys.right = true;
	keys.left = false;
	old_rect = state_info(state)->character->rect;
	old_forward = state_info(state)->character->forward;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;
	new_forward = state_info(state)->character->forward;

	TEST_ASSERT(new_rect.x == old_rect.x + 7 && new_rect.y == old_rect.y && old_forward == false && new_forward == true);

	// Ενώ πηγαίνει δεξία, με πατημένο το πάνω βέλος, ο χαρακτήρας μετακινείται 7 pixes μπροστά,
	// μπαίνει σε κατάσταση άλματος και πηδάει 15 pixes προς τα πάνω
	keys.right = false;
	keys.up = true;
	old_rect = state_info(state)->character->rect;
	bool old_jumping = state_info(state)->character->jumping;
	state_update(state, &keys);
	new_rect = state_info(state)->character->rect;
	bool new_jumping = state_info(state)->character->jumping;

	TEST_ASSERT(new_rect.x == old_rect.x + 7 && new_rect.y == old_rect.y - 15 && old_jumping == false && new_jumping == true);

	state_destroy(state);
}

// //Τεστ για set_utils module void test_set_utils()
void test_set_utils()
{
	Set set = set_create(compare_ptrs, free);

	set_insert(set, create_ints(2));
	int *p1 = create_ints(10);
	set_insert(set, p1);
	set_insert(set, create_ints(-30));
	set_insert(set, create_ints(0));
	set_insert(set, create_ints(39));
	int *p2 = create_ints(-9);
	set_insert(set, p2);

	TEST_ASSERT(set_find_eq_or_greater(set, p1) != NULL);
	TEST_ASSERT(set_find_eq_or_smaller(set, p2) != NULL);

	set_destroy(set);
}

void test_interface_init()
{
	interface_init();
}

State state;

static void update_and_draw()
{
	// Πληροφορίες για τα πλήκτρα (αρχικά κανένα δεν είναι πατημένο)
	struct key_state keys = {false, false, true, false, false, false};

	state_update(state, &keys);
	interface_draw_frame(state);
}

void test_interface_draw_frame()
{
	state = state_create();
	start_main_loop(update_and_draw);
}

void test_interface_close()
{
	interface_close();
}

// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {
	{"test_state_create", test_state_create},
	{"test_state_update", test_state_update},
	{"test_set_utils", test_set_utils},
	{"test_interface_init", test_interface_init},
	{"test_interface_draw_frame", test_interface_draw_frame},
	{"test_interface_close", test_interface_close},
	{NULL, NULL}, // τερματίζουμε τη λίστα με NULL
};