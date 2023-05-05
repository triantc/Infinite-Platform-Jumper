#include "raylib.h"

#include "state.h"
#include "interface.h"

// Assets
Texture ghost_png;
Texture bitcoin_png;
Sound game_over_snd;


void interface_init() {
	// Αρχικοποίηση του παραθύρου
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "game");
	SetTargetFPS(60);
    InitAudioDevice();

	// Φόρτωση εικόνων και ήχων
	bitcoin_png = LoadTextureFromImage(LoadImage("assets/bitcoin.png"));
	ghost_png = LoadTextureFromImage(LoadImage("assets/ghost.png"));
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
	ClearBackground(BLACK);

	// Σχεδιάζουμε τον χαρακτήρα και τις 2 μπάλες
    StateInfo info = state_info(state);
    
	float ball_x = info->ball->rect.x;
	float offset_x = SCREEN_WIDTH/3 - ball_x;

	DrawTexture(ghost_png, offset_x + ball_x, info->ball->rect.y, WHITE);

	List objs = state_objects(state, ball_x - SCREEN_WIDTH, ball_x + SCREEN_WIDTH);

	for (ListNode node = list_first(objs);
	node != LIST_EOF;
	node = list_next(objs, node))
	{
		Object obj = list_node_value(objs, node);
		if (obj->type == PLATFORM)
		{
			if (obj->unstable)
				DrawRectangle(obj->rect.x + offset_x, obj->rect.y, obj->rect.width, obj->rect.height, MAGENTA);
			else
				DrawRectangle(obj->rect.x + offset_x, obj->rect.y, obj->rect.width, obj->rect.height, GOLD);
		}
		if (obj->type == STAR)
		{
			DrawTexture(bitcoin_png, obj->rect.x + offset_x, obj->rect.y, WHITE);
		}
	}
	
	// Σχεδιάζουμε το σκορ και το FPS counter
	DrawText(TextFormat("%04i", info->score), 20, 20, 40, WHITE);
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