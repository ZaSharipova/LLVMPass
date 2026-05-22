#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static FILE *global_log_file = NULL;

__attribute__((constructor))
static void mypass_log_init__(void) {
    const char *log_path = getenv("MYPASS_LOG_FILE");
    if (!log_path) {
        log_path = "runtime_log.txt";
    }

    global_log_file = fopen(log_path, "w");
    if (!global_log_file) {
        perror("Error: failed to open log file.");
    }
}

__attribute__((destructor))
static void mypass_log_finish__(void) {
    if (global_log_file) {
        fclose(global_log_file);
        global_log_file = NULL;
    }
}

void mypass_log_i32__(int id, int32_t value) {
    if (global_log_file) {
        fprintf(global_log_file, "%d %d\n", id, (int)value);
    }
}

void mypass_log_i64__(int id, int64_t value) {
    if (global_log_file) {
        fprintf(global_log_file, "%d 0x%llx\n", id, (unsigned long long)value);
    }
}

void mypass_log_edge__(uint32_t from_id, uint32_t to_id) {
    if (global_log_file) {
        fprintf(global_log_file, "edge %u %u\n", from_id, to_id);
    }
}
