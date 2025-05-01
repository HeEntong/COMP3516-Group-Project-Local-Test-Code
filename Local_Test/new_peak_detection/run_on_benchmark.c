#include <stdio.h>
#include <stdlib.h>
#define NUM_FRAMES 200
#define MAX_PEAKS 30
#define NUM_SUBCARRIERS 117
#define TIME_HORIZON 1792

float MRC_ACF[NUM_FRAMES];
float csi_matrix[NUM_FRAMES][NUM_SUBCARRIERS];
float csi_matrix_t[NUM_SUBCARRIERS][NUM_FRAMES];
float acf[NUM_SUBCARRIERS][NUM_FRAMES]; // numerate all possible lags in [0, NUM_FRAMES-1] to calculate acf
int peak_indices[MAX_PEAKS];
int amp[TIME_HORIZON][NUM_SUBCARRIERS];

#define TYPICAL_BPM_MAX 25
#define TYPICAL_BPM_MIN 8
#define MAX_DATA_LEN 2048
#define SAMPLING_RATE 25

void print_1d_array(float *data, int data_len){
    printf("[");
    for (int i = 0; i < data_len; i++){
        printf("%f,", data[i]);
    }
    printf("]\n");
}

float flt_abs(float a){
    return a > 0 ? a : -a;
}


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

float get_acf(float *data, int data_len, int lag){
    float mean = 0.0;
    float sum = 0.0;
    for (int i = 0; i < data_len; i++){
        mean += data[i];
    }
    mean /= data_len;

    for (int j = lag; j < data_len; j++){
        sum += (data[j - lag] - mean) * (data[j] - mean);
    }
    return sum;
}

int find_first_peak(const float *ACF, float min_height, int min_width, float prominence, int sampling_rate, int window_length)
{
  int tau_min = (int)((60) * sampling_rate / TYPICAL_BPM_MAX);
  int tau_max = (int)((60) * sampling_rate / TYPICAL_BPM_MIN);
  printf("MIN: %d, MAX: %d\n", tau_min, tau_max);
  // Unlikely to happen, added for safety.
  if (tau_min < 0)
  {
    tau_min = 0;
  }
  if (tau_min >= NUM_FRAMES)
  {
    tau_min = NUM_FRAMES - 1;
  }
  if (tau_max < 0)
  {
    tau_max = 0;
  }
  if (tau_max > NUM_FRAMES)
  {
    tau_max = NUM_FRAMES - 1;
  }
  // The padded data yields n + (n - 1), while the tau takes from 0 to n - 1 (n in total).

  const float *data_to_use = ACF;
  float smoothed_ACF[NUM_FRAMES];

  // windowed linear smoothing
  if (window_length > 1)
  {
    int effective_window = window_length;
    if (effective_window % 2 == 0)
    {
      effective_window += 1;
    }
    int half_window = (int)((effective_window - 1) / 2);
    for (int i = 0; i < NUM_FRAMES; i++)
    {
      int start = i - half_window;
      if (start < 0)
      {
        start = 0;
      }
      int end = i + half_window;
      if (end >= NUM_FRAMES)
        end = NUM_FRAMES - 1;
      float sum = 0.0;
      int count = 0;
      for (int j = start; j <= end; j++)
      {
        sum += ACF[j];
        count++;
      }
      smoothed_ACF[i] = (count > 0) ? (sum / count) : ACF[i];
    }
    data_to_use = smoothed_ACF;
  }

  for (int i = tau_min; i <= tau_max; i++)
  {
    int is_peak = 0;
    if (tau_min == tau_max)
    {
      is_peak = 1;
    }
    // else if (i == tau_min)
    // {
    //   if (ACF[i] > ACF[i + 1])
    //   {
    //     is_peak = 1;
    //   }
    // }
    // else if (i == tau_max)
    // {
    //   if (ACF[i] > ACF[i - 1])
    //   {
    //     is_peak = 1;
    //   }
    // }
    else
    {
      if (data_to_use[i] > data_to_use[i - 1] && data_to_use[i] > data_to_use[i + 1])
      { // Is locally a tentative peak?
        is_peak = 1;
      }
    }
    if (!is_peak)
    {
      continue;
    }
    if (data_to_use[i] < min_height)
    {
      continue;
    }
    if (min_width > 0)
    {
      int half_width = (int)((min_width - 1) / 2);
      int start = i - half_width;
      int end = i + half_width;
      start = (start < tau_min) ? tau_min : start;
      end = (end > tau_max) ? tau_max : end;

      int is_max_in_window = 1;
      for (int j = start; j <= end; j++)
      {
        if (data_to_use[j] > data_to_use[i])
        {
          is_max_in_window = 0;
          break;
        }
      }
      if (!is_max_in_window)
      {
        continue;
      }
    }

    float left_min = data_to_use[i];
    if (i > tau_min)
    {
      for (int j = tau_min; j < i; j++)
      {
        if (data_to_use[j] < left_min)
        {
          left_min = data_to_use[j];
        }
      }
    }

    float right_min = data_to_use[i];
    if (i < tau_max)
    {
      for (int j = i + 1; j <= tau_max; j++)
      {
        if (data_to_use[j] < right_min)
        {
          right_min = data_to_use[j];
        }
      }
    }

    float prominence_val;
    if (i == tau_min)
    {
      prominence_val = data_to_use[i] - right_min;
    }
    else if (i == tau_max)
    {
      prominence_val = data_to_use[i] - left_min;
    }
    else
    {
      prominence_val = data_to_use[i] - (left_min > right_min ? left_min : right_min);
    }
    if (prominence_val < prominence)
    {
      continue;
    }
    return i;
  }
  return -1; // no detection
}



