/*
 * Analytics Module for Smart Monitor xApp
 * 
 * This module provides sophisticated analytics capabilities including:
 * - Statistical analysis of metrics
 * - Anomaly detection using multiple algorithms
 * - Trend analysis and prediction
 * - Intelligent recommendation generation
 * 
 * Author: xApp Template Generator
 * Version: 1.0.0
 */

#include "analytics.h"
#include "utils.h"
#include <json-c/json.h>

// String conversion functions
const char* analytics_metric_type_to_string(metric_type_t type) {
    switch (type) {
        case METRIC_THROUGHPUT: return "Throughput";
        case METRIC_LATENCY: return "Latency";
        case METRIC_PACKET_LOSS: return "Packet Loss";
        case METRIC_CPU_UTILIZATION: return "CPU Utilization";
        case METRIC_MEMORY_USAGE: return "Memory Usage";
        case METRIC_RSRP: return "RSRP";
        case METRIC_RSRQ: return "RSRQ";
        case METRIC_SINR: return "SINR";
        case METRIC_PRB_USAGE: return "PRB Usage";
        default: return "Unknown";
    }
}

const char* analytics_anomaly_severity_to_string(anomaly_severity_t severity) {
    switch (severity) {
        case ANOMALY_NONE: return "None";
        case ANOMALY_WARNING: return "Warning";
        case ANOMALY_CRITICAL: return "Critical";
        default: return "Unknown";
    }
}

const char* analytics_recommendation_type_to_string(recommendation_type_t type) {
    switch (type) {
        case RECOMMENDATION_NONE: return "None";
        case RECOMMENDATION_INCREASE_POWER: return "Increase Power";
        case RECOMMENDATION_DECREASE_POWER: return "Decrease Power";
        case RECOMMENDATION_HANDOVER: return "Handover";
        case RECOMMENDATION_LOAD_BALANCE: return "Load Balance";
        case RECOMMENDATION_RESOURCE_ALLOCATION: return "Resource Allocation";
        case RECOMMENDATION_PARAMETER_ADJUSTMENT: return "Parameter Adjustment";
        default: return "Unknown";
    }
}

// Initialize analytics context
analytics_context_t* analytics_init(const char* config_file) {
    analytics_context_t* ctx = malloc(sizeof(analytics_context_t));
    if (!ctx) {
        LOG_ERROR("Failed to allocate analytics context");
        return NULL;
    }
    
    memset(ctx, 0, sizeof(analytics_context_t));
    
    // Initialize default configuration
    ctx->config.window_size = 100;
    ctx->config.trend_window = 50;
    ctx->config.outlier_threshold = 2.0;
    ctx->config.correlation_threshold = 0.7;
    ctx->config.enable_ml_detection = true;
    ctx->config.enable_prediction = true;
    
    // Initialize default thresholds
    for (int i = 0; i < METRIC_COUNT; i++) {
        ctx->config.thresholds[i].enabled = true;
        ctx->config.thresholds[i].warning_threshold = 80.0;
        ctx->config.thresholds[i].critical_threshold = 95.0;
        ctx->config.thresholds[i].min_value = 0.0;
        ctx->config.thresholds[i].max_value = 100.0;
    }
    
    // Set specific thresholds for different metrics
    ctx->config.thresholds[METRIC_THROUGHPUT].min_value = 0.0;
    ctx->config.thresholds[METRIC_THROUGHPUT].max_value = 1000.0;
    ctx->config.thresholds[METRIC_THROUGHPUT].warning_threshold = 100.0;
    ctx->config.thresholds[METRIC_THROUGHPUT].critical_threshold = 50.0;
    
    ctx->config.thresholds[METRIC_LATENCY].min_value = 0.0;
    ctx->config.thresholds[METRIC_LATENCY].max_value = 200.0;
    ctx->config.thresholds[METRIC_LATENCY].warning_threshold = 50.0;
    ctx->config.thresholds[METRIC_LATENCY].critical_threshold = 100.0;
    
    ctx->config.thresholds[METRIC_PACKET_LOSS].min_value = 0.0;
    ctx->config.thresholds[METRIC_PACKET_LOSS].max_value = 100.0;
    ctx->config.thresholds[METRIC_PACKET_LOSS].warning_threshold = 1.0;
    ctx->config.thresholds[METRIC_PACKET_LOSS].critical_threshold = 5.0;
    
    // Initialize ML model with random weights
    ctx->ml_model.initialized = false;
    ctx->ml_model.learning_rate = 0.01;
    ctx->ml_model.bias = 0.0;
    for (int i = 0; i < 10; i++) {
        ctx->ml_model.weights[i] = (rand() / (double)RAND_MAX) - 0.5;
    }
    
    // Load configuration from file if provided
    if (config_file) {
        analytics_load_config(ctx, config_file);
    }
    
    LOG_INFO("Analytics initialized successfully");
    return ctx;
}

