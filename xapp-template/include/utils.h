#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

// Log levels
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_CRITICAL
} log_level_t;

// Color codes for console output
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BOLD    "\033[1m"

// Utility macros
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(value, min_val, max_val) MAX(min_val, MIN(value, max_val))

// Time utilities
#define MSEC_TO_NSEC(ms) ((ms) * 1000000L)
#define SEC_TO_NSEC(sec) ((sec) * 1000000000L)
#define NSEC_TO_MSEC(ns) ((ns) / 1000000L)
#define NSEC_TO_SEC(ns) ((ns) / 1000000000L)

// Memory utilities
#define SAFE_FREE(ptr) do { if (ptr) { free(ptr); (ptr) = NULL; } } while(0)
#define SAFE_CLOSE(fd) do { if ((fd) >= 0) { close(fd); (fd) = -1; } } while(0)

// String utilities
#define SAFE_STRNCPY(dest, src, size) do { \
    strncpy((dest), (src), (size) - 1); \
    (dest)[(size) - 1] = '\0'; \
} while(0)

// Logging context
typedef struct {
    FILE* log_file;
    log_level_t current_level;
    bool use_colors;
    bool log_to_console;
    bool log_to_file;
    char log_file_path[512];
    pthread_mutex_t log_mutex;
} log_context_t;

// Performance timer
typedef struct {
    struct timespec start_time;
    struct timespec end_time;
    bool running;
} performance_timer_t;

// Circular buffer for metrics
typedef struct {
    void* data;
    size_t element_size;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    pthread_mutex_t mutex;
} circular_buffer_t;

// Function prototypes

// Logging functions
int utils_init_logging(const char* log_file_path, log_level_t level);
void utils_cleanup_logging(void);
void utils_log(log_level_t level, const char* format, ...);
void utils_log_hex(log_level_t level, const char* prefix, const void* data, size_t size);
const char* utils_log_level_to_string(log_level_t level);
const char* utils_log_level_to_color(log_level_t level);

// Logging macros
#define LOG_DEBUG(fmt, ...) utils_log(LOG_LEVEL_DEBUG, "[DEBUG] %s:%d " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  utils_log(LOG_LEVEL_INFO, "[INFO] " fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  utils_log(LOG_LEVEL_WARNING, "[WARN] " fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) utils_log(LOG_LEVEL_ERROR, "[ERROR] %s:%d " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) utils_log(LOG_LEVEL_CRITICAL, "[CRITICAL] %s:%d " fmt, __func__, __LINE__, ##__VA_ARGS__)

// Time utilities
void utils_get_current_time(struct timespec* ts);
uint64_t utils_get_timestamp_ms(void);
uint64_t utils_get_timestamp_us(void);
double utils_timespec_diff(const struct timespec* start, const struct timespec* end);
void utils_sleep_ms(int milliseconds);
void utils_sleep_us(int microseconds);
char* utils_format_timestamp(time_t timestamp, char* buffer, size_t buffer_size);
char* utils_format_duration(double seconds, char* buffer, size_t buffer_size);

// Performance timer functions
void utils_timer_start(performance_timer_t* timer);
double utils_timer_stop(performance_timer_t* timer);
double utils_timer_elapsed(const performance_timer_t* timer);

// String utilities
char* utils_trim_whitespace(char* str);
char* utils_to_lowercase(char* str);
char* utils_to_uppercase(char* str);
bool utils_string_equals_ignore_case(const char* str1, const char* str2);
char* utils_string_replace(const char* source, const char* search, const char* replace);
char** utils_string_split(const char* str, const char* delimiter, int* count);
void utils_free_string_array(char** array, int count);
bool utils_string_starts_with(const char* str, const char* prefix);
bool utils_string_ends_with(const char* str, const char* suffix);

// File utilities
bool utils_file_exists(const char* filepath);
size_t utils_file_size(const char* filepath);
char* utils_read_file(const char* filepath, size_t* size);
int utils_write_file(const char* filepath, const void* data, size_t size);
int utils_copy_file(const char* src, const char* dest);
int utils_create_directory(const char* path);
int utils_remove_directory(const char* path);

