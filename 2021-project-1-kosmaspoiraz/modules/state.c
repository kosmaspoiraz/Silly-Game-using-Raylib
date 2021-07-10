
#include <stdlib.h>
#include <time.h>

#include "ADTVector.h"
#include "ADTList.h"
#include "state.h"

// Οι ολοκληρωμένες πληροφορίες της κατάστασης του παιχνιδιού.
// Ο τύπος State είναι pointer σε αυτό το struct, αλλά το ίδιο το struct
// δεν είναι ορατό στον χρήστη.

struct state
{
	Vector objects;	   // περιέχει στοιχεία Object (Εμπόδια / Εχθροί / Πύλες)
	List portal_pairs; // περιέχει PortaPair (ζευγάρια πυλών, είσοδος/έξοδος)

	struct state_info info;
};

// Ζευγάρια πυλών

typedef struct portal_pair
{
	Object entrance; // η πύλη entrance
	Object exit;	 // οδηγεί στην exit
} * PortalPair;

// Συγκρίνει 2 Objects a, b

int compare_right(Pointer a, Pointer b)
{
	Object portal = a;
	PortalPair pair = b;
	if (portal == pair->entrance)
	{
		return 0; // a είναι το entrance του ζευγαριού b
	}
	else
	{
		return 1; //
	}
}

int compare_left(Pointer a, Pointer b)
{
	Object portal = a;
	PortalPair pair = b;
	if (portal == pair->exit)
	{

		return 0; // a είναι το exit του ζευγαριού b
	}
	else
	{
		return 1; //
	}
}

