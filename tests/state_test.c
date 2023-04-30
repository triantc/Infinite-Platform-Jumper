//////////////////////////////////////////////////////////////////
//
// Test για το state.h module
//
//////////////////////////////////////////////////////////////////

#include "acutest.h"			// Απλή βιβλιοθήκη για unit testing

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
}


// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {
	{ "test_state_create", test_state_create },
	{ "test_state_update", test_state_update },

	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
};