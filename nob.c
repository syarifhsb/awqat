#define NOB_IMPLEMENTATION
#include "nob.h"

int main(int argc, char **argv)
{
  NOB_GO_REBUILD_URSELF(argc, argv);
  Nob_Cmd cmd = {0};

  // OBJECTS
  static struct {
    const char *obj_path;
    const char *src_path;
  } targets[] = {
    { .obj_path = "cJSON.o", .src_path = "cJSON.c" },
    { .obj_path = "awqat.o", .src_path = "awqat.c" },
  };

  // COMPILE
  for (size_t i = 0; i < ARRAY_LEN(targets); ++i) {
    if (!nob_needs_rebuild1(targets[i].obj_path, targets[i].src_path)) continue;
    nob_cc(&cmd);
    nob_cc_flags(&cmd);
    nob_cmd_append(&cmd, "-c");
    nob_cc_output(&cmd, targets[i].obj_path);
    nob_cc_inputs(&cmd, targets[i].src_path);
    if (!cmd_run(&cmd)) return 1;
  }

  // LINKING
  int ntarget = sizeof(targets) / sizeof(targets[0]);
  const char *obj_paths[ntarget];
  for (int i = 0; i < ntarget; i++) {
    obj_paths[i] = targets[i].obj_path;
  }

  if (!nob_needs_rebuild("awqat", obj_paths, ntarget)) return 0;
  nob_cc(&cmd);
  nob_cc_flags(&cmd);
  for (size_t i = 0; i < ARRAY_LEN(targets); ++i) {
    nob_cmd_append(&cmd, targets[i].obj_path);
  }
  nob_cmd_append(&cmd, "-lcurl");
  nob_cmd_append(&cmd, "-o", "awqat");
  if (!cmd_run(&cmd)) return 1;

  return 0;
}
