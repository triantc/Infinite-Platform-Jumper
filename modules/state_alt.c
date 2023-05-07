
#include <stdlib.h>

#include "set_utils.h"
#include "ADTSet.h"
#include "ADTVector.h"
#include "ADTList.h"
#include "state.h"


// Οι ολοκληρωμένες πληροφορίες της κατάστασης του παιχνιδιού.
// Ο τύπος State είναι pointer σε αυτό το struct, αλλά το ίδιο το struct
// δεν είναι ορατό στον χρήστη.

struct state {
	Set objects;			// περιέχει στοιχεία Object (Πλατφόρμες, Αστέρια)
	struct state_info info;	// Γενικές πληροφορίες για την κατάσταση του παιχνιδιού
	float speed_factor;		// Πολλαπλασιαστής ταχύτητς (1 = κανονική ταχύτητα, 2 = διπλάσια, κλπ)
};

// Δημιουργεί και επιστρέφει ένα αντικείμενο

static Object create_object(ObjectType type, float x, float y, float width, float height, VerticalMovement vert_mov, float speed, bool unstable) {
	Object obj = malloc(sizeof(*obj));
	obj->type = type;
	obj->rect.x = x;
	obj->rect.y = y;
	obj->rect.width = width;
	obj->rect.height = height;
	obj->vert_mov = vert_mov;
	obj->vert_speed = speed;
	obj->unstable = unstable;
	return obj;
}

// Προσθέτει αντικείμενα στην πίστα (η οποία μπορεί να περιέχει ήδη αντικείμενα).
// Τα αντικείμενα ξεκινάνε από x = start_x, και επεκτείνονται προς τα δεξιά.
//
// ΠΡΟΣΟΧΗ: όλα τα αντικείμενα έχουν συντεταγμένες x,y σε ένα
// καρτεσιανό επίπεδο.
// - Στο άξονα x το 0 είναι η αρχή της πίστας και οι συντεταγμένες
//   μεγαλώνουν προς τα δεξιά.
//
// - Στον άξονα y το 0 είναι το πάνω μέρος της πίστας, και οι συντεταγμένες μεγαλώνουν
//   προς τα _κάτω_. Η πίστα έχει σταθερό ύψος, οπότε όλες οι
//   συντεταγμένες y είναι ανάμεσα στο 0 και το SCREEN_HEIGHT.
//
// Πέρα από τις συντεταγμένες, αποθηκεύουμε και τις διαστάσεις width,height
// κάθε αντικειμένου. Τα x,y,width,height ορίζουν ένα παραλληλόγραμμο, οπότε
// μπορούν να αποθηκευτούν όλα μαζί στο obj->rect τύπου Rectangle (ορίζεται
// στο include/raylib.h). Τα x,y αναφέρονται στην πάνω-αριστερά γωνία του Rectangle.

static void add_objects(State state, float start_x) {
	// Προσθέτουμε PLATFORM_NUM πλατφόρμες, με τυχαία χαρακτηριστικά.

	for (int i = 0; i < PLATFORM_NUM; i++) {
		Object platform = create_object(
			PLATFORM,
			start_x + 150 + rand() % 80,						// x με τυχαία απόσταση από το προηγούμενο στο διάστημα [150, 230]
			SCREEN_HEIGHT/4 + rand() % SCREEN_HEIGHT/2,			// y τυχαία στο διάστημα [SCREEN_HEIGHT/4, 3*SCREEN_HEIGHT/4]
			i == 0 ? 250 : 50 + rand()%200,						// πλάτος τυχαία στο διάστημα [50, 250] (η πρώτη πάντα 250)
			20,													// ύψος
			i < 3 || rand() % 2 == 0 ? MOVING_UP : MOVING_DOWN,	// τυχαία αρχική κίνηση (οι πρώτες 3 πάντα πάνω)
			0.6 + 3*(rand()%100)/100,							// ταχύτητα τυχαία στο διάστημα [0.6, 3.6]
			i > 0 && (rand() % 10) == 0							// το 10% (τυχαία) των πλατφορμών είναι ασταθείς (εκτός από την πρώτη)
		);
		set_insert(state->objects, platform);

		// Στο 50% των πλατφορμών (τυχαία), εκτός της πρώτης, προσθέτουμε αστέρι
		if(i != 0 && rand() % 2 == 0) {
			Object star = create_object(
				STAR,
				start_x + 200 + rand() % 60,				// x με τυχαία απόσταση από την προηγούμενη πλατφόρμα στο διάστημα [200,260]
				SCREEN_HEIGHT/8 + rand() % SCREEN_HEIGHT/2,	// y τυχαία στο διάστημα [SCREEN_HEIGHT/8, 5*SCREEN_HEIGHT/8]
				30, 30,										// πλάτος, ύψος
				IDLE,										// χωρίς κίνηση
				0,										 	// ταχύτητα 0
				false										// 'unstable' πάντα false για τα αστέρια
			);
			set_insert(state->objects, star);
		}

		start_x = platform->rect.x + platform->rect.width;	// μετακίνηση των επόμενων αντικειμένων προς τα δεξιά
	}
}