// Cleanup analytics context
void analytics_cleanup(analytics_context_t* ctx) {
    if (ctx) {
        LOG_INFO("Cleaning up analytics context");
        free(ctx);
    }
}

// Load configuration from file
int analytics_load_config(analytics_context_t* ctx, const char* config_file) {
    if (!ctx || !config_file) {
        return -1;
    }
    
    json_object* config_obj = utils_json_load_file(config_file);
    if (!config_obj) {
        LOG_WARN("Failed to load analytics configuration file: %s", config_file);
        return -1;
    }
    
    LOG_INFO("Loading analytics configuration from %s", config_file);
    
    // Parse thresholds
    json_object* thresholds_obj;
    if (json_object_object_get_ex(config_obj, "thresholds", &thresholds_obj)) {
        // Parse specific metric thresholds
        for (int i = 0; i < METRIC_COUNT; i++) {
            const char* metric_name = analytics_metric_type_to_string(i);
            json_object* metric_obj;
            
            if (json_object_object_get_ex(thresholds_obj, metric_name, &metric_obj)) {
                utils_json_get_double(metric_obj, "warning", &ctx->config.thresholds[i].warning_threshold);
                utils_json_get_double(metric_obj, "critical", &ctx->config.thresholds[i].critical_threshold);
                utils_json_get_double(metric_obj, "min", &ctx->config.thresholds[i].min_value);
                utils_json_get_double(metric_obj, "max", &ctx->config.thresholds[i].max_value);
                utils_json_get_bool(metric_obj, "enabled", &ctx->config.thresholds[i].enabled);
            }
        }
    }
    
    json_object_put(config_obj);
    
    LOG_INFO("Analytics configuration loaded successfully");
    return 0;
}

// Add a metric to the analytics system
int analytics_add_metric(analytics_context_t* ctx, metric_type_t type, double value, uint32_t node_id, uint32_t cell_id) {
    if (!ctx || type >= METRIC_COUNT) {
        return -1;
    }
    
    metric_data_t metric = {
        .type = type,
        .value = value,
        .node_id = node_id,
        .cell_id = cell_id,
        .timestamp = time(NULL)
    };
    
    return analytics_process_metric(ctx, &metric);
}

// Process a metric through the analytics pipeline
int analytics_process_metric(analytics_context_t* ctx, const metric_data_t* metric) {
    if (!ctx || !metric || metric->type >= METRIC_COUNT) {
        return -1;
    }
    
    ctx->processed_metrics++;
    
    // Add to history
    metric_history_t* history = &ctx->history[metric->type];
    
    // Store in circular buffer
    history->data[history->head] = *metric;
    history->head = (history->head + 1) % 1000;
    
    if (history->count < 1000) {
        history->count++;
    } else {
        history->tail = (history->tail + 1) % 1000;
    }
    
    // Perform analytics if we have enough data
    if (history->count >= 10) {
        // Calculate statistics
        history->last_stats = analytics_calculate_stats(history->data, history->count);
        
        // Calculate trends if we have enough data
        if (history->count >= ctx->config.trend_window) {
            history->last_trend = analytics_calculate_trend(history->data, 
                                                           MIN(history->count, ctx->config.trend_window));
        }
        
        // Detect anomalies
        anomaly_result_t anomaly = analytics_detect_anomaly(ctx, metric);
        if (anomaly.severity > ANOMALY_NONE) {
            // Store anomaly
            ctx->recent_anomalies[ctx->anomaly_count % 100] = anomaly;
            ctx->anomaly_count++;
            ctx->detected_anomalies++;
            
            // Generate recommendation based on anomaly
            recommendation_result_t recommendation = analytics_generate_recommendation(ctx, metric, &anomaly);
            if (recommendation.type != RECOMMENDATION_NONE) {
                ctx->recent_recommendations[ctx->recommendation_count % 100] = recommendation;
                ctx->recommendation_count++;
                ctx->generated_recommendations++;
            }
        }
    }
    
    return 0;
}

