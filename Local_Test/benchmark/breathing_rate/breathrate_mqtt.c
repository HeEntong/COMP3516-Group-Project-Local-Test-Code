#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
// #include "fftw3.h"

#define MAX_ROWS 2000
#define MAX_SUBCARRIERS 120
#define MAX_DATA_LEN 1024
#define NUM_FRAMES 100 // windows size?
#define NUM_SUBCARRIERS 117

typedef struct {
    int num_subcarriers;
    float amplitude[MAX_ROWS][MAX_SUBCARRIERS];
    int num_rows;
} CSIData;

float csi_matrix[NUM_FRAMES][NUM_SUBCARRIERS]; // esp32 buffer
float csi_matrix_t[NUM_SUBCARRIERS][NUM_FRAMES]; // flipped esp32 buffer
static const char *TAG = "csi_recv";
static int tau = 0;
// Public API
CSIData load_csi_data(const char *file_path);
static float *global_acf = NULL;
static float *global_avg_acf = NULL;

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
              csi.amplitude[row][col / 2] = ( real_col * real_col + imaginary_col * imaginary_col);
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

// void compute_acf_full_fft(int16_t *signal, int length, float *acf)
// {
//     // Allocate memory for FFT input and output
//     float *fft_input = (float *)malloc(length * sizeof(float));
//     fftwf_complex *fft_output = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * (length / 2 + 1));
//     float *power_spectrum = (float *)malloc(length * sizeof(float));

//     if (!fft_input || !fft_output || !power_spectrum)
//     {
//         // ESP_LOGE(TAG, "Failed to allocate memory for FFT");
//         free(fft_input);
//         free(fft_output);
//         free(power_spectrum);
//         return;
//     }

    
//     // Copy the signal into the FFT input array
//     for (int i = 0; i < length; i++)
//     {
//         fft_input[i] = (float)signal[i];
//     }

//   // Perform FFT
//     //   dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
//     fftwf_plan plan = fftwf_plan_dft_r2c_1d(length, fft_input, fft_output, FFTW_ESTIMATE);
//     fftwf_execute(plan);
    
//     // Compute the power spectrum
//     for (int i = 0; i < length; i++)
//     {
//         float real = fft_input[2 * i];
//         float imag = fft_input[2 * i + 1];
//         power_spectrum[i] = real * real + imag * imag;
//     }

//     // Perform inverse FFT on the power spectrum
//     fftwf_plan inverse_plan = fftwf_plan_dft_c2r_1d(length, fft_output, fft_input, FFTW_ESTIMATE);
//     fftwf_execute(inverse_plan);

//     // Normalize the ACF
//     for (int i = 0; i < length; i++)
//     {
//         acf[i] = power_spectrum[i] / length;
//     }

//     // Free allocated memory
//     free(fft_input);
//     free(fft_output);
//     free(power_spectrum);
// }

void compute_acf_traditional(float *signal, int length, float *acf){
  // Compute the ACF using the traditional method
  for (int tau = 0; tau < NUM_FRAMES; tau++)
  {
      acf[tau] = 0.0;
      for (int t = 0; t < NUM_FRAMES; t++)
      {
          acf[tau] += (float)(signal[t] * signal[t + tau]);
      }
      acf[tau] /= NUM_FRAMES; // Normalize by the number of samples
      printf("ACF[%d]: %f\n", tau, acf[tau]);
  }
}


// Function to detect the first peak in the ACF with a minimum height
int detect_first_peak(float *acf, int length, float min_height, int min_width, float min_prominence)
{
  // Apply a simple smoothing filter (moving average) to reduce noise
  float *smoothed_acf = (float *)malloc(length * sizeof(float));
  int window_size = 5; // Smoothing window size (adjust as needed)
  for (int i = 0; i < length; i++)
  {
    float sum = 0.0;
    int count = 0;
    for (int j = -window_size / 2; j <= window_size / 2; j++)
    {
      int idx = i + j;
      if (idx >= 0 && idx < length)
      {
        sum += acf[idx];
        count++;
      }
    }
    smoothed_acf[i] = sum / count;
  }

  // Detect peaks with additional constraints
  int first_peak_tau = 1; // Default to 1 to avoid zero lag
  for (int tau = 1; tau < length - 1; tau++)
  {
    // Check if the current value is a peak
    if (smoothed_acf[tau] > smoothed_acf[tau - 1] &&
        smoothed_acf[tau] > smoothed_acf[tau + 1] &&
        smoothed_acf[tau] > min_height &&
        tau > 200 && tau < 600)
    { // Range constraint

      // Calculate prominence of the peak
      float left_max = 0.0, right_max = 0.0;
      for (int i = tau - 1; i >= 0; i--)
      {
        if (smoothed_acf[i] > left_max)
        {
          left_max = smoothed_acf[i];
        }
      }
      for (int i = tau + 1; i < length; i++)
      {
        if (smoothed_acf[i] > right_max)
        {
          right_max = smoothed_acf[i];
        }
      }
      float prominence = smoothed_acf[tau] - fmax(left_max, right_max);

      // Check prominence and width constraints
      if (prominence >= min_prominence)
      {
        // Verify peak width by ensuring neighboring values are below the peak
        int width = 2;
        while (tau - width >= 0 && smoothed_acf[tau - width] < smoothed_acf[tau])
          width++;
        while (tau + width < length && smoothed_acf[tau + width] < smoothed_acf[tau])
          width++;
        if (width >= min_width)
        {
          first_peak_tau = tau;
          break;
        }
      }
    }
  }

  // Free allocated memory
  free(smoothed_acf);

  printf("First peak detected at tau: %d\n", first_peak_tau);
  return first_peak_tau;
}

