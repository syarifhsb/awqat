#include "api.h"
#include "raylib.h"
#include "awq_api_calls.h"
#include "awq_process.h"

#include "config.h"

#define MAP_SOURCE_HEIGHT 1925
#define MAP_SOURCE_WIDTH  3840
#define MAP_HEIGHT        (MAP_SOURCE_HEIGHT/4.0f)
#define MAP_WIDTH         (MAP_SOURCE_WIDTH/4.0f)
#define WIN_HEIGHT        (MAP_HEIGHT + 130)
#define WIN_WIDTH         (MAP_WIDTH)
#define BG_COLOR          CLITERAL(Color){ 198, 236, 255, 255 }

#define LAT_OFFSET   0.0
#define LON_OFFSET -10.0

#define PRAYER_LABEL_GAP       30
#define PRAYER_LABEL_X_0       90
#define PRAYER_LABEL_Y_0       500
#define PRAYER_LABEL_WIDTH     130
#define PRAYER_LABEL_HEIGHT    30
#define PRAYER_LABEL_PADDING   8
#define PRAYER_LABEL_FONT_SIZE 14

#define MAX_INPUT_CHARS 32

typedef struct {
  int show;
  int frame_duration;
  Nob_String_Builder message;
} Notification;
Notification notif = {0};