// Calculate statistical analysis
stats_result_t analytics_calculate_stats(const metric_data_t* data, int count) {
    stats_result_t stats = {0};
    
    if (!data || count <= 0) {
        return stats;
    }
    
    // Calculate mean
    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        sum += data[i].value;
    }
    stats.mean = sum / count;
    
    // Calculate variance and standard deviation
    double variance_sum = 0.0;
    stats.min = data[0].value;
    stats.max = data[0].value;
    
    for (int i = 0; i < count; i++) {
        double diff = data[i].value - stats.mean;
        variance_sum += diff * diff;
        
        if (data[i].value < stats.min) stats.min = data[i].value;
        if (data[i].value > stats.max) stats.max = data[i].value;
    }
    
    stats.variance = variance_sum / count;
    stats.std_dev = sqrt(stats.variance);
    
    // Calculate median (simplified - sort and take middle)
    double* sorted_values = malloc(count * sizeof(double));
    for (int i = 0; i < count; i++) {
        sorted_values[i] = data[i].value;
    }
    
    // Simple bubble sort for median calculation
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (sorted_values[j] > sorted_values[j + 1]) {
                double temp = sorted_values[j];
                sorted_values[j] = sorted_values[j + 1];
                sorted_values[j + 1] = temp;
            }
        }
    }
    
    if (count % 2 == 0) {
        stats.median = (sorted_values[count/2 - 1] + sorted_values[count/2]) / 2.0;
    } else {
        stats.median = sorted_values[count/2];
    }
    
    free(sorted_values);
    
    // Calculate z-score for last value
    if (count > 0) {
        stats.z_score = analytics_calculate_z_score(data[count-1].value, stats.mean, stats.std_dev);
        stats.is_outlier = analytics_is_outlier(stats.z_score, 2.0);
    }
    
    return stats;
}

// Calculate trend analysis
trend_result_t analytics_calculate_trend(const metric_data_t* data, int count) {
    trend_result_t trend = {0};
    
    if (!data || count <= 1) {
        return trend;
    }
    
    // Simple linear regression
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
    
    for (int i = 0; i < count; i++) {
        double x = (double)i;
        double y = data[i].value;
        
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x2 += x * x;
    }
    
    double n = (double)count;
    trend.slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
    trend.intercept = (sum_y - trend.slope * sum_x) / n;
    
    // Calculate correlation coefficient
    double mean_x = sum_x / n;
    double mean_y = sum_y / n;
    
    double sum_x_dev = 0.0, sum_y_dev = 0.0, sum_xy_dev = 0.0;
    
    for (int i = 0; i < count; i++) {
        double x_dev = (double)i - mean_x;
        double y_dev = data[i].value - mean_y;
        
        sum_x_dev += x_dev * x_dev;
        sum_y_dev += y_dev * y_dev;
        sum_xy_dev += x_dev * y_dev;
    }
    
    if (sum_x_dev > 0 && sum_y_dev > 0) {
        trend.correlation = sum_xy_dev / sqrt(sum_x_dev * sum_y_dev);
    }
    
    // Determine trend direction
    const double slope_threshold = 0.1;
    trend.is_increasing = (trend.slope > slope_threshold);
    trend.is_decreasing = (trend.slope < -slope_threshold);
    trend.is_stable = (fabs(trend.slope) <= slope_threshold);
    
    return trend;
}

// Calculate Z-score
double analytics_calculate_z_score(double value, double mean, double std_dev) {
    if (std_dev <= 0) {
        return 0.0;
    }
    return (value - mean) / std_dev;
}

// Check if value is an outlier
bool analytics_is_outlier(double z_score, double threshold) {
    return fabs(z_score) > threshold;
}

