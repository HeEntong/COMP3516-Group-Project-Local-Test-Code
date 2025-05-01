#ifndef BRR_H
#define BRR_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>



#define MAX_ROWS 2000
#define MAX_SUBCARRIERS 120
#define MAX_DATA_LEN 1024



typedef struct {
    int num_subcarriers;
    float amplitude[MAX_ROWS][MAX_SUBCARRIERS];
    int num_rows;
} CSIData;

// Public API
CSIData load_csi_data(const char *file_path);

float apply_mrc_across_subcarriers(float* subcarriers_amplitudes_at_a_time, int num_of_subcarriers);

float sample_auto_covariance(float *data, int start, int end, int lag);

float* estimate_breathing_rate_windowed(CSIData *csi_data, int window_size, int step_size, int sampling_rate);


#endif