int main(){
    char line[MAX_DATA_LEN];
    int step_size = 15;
    int num_windows = (int)((TIME_HORIZON - NUM_FRAMES) / step_size);
    printf("Number of windows: %d\n", num_windows);
    float *datapath = "test_data_downsampled.txt";
    int T = 0;
    float prev_bpm = 0.0;

    FILE *file = fopen(datapath, "r");
        // printf("%d\n", num_windows);
        if (!file) {
            printf("Error opening file\n");
            return 1;
        }

    while (fgets(line, sizeof(line), file)){
        // csi.rows[row].len = len;
        int sc = 0;
        char *token = strtok(line, ",");
            while (token != NULL) {
                // printf("T: %d; start_index: %d, T - start_index: %d\n", T, start_index, T - start_index);
                amp[T][sc] = atof(token);
                sc++;
                token = strtok(NULL, ",");
            }
        T++;
    }

    fclose(file);

    FILE *output_file = fopen("bpms_193124.txt", "w");
    if (!output_file) {
        printf("Error opening output file\n");
        return 1;
    }

    for (int win_num = 0; win_num < num_windows; win_num++){
        int start_index = win_num * step_size;
        int end_index = win_num * step_size + NUM_FRAMES;

        for (int t = start_index; t < end_index; t++){
            for (int sc = 0; sc < NUM_SUBCARRIERS; sc++){
                csi_matrix[t - start_index][sc] = amp[t][sc];
            }
        }
        

        for (int t = 0; t < NUM_FRAMES; t++){
            for (int sc = 0; sc < NUM_SUBCARRIERS; sc++){
                csi_matrix_t[sc][t] = csi_matrix[t][sc];
            }
        }

        for (int sc = 0; sc < NUM_SUBCARRIERS; sc++){
            for (int lag = 0; lag < NUM_FRAMES; lag++){
                acf[sc][lag] = get_acf(csi_matrix_t[sc], NUM_FRAMES, lag);
                if (acf[sc][0] != 0.0){
                    if (lag != 0){
                        acf[sc][lag] = acf[sc][lag] / acf[sc][0];
                    }
                }
            }
            if (acf[sc][0] != 0.0){
                acf[sc][0] = 1.0;
            }

            // print_1d_array(acf[sc], NUM_FRAMES);
        }


        float weight = 0.0;
        for (int sc = 0; sc < NUM_SUBCARRIERS; sc++){
            weight += (acf[sc][1] > 0 ? acf[sc][1] : 0.0);
        }

        // for (int sc = 0; sc < NUM_SUBCARRIERS; sc++){
        //     for (int lag = 0; lag < NUM_FRAMES; lag++){
        //         printf("%f, ", acf[sc][lag]);
        //     }
        //     printf("\n");
        // }

        // printf("Weight: %f\n", weight);
        
        for (int lag = 0; lag < NUM_FRAMES; lag++){
            float combined_acf = 0.0;
            for (int sc = 0; sc < NUM_SUBCARRIERS; sc++){
                if (acf[sc][1] > 0){
                    combined_acf += (acf[sc][lag] * acf[sc][1] * 1.0) / weight;
                }
            }
            MRC_ACF[lag] = combined_acf;
        }


        int first_peak_idx = find_first_peak(MRC_ACF, 0.02, 2, 0.1, SAMPLING_RATE, 12);
        printf("Window %d summary: \n", win_num);
        print_1d_array(MRC_ACF, NUM_FRAMES);
        if (first_peak_idx != -1){
            float bpm = (float)(60 * SAMPLING_RATE / (first_peak_idx * 1.0));
            printf("peak index: %d\n", first_peak_idx);
            printf("BPM estimation: %f\n", bpm);
            fprintf(output_file, "%f\n", bpm);
            prev_bpm = bpm;
        }
        else{
            printf("No BPM detected\n");
            fprintf(output_file, "%f\n", prev_bpm);
        }

        
       
    }
    fclose(output_file);
}