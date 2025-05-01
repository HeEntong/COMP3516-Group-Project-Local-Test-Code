#ifndef MOTION_DETECTION_H
#define MOTION_DETECTION_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX_ROWS 1000
#define MAX_SUBCARRIERS 200
#define MAX_DATA_LEN 1024

typedef struct {
    int len;
    float amplitude[MAX_SUBCARRIERS];
} CSIRow;

typedef struct {
    CSIRow rows[MAX_ROWS];
    int num_rows;
} CSIData;

// Public API
CSIData load_csi_data(const char *file_path);
void csi_data_parsing(CSIData *csi_data);
float sample_auto_covariance(float *data, int data_len, int lag);
float rho_G(float *data, int data_len, int lag);

float* get_subcarrier(CSIData *csi_data, int subcarrier_idx, int row_idx);


int* motion_detection(CSIData *csi_data, float threshold, int window_size, int step_size);

void print_detection_results(const int* results, int length);

#endif