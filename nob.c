#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER  "build/"
#define SOURCE_FOLDER "src/"

int has_h_extension(const char *pathname) {
  const char *ext = strrchr(pathname, '.');
  return ext != NULL && strcmp(ext, ".h") == 0;
}

int main(int argc, char **argv)
{
  NOB_GO_REBUILD_URSELF(argc, argv);
  Nob_Cmd cmd = {0};

  // TARGETS
  static struct {
    const char *obj_path;
    const char *src_path;
  } targets[] = {
    { .obj_path = BUILD_FOLDER"config.o", .src_path = SOURCE_FOLDER"config.c" },
    { .obj_path = BUILD_FOLDER"cJSON.o",  .src_path = SOURCE_FOLDER"cJSON.c" },
    { .obj_path = BUILD_FOLDER"api.o",    .src_path = SOURCE_FOLDER"api.c" },
    { .obj_path = BUILD_FOLDER"awqat.o",  .src_path = SOURCE_FOLDER"awqat.c" },
  };

  nob_mkdir_if_not_exists(BUILD_FOLDER);

  int rebuild_everything = 0;
  // Rebuild every objects if any .h has been modified
  Nob_File_Paths path = {0};
  nob_read_entire_dir(SOURCE_FOLDER, &path);
  for (int i = 0; i < path.count; i++) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), SOURCE_FOLDER "%s", path.items[i]);
    if (nob_needs_rebuild1("awqat", full_path)) {
      rebuild_everything = 1;
      break;
    }
  }

  // COMPILE
  for (size_t i = 0; i < ARRAY_LEN(targets); ++i) {
    if (!rebuild_everything &&
        !nob_needs_rebuild1(targets[i].obj_path, targets[i].src_path)) continue;
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
