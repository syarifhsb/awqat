#include "api.h"
#include "raylib.h"
#include "awq_api_calls.h"
#include "awq_process.h"

#include "config.h"

#define MAP_HEIGHT 1925
#define MAP_WIDTH  3840
#define WIN_HEIGHT MAP_HEIGHT/4
#define WIN_WIDTH  MAP_WIDTH/4
#define LAT_OFFSET   0.0
#define LON_OFFSET -10.0

#define PRAYER_LABEL_GAP       30
#define PRAYER_LABEL_X_0       90
#define PRAYER_LABEL_Y_0       410
#define PRAYER_LABEL_WIDTH     130
#define PRAYER_LABEL_HEIGHT    30
#define PRAYER_LABEL_PADDING   8
#define PRAYER_LABEL_FONT_SIZE 14

void draw_location(const char *loc_name, float lat, float lon) {
  Vector2 loc = {
              ((lon + 180.0f + LON_OFFSET) / 360.0f * WIN_WIDTH),
              ((90.0f - lat + LAT_OFFSET) / 180.0f * WIN_HEIGHT)};

  Vector2 loc_point_offset = {7.0, -11.0};
  float font_size = 14.0;

  DrawCircleV(loc, 3.5f, BLACK);
  DrawCircleV(loc, 2.5f, GOLD);
  DrawText(loc_name,
      loc.x + loc_point_offset.x,
      loc.y + loc_point_offset.y,
      font_size, BLACK);

  return;
}

void draw_label(const char *text, Vector2 pos, Vector2 box_size, float padding, float font_size, Color text_color) {
  Font default_font = GetFontDefault();

  Rectangle box = {
      pos.x,
      pos.y,
      box_size.x,
      box_size.y
  };

  Vector2 text_pos = { pos.x + padding, pos.y + padding };

  DrawRectangleRec(box, (Color){ 0, 0, 0, 120 });
  DrawRectangleLinesEx(box, 1.5f, (Color){ 0, 0, 0, 255 });
  DrawTextEx(default_font, text, (Vector2)text_pos, font_size, 2, text_color);

  return;
}

int main() {
  InitWindow(WIN_WIDTH, WIN_HEIGHT, "Awqat GUI");
  SetExitKey(KEY_Q);
  SetTargetFPS(30);

  Image world = LoadImage("assets/world-map.png");

  ImageResize(&world, WIN_WIDTH, WIN_HEIGHT);

  Texture2D world_texture = LoadTextureFromImage(world);
  UnloadImage(world);

  Prayer prayers[5] = {{"Fajr",    {0,0}, {0,0}},
                       {"Dhuhr",   {0,0}, {0,0}},
                       {"Asr",     {0,0}, {0,0}},
                       {"Maghrib", {0,0}, {0,0}},
                       {"Isha",    {0,0}, {0,0}}};

  // Display map before fetching
  BeginDrawing();
  ClearBackground(YELLOW);
  DrawTexture(world_texture, WIN_WIDTH/2 - world_texture.width/2, WIN_HEIGHT/2 - world_texture.height/2, WHITE);

  for (size_t i = 0; i < NOB_ARRAY_LEN(prayers); i++)
  {
    char buf[32] = {0};
    snprintf(buf, sizeof(buf), "%s: %02d:%02d", prayers[i].name, prayers[i].time.time_h, prayers[i].time.time_m);
    draw_label(buf,
        (Vector2){ PRAYER_LABEL_X_0 + (PRAYER_LABEL_WIDTH + PRAYER_LABEL_GAP) * i, PRAYER_LABEL_Y_0 },
        (Vector2){ PRAYER_LABEL_WIDTH, PRAYER_LABEL_HEIGHT },
        PRAYER_LABEL_PADDING,
        PRAYER_LABEL_FONT_SIZE,
        WHITE);
  }

  EndDrawing();
  // Display map before fetching

  // Fetching
  float user_lat, user_lon;
  awq_get_user_coord(&user_lat, &user_lon, IP_API_URL);

  Nob_String_Builder loc_name = {0};
  awq_get_location_name(user_lat, user_lon, NOMINATIM_URL, &loc_name);

  Main main_st = {0};
  Params awq_params = {0};
  memcpy(main_st.prayers, prayers, sizeof(prayers));
  main_st.time_now = awq_get_time_now();

  char buf[32] = {0};
  snprintf(buf, sizeof(buf), "%f", user_lat);
  awq_add_param(&awq_params, "latitude", buf);

  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%f", user_lon);
  awq_add_param(&awq_params, "longitude", buf);

  cJSON *aladhan_data = awq_get_prayer_times(&awq_params, ALADHAN_TIMINGS_URL);
  awq_delete_params(&awq_params);
  awq_process(&main_st, aladhan_data);
  cJSON_Delete(aladhan_data);
  // End Fetching



  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(YELLOW);
    DrawTexture(world_texture, WIN_WIDTH/2 - world_texture.width/2, WIN_HEIGHT/2 - world_texture.height/2, WHITE);

  for (size_t i = 0; i < NOB_ARRAY_LEN(main_st.prayers); i++)
  {
    char buf[32] = {0};
    snprintf(buf, sizeof(buf), "%s: %02d:%02d", main_st.prayers[i].name, main_st.prayers[i].time.time_h, main_st.prayers[i].time.time_m);

    Color text_color;
    if (&main_st.prayers[i] == main_st.next_prayer)
      text_color = GOLD;
    else
      text_color = WHITE;

    draw_label(buf,
        (Vector2){ PRAYER_LABEL_X_0 + (PRAYER_LABEL_WIDTH + PRAYER_LABEL_GAP) * i, PRAYER_LABEL_Y_0 },
        (Vector2){ PRAYER_LABEL_WIDTH, PRAYER_LABEL_HEIGHT },
        PRAYER_LABEL_PADDING,
        PRAYER_LABEL_FONT_SIZE,
        text_color);
  }

    draw_location(loc_name.items, user_lat, user_lon);

    EndDrawing();
  }

  UnloadTexture(world_texture);

  CloseWindow();

  awq_cleanup(&main_st);
  nob_sb_free(loc_name);

  return 0;
}
