/*
 * Utility Module for Smart Monitor xApp
 * 
 * This module provides utility functions including:
 * - Logging system
 * - Time utilities
 * - String utilities
 * - File utilities
 * - JSON utilities
 * - Memory utilities
 * - System utilities
 * 
 * Author: xApp Template Generator
 * Version: 1.0.0
 */

#include "utils.h"
#include <json-c/json.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <pthread.h>

// Global logging context
log_context_t g_log_ctx = {0};

// Initialize logging system
int utils_init_logging(const char* log_file_path, log_level_t level) {
    memset(&g_log_ctx, 0, sizeof(log_context_t));
    
    g_log_ctx.current_level = level;
    g_log_ctx.use_colors = isatty(STDOUT_FILENO);
    g_log_ctx.log_to_console = true;
    g_log_ctx.log_to_file = (log_file_path != NULL);
    
    if (log_file_path) {
        strncpy(g_log_ctx.log_file_path, log_file_path, sizeof(g_log_ctx.log_file_path) - 1);
        g_log_ctx.log_file = fopen(log_file_path, "a");
        if (!g_log_ctx.log_file) {
            fprintf(stderr, "Failed to open log file: %s\n", log_file_path);
            g_log_ctx.log_to_file = false;
        }
    }
    
    pthread_mutex_init(&g_log_ctx.log_mutex, NULL);
    
    return 0;
}

// Cleanup logging system
void utils_cleanup_logging(void) {
    if (g_log_ctx.log_file) {
        fclose(g_log_ctx.log_file);
        g_log_ctx.log_file = NULL;
    }
    
    pthread_mutex_destroy(&g_log_ctx.log_mutex);
}

// Convert log level to string
const char* utils_log_level_to_string(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO: return "INFO";
        case LOG_LEVEL_WARNING: return "WARN";
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

// Convert log level to color
const char* utils_log_level_to_color(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_DEBUG: return COLOR_CYAN;
        case LOG_LEVEL_INFO: return COLOR_GREEN;
        case LOG_LEVEL_WARNING: return COLOR_YELLOW;
        case LOG_LEVEL_ERROR: return COLOR_RED;
        case LOG_LEVEL_CRITICAL: return COLOR_RED COLOR_BOLD;
        default: return COLOR_RESET;
    }
}

// Main logging function
void utils_log(log_level_t level, const char* format, ...) {
    if (level < g_log_ctx.current_level) {
        return;
    }
    
    pthread_mutex_lock(&g_log_ctx.log_mutex);
    
    // Get timestamp
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Format message
    va_list args;
    va_start(args, format);
    char message[1024];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // Log to console
    if (g_log_ctx.log_to_console) {
        if (g_log_ctx.use_colors) {
            fprintf(stdout, "%s[%s] %s%s %s%s\n", 
                   utils_log_level_to_color(level),
                   timestamp,
                   COLOR_BOLD,
                   utils_log_level_to_string(level),
                   COLOR_RESET,
                   message);
        } else {
            fprintf(stdout, "[%s] %s %s\n", 
                   timestamp,
                   utils_log_level_to_string(level),
                   message);
        }
        fflush(stdout);
    }
    
    // Log to file
    if (g_log_ctx.log_to_file && g_log_ctx.log_file) {
        fprintf(g_log_ctx.log_file, "[%s] %s %s\n", 
               timestamp,
               utils_log_level_to_string(level),
               message);
        fflush(g_log_ctx.log_file);
    }
    
    pthread_mutex_unlock(&g_log_ctx.log_mutex);
}

// Get current time
void utils_get_current_time(struct timespec* ts) {
    clock_gettime(CLOCK_REALTIME, ts);
}

