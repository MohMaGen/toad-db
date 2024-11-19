#include <stdio.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#include "./nob.h/nob.h"


bool strendswith(const char* str, const char* suf) {
    size_t str_len = strlen(str), suf_len = strlen(suf); 
    if (suf_len > str_len) return false;

    return strcmp(str + str_len - suf_len, suf) == 0; 
}

bool strstartswith(const char *str, const char* pref) {
    size_t str_len = strlen(str), pref_len = strlen(pref);
    if (str_len < pref_len) return false;

    for (size_t i = 0; i < pref_len; i++)
        if (str[i] != pref[i]) return false;

    return true;
}

#define SOURCE_DIR "src"
#define BUILD_DIR "build"
#define EXPERIMENT_DIR "experiments"
#define CCPP "clang++"

#define COMPILER_FLAGS_STRING   "-Wall --std=c++23 -Werror -pedantic -g"
#define COMPILER_FLAGS          "-Wall", "--std=c++23", "-Werror", "-pedantic", "-g"
#define INCLUDE_DIRS_STRING     "-I" SOURCE_DIR
#define INCLUDE_DIRS            "-I", SOURCE_DIR
//#define LD_FLAGS_STRING         "-lncurses"
//#define LD_FLAGS                "-lncurses"

#define LD_FLAGS_STRING         ""
#define LD_FLAGS                ""

Nob_File_Paths headers = { 0 };
Nob_File_Paths sources = { 0 };
Nob_File_Paths objects = { 0 };
Nob_File_Paths experiments_srcs = { 0 };
Nob_File_Paths experiments_bins = { 0 };

bool generate_compile_commands(void) {
    FILE *compile_commands = fopen("compile_commands.json", "w");
    if (!compile_commands) return false;

    nob_log(NOB_INFO, "update `compile_commands.json`.");

    fprintf(compile_commands, "[\n");

    for (size_t i = 0; i < sources.count; i++) {
        fprintf(compile_commands, "    {\n");

        const char *object = objects.items[i];
        const char *source = sources.items[i];

        fprintf(compile_commands, "       \"file\":\"%s\",\n", source); 
        fprintf(compile_commands, "       \"directory\":\"%s\",\n", nob_get_current_dir_temp()); 
        fprintf(compile_commands, "       \"command\":\"%s %s %s %s -o %s %s\"\n",
                    CCPP, COMPILER_FLAGS_STRING, INCLUDE_DIRS_STRING, LD_FLAGS_STRING,
                    object, source);

        fprintf(compile_commands, "    },\n");
    }
    for (size_t i = 0; i < experiments_srcs.count; i++) {
        fprintf(compile_commands, "    {\n");

        const char *source = experiments_srcs.items[i];
        const char *object = experiments_bins.items[i];

        fprintf(compile_commands, "       \"file\":\"%s\",\n", source); 
        fprintf(compile_commands, "       \"directory\":\"%s\",\n", nob_get_current_dir_temp()); 
        fprintf(compile_commands, "       \"command\":\"%s %s %s %s -o %s %s\"\n",
                    CCPP, COMPILER_FLAGS_STRING, INCLUDE_DIRS_STRING, LD_FLAGS_STRING,
                    object, source);

        fprintf(compile_commands, "    }%s", i == sources.count-1 ? "\n" : ",\n");
    }

    fprintf(compile_commands, "]\n");

    fclose(compile_commands);

    return true;
}


void register_files(void) {
    Nob_File_Paths files = { 0 };

    nob_read_entire_dir(SOURCE_DIR, &files); 
    for (size_t i = 0; i < files.count; i++) {
        const char *curr = files.items[i];

        if (strendswith(curr, ".hpp")) {
            char *header = nob_temp_sprintf("%s/%s", SOURCE_DIR, curr);

            nob_log(NOB_INFO, "register header: `%s`.", header); 
            nob_da_append(&headers, header); 
        } 

        if (strendswith(curr, ".cpp")) {
            char *source = nob_temp_sprintf("%s/%s", SOURCE_DIR, curr);
            char *object = nob_temp_sprintf("%s/__obj_%s.o", BUILD_DIR, curr);

            nob_log(NOB_INFO, "register source: `%s` => `%s`.", source, object);
            nob_da_append(&sources, source);
            nob_da_append(&objects, object);
        }
    }
}

