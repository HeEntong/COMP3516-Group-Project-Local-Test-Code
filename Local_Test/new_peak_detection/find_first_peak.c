#include <stdio.h>
#include <stdlib.h>
#define NUM_FRAMES 250
#define MAX_PEAKS 30

float MRC_ACF[NUM_FRAMES];
int peak_indices[MAX_PEAKS];

#define TYPICAL_BPM_MAX 30
#define TYPICAL_BPM_MIN 8
#define MAX_DATA_LEN 512
#define SAMPLING_RATE 25


int find_peaks(const float *ACF, float min_height, int min_width, float prominence, int sampling_rate, int window_length){
    int tau_min = (int)((60) * sampling_rate / TYPICAL_BPM_MAX);
    int tau_max = (int)((60) * sampling_rate / TYPICAL_BPM_MIN);
    printf("tau_min %d, tau_max %d\n", tau_min, tau_max);
    int peak_count = 0;
    // Unlikely to happen, added for safety.
    if (tau_min < 0){
        tau_min = 0;
    }
    if (tau_min >= NUM_FRAMES){
        tau_min = NUM_FRAMES - 1;
    }
    if (tau_max < 0){
        tau_max = 0;
    }
    if (tau_max > NUM_FRAMES){
        tau_max = NUM_FRAMES - 1;
    }
    // The padded data yields n + (n - 1), while the tau takes from 0 to n - 1 (n in total).
    
    const float *data_to_use = ACF;
    float smoothed_ACF[NUM_FRAMES];

    // windowed linear smoothing
    if (window_length > 1){
        int effective_window = window_length;
        if (effective_window % 2 == 0){
            effective_window += 1;
        }
        int half_window = (int)((effective_window - 1) / 2);
        for (int i = 0; i < NUM_FRAMES; i++){
            int start = i - half_window;
            if (start < 0){
                start = 0;
            }
            int end = i + half_window;
            if (end >= NUM_FRAMES) end = NUM_FRAMES - 1;
            float sum = 0.0;
            int count = 0;
            for (int j = start; j <= end; j++){
                sum += ACF[j];
                count++;
            }
            smoothed_ACF[i] = (count > 0) ? (sum / count) : ACF[i];
        }
        data_to_use = smoothed_ACF;
        
        // FILE *output_file = fopen("smoothed_data.txt", "w");
        // if (!output_file) {
        //     printf("Error opening output file\n");
        //     return 1;
        // }
        // for (int i = 0; i < NUM_FRAMES; i++){
        //     fprintf(output_file, "%f\n", smoothed_ACF[i]);
        // }
        // fclose(output_file);
    }

    for (int i = tau_min; i <= tau_max; i++){
        int is_peak = 0;
        if (tau_min == tau_max){
            is_peak = 1;
        } else if (i == tau_min){
            if (ACF[i] > ACF[i + 1]){
                is_peak = 1;
            }
        } else if (i == tau_max){
            if (ACF[i] > ACF[i - 1]){
                is_peak = 1;
            }
        } 
        else {
            if (ACF[i] > ACF[i - 1] && ACF[i] > ACF[i + 1]){ // Is locally a tentative peak?
                is_peak = 1;
            }
        }
        if (!is_peak){
            continue;
        }
        if (ACF[i] < min_height){
            continue;
        }
        if (min_width > 0){
            int half_width = (int)((min_width - 1) / 2);
            int start = i - half_width;
            int end = i + half_width;
            start = (start < tau_min) ? tau_min : start;
            end = (end > tau_max) ? tau_max : end;

            int is_max_in_window = 1;
            for (int j = start; j <= end; j++){
                if (ACF[j] > ACF[i]){
                    is_max_in_window = 0;
                    break;
                }
            }
            if (!is_max_in_window){
                continue;
            }
        }

        float left_min = ACF[i];
        if (i > tau_min) {
            for (int j = tau_min; j < i; j++){
                if (ACF[j] < left_min) {
                    left_min = ACF[j];
                }
            }
        }

        float right_min = ACF[i];
        if (i < tau_max){
            for (int j = i + 1; j <= tau_max; j++){
                if (ACF[j] < right_min){
                    right_min = ACF[j];
                }
            }
        }

        float prominence_val;
        if (i == tau_min){
            prominence_val = ACF[i] - right_min;
        }
        else if (i == tau_max){
            prominence_val = ACF[i] - left_min;
        }
        else{
            prominence_val = ACF[i] - (left_min > right_min ? left_min : right_min);
        }
        if (prominence_val < prominence){
            continue;
        }
        peak_indices[peak_count] = i;
        peak_count++;
    }
    return peak_count;
}