// Get timestamp in milliseconds
uint64_t utils_get_timestamp_ms(void) {
    struct timespec ts;
    utils_get_current_time(&ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// Get timestamp in microseconds
uint64_t utils_get_timestamp_us(void) {
    struct timespec ts;
    utils_get_current_time(&ts);
    return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

// Calculate time difference
double utils_timespec_diff(const struct timespec* start, const struct timespec* end) {
    return (end->tv_sec - start->tv_sec) + (end->tv_nsec - start->tv_nsec) / 1000000000.0;
}

// Sleep for milliseconds
void utils_sleep_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

// Sleep for microseconds
void utils_sleep_us(int microseconds) {
    struct timespec ts;
    ts.tv_sec = microseconds / 1000000;
    ts.tv_nsec = (microseconds % 1000000) * 1000;
    nanosleep(&ts, NULL);
}

// Format timestamp
char* utils_format_timestamp(time_t timestamp, char* buffer, size_t buffer_size) {
    struct tm* tm_info = localtime(&timestamp);
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info);
    return buffer;
}

// Format duration
char* utils_format_duration(double seconds, char* buffer, size_t buffer_size) {
    if (seconds < 60) {
        snprintf(buffer, buffer_size, "%.2f seconds", seconds);
    } else if (seconds < 3600) {
        snprintf(buffer, buffer_size, "%.1f minutes", seconds / 60.0);
    } else if (seconds < 86400) {
        snprintf(buffer, buffer_size, "%.1f hours", seconds / 3600.0);
    } else {
        snprintf(buffer, buffer_size, "%.1f days", seconds / 86400.0);
    }
    return buffer;
}

// Performance timer functions
void utils_timer_start(performance_timer_t* timer) {
    if (timer) {
        utils_get_current_time(&timer->start_time);
        timer->running = true;
    }
}

double utils_timer_stop(performance_timer_t* timer) {
    if (timer && timer->running) {
        utils_get_current_time(&timer->end_time);
        timer->running = false;
        return utils_timespec_diff(&timer->start_time, &timer->end_time);
    }
    return 0.0;
}

double utils_timer_elapsed(const performance_timer_t* timer) {
    if (timer && timer->running) {
        struct timespec now;
        utils_get_current_time(&now);
        return utils_timespec_diff(&timer->start_time, &now);
    } else if (timer) {
        return utils_timespec_diff(&timer->start_time, &timer->end_time);
    }
    return 0.0;
}

// String utilities
char* utils_trim_whitespace(char* str) {
    if (!str) return NULL;
    
    // Trim leading whitespace
    while (isspace((unsigned char)*str)) str++;
    
    // If string is empty
    if (*str == 0) return str;
    
    // Trim trailing whitespace
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    // Null terminate
    *(end + 1) = '\0';
    
    return str;
}

char* utils_to_lowercase(char* str) {
    if (!str) return NULL;
    
    for (char* p = str; *p; p++) {
        *p = tolower((unsigned char)*p);
    }
    return str;
}

char* utils_to_uppercase(char* str) {
    if (!str) return NULL;
    
    for (char* p = str; *p; p++) {
        *p = toupper((unsigned char)*p);
    }
    return str;
}

bool utils_string_equals_ignore_case(const char* str1, const char* str2) {
    if (!str1 || !str2) return false;
    
    return strcasecmp(str1, str2) == 0;
}

bool utils_string_starts_with(const char* str, const char* prefix) {
    if (!str || !prefix) return false;
    
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool utils_string_ends_with(const char* str, const char* suffix) {
    if (!str || !suffix) return false;
    
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (suffix_len > str_len) return false;
    
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

// File utilities
bool utils_file_exists(const char* filepath) {
    if (!filepath) return false;
    
    return access(filepath, F_OK) == 0;
}

size_t utils_file_size(const char* filepath) {
    if (!filepath) return 0;
    
    struct stat st;
    if (stat(filepath, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

char* utils_read_file(const char* filepath, size_t* size) {
    if (!filepath) return NULL;
    
    FILE* file = fopen(filepath, "rb");
    if (!file) return NULL;
    
    // Get file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate buffer
    char* buffer = malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    // Read file
    size_t read_size = fread(buffer, 1, file_size, file);
    fclose(file);
    
    if (read_size != file_size) {
        free(buffer);
        return NULL;
    }
    
    buffer[file_size] = '\0';
    
    if (size) *size = file_size;
    return buffer;
}

int utils_write_file(const char* filepath, const void* data, size_t size) {
    if (!filepath || !data) return -1;
    
    FILE* file = fopen(filepath, "wb");
    if (!file) return -1;
    
    size_t written = fwrite(data, 1, size, file);
    fclose(file);
    
    return (written == size) ? 0 : -1;
}

// JSON utilities
bool utils_json_get_string(json_object* obj, const char* key, char* value, size_t value_size) {
    if (!obj || !key || !value) return false;
    
    json_object* json_value;
    if (!json_object_object_get_ex(obj, key, &json_value)) {
        return false;
    }
    
    const char* str_value = json_object_get_string(json_value);
    if (!str_value) return false;
    
    strncpy(value, str_value, value_size - 1);
    value[value_size - 1] = '\0';
    
    return true;
}

bool utils_json_get_int(json_object* obj, const char* key, int* value) {
    if (!obj || !key || !value) return false;
    
    json_object* json_value;
    if (!json_object_object_get_ex(obj, key, &json_value)) {
        return false;
    }
    
    if (json_object_get_type(json_value) != json_type_int) {
        return false;
    }
    
    *value = json_object_get_int(json_value);
    return true;
}

bool utils_json_get_double(json_object* obj, const char* key, double* value) {
    if (!obj || !key || !value) return false;
    
    json_object* json_value;
    if (!json_object_object_get_ex(obj, key, &json_value)) {
        return false;
    }
    
    if (json_object_get_type(json_value) == json_type_double) {
        *value = json_object_get_double(json_value);
        return true;
    } else if (json_object_get_type(json_value) == json_type_int) {
        *value = (double)json_object_get_int(json_value);
        return true;
    }
    
    return false;
}

bool utils_json_get_bool(json_object* obj, const char* key, bool* value) {
    if (!obj || !key || !value) return false;
    
    json_object* json_value;
    if (!json_object_object_get_ex(obj, key, &json_value)) {
        return false;
    }
    
    if (json_object_get_type(json_value) != json_type_boolean) {
        return false;
    }
    
    *value = json_object_get_boolean(json_value);
    return true;
}

json_object* utils_json_load_file(const char* filepath) {
    if (!filepath) return NULL;
    
    return json_object_from_file(filepath);
}

int utils_json_save_file(json_object* obj, const char* filepath) {
    if (!obj || !filepath) return -1;
    
    return json_object_to_file(filepath, obj);
}

// Memory utilities
void* utils_malloc_zero(size_t size) {
    void* ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void* utils_realloc_safe(void* ptr, size_t new_size) {
    void* new_ptr = realloc(ptr, new_size);
    if (!new_ptr && new_size > 0) {
        free(ptr);
        return NULL;
    }
    return new_ptr;
}

char* utils_strdup_safe(const char* str) {
    if (!str) return NULL;
    
    size_t len = strlen(str) + 1;
    char* copy = malloc(len);
    if (copy) {
        memcpy(copy, str, len);
    }
    return copy;
}

// System utilities
int utils_get_cpu_count(void) {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

double utils_get_cpu_usage(void) {
    static unsigned long long last_total = 0;
    static unsigned long long last_idle = 0;
    
    FILE* file = fopen("/proc/stat", "r");
    if (!file) return 0.0;
    
    char line[256];
    if (!fgets(line, sizeof(line), file)) {
        fclose(file);
        return 0.0;
    }
    fclose(file);
    
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    
    unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal;
    
    if (last_total == 0) {
        last_total = total;
        last_idle = idle;
        return 0.0;
    }
    
    unsigned long long total_diff = total - last_total;
    unsigned long long idle_diff = idle - last_idle;
    
    last_total = total;
    last_idle = idle;
    
    if (total_diff == 0) return 0.0;
    
    return (double)(total_diff - idle_diff) / total_diff * 100.0;
}

size_t utils_get_memory_usage(void) {
    FILE* file = fopen("/proc/self/status", "r");
    if (!file) return 0;
    
    char line[256];
    size_t memory_usage = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line, "VmRSS: %zu kB", &memory_usage);
            memory_usage *= 1024;  // Convert to bytes
            break;
        }
    }
    
    fclose(file);
    return memory_usage;
}

int utils_get_process_id(void) {
    return getpid();
}

char* utils_get_hostname(char* buffer, size_t buffer_size) {
    if (!buffer) return NULL;
    
    if (gethostname(buffer, buffer_size) == 0) {
        return buffer;
    }
    
    return NULL;
}

// Hash functions
uint32_t utils_hash_string(const char* str) {
    if (!str) return 0;
    
    uint32_t hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    return hash;
}

uint32_t utils_hash_bytes(const void* data, size_t size) {
    if (!data) return 0;
    
    const unsigned char* bytes = (const unsigned char*)data;
    uint32_t hash = 5381;
    
    for (size_t i = 0; i < size; i++) {
        hash = ((hash << 5) + hash) + bytes[i];
    }
    
    return hash;
}

// Random utilities
void utils_random_seed(void) {
    srand(time(NULL));
}

int utils_random_int(int min, int max) {
    if (min >= max) return min;
    return min + rand() % (max - min + 1);
}

double utils_random_double(double min, double max) {
    if (min >= max) return min;
    return min + (max - min) * rand() / RAND_MAX;
}

// Validation utilities
bool utils_validate_node_id(uint32_t node_id) {
    return node_id > 0 && node_id < 0xFFFFFFFF;
}

bool utils_validate_subscription_id(uint32_t subscription_id) {
    return subscription_id > 0 && subscription_id < 0xFFFFFFFF;
}

bool utils_validate_metric_value(double value, double min_val, double max_val) {
    return value >= min_val && value <= max_val && !isnan(value) && !isinf(value);
}

bool utils_validate_config_path(const char* path) {
    if (!path) return false;
    
    // Check if path exists and is readable
    return access(path, R_OK) == 0;
}

// Error handling
const char* utils_errno_to_string(int error_code) {
    return strerror(error_code);
}