void register_experiments(void) {
    Nob_File_Paths files = { 0 };

    nob_read_entire_dir(EXPERIMENT_DIR, &files); 
    for (size_t i = 0; i < files.count; i++) {
        const char *curr = files.items[i];
        if (strendswith(curr, ".cpp")) {
            char *source = nob_temp_sprintf("%s/%s", EXPERIMENT_DIR, curr);
            char *binary = nob_temp_sprintf("%s/%s_exp", BUILD_DIR, curr);

            nob_log(NOB_INFO, "register experiment: `%s` => `%s`.", source, binary); 
            nob_da_append(&experiments_srcs, source);
            nob_da_append(&experiments_bins, binary);
        }
    }
}

bool build_experiments(Nob_Cmd *cmd) {
    Nob_Procs procs = { 0 };

    for (size_t i = 0; i < experiments_srcs.count; i++) {
        Nob_File_Paths deps = { 0 };
        nob_da_append(&deps, experiments_srcs.items[i]);
        nob_da_append_many(&deps, objects.items, objects.count);
        nob_da_append_many(&deps, headers.items, objects.count);

        int ret = nob_needs_rebuild(experiments_bins.items[i], deps.items, deps.count); 
        if (ret < 0) return false;
        if (!ret) continue;

        nob_cmd_append(cmd, CCPP, COMPILER_FLAGS, INCLUDE_DIRS, LD_FLAGS, "-o", experiments_bins.items[i], experiments_srcs.items[i]); 
        nob_da_append_many(cmd, objects.items, objects.count);

        Nob_Proc proc = nob_cmd_run_async_and_reset(cmd); 
        nob_da_append(&procs, proc);
    }

    if (!nob_procs_wait(procs)) return false; 

    return true;
}


int main(int argc, char **argv) {
    int result = 0;
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_DIR)) return 1;

    register_files();
    register_experiments();
    generate_compile_commands();

    Nob_Cmd cmd = { 0 };
    Nob_Procs procs = { 0 };

    for (size_t i = 0; i < objects.count; i++) {
        const char *object = objects.items[i];
        const char *source = sources.items[i];

        Nob_File_Paths deps = { 0 };
        nob_da_append(&deps, nob_temp_strdup(source));
        nob_da_append_many(&deps, headers.items, headers.count);

        int ret = nob_needs_rebuild(object, deps.items, deps.count); 
        if (ret < 0) return 1;
        if (!ret) continue;

        nob_cmd_append(&cmd, CCPP, COMPILER_FLAGS, INCLUDE_DIRS, LD_FLAGS, 
                        "-o", object, "-c", source);
        Nob_Proc proc = nob_cmd_run_async_and_reset(&cmd); 
        nob_da_append(&procs, proc);
        nob_da_free(deps);
    }

    if (!nob_procs_wait(procs)) nob_return_defer(1);

    if (!build_experiments(&cmd)) nob_return_defer(1); 

    if (argc > 1) {
        if (strcmp(argv[1], "run") == 0) {
            if (argc < 3) {
                nob_log(NOB_ERROR, "Expected target name to run!");
                nob_return_defer(1); 
            }

            for (size_t i = 0; i < experiments_bins.count; i++) {
                if (strstartswith(experiments_bins.items[i],
                                  nob_temp_sprintf("%s/%s", BUILD_DIR, argv[2]))) {

                    nob_log(NOB_INFO, "Run: `%s`.", experiments_bins.items[i]);
                    nob_cmd_append(&cmd, experiments_bins.items[i]);
                    if (!nob_cmd_run_sync_and_reset(&cmd)) nob_return_defer(1);
                }
            }
        }
        if (strcmp(argv[1], "list") == 0) {
            nob_log(NOB_INFO, "");
            nob_log(NOB_INFO, "");
            nob_log(NOB_INFO, "");
            nob_log(NOB_INFO, "  ---===< LIST TARGETS >===---");
            nob_log(NOB_INFO, "");
            for (size_t i = 0; i < experiments_bins.count; i++) {
                nob_log(NOB_INFO, "   < target > `%s`", experiments_bins.items[i] + strlen(BUILD_DIR) + 1);
            }
        }
    }

defer:
    nob_da_free(sources);
    nob_da_free(objects);
    nob_da_free(headers);
    nob_da_free(experiments_bins);
    nob_da_free(experiments_srcs);
    return result;
}
