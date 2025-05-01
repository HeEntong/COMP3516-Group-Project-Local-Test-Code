#include "BRR.h"

#define MAX_PEAKS 20
#define WING_LENGTH 2
#define MAX_DATA_LEN 2048
#define TYPICAL_BPM_MIN 10
#define TYPICAL_BPM_MAX 20
#define NUM_FRAMES 2000
#define NUM_SUBCARRIERS 117

int16_t csi_matrix[NUM_FRAMES][NUM_SUBCARRIERS];
float mrc_matrix[NUM_FRAMES]; // calculated mrc from csi in buffer, G(t, f) = |H(t, f)|^2; mrc(t) = \sum_f G(t, f)

// void peak_detection(float *data, int data_len, int wing_length, int *peak_index, float *prominence){
//     // float max_prominence = 0;
//     int peak_count = 0;
//     int peak_found = 0;
//     float peak_value = 0;
//     int peak_value_index = 0;
//     float prom = 0.0;
//     for (int i = 1; i < data_len - 1; i++){
//         if (data[i] > data[i - 1] && data[i] > data[i + 1]){
//             peak_found = 1;
//             peak_value = data[i];
//             peak_value_index = i;
//             for (int j = i - wing_length; j <= i + wing_length; j++){
//                 if (j < 0 || j >= data_len){
//                     continue;
//                 }
//                 if (data[j] > peak_value){
//                     peak_found = 0;
//                     break;
//                 }
//             }
//             // printf("Peak value: %f, Peak index: %d\n", peak_value, peak_value_index);
//             if (peak_found){
//                 prom = peak_value - data[peak_value_index - wing_length] + peak_value - data[peak_value_index + wing_length];
//                 // printf("Peak value: %f, Peak index: %d, Prominence: %f\n", peak_value, peak_value_index, prom);
//                 // if (prominence > max_prominence){
//                 // max_prominence = prominence;
//                 peak_index[peak_count] = peak_value_index;
//                 peak_count++;
//                 prominence[peak_count] = prom;
//                 // }
//             }
//         }
//     }

//     FILE *output_file = fopen("peak_indices.txt", "w");
//     if (!output_file) {
//         printf("Error opening output file\n");
//         return 1;
//     }
//     for (int i = 0; i < peak_count; i++){
//         fprintf(output_file, "%d\n", peak_index[i]);
//     }
//     fclose(output_file);
//     printf("Peak indices written to peak_indices.txt\n");
// }

float sample_auto_covariance(float *data, int start, int end, int lag) {
    int data_len = end - start;
    if (data_len <= lag){
        return 0;
    }
    float mean = 0.0;
    for (int i = start; i < end; i++){
        mean += data[i];
    }
    mean /= data_len;
    float acf = 0.0;
    for (int i = start; i < end - lag; i++){
        acf += (data[i] - mean) * (data[i + lag] - mean);
    }
    return acf;
}

// To Laoyu: call find_first_peak(csi_matrix, NUM_FRAMES, WING_LENGTH, sampling_rate) to get first peak index, and use (float)sampling_rate / first_peak * 60.0 to get bpm. I think esp32 can only survive lower sampling rate.

int find_first_peak(int wing_length, int sampling_rate){
    int typical_period_min_samples = (int)((60 / TYPICAL_BPM_MAX) * sampling_rate);
    int typical_period_max_samples = (int)((60 / TYPICAL_BPM_MIN) * sampling_rate);
    // printf("Typical period min samples: %d, Typical period max samples: %d\n", typical_period_min_samples, typical_period_max_samples);
    int peak_found = 0;

    float acf[typical_period_max_samples - typical_period_min_samples];
    for (int lag = typical_period_min_samples; lag <= typical_period_max_samples; lag++){
        acf[lag - typical_period_min_samples] = sample_auto_covariance(csi_matrix, 0, NUM_FRAMES, lag) / sample_auto_covariance(csi_matrix, 0, NUM_FRAMES, 0);
    }
    int data_len = typical_period_max_samples - typical_period_min_samples + 1;


    for (int i = 0; i <= data_len; i++){
        if (acf[i] > acf[i - 1] && acf[i] > acf[i + 1]){
            peak_found = 1;
            for (int j = i - wing_length; j <= i + wing_length; j++){
                if (j < 0 || j >= data_len){
                    continue;
                }
                if (acf[j] > acf[i]){
                    peak_found = 0;
                    break;
                }
            }
            if (peak_found){
                return i; // first peak suffices.
            }
        }
    }
    return -1;
}

int main(){
    float *datapath = "acf_data.txt";
    FILE *file = fopen(datapath, "r");
    if (!file) {
        printf("Error opening file\n");
        return 1;
    }
    float data[MAX_DATA_LEN];
    int data_len = 0;
    while (fscanf(file, "%f", &data[data_len]) != EOF && data_len < MAX_DATA_LEN) {
        data_len++;
    }
    fclose(file);
    // for (int i = 0; i < data_len; i++){
    //     printf("%f\n", data[i]);
    // }
    int peak_index[MAX_PEAKS];
    float prominence[MAX_PEAKS];
    peak_detection(data, data_len, WING_LENGTH, peak_index, prominence);
    // for (int i = 0; i < peak_count; i++){
    //     printf("Peak index: %d, data val: %f\n, prominence: %f\n", peak_index[i], (float)(data[peak_index[i]]), prominence[i]);
    // }

    int first_peak = find_first_peak(data, data_len, WING_LENGTH, 100);
    if (first_peak != -1){
        printf("First peak found at index: %d, data value: %f\n", first_peak, data[first_peak]);
        float bpm = (float)100 / first_peak * 60.0;
        printf("Estimated BPM: %f\n", bpm);
    } else {
        printf("No peak found in the specified range\n");
    }
}