#include "raylib.h"

#include "state.h"
#include "interface.h"

// Assets
Texture bird_img;
Sound game_over_snd;


void interface_init() {
	// Αρχικοποίηση του παραθύρου
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "game");
	SetTargetFPS(60);
    InitAudioDevice();

	// Φόρτωση εικόνων και ήχων
	bird_img = LoadTextureFromImage(LoadImage("assets/bird.png"));
	game_over_snd = LoadSound("assets/game_over.mp3");
}

void interface_close() {
	CloseAudioDevice();
	CloseWindow();
}

// Draw game (one frame)
void interface_draw_frame(State state) {
	BeginDrawing();

	// Καθαρισμός, θα τα σχεδιάσουμε όλα από την αρχή
	ClearBackground(RAYWHITE);

	// Σχεδιάζουμε τον χαρακτήρα και τις 2 μπάλες
    StateInfo info = state_info(state);
    
	float ball_x = info->ball->rect.x;
	float offset_x = SCREEN_WIDTH - 700 - ball_x;

	DrawTexture(bird_img, SCREEN_WIDTH/3, info->ball->rect.y, WHITE);

	List objs = state_objects(state, ball_x - SCREEN_WIDTH, ball_x + SCREEN_WIDTH);

	for (ListNode node = list_first(objs);
	node != LIST_EOF;
	node = list_next(objs, node))
	{
		Object obj = list_node_value(objs, node);
		if (obj->type == PLATFORM)
		{
			if (obj->unstable)
			{
				/* code */
			}
			DrawRectangle(obj->rect.x + offset_x, obj->rect.y, obj->rect.width, obj->rect.height, GREEN);
			
		}
		if (obj->type == STAR)
		{
			DrawCircle(obj->rect.x + offset_x, obj->rect.y, 20, YELLOW);
		}
		
	}
	
	// Σχεδιάζουμε το σκορ και το FPS counter
	DrawText(TextFormat("%04i", info->score), 20, 20, 40, GRAY);
	DrawFPS(SCREEN_WIDTH - 80, 0);

	// Αν το παιχνίδι έχει τελειώσει, σχεδιάζομαι το μήνυμα για να ξαναρχίσει
	if (!info->playing) {
		DrawText(
			"PRESS [ENTER] TO PLAY AGAIN",
			 GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2,
			 GetScreenHeight() / 2 - 50, 20, GRAY
		);
	}

	EndDrawing();
}