// Δεσμεύει μνήμη για έναν int, αντιγράφει το value εκεί και επιστρέφει pointer
int *create_int(int value)
{
	int *pointer = malloc(sizeof(int)); // δέσμευση μνήμης
	*pointer = value;					// αντιγραφή του value στον νέο int
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
	state->info.wins = 0;			// Δεν έχουμε νίκες ακόμα
	state->info.playing = true;		// Το παιχνίδι ξεκινάει αμέσως
	state->info.paused = false;		// Χωρίς να είναι paused.

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
	character->rect.y = SCREEN_HEIGHT - state->info.character->rect.height;

	// Δημιουργία των objects (πύλες / εμπόδια / εχθροί) και προσθήκη στο vector
	// state->objects. Η πίστα περιέχει συνολικά 4*PORTAL_NUM αντικείμενα, από
	// τα οποία τα PORTAL_NUM είναι πύλες, και τα υπόλοια εμπόδια και εχθροί.

	state->objects = vector_create(0, free); // Δημιουργία του vector

	for (int i = 0; i < 4 * PORTAL_NUM; i++)
	{
		// Δημιουργία του Object και προσθήκη στο vector
		Object obj = malloc(sizeof(*obj));
		vector_insert_last(state->objects, obj);

		// Κάθε 4 αντικείμενα υπάρχει μια πύλη. Τα υπόλοιπα αντικείμενα
		// επιλέγονται τυχαία.

		if (i % 4 == 3)
		{						// Το 4ο, 8ο, 12ο κλπ αντικείμενο
			obj->type = PORTAL; // είναι πύλη.
			obj->rect.width = 100;
			obj->rect.height = 5;
		}
		else if (rand() % 2 == 0)
		{						  // Για τα υπόλοιπα, με πιθανότητα 50%
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
		obj->rect.y = SCREEN_HEIGHT - obj->rect.height;
	}

	// TODO: αρχικοποίηση της λίστας obj->portal_pairs

	//Επιλογή portals απο το state->objects και εισαγωγή σε vector
	Vector portals = vector_create(0, free);
	for (VectorNode obj_node = vector_first(state->objects); obj_node != VECTOR_EOF; obj_node = vector_next(state->objects, obj_node))
	{
		Object obj = vector_node_value(state->objects, obj_node);
		if (obj->type == PORTAL)
		{
			vector_insert_last(portals, obj);
		}
	}

	//Δημιουργία της λίστας obj->portal_pairs
	state->portal_pairs = list_create(free);

	//Γέμισμα της λίστας obj->portal_pairs
	int array_used[PORTAL_NUM] = {0};
	int j = 0;

	for (VectorNode obj_node = vector_first(portals); obj_node != VECTOR_EOF; obj_node = vector_next(portals, obj_node))
	{
		Object obj = vector_node_value(portals, obj_node);
		PortalPair pair = malloc(sizeof(PortalPair));
		int i = rand() % vector_size(portals);
		int count = 0;
		bool used = true;

		//όοο υπάρχουν πόρτες δεξιά του επιλεγμένου στο vector
		while (used == true && i < PORTAL_NUM)
		{
			for (int k = 0; k < sizeof(array_used); k++)
			{
				if (array_used[k] == i)
				{
					used = true;
					break;
				}
				else
				{
					used = false;
				}
			}
			if (used == false)
			{
				break;
			}
			i++;
			count++;
		}
		//όοο υπάρχουν πόρτες αριστερά του επιλεγμένου στο vector
		if (used == true)
		{
			i = i - count;
		}
		while (used == true && i > 0)
		{
			for (int k = 0; k < PORTAL_NUM; k++)
			{
				if (array_used[k] == i)
				{
					used = true;
					break;
				}
				else
				{
					used = false;
				}
			}
			if (used == false)
			{
				break;
			}
			i--;
		}

		pair->entrance = obj;
		pair->exit = vector_get_at(portals, i);
		array_used[j] = i; //αποθήκευση αριθμού πόρτας που χρησιμοποιήθηκε σαν έξοδος
		j++;

		list_insert_next(state->portal_pairs, LIST_BOF, pair);
	}

	// vector_destroy(portals);

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

// Επιστρέφει μια λίστα με όλα τα αντικείμενα του παιχνιδιού στην κατάσταση state,
// των οποίων η συντεταγμένη x είναι ανάμεσα στο x_from και x_to.
List state_objects(State state, float x_from, float x_to)
{
	//Δημιουργία λίστας προς επιστροφή
	List list_objs = list_create(NULL);

	//Γέμισμα της λίστας με αντικείμενα εντός του διαστήματος [x_from, x_to]
	for (VectorNode obj_node = vector_first(state->objects); obj_node != VECTOR_EOF; obj_node = vector_next(state->objects, obj_node))
	{
		Object obj = vector_node_value(state->objects, obj_node);
		if (obj->rect.x >= x_from && obj->rect.x <= x_to)
		{
			list_insert_next(list_objs, LIST_EOF, obj);
		}
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

			//Κίνηση εχθρών 5pixes per frame στην κατεύθυνσή τους
			for (VectorNode obj_node = vector_first(state->objects); obj_node != VECTOR_EOF; obj_node = vector_next(state->objects, obj_node))
			{
				Object obj = vector_node_value(state->objects, obj_node);
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

			//Συγκρούσεις
			for (VectorNode obj_node = vector_first(state->objects); obj_node != VECTOR_EOF; obj_node = vector_next(state->objects, obj_node))
			{
				int i = 0;
				Object obj = vector_node_value(state->objects, obj_node);

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
							PortalPair pp = list_find(state->portal_pairs, obj, compare_right);

							//Bρίσκω τον αριθμό της πόρτας για να ελέγξω αν ο παίκτης νικάει
							for (VectorNode portal_node = vector_first(state->objects); portal_node != VECTOR_EOF; portal_node = vector_next(state->objects, portal_node))
							{
								Object portal = vector_node_value(state->objects, portal_node);
								if (portal->type == PORTAL)
								{
									if (portal == pp->exit)
									{
										state->info.current_portal = i / 4;
									}
									else
									{
										i = i + 3;
									}
								}
							}

							//Έλεγχος νίκης
							if (state->info.current_portal == PORTAL_NUM)
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
								state->info.character->rect.x = pp->exit->rect.x + pp->exit->rect.width + 7;
							}
						}
						//Κίνηση προς τα αριστερά
						else
						{
							PortalPair pp = list_find(state->portal_pairs, obj, compare_left);

							//Bρίσκω τον αριθμό της πόρτας για να ελέγξω αν ο παίκτης νικάει
							for (VectorNode portal_node = vector_first(state->objects); portal_node != VECTOR_EOF; portal_node = vector_next(state->objects, portal_node))
							{
								Object portal = vector_node_value(state->objects, portal_node);
								if (portal->type == PORTAL)
								{
									if (portal == pp->entrance)
									{
										state->info.current_portal = i / 4;
									}
									else
									{
										i = i + 3;
									}
								}
							}

							//έλεγχος νίκης
							if (state->info.current_portal == PORTAL_NUM)
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
								state->info.character->rect.x = pp->entrance->rect.x - pp->entrance->rect.width;
							}
						}
					}
				}

				//Συγκρούσεις εχθρών
				if (obj->type == ENEMY)
				{
					for (VectorNode obj2_node = vector_first(state->objects); obj2_node != VECTOR_EOF; obj2_node = vector_next(state->objects, obj2_node))
					{
						Object obj2 = vector_node_value(state->objects, obj2_node);
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
									PortalPair pair = list_find(state->portal_pairs, obj2, compare_right);
									obj->rect.x = pair->exit->rect.x + pair->exit->rect.width;
								}
								//Κίνηση προς τα αριστερά
								else
								{
									PortalPair pair = list_find(state->portal_pairs, obj2, compare_left);
									obj->rect.x = pair->entrance->rect.x - pair->entrance->rect.width;
								}
							}
						}
					}
				}
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
			if (keys->n)
			{
				state->info.paused = false;
				keys->p = false;
				state_update(state, keys);
			}
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
	vector_destroy(state->objects);

	list_destroy(state->portal_pairs);

	free(state->info.character);

	free(state);
}