void draw_location(const char *loc_name, float lat, float lon) {
  Vector2 loc = {
              ((lon + 180.0f + LON_OFFSET) / 360.0f * MAP_WIDTH),
              ((90.0f - lat + LAT_OFFSET) / 180.0f * MAP_HEIGHT)};

  Vector2 loc_text_offset = {7.0, -11.0};
  float font_size = 14.0;

  DrawCircleV(loc, 3.5f, BLACK);
  DrawCircleV(loc, 2.5f, GOLD);
  DrawText(loc_name,
      loc.x + loc_text_offset.x,
      loc.y + loc_text_offset.y,
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

void draw_search_bar(Vector2 pos, Vector2 box_size, float padding, float font_size, Color text_color, char *city, int search_bar_focus, size_t frames_counter, int letter_count) {
  Font default_font = GetFontDefault();

  Rectangle box = {
      pos.x,
      pos.y,
      box_size.x,
      box_size.y
  };

  Vector2 text_pos = { pos.x + padding, pos.y + padding };

  Color outline;
  if (search_bar_focus) outline = RED;
  else outline = BLACK;

  DrawRectangleRec(box, (Color){ 0, 0, 0, 120 });
  DrawRectangleLinesEx(box, 1.5f, outline);

  char buf[256];
  snprintf(buf, sizeof(buf), "Search by city: %s", city);

  if (search_bar_focus &&
      letter_count < MAX_INPUT_CHARS &&
      ((frames_counter/20)%2) == 0) // Blink period every 40 frames
    snprintf(buf, sizeof(buf), "Search by city: %s|", city);
  else
    snprintf(buf, sizeof(buf), "Search by city: %s", city);


  DrawTextEx(default_font, buf, (Vector2)text_pos, font_size, 2, text_color);

  return;
}

void destroy_notification(Notification *notif) {
  notif->show = 0;
  notif->frame_duration = 0;
  nob_sb_free(notif->message);
  notif->message = (Nob_String_Builder){0};

  return;
}

void create_notification(Notification *notif, char *message, int frame_duration) {
  if (notif->show) {
    destroy_notification(notif);
  }

  nob_sb_append_cstr(&(notif->message), message);
  nob_sb_append_null(&(notif->message));
  notif->frame_duration = frame_duration;
  notif->show = 1;

  return;
}

void draw_notification(Notification *notif) {
  if (!(notif->show)) return;

  if (notif->frame_duration <= 0) {
    destroy_notification(notif);
    return;
  }

  int padding = 8;
  Font default_font = GetFontDefault();

  Rectangle box = {
    10,
    10,
    500,
    32,
  };

  Vector2 text_pos = { box.x + padding, box.y + padding };

  DrawRectangleRec(box, (Color){ 0, 0, 0, 120 });
  DrawRectangleLinesEx(box, 1.5f, BLACK);

  char buf[256];
  snprintf(buf, sizeof(buf), "%s", notif->message.items);

  DrawTextEx(default_font, buf, (Vector2)text_pos, 16, 2, WHITE);

  (notif->frame_duration)--;

  return;
}

void update_search_text(Rectangle search_box, int search_bar_focus, int *letter_count, char *city) {
  if (search_bar_focus) {
    int key = GetCharPressed();
    while (key > 0)
    {
      // NOTE: Only allow keys in range [32..125]
      if ((key >= 32) && (key <= 125) && (*letter_count < MAX_INPUT_CHARS))
      {
        city[*letter_count] = (char)key;
        city[*letter_count+1] = '\0'; // Add null terminator at the end of the string
        (*letter_count)++;
      }

      key = GetCharPressed();  // Check next character in the queue
    }

    if (IsKeyPressed(KEY_BACKSPACE))
    {
      (*letter_count)--;
      if (*letter_count < 0) *letter_count = 0;
      city[*letter_count] = '\0';
    }

  }

  return;
}

void launch_search(Main *main_st, float *lat, float *lon, Rectangle search_box, int *search_bar_focus, char *city, char *nominatim_url) {
  if (*search_bar_focus) {
    int key = GetCharPressed();

    if (!IsKeyPressed(KEY_ENTER)) return;
  } else return;

  *search_bar_focus = 0;

  int status = awq_get_coord_by_city(lat, lon, city, nominatim_url, &(main_st->city));

  if (status == AWQ_FETCH_ERR_CITY_NOT_FOUND) {
    char buf[256];
    snprintf(buf, sizeof(buf), "City not found: %s",  city);
    create_notification(&notif, buf, 120);
  }

  Params awq_params = {0};

  char buf[32] = {0};
  snprintf(buf, sizeof(buf), "%f", *lat);
  awq_add_param(&awq_params, "latitude", buf);

  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%f", *lon);
  awq_add_param(&awq_params, "longitude", buf);

  cJSON *aladhan_data = awq_get_prayer_times(&awq_params, ALADHAN_TIMINGS_URL);
  awq_delete_params(&awq_params);
  awq_process(main_st, aladhan_data);
  cJSON_Delete(aladhan_data);

  return;
}

void set_search_focus(Rectangle search_box, int *search_bar_focus, int *mouse_on_text) {
  if (CheckCollisionPointRec(GetMousePosition(), search_box)) *mouse_on_text = 1;
  else *mouse_on_text = 0;

  if (*mouse_on_text && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    *search_bar_focus = 1;
  }

  if (!*mouse_on_text && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    *search_bar_focus = 0;
  }

  if (*search_bar_focus && IsKeyPressed(KEY_ESCAPE)) {
    *search_bar_focus = 0;
  }

  return;
}

void set_mouse_cursor(Rectangle search_box, int mouse_on_text) {
  if (CheckCollisionPointRec(GetMousePosition(), search_box))
    SetMouseCursor(MOUSE_CURSOR_IBEAM);
  else
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);

  return;
}

int main() {
  InitWindow(WIN_WIDTH, WIN_HEIGHT, "Awqat GUI");
  SetExitKey(KEY_NULL);
  SetTargetFPS(30);

  int mouse_on_text = 0;
  int search_bar_focus = 0;
  size_t frames_counter = 0;

  Image world = LoadImage("assets/world-map.png");

  ImageResize(&world, MAP_WIDTH, MAP_HEIGHT);

  Texture2D world_texture = LoadTextureFromImage(world);
  UnloadImage(world);

  Prayer prayers[5] = {{"Fajr",    {0,0}, {0,0}},
    {"Dhuhr",   {0,0}, {0,0}},
    {"Asr",     {0,0}, {0,0}},
    {"Maghrib", {0,0}, {0,0}},
    {"Isha",    {0,0}, {0,0}}};

  // Display map before fetching
  BeginDrawing();
  ClearBackground(BG_COLOR);
  DrawTexture(world_texture, MAP_WIDTH/2 - world_texture.width/2.0f, MAP_HEIGHT/2 - world_texture.height/2.0f, WHITE);

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

  Rectangle search_box = {
      PRAYER_LABEL_X_0,
      PRAYER_LABEL_Y_0 + 50,
      PRAYER_LABEL_WIDTH * 5 + PRAYER_LABEL_GAP * 4,
      PRAYER_LABEL_HEIGHT
  };
  int letter_count = 0;
  char city[MAX_INPUT_CHARS + 1] = "\0";

  draw_search_bar(
      (Vector2){ PRAYER_LABEL_X_0, PRAYER_LABEL_Y_0 + 50 },
      (Vector2){ PRAYER_LABEL_WIDTH * 5 + PRAYER_LABEL_GAP * 4, PRAYER_LABEL_HEIGHT },
      PRAYER_LABEL_PADDING,
      PRAYER_LABEL_FONT_SIZE,
      WHITE, city,
      search_bar_focus, frames_counter, letter_count);

  EndDrawing();
  // Display map before fetching

  // Fetching
  float lat, lon;
  awq_get_user_coord(&lat, &lon, IP_API_URL);

  Main main_st = {0};
  Params awq_params = {0};
  awq_get_location_name(lat, lon, NOMINATIM_URL, &(main_st.city));

  memcpy(main_st.prayers, prayers, sizeof(prayers));
  main_st.time_now = awq_get_time_now();

  char buf[32] = {0};
  snprintf(buf, sizeof(buf), "%f", lat);
  awq_add_param(&awq_params, "latitude", buf);

  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%f", lon);
  awq_add_param(&awq_params, "longitude", buf);

  cJSON *aladhan_data = awq_get_prayer_times(&awq_params, ALADHAN_TIMINGS_URL);
  awq_delete_params(&awq_params);
  awq_process(&main_st, aladhan_data);
  cJSON_Delete(aladhan_data);
  // End Fetching

  // Main Game Loop
  while (!WindowShouldClose()) {
    // Update Time
    main_st.time_now = awq_get_time_now();

    set_search_focus(search_box, &search_bar_focus, &mouse_on_text);
    set_mouse_cursor(search_box, mouse_on_text);
    update_search_text(search_box, search_bar_focus, &letter_count, city);
    launch_search(&main_st, &lat, &lon, search_box, &search_bar_focus, city, NOMINATIM_URL);
    if (search_bar_focus) frames_counter++;
    else frames_counter = 0;

    BeginDrawing();

    ClearBackground(BG_COLOR);
    DrawTexture(world_texture, MAP_WIDTH/2 - world_texture.width/2.0f, MAP_HEIGHT/2 - world_texture.height/2.0f, WHITE);

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

    draw_search_bar(
        (Vector2){ PRAYER_LABEL_X_0, PRAYER_LABEL_Y_0 + 50 },
        (Vector2){ PRAYER_LABEL_WIDTH * 5 + PRAYER_LABEL_GAP * 4, PRAYER_LABEL_HEIGHT },
        PRAYER_LABEL_PADDING,
        PRAYER_LABEL_FONT_SIZE,
        WHITE, city,
        search_bar_focus, frames_counter, letter_count);

    draw_notification(&notif);
    draw_location(main_st.city.items, lat, lon);

    EndDrawing();
  }
  // End - Main Game Loop

  UnloadTexture(world_texture);

  CloseWindow();

  awq_cleanup(&main_st);

  return 0;
}
