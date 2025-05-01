#include "motion_detection.h"


CSIData load_csi_data(const char *file_path) {
    CSIData csi = {0};
    FILE *file = fopen(file_path, "r");
    if (!file) {
        printf("Error opening file\n");
        return csi;
    }

    printf("Loading CSI data from %s\n", file_path);
    char line[1024];
    int row = 0;

    int col = 0;
    while (fgets(line, sizeof(line), file) && row < MAX_ROWS) {
        // csi.rows[row].len = len;
        char *token = strtok(line, ",");
        col = 0;
        int real_col = 0;
        int imaginary_col = 0;
        while (token != NULL) {
            if (col % 2 == 0) {
                real_col = atof(token);
            } else {
                imaginary_col = atof(token);
                csi.rows[row].amplitude[col / 2] = real_col * real_col + imaginary_col * imaginary_col;
            }
            token = strtok(NULL, ",");
            col++;
        }
        csi.rows[row].len = col / 2;
        row++;
    }
    csi.num_rows = row;
    fclose(file);
    return csi;
}


float sample_auto_covariance(float *data, int data_len, int lag) {
    if (data_len <= lag){
        return 0;
    }
    long double mean = 0;
    for (int i = 0; i < data_len; i++){
        mean += data[i];
    }
    mean /= data_len;
    long double sum = 0;
    for (int i = lag; i < data_len; i++) {
        sum += (data[i] - mean) * (data[i - lag] - mean);
    }
    return sum / data_len;
}

float rho_G(float *data, int data_len, int lag) {
    long double acf_g = sample_auto_covariance(data, data_len, lag);
    long double acf_0 = sample_auto_covariance(data, data_len, 0);
    if (acf_0 == 0){
        return -1;
    }
    return acf_g / acf_0;
}

int* motion_detection(CSIData *csi_data, float threshold, int window_size, int step_size) {
    int time_horizon = csi_data -> num_rows;
    int num_windows = (time_horizon - window_size) / step_size + 1;
    int *detection_outcome = malloc(num_windows * sizeof(int));
    if (!detection_outcome){
        return NULL;
    }

    int num_subcarriers = csi_data -> rows[0].len;
    printf("Number of subcarriers: %d\n", num_subcarriers);
    printf("Time horizon: %d\n", time_horizon);
    printf("Number of windows: %d\n", num_windows);

    for (int i = 0; i < num_windows; i++){
        int start = i * step_size;
        int end = start + window_size;
        if (end > time_horizon) end = time_horizon;

        float sum_rho = 0;
        int valid_subcarriers = 0;

        for (int j = 0; j < num_subcarriers; j++){
            float window_data[window_size];
            for (int k = start; k < end; k++){
                window_data[k - start] = csi_data -> rows[k].amplitude[j];
            }
            
            float rho = rho_G(window_data, end - start, 1);
            if (rho != -1){
                sum_rho += rho;
                valid_subcarriers++;
            }
        }

        if (valid_subcarriers == 0){
            detection_outcome[i] = 0;
            continue;
        }

        float estimator = sum_rho / num_subcarriers;
        detection_outcome[i] = (estimator > threshold) ? 1 : 0;
    }

    return detection_outcome;
}

int main(){
    CSIData csi = load_csi_data("test.csv");
    int window_size = 100;
    int step_size = 25;
    int *result = motion_detection(&csi, 0.4, window_size, step_size);
    int num_windows = (csi.num_rows - window_size) / step_size + 1;
    for (int i = 0; i < num_windows; i++){
        printf("%d ", result[i]);
    }
    free(result);
    return 0;
}