int find_first_peak(const float *ACF, float min_height, int min_width, float prominence, int sampling_rate, int window_length){
    int tau_min = (int)((60) * sampling_rate / TYPICAL_BPM_MAX);
    int tau_max = (int)((60) * sampling_rate / TYPICAL_BPM_MIN);
    // Unlikely to happen, added for safety.
    if (tau_min < 0){
        tau_min = 0;
    }
    if (tau_min >= NUM_FRAMES){
        tau_min = NUM_FRAMES - 1;
    }
    if (tau_max < 0){
        tau_max = 0;
    }
    if (tau_max > NUM_FRAMES){
        tau_max = NUM_FRAMES - 1;
    }
    // The padded data yields n + (n - 1), while the tau takes from 0 to n - 1 (n in total).
    
    const float *data_to_use = ACF;
    float smoothed_ACF[NUM_FRAMES];

    // windowed linear smoothing
    if (window_length > 1){
        int effective_window = window_length;
        if (effective_window % 2 == 0){
            effective_window += 1;
        }
        int half_window = (int)((effective_window - 1) / 2);
        for (int i = 0; i < NUM_FRAMES; i++){
            int start = i - half_window;
            if (start < 0){
                start = 0;
            }
            int end = i + half_window;
            if (end >= NUM_FRAMES) end = NUM_FRAMES - 1;
            float sum = 0.0;
            int count = 0;
            for (int j = start; j <= end; j++){
                sum += ACF[j];
                count++;
            }
            smoothed_ACF[i] = (count > 0) ? (sum / count) : ACF[i];
        }
        data_to_use = smoothed_ACF;
    }

    for (int i = tau_min; i <= tau_max; i++){
        int is_peak = 0;
        if (tau_min == tau_max){
            is_peak = 1;
        }
        else {
            if (ACF[i] > ACF[i - 1] && ACF[i] > ACF[i + 1]){ // Is locally a tentative peak?
                is_peak = 1;
            }
        }
        if (!is_peak){
            continue;
        }
        if (ACF[i] < min_height){
            continue;
        }
        if (min_width > 0){
            int half_width = (int)((min_width - 1) / 2);
            int start = i - half_width;
            int end = i + half_width;
            start = (start < tau_min) ? tau_min : start;
            end = (end > tau_max) ? tau_max : end;

            int is_max_in_window = 1;
            for (int j = start; j <= end; j++){
                if (ACF[j] > ACF[i]){
                    is_max_in_window = 0;
                    break;
                }
            }
            if (!is_max_in_window){
                continue;
            }
        }

        float left_min = ACF[i];
        if (i > tau_min) {
            for (int j = tau_min; j < i; j++){
                if (ACF[j] < left_min) {
                    left_min = ACF[j];
                }
            }
        }

        float right_min = ACF[i];
        if (i < tau_max){
            for (int j = i + 1; j <= tau_max; j++){
                if (ACF[j] < right_min) {
                    right_min = ACF[j];
                }
            }
        }

        float prominence_val;
        if (i == tau_min){
            prominence_val = ACF[i] - right_min;
        }
        else if (i == tau_max){
            prominence_val = ACF[i] - left_min;
        }
        else{
            prominence_val = ACF[i] - (left_min > right_min ? left_min : right_min);
        }
        if (prominence_val < prominence){
            continue;
        }
        return i;
    }
    return -1; // no detection
}

int main(){
    float *datapath = "acf_data.txt";
    FILE *file = fopen(datapath, "r");
    if (!file) {
        printf("Error opening file\n");
        return 1;
    }
    // float MRC_ACF[MAX_DATA_LEN];
    int data_len = 0;
    while (fscanf(file, "%f", &MRC_ACF[data_len]) != EOF && data_len < MAX_DATA_LEN) {
        data_len++;
    }
    fclose(file);
    for (int i = 0; i < data_len; i++){
        printf("%f \n", MRC_ACF[i]);
    }
    int peak_num = find_peaks(MRC_ACF, 0.1, 3, 0.1, SAMPLING_RATE, 0);
    for (int i = 0; i < peak_num; i++){
        printf("%d\n", peak_indices[i]);
    }
    printf("Peak number: %d\n", peak_num);
    FILE *output_file = fopen("peak_indices.txt", "w");
    if (!output_file) {
        printf("Error opening output file\n");
        return 1;
    }
    for (int i = 0; i < peak_num; i++){
        fprintf(output_file, "%d\n", peak_indices[i]);
    }
    fclose(output_file);
    printf("Peak indices written to peak_indices.txt\n");

    if (peak_num > 0){
        int first_peak_idx = peak_indices[0];
        float bpm = (60 * (SAMPLING_RATE / (float)first_peak_idx));
        printf("Estimated BPM: %f\n", bpm);
    }
}