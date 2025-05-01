#include "BRR.h"


CSIData load_csi_data(const char *file_path) {
    CSIData csi = {0};
    FILE *file = fopen(file_path, "r");
    if (!file) {
        printf("Error opening file\n");
        return csi;
    }

    printf("Loading CSI data from %s\n", file_path);
    char line[MAX_DATA_LEN];
    int row = 0;

    int col = 0;
    while (fgets(line, sizeof(line), file) && row < MAX_ROWS) {
        // csi.rows[row].len = len;
        // printf("Line content: %s\n", line);
        char *token = strtok(line, ",");
        col = 0;
        int real_col = 0;
        int imaginary_col = 0;
        while (token != NULL) {
            if (col % 2 == 0) {
                real_col = atof(token);
            } else {
                imaginary_col = atof(token);
                csi.amplitude[row][col / 2] = real_col * real_col + imaginary_col * imaginary_col;
            }
            token = strtok(NULL, ",");
            col++;
        }
        csi.num_subcarriers = col / 2;
        row++;
    }
    csi.num_rows = row;
    fclose(file);
    return csi;
}

// Get G(t, f) = |H(t, f)|^2

float apply_mrc_across_subcarriers(float* subcarriers_amplitudes_at_a_time, int num_of_subcarriers){
    float sum = 0.0;
    for (int i = 0; i < num_of_subcarriers; i++){
        sum += subcarriers_amplitudes_at_a_time[i];
    }
    return sum / 1e4;
}
// calculate \sum_{t=1}^T G(t, f) for some subcarrier f

float sample_auto_covariance(float *data, int start, int end, int lag) {
    int data_len = end - start;
    if (data_len <= lag){
        return 0;
    }
    if (data_len <= lag){
        return 0.0;
    }
    float mean = 0;
    for (int i = start; i < end; i++){
        mean += data[i];
    }
    mean /= data_len;
    float sum = 0;
    for (int i = start + lag; i < end; i++) {
        sum += (data[i] - mean) * (data[i - lag] - mean);
    }
    float result = sum / data_len;
    // printf("Sample auto covariance for lag %d: %f; Sum: %f, data length: %d\n", lag, result, sum, data_len);
    return result;
}


float *estimate_breathing_rate_windowed(CSIData *csi_data, int window_size, int step_size, int sampling_rate){
    printf("Estimating breathing rate using windowed method...\n");
    int time_horizon = csi_data -> num_rows;
    int num_windows = (time_horizon - window_size) / step_size + 1;
    int num_subcarriers = csi_data -> num_subcarriers;

    float *estimated_bpm = malloc(num_windows * sizeof(float));
    if (!estimated_bpm){
        return NULL;
    }
    for (int i = 0; i < num_windows; i++){
        estimated_bpm[i] = 1.0;
    }

    float csi_combined_signal[time_horizon];
    for (int i = 0; i < time_horizon; i++){
        csi_combined_signal[i] = apply_mrc_across_subcarriers(csi_data -> amplitude[i], num_subcarriers);
    }

    int typical_bpm_min = 10;
    int typical_bpm_max = 20;
    int typical_period_min_sec = 60 / typical_bpm_max;
    int typical_period_max_sec = 60 / typical_bpm_min;
    int typical_period_min_samples = typical_period_min_sec * sampling_rate;
    int typical_period_max_samples = typical_period_max_sec * sampling_rate;


    int min_signal_length_for_acf = sampling_rate * 2;

    for (int start_index = 0; start_index < num_windows; start_index++){
        int start = start_index * step_size;
        int end = start + window_size;
        if (end > time_horizon){
            break;
        }
        float acf[window_size];
        for (int lag = 0; lag < window_size; lag++){
            acf[lag] = sample_auto_covariance(csi_combined_signal, start, end, lag);
        }
        // printf("ACF values for window %d:\n", start_index);
        float max_acf = 0.0;
        int max_lag = 0;
        for (int lag = typical_period_min_samples; lag < typical_period_max_samples; lag++){
            if (acf[lag] > max_acf){
                max_acf = acf[lag];
                max_lag = lag;
            }
            // printf("Lag %d: %f\n", lag, acf[lag]);
        }
        
        float bpm = (float)sampling_rate / max_lag * 60.0;
        
        if (bpm < typical_bpm_min || bpm > typical_bpm_max){
            estimated_bpm[start_index] = 0.0;
        } else {
            estimated_bpm[start_index] = bpm;
        }
    }

    return estimated_bpm;
}

// Calculate the heartbeat rate according to acf


int main(){
    CSIData csi = load_csi_data("BRtest.csv");
    printf("Read CSI data with %d rows and %d subcarriers\n", csi.num_rows, csi.num_subcarriers);

    int sampling_rate = 100;
    float window_duration_sec = 10;
    float step_duration_sec = 0.2;

    int window_size = (int)(window_duration_sec * sampling_rate);
    int step_size = (int)(step_duration_sec * sampling_rate);

    int time_horizon = csi.num_rows;
    int num_windows = (time_horizon - window_size) / step_size + 1;

    float *estimated_bpm = estimate_breathing_rate_windowed(&csi, window_size, step_size, sampling_rate);

    printf("Window size: %d, Step size: %d, Number of windows: %d\n", window_size, step_size, num_windows);
    
    printf("Estimated breathing rate:\n");
    for (int i = 0; i < num_windows; i++){
        printf("Window %d: %f\n", i, estimated_bpm[i]);
    }

    return 0;
}