// Detect anomalies using multiple algorithms
anomaly_result_t analytics_detect_anomaly(analytics_context_t* ctx, const metric_data_t* metric) {
    anomaly_result_t anomaly = {0};
    anomaly.metric_type = metric->type;
    anomaly.detected_at = time(NULL);
    
    // Try threshold-based detection first
    anomaly_result_t threshold_result = analytics_threshold_detection(ctx, metric);
    if (threshold_result.severity > ANOMALY_NONE) {
        return threshold_result;
    }
    
    // Try statistical detection
    anomaly_result_t statistical_result = analytics_statistical_detection(ctx, metric);
    if (statistical_result.severity > ANOMALY_NONE) {
        return statistical_result;
    }
    
    // Try ML detection if enabled
    if (ctx->config.enable_ml_detection) {
        anomaly_result_t ml_result = analytics_ml_detection(ctx, metric);
        if (ml_result.severity > ANOMALY_NONE) {
            return ml_result;
        }
    }
    
    return anomaly;
}

// Threshold-based anomaly detection
anomaly_result_t analytics_threshold_detection(analytics_context_t* ctx, const metric_data_t* metric) {
    anomaly_result_t anomaly = {0};
    anomaly.metric_type = metric->type;
    anomaly.actual_value = metric->value;
    anomaly.detected_at = time(NULL);
    
    const threshold_config_t* threshold = &ctx->config.thresholds[metric->type];
    
    if (!threshold->enabled) {
        return anomaly;
    }
    
    // Check critical threshold
    if (metric->value >= threshold->critical_threshold) {
        anomaly.severity = ANOMALY_CRITICAL;
        anomaly.threshold_value = threshold->critical_threshold;
        anomaly.confidence = 1.0;
        snprintf(anomaly.description, sizeof(anomaly.description),
                "Critical threshold exceeded: %.2f >= %.2f (%s)",
                metric->value, threshold->critical_threshold,
                analytics_metric_type_to_string(metric->type));
    }
    // Check warning threshold
    else if (metric->value >= threshold->warning_threshold) {
        anomaly.severity = ANOMALY_WARNING;
        anomaly.threshold_value = threshold->warning_threshold;
        anomaly.confidence = 0.8;
        snprintf(anomaly.description, sizeof(anomaly.description),
                "Warning threshold exceeded: %.2f >= %.2f (%s)",
                metric->value, threshold->warning_threshold,
                analytics_metric_type_to_string(metric->type));
    }
    // Check minimum value
    else if (metric->value <= threshold->min_value) {
        anomaly.severity = ANOMALY_WARNING;
        anomaly.threshold_value = threshold->min_value;
        anomaly.confidence = 0.7;
        snprintf(anomaly.description, sizeof(anomaly.description),
                "Minimum value violation: %.2f <= %.2f (%s)",
                metric->value, threshold->min_value,
                analytics_metric_type_to_string(metric->type));
    }
    
    return anomaly;
}

// Statistical anomaly detection
anomaly_result_t analytics_statistical_detection(analytics_context_t* ctx, const metric_data_t* metric) {
    anomaly_result_t anomaly = {0};
    anomaly.metric_type = metric->type;
    anomaly.actual_value = metric->value;
    anomaly.detected_at = time(NULL);
    
    metric_history_t* history = &ctx->history[metric->type];
    
    if (history->count < 10) {
        return anomaly;  // Not enough data
    }
    
    const stats_result_t* stats = &history->last_stats;
    
    // Check for statistical outliers
    if (stats->is_outlier) {
        anomaly.severity = (fabs(stats->z_score) > 3.0) ? ANOMALY_CRITICAL : ANOMALY_WARNING;
        anomaly.threshold_value = stats->mean + (stats->z_score > 0 ? 1 : -1) * 2.0 * stats->std_dev;
        anomaly.confidence = MIN(fabs(stats->z_score) / 3.0, 1.0);
        snprintf(anomaly.description, sizeof(anomaly.description),
                "Statistical outlier detected: %.2f (z-score: %.2f) (%s)",
                metric->value, stats->z_score,
                analytics_metric_type_to_string(metric->type));
    }
    
    return anomaly;
}