// Breathing rate estimation function
int breathing_rate_estimation()
{
//   if (filled_frames < NUM_FRAMES)
//   {
//     ESP_LOGW(TAG, "Not enough frames to estimate breathing rate");
//     tau = -1; // Indicate no valid lag
//     return 0; // Not enough data to estimate breathing rate
//   }

  if (!global_acf || !global_avg_acf)
  {
    // ESP_LOGE(TAG, "Global ACF memory not initialized");
    tau = -1; // Indicate no valid lag
    return 0;
  }

  // Reset the average ACF
  memset(global_avg_acf, 0, NUM_FRAMES * sizeof(float));

  // Compute weights based on positive lag 1 values
  float weights[NUM_SUBCARRIERS] = {0};
  float weight_sum = 0.0;
  
  for (int subcarrier = 0; subcarrier < NUM_SUBCARRIERS; subcarrier++)
  {
    compute_acf_traditional(csi_matrix_t[subcarrier], NUM_FRAMES, global_acf);


    // Use the lag 1 value as the weight if it's positive
    float lag1_value = global_acf[1];
    if (lag1_value > 0)
    {
      weights[subcarrier] = lag1_value;
      weight_sum += lag1_value;
    }
    else
    {
      weights[subcarrier] = 0; // Ignore negative or zero lag 1 values
    }
  }

  // Normalize weights to sum to 1
  if (weight_sum > 0)
  {
    for (int subcarrier = 0; subcarrier < NUM_SUBCARRIERS; subcarrier++)
    {
      weights[subcarrier] /= weight_sum;
    }
  }
  else
  {
    // ESP_LOGW(TAG, "All lag 1 values are non-positive, cannot compute weights");
    tau = -1;
    return 0;
  }

  // Combine subcarriers using the weights
  for (int subcarrier = 0; subcarrier < NUM_SUBCARRIERS; subcarrier++)
  {
    if (weights[subcarrier] > 0)
    { // Only include valid subcarriers
      compute_acf_traditional(csi_matrix_t[subcarrier], NUM_FRAMES, global_acf);

      // Add the weighted ACF to the combined signal
      for (int tau = 0; tau < NUM_FRAMES; tau++)
      {
        global_avg_acf[tau] += weights[subcarrier] * global_acf[tau];
      }

      printf("Subcarrier %d ACF: ", subcarrier);
    }
  }

  // Detect the first peak in the combined signal
  float min_height = 0.1; // Minimum height threshold for a valid peak
  int min_width = 3;      // Minimum width of the peak
  float min_prominence = 0.06; // Minimum prominence of the peak
  int first_peak_tau = detect_first_peak(global_avg_acf, NUM_FRAMES, min_height, min_width, min_prominence);
  if (first_peak_tau == -1)
  {
    // ESP_LOGW(TAG, "No significant peak detected in combined ACF");
    tau = -1;
    return 0; // No breathing rate detected
  }

  // Convert the lag to breathing rate (in breaths per minute)
  float sampling_rate = 100.0; // Assuming 100 Hz sampling rate
  float breathing_rate = (float)((60.0 * sampling_rate) / first_peak_tau);

  tau = first_peak_tau; // Store the lag for further analysis

  return breathing_rate;
}

int main(){
    CSIData csi_data = load_csi_data("./BRtestsmall.csv");
    if (csi_data.num_rows > 0) {
        printf("Loaded %d rows of CSI data\n", csi_data.num_rows);
    } else {
        printf("Failed to load CSI data\n");
    }
    int window_size = NUM_FRAMES;
    int step_size = 50;
    int num_window = (csi_data.num_rows - window_size) / step_size + 1;

    printf("Number of windows: %d\n", num_window);
    printf("Window size: %d\n", window_size);
    printf("Step size: %d\n", step_size);

    for (int win_num = 0; win_num < num_window; win_num++) {
        int start_idx = win_num * step_size;
        int end_idx = start_idx + window_size;
  
        for (int t = 0; t < NUM_FRAMES; t++){
            for (int subcarrier = 0; subcarrier < NUM_SUBCARRIERS; subcarrier++){
                csi_matrix[t][subcarrier] = csi_data.amplitude[t][subcarrier];
                csi_matrix_t[subcarrier][t] = csi_data.amplitude[t][subcarrier];
            }
        }
  
        int bpm = breathing_rate_estimation();
        printf("Breathing rate estimation for window %d: %d bpm\n", win_num, bpm);
    }
}