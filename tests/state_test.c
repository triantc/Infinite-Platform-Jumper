//////////////////////////////////////////////////////////////////
//
// Test για το state.h module
//
//////////////////////////////////////////////////////////////////

#include "acutest.h"			// Απλή βιβλιοθήκη για unit testing
#include "set_utils.h"
#include "state.h"

void test_state_create() {

	State state = state_create();
	TEST_ASSERT(state != NULL);

	StateInfo info = state_info(state);
	TEST_ASSERT(info != NULL);

	TEST_ASSERT(info->playing);
	TEST_ASSERT(!info->paused);
	TEST_ASSERT(info->score == 0);

	// Προσθέστε επιπλέον ελέγχους
	TEST_ASSERT(info->ball != NULL);

	for (int i = 1; i < 4; i++)
	{
		List objs = state_objects(state, (i - 1) * SCREEN_WIDTH, i * SCREEN_WIDTH);
		TEST_ASSERT(objs != NULL);
		TEST_ASSERT(list_size(objs) >= 2);

		float start_x = (i - 1) * SCREEN_WIDTH;
		int star_start_x = start_x;  // Για να μπορούμε να ελέγξουμε μετά το x για το STAR
		for (ListNode node = list_first(objs);
		node != LIST_EOF;
		node = list_next(objs, node))
		{
			Object obj = list_node_value(objs, node);
			if (obj->type == PLATFORM)
			{	
				// Αποθηκεύουμε το start_x γιατί μετά θα το αυξήσουμε
				star_start_x = start_x;  
				// Όταν ελέγχουμε από την αρχή της οθόνης ελέγχουμε με +150
				if (i == 1)
					TEST_ASSERT(obj->rect.x >= start_x + 150);
				else
					TEST_ASSERT(obj->rect.x >= start_x);		
				start_x = obj->rect.x + obj->rect.width;
				TEST_ASSERT(obj->rect.y > SCREEN_HEIGHT/4);
				TEST_ASSERT(obj->rect.width >= 50);
				TEST_ASSERT(obj->rect.height == 20);
				TEST_ASSERT(obj->vert_speed >= 0.6 && obj->vert_speed <= 3.6);
			}
			if (obj->type == STAR)
			{	
				TEST_ASSERT(obj->rect.x >= star_start_x + 200);
				TEST_ASSERT(obj->rect.y >= SCREEN_HEIGHT/8);
				TEST_ASSERT(obj->rect.width == 30);
				TEST_ASSERT(obj->rect.height == 30);
				TEST_ASSERT(obj->vert_speed == 0);
			}
		}
	}
}

int compare(Pointer a, Pointer b)
{
	Object ia = a;
	Object ib = b;
	return ia->rect.x - ib->rect.x;
}

void test_state_update() {
	State state = state_create();
	TEST_ASSERT(state != NULL && state_info(state) != NULL);

	// Πληροφορίες για τα πλήκτρα (αρχικά κανένα δεν είναι πατημένο)
	struct key_state keys = { false, false, false, false, false, false, false };
	
	// Χωρίς κανένα πλήκτρο, η μπάλα μετακινείται 4 pixels δεξιά
	Rectangle old_rect = state_info(state)->ball->rect;
	state_update(state, &keys);
	Rectangle new_rect = state_info(state)->ball->rect;

	TEST_ASSERT( new_rect.x == old_rect.x + 4 );

	// Με πατημένο το δεξί βέλος, η μπάλα μετακινείται 6 pixels δεξιά
	keys.right = true;
	old_rect = state_info(state)->ball->rect;
	state_update(state, &keys);
	new_rect = state_info(state)->ball->rect;

	TEST_CHECK( new_rect.x == old_rect.x + 6 );

	// Προσθέστε επιπλέον ελέγχους

	// Με πατημένο το αριστερό βέλος, η μπάλα μετακινείται 1 pixel δεξιά
	keys.right = false;
	keys.left = true;
	old_rect = state_info(state)->ball->rect;
	state_update(state, &keys);
	new_rect = state_info(state)->ball->rect;
	TEST_ASSERT( new_rect.x == old_rect.x + 1 );
	keys.left = false;

	// Όταν vert_mov == IDLE και πατημένο το πάνω βέλος
	state_info(state)->ball->vert_mov = IDLE;
	keys.up = true;
	state_update(state, &keys);
	TEST_ASSERT(state_info(state)->ball->vert_mov == JUMPING);
	TEST_ASSERT(state_info(state)->ball->vert_speed == 17);
	keys.up = false;

	// Συγκρούσεις
	state_info(state)->ball->rect.y = SCREEN_HEIGHT;
	state_update(state, &keys);
	TEST_ASSERT(state_info(state)->playing == false);
	state_info(state)->playing = true;

	List objs = state_objects(state, 0, SCREEN_WIDTH * 8);
	for (ListNode node = list_first(objs);
	node != LIST_EOF;
	node = list_next(objs, node))
	{
		Object obj = list_node_value(objs, node);
		if (obj->type == STAR)
		{
			state_info(state)->ball->rect.x = obj->rect.x;
			state_info(state)->ball->rect.y = obj->rect.y;
			state_update(state, &keys);
			TEST_ASSERT(list_find_node(objs, node, compare) == NULL);
		}
		if (obj->type == PLATFORM)
		{
			state_info(state)->ball->vert_mov = IDLE;
			state_info(state)->ball->rect.x = obj->rect.x - state_info(state)->ball->rect.width;
			state_info(state)->ball->rect.y = obj->rect.y - state_info(state)->ball->rect.height;
			state_update(state, &keys);
			state_update(state, &keys);
			TEST_ASSERT(state_info(state)->ball->rect.y = obj->rect.y - state_info(state)->ball->rect.height);

			obj->vert_mov = FALLING;
			obj->rect.y = SCREEN_HEIGHT;
			state_update(state, &keys);
			TEST_ASSERT(list_find_node(objs, node, compare) == NULL);
		}
	}
}

int compare_ints(Pointer a, Pointer b)
{
    int* ia = a;
    int* ib = b;
    return *ia - *ib;
}

int* create_int1(int value)
{
    int* pointer = malloc(sizeof(int));
    *pointer = value;
    return pointer;
}

void test_set_utils()
{
    Set set = set_create(compare_ints, NULL);
    int values[3] = {5, 10, 15};
    for (int i = 0; i < 3; i++)
        set_insert(set, create_int1(values[i]));
    int* x = set_find_eq_or_greater(set, create_int1(10));
    TEST_ASSERT(*x == 10);
	// Αφαιρώ το 10 από το set και ξαναελέγχω
	set_remove(set, x); 
	x = set_find_eq_or_greater(set, create_int1(10));
	TEST_ASSERT(*x == 15);
	x = set_find_eq_or_smaller(set, create_int1(10));
	TEST_ASSERT(*x == 5);
}

// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {
	{ "test_set_utils", test_set_utils },
	{ "test_state_create", test_state_create },
	{ "test_state_update", test_state_update },

	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};