// Δημιουργεί και επιστρέφει την αρχική κατάσταση του παιχνιδιού

int compare_objects(Pointer a, Pointer b)
{
    Object ia = a;
    Object ib = b;
    float x = ia->rect.x;
    float y = ib->rect.x;
    return (int) x - (int) y;          // y - x?
}


State state_create() {
	// Δημιουργία του state
	State state = malloc(sizeof(*state));

	// Γενικές πληροφορίες
	state->info.playing = true;				// Το παιχνίδι ξεκινάει αμέσως
	state->info.paused = false;				// Χωρίς να είναι paused.
	state->speed_factor = 1;				// Κανονική ταχύτητα
	state->info.score = 0;				// Αρχικό σκορ 0

	// Δημιουργούμε το vector των αντικειμένων, και προσθέτουμε αντικείμενα
	// ξεκινώντας από start_x = 0.
	state->objects = set_create(compare_objects, NULL);
	add_objects(state, 0);

	// Δημιουργούμε την μπάλα τοποθετώντας τη πάνω στην πρώτη πλατφόρμα
	Object first_platform = set_node_value(state->objects, set_first(state->objects));
	state->info.ball = create_object(
		BALL,
		first_platform->rect.x,			// x στην αρχή της πλατφόρμας
		first_platform->rect.y - 40,	// y πάνω στην πλατφόρμα
		45, 45,							// πλάτος, ύψος
		IDLE,							// χωρίς αρχική κατακόρυφη κίνηση
		0,								// αρχική ταχύτητα 0
		false							// "unstable" πάντα false για την μπάλα
	);

	return state;
}

// Επιστρέφει τις βασικές πληροφορίες του παιχνιδιού στην κατάσταση state

StateInfo state_info(State state) {
	return &state->info;
}

int* create_int(int value)
{
    int* pointer = malloc(sizeof(int));
    *pointer = value;
    return pointer;
}

float* create_float(float value)
{
    float* pointer = malloc(sizeof(float));
    *pointer = value;
    return pointer;
}

// Επιστρέφει μια λίστα με όλα τα αντικείμενα του παιχνιδιού στην κατάσταση state,
// των οποίων η συντεταγμένη x είναι ανάμεσα στο x_from και x_to.

List state_objects(State state, float x_from, float x_to) {
	List objects = list_create(NULL);
    Pointer ptr_x_from = set_find_eq_or_greater(state->objects, create_float(x_from));
    Pointer ptr_x_to = set_find_eq_or_smaller(state->objects, create_float(x_to));
    if (ptr_x_from != NULL && ptr_x_to != NULL)
    {
        for (SetNode node = set_find_node(state->objects, ptr_x_from);
        node != set_next(state->objects, set_find_node(state->objects, ptr_x_to));
        node = set_next(state->objects, node))
        {
            list_insert_next(objects, list_last(objects), set_node_value(state->objects, node));
        }
    }
	return objects;
}

// Ενημερώνει την κατάσταση state του παιχνιδιού μετά την πάροδο 1 frame.
// Το keys περιέχει τα πλήκτρα τα οποία ήταν πατημένα κατά το frame αυτό.