// Machine learning based anomaly detection (simplified)
anomaly_result_t analytics_ml_detection(analytics_context_t* ctx, const metric_data_t* metric) {
    anomaly_result_t anomaly = {0};
    anomaly.metric_type = metric->type;
    anomaly.actual_value = metric->value;
    anomaly.detected_at = time(NULL);
    
    // Simple ML model prediction
    double prediction = analytics_predict_ml(ctx, metric);
    double error = fabs(metric->value - prediction);
    
    metric_history_t* history = &ctx->history[metric->type];
    
    if (history->count < 20) {
        return anomaly;  // Not enough data for ML
    }
    
    // Calculate error threshold based on historical data
    double error_threshold = history->last_stats.std_dev * 2.0;
    
    if (error > error_threshold) {
        anomaly.severity = (error > error_threshold * 1.5) ? ANOMALY_CRITICAL : ANOMALY_WARNING;
        anomaly.threshold_value = prediction;
        anomaly.confidence = MIN(error / (error_threshold * 2.0), 1.0);
        snprintf(anomaly.description, sizeof(anomaly.description),
                "ML anomaly detected: %.2f (predicted: %.2f, error: %.2f) (%s)",
                metric->value, prediction, error,
                analytics_metric_type_to_string(metric->type));
    }
    
    return anomaly;
}

// Generate intelligent recommendations
recommendation_result_t analytics_generate_recommendation(analytics_context_t* ctx, const metric_data_t* metric, const anomaly_result_t* anomaly) {
    recommendation_result_t recommendation = {0};
    recommendation.node_id = metric->node_id;
    recommendation.cell_id = metric->cell_id;
    recommendation.generated_at = time(NULL);
    
    if (!anomaly || anomaly->severity == ANOMALY_NONE) {
        return recommendation;
    }
    
    // Generate recommendations based on metric type and anomaly
    switch (metric->type) {
        case METRIC_THROUGHPUT:
            if (metric->value < ctx->config.thresholds[metric->type].warning_threshold) {
                recommendation.type = RECOMMENDATION_INCREASE_POWER;
                recommendation.confidence = 0.8;
                recommendation.expected_improvement = 20.0;
                snprintf(recommendation.description, sizeof(recommendation.description),
                        "Increase transmission power to improve throughput");
                snprintf(recommendation.parameters, sizeof(recommendation.parameters),
                        "power_increase=5dB");
            }
            break;
            
        case METRIC_LATENCY:
            if (metric->value > ctx->config.thresholds[metric->type].warning_threshold) {
                recommendation.type = RECOMMENDATION_PARAMETER_ADJUSTMENT;
                recommendation.confidence = 0.7;
                recommendation.expected_improvement = 15.0;
                snprintf(recommendation.description, sizeof(recommendation.description),
                        "Adjust scheduling parameters to reduce latency");
                snprintf(recommendation.parameters, sizeof(recommendation.parameters),
                        "scheduling_weight=0.8");
            }
            break;
            
        case METRIC_PACKET_LOSS:
            if (metric->value > ctx->config.thresholds[metric->type].warning_threshold) {
                recommendation.type = RECOMMENDATION_HANDOVER;
                recommendation.confidence = 0.6;
                recommendation.expected_improvement = 30.0;
                snprintf(recommendation.description, sizeof(recommendation.description),
                        "Consider handover to reduce packet loss");
                snprintf(recommendation.parameters, sizeof(recommendation.parameters),
                        "handover_threshold=-105dBm");
            }
            break;
            
        case METRIC_PRB_USAGE:
            if (metric->value > ctx->config.thresholds[metric->type].warning_threshold) {
                recommendation.type = RECOMMENDATION_LOAD_BALANCE;
                recommendation.confidence = 0.9;
                recommendation.expected_improvement = 25.0;
                snprintf(recommendation.description, sizeof(recommendation.description),
                        "Implement load balancing to reduce PRB usage");
                snprintf(recommendation.parameters, sizeof(recommendation.parameters),
                        "load_balance_factor=0.7");
            }
            break;
            
        default:
            recommendation.type = RECOMMENDATION_PARAMETER_ADJUSTMENT;
            recommendation.confidence = 0.5;
            recommendation.expected_improvement = 10.0;
            snprintf(recommendation.description, sizeof(recommendation.description),
                    "General parameter adjustment recommended");
            snprintf(recommendation.parameters, sizeof(recommendation.parameters),
                    "generic_adjustment=true");
            break;
    }
    
    return recommendation;
}

