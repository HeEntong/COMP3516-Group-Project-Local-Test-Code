from breathrate_retrieval import *

if __name__ == "__main__":
    # csi_file_path = "benchmark/breathing_rate/evaluation/CSI20250227_191018.csv"
    csi_file_path = "./evaluation/CSI20250227_193124.csv" 

    output_plots_directory = "./window_plots_results_2" 

    output_results_directory = "./results" 


    csi_data_df = load_csi_data(csi_file_path)

    if csi_data_df is not None and not csi_data_df.empty:
        print(f"Loaded {len(csi_data_df)} rows of CSI data.")

        # --- NEW CHECK: Check number of subcarriers per timestamp in original data ---
        # print("\nChecking number of subcarriers per timestamp in original csi_df:")
        # # Apply the extraction function to each 'data' string
        # def count_values_in_data_string(data_string):
        #     if not isinstance(data_string, str):
        #         return 0 # Return 0 for non-string entries
        #     try:
        #         # Remove brackets and split by comma
        #         values_list = data_string.strip('[]').split(',')
        #         # Filter out empty strings that might result from splitting, e.g., '[,1,2,].split(',') -> ['', '1', '2', '']
        #         valid_values = [v for v in values_list if v.strip()]
        #         return len(valid_values)
        #     except Exception as e:
        #         print(f"Warning: Error processing data string '{data_string[:50]}...': {e}")
        #         return 0 # Return 0 in case of parsing error

        # # Apply the function to the 'data' column
        # value_counts_per_timestamp = csi_data_df['data'].apply(count_values_in_data_string)

        # # Print the statistics of the counts
        # if not value_counts_per_timestamp.empty:
        #     print("Count of each numerical value count found:")
        #     print(value_counts_per_timestamp.value_counts().sort_index()) # Sort for better readability
        # else:
        #     print("Could not process any 'data' strings.")

        # RESULTS: data
            # 106       1
            # 234    7164
            # 256       2
        # print("--- END NEW CHECK ---")


        sampling_rate = 100 # Hz
        window_duration_sec = 15 # s
        step_duration_sec = 1 # s

        estimated_bpms_with_timestamps = estimate_breathing_rate_windowed(
            csi_data_df.copy(),
            sampling_rate=sampling_rate,
            window_duration_sec=window_duration_sec,
            step_duration_sec=step_duration_sec,
            output_plots_dir=output_plots_directory 
        )

        if estimated_bpms_with_timestamps:
            print(f"Estimated {len(estimated_bpms_with_timestamps)} BPM values using sliding windows.")
            print(f"Plots for each window saved to {output_plots_directory}")

            save_estimated_bpms_to_csv(estimated_bpms_with_timestamps, output_dir=output_results_directory)
            
        else:
            print("No BPM values were estimated from the CSI data.")

        ground_truth = './evaluation/gt_20250227_193124.csv'
        ground_truth_data = pd.read_csv(ground_truth)
        
        
    else:
        print("Failed to load CSI data or data is empty.")