void state_update(State state, KeyState keys) {

    if (state->info.playing) {
        if (!state->info.paused)
		{
            // Λίστα με τα αντικείμενα που βρίσκονται σε απόσταση το πολύ 2 οθόνων από τη μπάλα
            List objects = state_objects(state, state->info.ball->rect.x - SCREEN_WIDTH * 2, state->info.ball->rect.x + SCREEN_WIDTH * 2);

            // Οριζόντια κίνηση μπάλας
			if (keys->right)
				state->info.ball->rect.x += 6 * state->speed_factor;
			else if (keys->left)
				state->info.ball->rect.x += 1 * state->speed_factor;
			else
				state->info.ball->rect.x += 4 * state->speed_factor;
            
            // Κατακόρυφη κίνηση μπάλας
			if (state->info.ball->vert_mov == JUMPING)
			{
				state->info.ball->rect.y -= state->info.ball->vert_speed;
				state->info.ball->vert_speed *= 0.85;
				if (state->info.ball->vert_speed <= 0.5)
					state->info.ball->vert_mov = FALLING;
			}
			else if (state->info.ball->vert_mov == FALLING)
			{
				state->info.ball->rect.y += state->info.ball->vert_speed;
				if (state->info.ball->vert_speed < 7)
					state->info.ball->vert_speed *= 1.1;
			}
			else if (state->info.ball->vert_mov == IDLE && keys->up)
			{
				state->info.ball->vert_mov = JUMPING;
				state->info.ball->vert_speed = 17;
			}
            
            // Κατακόρυφη κίνηση πλατφόρμας
            for (ListNode node = list_first(objects);
            node != LIST_EOF;
            node = list_next(objects, node))
            {
                Object obj = list_node_value(objects, node);
                if (obj->type == PLATFORM)
				{
					if (obj->vert_mov == MOVING_UP)
					{
						obj->rect.y -= obj->vert_speed * state->speed_factor;
						if (obj->rect.y <= SCREEN_HEIGHT/4)
						{
							obj->vert_mov = MOVING_DOWN;
							obj->rect.y = SCREEN_HEIGHT/4;
						}
					}
					else if (obj->vert_mov == MOVING_DOWN)
					{
						obj->rect.y += obj->vert_speed * state->speed_factor;
						if (obj->rect.y >= 3*SCREEN_HEIGHT/4)
						{
							obj->vert_mov = MOVING_UP;
							obj->rect.y = 3*SCREEN_HEIGHT/4;
						}
					}
					else if (obj->vert_mov == FALLING)
						obj->rect.y += 4 * state->speed_factor;
				}
            }

            // Συμπεριφορά μπάλας σε κατακόρυφη ηρεμία (IDLE)
            if (state->info.ball->vert_mov == IDLE)
                for (ListNode node = list_first(objects);
                node != LIST_EOF;
                node = list_next(objects, node))
                {
                    Object obj = list_node_value(objects, node);
                    if (obj->type == PLATFORM)
					{
						if (state->info.ball->rect.x + state->info.ball->rect.width >= obj->rect.x 
						&& state->info.ball->rect.x <= obj->rect.x + obj->rect.width
						&& (int) state->info.ball->rect.y + (int) state->info.ball->rect.height == (int) obj->rect.y)
						{
							state->info.ball->rect.y = obj->rect.y - state->info.ball->rect.height;
						}
						else
						{
							state->info.ball->vert_mov = FALLING;
							state->info.ball->vert_speed = 1.5;
						}
					}
                }
            
            // Συγκρούσεις
            if (state->info.ball->rect.y >= SCREEN_HEIGHT - state->info.ball->rect.height)
                state->info.playing = false;

            // Βοηθάει για να αφαιρώ γρήγορα όταν χρειάζεται με list_remove_next
            ListNode previous_node = list_first(objects);
            float max_platform_x = 0, max_platform_width = 0;  // Χρησιμέυει στη δημιουργία νέων αντικειμένων αργότερα
            for (ListNode node = list_first(objects);
            node != LIST_EOF;
            node = list_next(objects, node))
            {
                Object obj = list_node_value(objects, node);
                if (obj->type == STAR)
                {
                    if (CheckCollisionRecs(state->info.ball->rect, obj->rect))
                    {
                        list_remove_next(objects, previous_node);
                        state->info.score += 10;
                    }
                }

                if (obj->type == PLATFORM)
                {
                    if (obj->vert_mov == FALLING && obj->rect.y + obj->rect.height >= SCREEN_HEIGHT)
                    {
                        list_remove_next(objects, previous_node);
                    }
                    if (state->info.ball->vert_mov == FALLING
                    && (int) state->info.ball->rect.y + (int) state->info.ball->rect.height >= (int) obj->rect.y 
                    && CheckCollisionRecs(state->info.ball->rect, obj->rect))
                    {
                       if (obj->unstable)
							obj->vert_mov = FALLING;
						state->info.ball->vert_mov = IDLE;
						state->info.ball->rect.y = obj->rect.y - state->info.ball->rect.height;
                    }

                    // Χρησιμέυει στη δημιουργία νέων αντικειμένων αργότερα
                    if (obj->rect.x > max_platform_x)
                    {
                        max_platform_x = obj->rect.x;
                        max_platform_width = obj->rect.width;
                    }
                }
                // Βοηθάει για να αφαιρώ γρήγορα όταν χρειάζεται με list_remove_next
                previous_node = node;
            }
            
            // Δημιουργία νέων αντικειμένων
			if (max_platform_x - state->info.ball->rect.x <= SCREEN_WIDTH)
			{
				state->speed_factor *= 1.1; 
				add_objects(state, max_platform_x + max_platform_width);
			}
		
			// Εκκίνηση και διακοπή
			if (state->info.playing == false && keys->enter)
				state->info.playing = true;
			if (keys->p)
				state->info.paused = true;
        }
        else
        {
            if (keys->n)
			{
				state->info.paused = false;
				state_update(state, keys);
				state->info.paused = true;
			}
			if (keys->p)
				state->info.paused = false;
        }
    }
    else if (keys->enter) {
		state_destroy(state);
		srand(0);
		state_create();			// επαναφορά στην αρχική κατάσταση
	}
}

// Καταστρέφει την κατάσταση state ελευθερώνοντας τη δεσμευμένη μνήμη.

void state_destroy(State state) {
	// Προς υλοποίηση
    free(state->objects);
	free(state);
}