// JSON utilities
bool utils_json_get_string(json_object* obj, const char* key, char* value, size_t value_size);
bool utils_json_get_int(json_object* obj, const char* key, int* value);
bool utils_json_get_double(json_object* obj, const char* key, double* value);
bool utils_json_get_bool(json_object* obj, const char* key, bool* value);
json_object* utils_json_load_file(const char* filepath);
int utils_json_save_file(json_object* obj, const char* filepath);

// Memory utilities
void* utils_malloc_zero(size_t size);
void* utils_realloc_safe(void* ptr, size_t new_size);
char* utils_strdup_safe(const char* str);
void utils_memory_info(size_t* total, size_t* available, size_t* used);

// System utilities
int utils_get_cpu_count(void);
double utils_get_cpu_usage(void);
size_t utils_get_memory_usage(void);
int utils_get_process_id(void);
char* utils_get_hostname(char* buffer, size_t buffer_size);
bool utils_is_process_running(int pid);
int utils_execute_command(const char* command, char* output, size_t output_size);

// Network utilities
bool utils_is_valid_ip(const char* ip);
bool utils_is_port_open(const char* host, int port);
char* utils_get_local_ip(char* buffer, size_t buffer_size);

// Circular buffer functions
circular_buffer_t* utils_circular_buffer_create(size_t capacity, size_t element_size);
void utils_circular_buffer_destroy(circular_buffer_t* buffer);
bool utils_circular_buffer_push(circular_buffer_t* buffer, const void* element);
bool utils_circular_buffer_pop(circular_buffer_t* buffer, void* element);
bool utils_circular_buffer_peek(circular_buffer_t* buffer, void* element, size_t index);
size_t utils_circular_buffer_size(const circular_buffer_t* buffer);
bool utils_circular_buffer_is_full(const circular_buffer_t* buffer);
bool utils_circular_buffer_is_empty(const circular_buffer_t* buffer);
void utils_circular_buffer_clear(circular_buffer_t* buffer);

// Hash functions
uint32_t utils_hash_string(const char* str);
uint32_t utils_hash_bytes(const void* data, size_t size);
uint64_t utils_hash_combine(uint64_t hash1, uint64_t hash2);

// Random number utilities
void utils_random_seed(void);
int utils_random_int(int min, int max);
double utils_random_double(double min, double max);
void utils_random_bytes(void* buffer, size_t size);
char* utils_random_string(char* buffer, size_t length);

// Error handling utilities
const char* utils_errno_to_string(int error_code);
void utils_print_stack_trace(void);
void utils_set_error_handler(void (*handler)(const char* message));

// Configuration utilities
bool utils_parse_duration(const char* duration_str, int* seconds);
bool utils_parse_size(const char* size_str, size_t* bytes);
bool utils_parse_ip_port(const char* address, char* ip, size_t ip_size, int* port);

// Validation utilities
bool utils_validate_node_id(uint32_t node_id);
bool utils_validate_subscription_id(uint32_t subscription_id);
bool utils_validate_metric_value(double value, double min_val, double max_val);
bool utils_validate_config_path(const char* path);

// Debug utilities
void utils_print_memory_layout(const void* ptr, size_t size);
void utils_print_hex_dump(const void* data, size_t size);
void utils_print_binary(uint64_t value, int bits);

// Thread utilities
int utils_create_thread(pthread_t* thread, void* (*start_routine)(void*), void* arg);
int utils_join_thread(pthread_t thread, void** retval);
void utils_thread_set_name(const char* name);
char* utils_thread_get_name(char* buffer, size_t buffer_size);

// Signal utilities
void utils_install_signal_handlers(void);
void utils_block_signals(void);
void utils_unblock_signals(void);

// Global logging context
extern log_context_t g_log_ctx;

#endif // UTILS_H