#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

// Metric types
typedef enum {
    METRIC_THROUGHPUT,
    METRIC_LATENCY,
    METRIC_PACKET_LOSS,
    METRIC_CPU_UTILIZATION,
    METRIC_MEMORY_USAGE,
    METRIC_RSRP,
    METRIC_RSRQ,
    METRIC_SINR,
    METRIC_PRB_USAGE,
    METRIC_COUNT
} metric_type_t;

// Anomaly severity levels
typedef enum {
    ANOMALY_NONE,
    ANOMALY_WARNING,
    ANOMALY_CRITICAL
} anomaly_severity_t;

// Recommendation types
typedef enum {
    RECOMMENDATION_NONE,
    RECOMMENDATION_INCREASE_POWER,
    RECOMMENDATION_DECREASE_POWER,
    RECOMMENDATION_HANDOVER,
    RECOMMENDATION_LOAD_BALANCE,
    RECOMMENDATION_RESOURCE_ALLOCATION,
    RECOMMENDATION_PARAMETER_ADJUSTMENT
} recommendation_type_t;

// Metric data point
typedef struct {
    metric_type_t type;
    double value;
    uint32_t node_id;
    uint32_t cell_id;
    time_t timestamp;
} metric_data_t;

// Statistical analysis results
typedef struct {
    double mean;
    double variance;
    double std_dev;
    double min;
    double max;
    double median;
    double z_score;
    bool is_outlier;
} stats_result_t;

// Trend analysis results
typedef struct {
    double slope;
    double intercept;
    double correlation;
    bool is_increasing;
    bool is_decreasing;
    bool is_stable;
} trend_result_t;

// Anomaly detection result
typedef struct {
    metric_type_t metric_type;
    anomaly_severity_t severity;
    double threshold_value;
    double actual_value;
    double confidence;
    time_t detected_at;
    char description[256];
} anomaly_result_t;

// Recommendation result
typedef struct {
    recommendation_type_t type;
    uint32_t node_id;
    uint32_t cell_id;
    double confidence;
    double expected_improvement;
    time_t generated_at;
    char description[512];
    char parameters[256];
} recommendation_result_t;

// Threshold configuration
typedef struct {
    double warning_threshold;
    double critical_threshold;
    double min_value;
    double max_value;
    bool enabled;
} threshold_config_t;

// Analytics configuration
typedef struct {
    threshold_config_t thresholds[METRIC_COUNT];
    int window_size;
    int trend_window;
    double outlier_threshold;
    double correlation_threshold;
    bool enable_ml_detection;
    bool enable_prediction;
} analytics_config_t;

// Metric history for trend analysis
typedef struct {
    metric_data_t data[1000];  // Circular buffer
    int head;
    int tail;
    int count;
    stats_result_t last_stats;
    trend_result_t last_trend;
} metric_history_t;

// Analytics context
typedef struct {
    analytics_config_t config;
    metric_history_t history[METRIC_COUNT];
    
    // Anomaly detection state
    anomaly_result_t recent_anomalies[100];
    int anomaly_count;
    
    // Recommendation state
    recommendation_result_t recent_recommendations[100];
    int recommendation_count;
    
    // ML models (simplified)
    struct {
        bool initialized;
        double weights[10];
        double bias;
        double learning_rate;
    } ml_model;
    
    // Performance counters
    uint64_t processed_metrics;
    uint64_t detected_anomalies;
    uint64_t generated_recommendations;
    
} analytics_context_t;

// Function prototypes

// Context management
analytics_context_t* analytics_init(const char* config_file);
void analytics_cleanup(analytics_context_t* ctx);
int analytics_load_config(analytics_context_t* ctx, const char* config_file);

// Metric processing
int analytics_process_metric(analytics_context_t* ctx, const metric_data_t* metric);
int analytics_add_metric(analytics_context_t* ctx, metric_type_t type, double value, uint32_t node_id, uint32_t cell_id);

// Statistical analysis
stats_result_t analytics_calculate_stats(const metric_data_t* data, int count);
trend_result_t analytics_calculate_trend(const metric_data_t* data, int count);
double analytics_calculate_z_score(double value, double mean, double std_dev);
bool analytics_is_outlier(double z_score, double threshold);

// Anomaly detection
anomaly_result_t analytics_detect_anomaly(analytics_context_t* ctx, const metric_data_t* metric);
anomaly_result_t analytics_threshold_detection(analytics_context_t* ctx, const metric_data_t* metric);
anomaly_result_t analytics_statistical_detection(analytics_context_t* ctx, const metric_data_t* metric);
anomaly_result_t analytics_ml_detection(analytics_context_t* ctx, const metric_data_t* metric);

// Recommendation generation
recommendation_result_t analytics_generate_recommendation(analytics_context_t* ctx, const metric_data_t* metric, const anomaly_result_t* anomaly);
recommendation_result_t analytics_performance_recommendation(analytics_context_t* ctx, const metric_data_t* metric);
recommendation_result_t analytics_resource_recommendation(analytics_context_t* ctx, const metric_data_t* metric);
recommendation_result_t analytics_mobility_recommendation(analytics_context_t* ctx, const metric_data_t* metric);

// Machine learning (simplified)
int analytics_train_ml_model(analytics_context_t* ctx, const metric_data_t* training_data, int count);
double analytics_predict_ml(analytics_context_t* ctx, const metric_data_t* metric);
int analytics_update_ml_model(analytics_context_t* ctx, const metric_data_t* metric, double target);

// Data access
metric_history_t* analytics_get_history(analytics_context_t* ctx, metric_type_t type);
anomaly_result_t* analytics_get_recent_anomalies(analytics_context_t* ctx, int* count);
recommendation_result_t* analytics_get_recent_recommendations(analytics_context_t* ctx, int* count);

// Utility functions
const char* analytics_metric_type_to_string(metric_type_t type);
const char* analytics_anomaly_severity_to_string(anomaly_severity_t severity);
const char* analytics_recommendation_type_to_string(recommendation_type_t type);

// Reporting
void analytics_print_stats(const stats_result_t* stats);
void analytics_print_trend(const trend_result_t* trend);
void analytics_print_anomaly(const anomaly_result_t* anomaly);
void analytics_print_recommendation(const recommendation_result_t* recommendation);
void analytics_generate_report(analytics_context_t* ctx, FILE* output);

// Performance monitoring
void analytics_print_performance(const analytics_context_t* ctx);

#endif // ANALYTICS_H