// Simple ML prediction
double analytics_predict_ml(analytics_context_t* ctx, const metric_data_t* metric) {
    if (!ctx->ml_model.initialized) {
        return metric->value;  // No prediction available
    }
    
    // Simple feature vector (last 10 values)
    metric_history_t* history = &ctx->history[metric->type];
    
    if (history->count < 10) {
        return metric->value;
    }
    
    double prediction = ctx->ml_model.bias;
    
    // Use last 10 values as features
    for (int i = 0; i < 10 && i < history->count; i++) {
        int idx = (history->head - 1 - i + 1000) % 1000;
        prediction += ctx->ml_model.weights[i] * history->data[idx].value;
    }
    
    return prediction;
}

// Get analytics history
metric_history_t* analytics_get_history(analytics_context_t* ctx, metric_type_t type) {
    if (!ctx || type >= METRIC_COUNT) {
        return NULL;
    }
    return &ctx->history[type];
}

// Get recent anomalies
anomaly_result_t* analytics_get_recent_anomalies(analytics_context_t* ctx, int* count) {
    if (!ctx || !count) {
        return NULL;
    }
    
    *count = MIN(ctx->anomaly_count, 100);
    return ctx->recent_anomalies;
}

// Get recent recommendations
recommendation_result_t* analytics_get_recent_recommendations(analytics_context_t* ctx, int* count) {
    if (!ctx || !count) {
        return NULL;
    }
    
    *count = MIN(ctx->recommendation_count, 100);
    return ctx->recent_recommendations;
}

// Print statistics
void analytics_print_stats(const stats_result_t* stats) {
    if (!stats) return;
    
    printf("Statistics: mean=%.2f, std_dev=%.2f, min=%.2f, max=%.2f, median=%.2f, z_score=%.2f\n",
           stats->mean, stats->std_dev, stats->min, stats->max, stats->median, stats->z_score);
}

// Print trend analysis
void analytics_print_trend(const trend_result_t* trend) {
    if (!trend) return;
    
    printf("Trend: slope=%.4f, correlation=%.2f, increasing=%s, decreasing=%s, stable=%s\n",
           trend->slope, trend->correlation,
           trend->is_increasing ? "Yes" : "No",
           trend->is_decreasing ? "Yes" : "No",
           trend->is_stable ? "Yes" : "No");
}

// Print anomaly
void analytics_print_anomaly(const anomaly_result_t* anomaly) {
    if (!anomaly) return;
    
    printf("Anomaly: %s (%s) - %s\n",
           analytics_metric_type_to_string(anomaly->metric_type),
           analytics_anomaly_severity_to_string(anomaly->severity),
           anomaly->description);
}

// Print recommendation
void analytics_print_recommendation(const recommendation_result_t* recommendation) {
    if (!recommendation) return;
    
    printf("Recommendation: %s (confidence: %.2f%%) - %s\n",
           analytics_recommendation_type_to_string(recommendation->type),
           recommendation->confidence * 100.0,
           recommendation->description);
}

// Print performance statistics
void analytics_print_performance(const analytics_context_t* ctx) {
    if (!ctx) return;
    
    LOG_INFO("Analytics Performance:");
    LOG_INFO("  Processed Metrics: %llu", (unsigned long long)ctx->processed_metrics);
    LOG_INFO("  Detected Anomalies: %llu", (unsigned long long)ctx->detected_anomalies);
    LOG_INFO("  Generated Recommendations: %llu", (unsigned long long)ctx->generated_recommendations);
    
    if (ctx->processed_metrics > 0) {
        LOG_INFO("  Anomaly Rate: %.2f%%", 
                (double)ctx->detected_anomalies / ctx->processed_metrics * 100.0);
        LOG_INFO("  Recommendation Rate: %.2f%%", 
                (double)ctx->generated_recommendations / ctx->processed_metrics * 100.0);
    }
}