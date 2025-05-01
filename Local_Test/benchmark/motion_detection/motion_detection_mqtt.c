
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>


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

#define NUM_SUBCARRIERS 117
#define NUM_FRAMES 100

float csi_matrix[NUM_FRAMES][NUM_SUBCARRIERS]; // esp32 buffer

int motion = 0;


int motion_detection()
{
  // if (filled_frames < NUM_FRAMES)
  // {
  //   return false; // Not enough data to determine motion
  // }
  float sum_acf_lag1 = 0.0; // Sum of normalized ACF values at lag 1

  // Iterate over each subcarrier
  for (int subcarrier = 0; subcarrier < NUM_SUBCARRIERS; subcarrier++)
  {
    float sum_lag0 = 0.0; // ACF at lag 0
    float sum_lag1 = 0.0; // ACF at lag 1

    // Compute ACF for lag 0
    for (int t = 0; t < NUM_FRAMES; t++)
    {
      sum_lag0 += csi_matrix[t][subcarrier] * csi_matrix[t][subcarrier];
    }

    // Compute ACF for lag 1
    for (int t = 0; t < NUM_FRAMES - 1; t++)
    {
      sum_lag1 += csi_matrix[t][subcarrier] * csi_matrix[t + 1][subcarrier];
    }

    // Normalize ACF values by dividing by lag 0 (max value)
    float normalized_lag1 = 0.0;
    if (sum_lag0 > 0)
    { // Avoid division by zero
      normalized_lag1 = sum_lag1 / sum_lag0;
    }

    // Accumulate the normalized lag 1 values
    sum_acf_lag1 += normalized_lag1;
  }

  // Compute the average normalized ACF lag 1 across all subcarriers
  float avg_acf_lag1 = sum_acf_lag1 / NUM_SUBCARRIERS;
  printf("Average ACF Lag 1: %f\n", avg_acf_lag1);
  // Determine motion based on a threshold
  motion = (avg_acf_lag1 > 0.35) ? 1 : 0; // Threshold for motion detection



  return motion;
}

int main(){
  CSIData csi = load_csi_data("test.csv");
  if (csi.num_rows > 0) {
      printf("Loaded %d rows of CSI data\n", csi.num_rows);
  } else {
      printf("Failed to load CSI data\n");
  }

  int window_size = NUM_FRAMES;
  int step_size = 50;

  

  int num_window = (csi.num_rows - window_size) / step_size + 1;

  for (int win_num = 0; win_num < num_window; win_num++) {
      int start_idx = win_num * step_size;
      int end_idx = start_idx + window_size;

      for (int t = 0; t < NUM_FRAMES; t++){
          for (int subcarrier = 0; subcarrier < NUM_SUBCARRIERS; subcarrier++){
              csi_matrix[t][subcarrier] = csi.rows[start_idx + t].amplitude[subcarrier];
          }
      }

      int win_result = motion_detection();
      printf("Window %d: Motion detected: %d\n", win_num, win_result);
  }
}