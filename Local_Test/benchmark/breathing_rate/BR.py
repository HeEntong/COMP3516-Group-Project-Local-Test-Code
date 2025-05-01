# %%
import matplotlib
matplotlib.use('Agg')
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import math 
from datetime import datetime, timedelta 
from scipy.signal import savgol_filter, find_peaks, butter, sosfiltfilt 
import os 

# %%
def load_csi_data(file_path: str) -> pd.DataFrame:
    try:
        df = pd.read_csv(file_path)

        if 'timestamp' not in df.columns or 'data' not in df.columns: # For alignment?
            print("Error: CSV file must contain 'timestamp' and 'data' columns.")
            return None
        df['timestamp'] = pd.to_datetime(df['timestamp'])
        # Convert 'timestamp' to datetime format
        return df
    except FileNotFoundError:
        print(f"Error: File not found at {file_path}")
        return None
    except Exception as e:
        print(f"Error loading or processing CSV file: {e}")
        return None

# %%
def nextpow2(n: int) -> int:
    if n <= 0:
        return 1
    return 2**math.ceil(math.log2(n))

def autocorr(x: np.ndarray) -> np.ndarray:
    """Calculate autocorrelation of a signal using FFT."""
    x = np.asarray(x)
    if x.size == 0:
        return np.array([])
    x = x - np.mean(x)
    nFFT = nextpow2(len(x))
    F = np.fft.fft(x, nFFT)
    F = F * np.conj(F)
    acf = np.fft.ifft(F)
    acf = acf[0:len(x)]
    acf = np.real(acf)
    if acf[0] != 0:
        acf = acf / acf[0]
    else:
        acf = np.zeros_like(acf)
    return acf
    
def extract_and_process_csi_amplitude(csi_data_str):
    # Get G(t, f) = |H(f, t)|^2, accross time and frequency domain.
    """
    Extracts and processes the amplitude of CSI data from the 'data' column string.

    Args:
        csi_data_str (str): A string representation of the CSI data array
                            from the 'data' column (e.g., '[-5,-28, ...]').

    Returns:
        numpy.ndarray: An array of CSI amplitudes for each subcarrier,
                       or an empty array if parsing fails.
    """
    if not isinstance(csi_data_str, str):
        return np.array([])
    try:
        values_str = csi_data_str.strip('[]').strip()
        if not values_str: 
            return np.array([])

        csi_values_str_list = values_str.split(',')
        csi_values = [float(x) for x in csi_values_str_list]

        imag_parts = np.array(csi_values[0::2])
        real_parts = np.array(csi_values[1::2])

        if len(imag_parts) != len(real_parts) or len(imag_parts) == 0:
             print(f"Warning: Mismatch in imaginary and real parts length or empty. Imag: {len(imag_parts)}, Real: {len(real_parts)}") 
             return np.array([])

        amplitudes = np.sqrt(imag_parts**2 + real_parts**2)

        return amplitudes
    except Exception as e:
        print(f"Error parsing CSI data string '{csi_data_str[:50]}...': {e}")
        return np.array([])
    
def extract_amplitudes_all_subcarriers(csi_df: pd.DataFrame) -> pd.DataFrame:
    """
    Extracts CSI amplitudes for all subcarriers over time into a DataFrame.

    Args:
        csi_df (pd.DataFrame): DataFrame containing loaded CSI data with 'timestamp' and 'data'.

    Returns:
        pd.DataFrame: DataFrame with timestamp as index, columns as subcarrier indices,
                      and values as amplitude. Returns empty DataFrame on failure or empty input.
    """
    if csi_df.empty or 'data' not in csi_df.columns or 'timestamp' not in csi_df.columns:
        return pd.DataFrame()

    all_amplitudes = csi_df['data'].apply(extract_and_process_csi_amplitude)

    valid_amplitudes = all_amplitudes[all_amplitudes.apply(lambda x: len(x) > 0)]

    if valid_amplitudes.empty:
        print("No valid amplitude data extracted from CSI DataFrame.")
        return pd.DataFrame()
    try:
        amplitudes_df = pd.DataFrame(valid_amplitudes.tolist(), index=valid_amplitudes.index)
        amplitudes_df.columns = [f'subcarrier_{i}' for i in range(amplitudes_df.shape[1])]
        amplitudes_df['timestamp'] = csi_df.loc[amplitudes_df.index, 'timestamp']
        amplitudes_df = amplitudes_df.set_index('timestamp').sort_index()
        return amplitudes_df
    except Exception as e:
        print(f"Error converting amplitudes to DataFrame: {e}")
        return pd.DataFrame()
    

def apply_mrc_across_subcarriers(amplitudes_array):
    """
    Applies a simple MRC-like combination across subcarriers by summing their power.
    This is used for generating the raw and smoothed time series plots.

    Args:
        amplitudes_array (numpy.ndarray): An array of CSI amplitudes for each subcarrier
                                         at a single time point (from one row of the amplitude DataFrame).

    Returns:
        float: The combined signal value (sum of power), or NaN if input is empty.
    """
    if len(amplitudes_array) == 0:
        return np.nan
    combined_power = np.sum(amplitudes_array**2)
    return combined_power

# %%
def plot_window_data(window_data: dict, sampling_rate: int = 100, output_dir: str = "./window_plots_results_2"):
    """
    Plots the raw and smoothed signal and its ACF for a single window and saves the plots to files.
    Raw and smoothed signals are plotted in separate files.
    Marks the selected peak on the ACF plot if available.
    The filename includes the minutes and seconds part of the window's end timestamp.

    Args:
        window_data (dict): Dictionary containing data for a window.
                            Expected keys: 'window_start_timestamp', 'window_end_timestamp',
                            'estimated_bpm', 'raw_signal', 'smoothed_signal', 'acf_values',
                            'selected_peak_lag_samples' (optional).
        sampling_rate (int): The sampling rate of the original CSI data in Hz.
        output_dir (str): Directory to save the plot files.
    """
    start_ts = window_data['window_start_timestamp']
    end_ts = window_data['window_end_timestamp']
    estimated_bpm = window_data['estimated_bpm']
    raw_signal = window_data['raw_signal']
    smoothed_signal = window_data['smoothed_signal']
    acf_values = window_data['acf_values']
    selected_peak_lag_samples = window_data.get('selected_peak_lag_samples', None)

    # print(f"\n--- Plotting data for window: {start_ts} to {end_ts} ---\nEstimated BPM: {estimated_bpm}")

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    if isinstance(start_ts, np.datetime64) or isinstance(start_ts, pd.Timestamp):
         start_ts_dt = pd.to_datetime(start_ts).to_pydatetime()
    else:
         print(f"Warning: Unexpected type for start_ts: {type(start_ts)}. Attempting to use directly.")
         start_ts_dt = start_ts

    if isinstance(end_ts, np.datetime64) or isinstance(end_ts, pd.Timestamp):
         end_ts_dt = pd.to_datetime(end_ts).to_pydatetime()
         end_min_sec_str = end_ts_dt.strftime('%M%S.%f')
    else:
         print(f"Warning: Unexpected type for end_ts: {type(end_ts)}. Attempting to use directly.")
         end_min_sec_str = "UnknownMinSec"


    bpm_str = str(estimated_bpm) if estimated_bpm is not None else "None"
    file_prefix = f"window_EndSec{end_min_sec_str}"
    file_prefix = file_prefix.replace(":", "-").replace(" ", "_").replace(".", "_")



    plt.figure(figsize=(12, 5))
    if len(raw_signal) > 0:
        time_axis_signal = np.arange(len(raw_signal)) / sampling_rate
        plt.plot(time_axis_signal, raw_signal, label='Raw Signal')
        plt.title(f"Raw Signal for Window ({start_ts} to {end_ts})\nEstimated BPM: {estimated_bpm}")
        plt.xlabel("Time (s)")
        plt.ylabel("Signal Amplitude/Power")
        plt.grid(True)
        plt.legend()
        raw_signal_plot_path = os.path.join(output_dir, f"{file_prefix}_raw_signal.png")
        try:
            plt.savefig(raw_signal_plot_path)
        except Exception as e:
            print(f"Error saving raw signal plot {raw_signal_plot_path}: {e}")
        plt.close()
    else:
        plt.figure()
        plt.title(f"Raw Signal for Window ({start_ts} to {end_ts})\nEstimated BPM: {estimated_bpm}\n(No raw signal data)")
        raw_signal_plot_path_placeholder = os.path.join(output_dir, f"{file_prefix}_raw_signal_empty.png")
        try:
             plt.savefig(raw_signal_plot_path_placeholder)
        except Exception as e:
             print(f"Error saving empty raw signal plot placeholder {raw_signal_plot_path_placeholder}: {e}")
        plt.close()


    plt.figure(figsize=(12, 5))
    if len(smoothed_signal) > 0:
        time_axis_smoothed = np.arange(len(smoothed_signal)) / sampling_rate 
        plt.plot(time_axis_smoothed, smoothed_signal, label='Smoothed Signal', color='orange') 
        plt.title(f"Smoothed Signal for Window ({start_ts} to {end_ts})\nEstimated BPM: {estimated_bpm}")
        plt.xlabel("Time (s)")
        plt.ylabel("Smoothed Signal Amplitude/Power")
        plt.grid(True)
        plt.legend()
        smoothed_signal_plot_path = os.path.join(output_dir, f"{file_prefix}_smoothed_signal.png")
        try:
            plt.savefig(smoothed_signal_plot_path)
        except Exception as e:
            print(f"Error saving smoothed signal plot {smoothed_signal_plot_path}: {e}")
        plt.close()
    else:
        plt.figure()
        plt.title(f"Smoothed Signal for Window ({start_ts} to {end_ts})\nEstimated BPM: {estimated_bpm}\n(No smoothed signal data)")
        smoothed_signal_plot_path_placeholder = os.path.join(output_dir, f"{file_prefix}_smoothed_signal_empty.png")
        try:
             plt.savefig(smoothed_signal_plot_path_placeholder)
        except Exception as e:
             print(f"Error saving empty smoothed signal plot placeholder {smoothed_signal_plot_path_placeholder}: {e}")
        plt.close()


    plt.figure(figsize=(12, 5))
    if len(acf_values) > 0:
        lags = np.arange(len(acf_values)) / sampling_rate 
        plt.plot(lags, acf_values)

        if selected_peak_lag_samples is not None:
            if selected_peak_lag_samples < len(acf_values):
                selected_peak_lag_seconds = selected_peak_lag_samples / sampling_rate
                plt.plot(selected_peak_lag_seconds, acf_values[selected_peak_lag_samples], "x", color='red', markersize=10, label=f'Selected Peak ({selected_peak_lag_seconds:.2f}s)')
                plt.text(selected_peak_lag_seconds, acf_values[selected_peak_lag_samples], f' {selected_peak_lag_seconds:.2f}s', verticalalignment='bottom')
                plt.legend()
            else:
                print(f"Warning: Selected peak index {selected_peak_lag_samples} is out of bounds for ACF values (length {len(acf_values)}).")
        
        plt.title(f"Autocorrelation Function (ACF) for Window ({start_ts} to {end_ts})\nEstimated BPM: {estimated_bpm}")
        plt.xlabel("Lag (s)")
        plt.ylabel("ACF Value")
        plt.grid(True)

        acf_plot_path = os.path.join(output_dir, f"{file_prefix}_acf.png")
        try:
            plt.savefig(acf_plot_path)
        except Exception as e:
             print(f"Error saving ACF plot {acf_plot_path}: {e}")
        plt.close()
    else:
        plt.figure()
        plt.title(f"Autocorrelation Function (ACF) for Window ({start_ts} to {end_ts})\nEstimated BPM: {estimated_bpm}\n(No ACF data)")
        acf_plot_path_placeholder = os.path.join(output_dir, f"{file_prefix}_acf_empty.png")
        try:
             plt.savefig(acf_plot_path_placeholder)
        except Exception as e:
             print(f"Error saving empty ACF plot placeholder {acf_plot_path_placeholder}: {e}")
        plt.close()

# %%
def save_estimated_bpms_to_csv(estimated_results: list, output_dir: str = "./results"):
    """
    Saves the estimated BPMs and their corresponding timestamps to a CSV file.
    The filename is based on the timestamp of the first valid result.

    Args:
        estimated_results (list): A list of tuples (estimated_bpm, window_end_timestamp).
        output_dir (str): The directory to save the output CSV file.
    """
    if not estimated_results:
        print("No estimated BPM results to save.")
        return

    first_valid_result_time = None
    for result in estimated_results:
        if result[0] is not None:
            first_valid_result_time = result[1]
            break

    if first_valid_result_time is None and estimated_results:
         first_timestamp = estimated_results[0][1]
    elif first_valid_result_time is not None:
        first_timestamp = first_valid_result_time
    else:
        print("No results to save CSV filename from.")
        return


    if isinstance(first_timestamp, np.datetime64) or isinstance(first_timestamp, pd.Timestamp):
         first_timestamp_dt = pd.to_datetime(first_timestamp).to_pydatetime()
    else:
         print(f"Warning: Unexpected type for first_timestamp: {type(first_timestamp)}. Attempting to use directly.")
         first_timestamp_dt = first_timestamp

   
    csv_filename = f"estimated_bpms_dataset2.csv"
    csv_filename = csv_filename.replace(":", "-").replace(" ", "_").replace(".", "_").replace("_csv", ".csv")
    output_csv_path = os.path.join(output_dir, csv_filename)

    results_df = pd.DataFrame(estimated_results, columns=['bpm', 'time'])

    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir)

    try:
        results_df.to_csv(output_csv_path, index=False) 
        print(f"\nEstimated BPM results saved to {output_csv_path}")
    except Exception as e:
        print(f"Error saving estimated BPM results to {output_csv_path}: {e}")


def estimate_breathing_rate_windowed(csi_df: pd.DataFrame, sampling_rate: int = 100, window_duration_sec: int = 20, step_duration_sec: int = 1, output_plots_dir: str = "./window_plots_results_2"):
    """
    Estimates breathing rate from CSI data using ACF peak detection in sliding windows.
    Calculates ACF for each subcarrier and sums them before peak detection (MRC of ACFs concept).
    Plots the raw signal, smoothed combined signal, and the combined ACF for each window in separate files.

    Args:
        csi_df (pandas.DataFrame): DataFrame containing loaded CSI data with 'timestamp' and 'data' columns.
        sampling_rate (int): The sampling rate of the CSI data in Hz.
        window_duration_sec (int): The duration of the sliding window in seconds.
        step_duration_sec (int): The step size of the sliding window in seconds.
        output_plots_dir (str): Directory to save the plot files for each window.

    Returns:
        list: A list of tuples, where each tuple is (estimated_bpm, window_end_timestamp).
              Estimated BPM can be None if no valid breathing peak is found.
    """
    estimated_results = []

    if 'data' not in csi_df.columns or 'timestamp' not in csi_df.columns:
        print("Required columns ('data' and/or 'timestamp') not found in CSI data.")
        return estimated_results

    csi_amplitudes_df = extract_amplitudes_all_subcarriers(csi_df.copy())

    if csi_amplitudes_df.empty:
        print("No valid amplitude data extracted. Cannot proceed with estimation.")
        csi_amplitudes_df = pd.DataFrame() 
    else:
        print(f"Original csi_amplitudes_df shape: {csi_amplitudes_df.shape}, NaNs before interpolation: {csi_amplitudes_df.isnull().sum().sum()}")
        csi_amplitudes_df = csi_amplitudes_df.interpolate(method='linear', limit_direction='both')
        # After interpolation, some NaNs at start/end might remain, use ffill/bfill as fallback
        csi_amplitudes_df = csi_amplitudes_df.ffill().bfill()
        print(f"csi_amplitudes_df shape after interpolation: {csi_amplitudes_df.shape}, NaNs after interpolation: {csi_amplitudes_df.isnull().sum().sum()}")


    csi_df['amplitudes'] = csi_df['data'].apply(extract_and_process_csi_amplitude)
    csi_df['combined_signal'] = csi_df['amplitudes'].apply(apply_mrc_across_subcarriers)
    processed_df_for_plots = csi_df.dropna(subset=['combined_signal']).copy()

    if processed_df_for_plots.empty:
         print("No valid combined signal data for plotting. Plots may be empty.")
         return estimated_results 


    processed_df_for_plots = processed_df_for_plots.sort_values(by='timestamp')
  
    typical_bpm_min = 8
    typical_bpm_max = 40

    typical_period_min_sec = 60 / typical_bpm_max  
    typical_period_max_sec = 60 / typical_bpm_min  

    typical_period_min_samples = int(typical_period_min_sec * sampling_rate)
    typical_period_max_samples = int(typical_period_max_sec * sampling_rate)

    window_size_samples = int(window_duration_sec * sampling_rate)
    step_size_samples = int(step_duration_sec * sampling_rate)

    if window_size_samples <= 0 or step_size_samples <= 0:
        print("Invalid window size or step size.")
        return estimated_results

    min_signal_length_for_acf = int(sampling_rate * 2) 

    for start_index in range(0, len(processed_df_for_plots) - window_size_samples + 1, step_size_samples):
        end_index = start_index + window_size_samples

        window_data_for_plots = processed_df_for_plots.iloc[start_index:end_index]
        raw_window_signal_for_plot = window_data_for_plots['combined_signal'].values
        window_start_timestamp = window_data_for_plots['timestamp'].iloc[0]
        window_end_timestamp = window_data_for_plots['timestamp'].iloc[-1]

        smoothed_window_signal_for_plot = np.array([]) 
        breathing_cutoff_freq_lowpass_plot = 1.2
        filter_order_plot = 6
        if len(raw_window_signal_for_plot) > filter_order_plot:
            try:
                sos_plot = butter(filter_order_plot, breathing_cutoff_freq_lowpass_plot, "low", fs=sampling_rate, output="sos")
                smoothed_window_signal_for_plot = sosfiltfilt(sos_plot, raw_window_signal_for_plot)
            except Exception as e:
                 print(f"Error smoothing combined signal for plotting for window starting at index {start_index}: {e}")
                 smoothed_window_signal_for_plot = raw_window_signal_for_plot 


        if csi_amplitudes_df.empty:
             window_amplitudes_df = pd.DataFrame() 
        else:
            window_amplitudes_df = csi_amplitudes_df.loc[window_data_for_plots['timestamp']] 

        if len(window_amplitudes_df) < min_signal_length_for_acf or window_amplitudes_df.shape[1] == 0:
             print(f"Window starting at index {start_index} has insufficient data ({len(window_amplitudes_df)} samples, {window_amplitudes_df.shape[1]} subcarriers) for ACF calculation. Skipping estimation.")
             estimated_results.append((None, window_end_timestamp))

             continue


        num_subcarriers = window_amplitudes_df.shape[1]
        sum_of_subcarrier_acfs = np.zeros(window_size_samples) 

        breathing_cutoff_freq_lowpass_acf = 2 
        filter_order_acf = 5 

        can_filter_subcarriers = window_size_samples > filter_order_acf


        for subcarrier_index in range(num_subcarriers):
            subcarrier_signal = window_amplitudes_df.iloc[:, subcarrier_index].values

            if can_filter_subcarriers and len(subcarrier_signal) > filter_order_acf:
                try:
                    sos_acf = butter(filter_order_acf, breathing_cutoff_freq_lowpass_acf, "low", fs=sampling_rate, output="sos")
                    smoothed_subcarrier_signal = sosfiltfilt(sos_acf, subcarrier_signal.astype(float))
                except Exception as e:
                    print(f"Warning: Error filtering subcarrier {subcarrier_index} for window starting at index {start_index}: {e}. Using raw subcarrier signal for ACF.")
                    smoothed_subcarrier_signal = subcarrier_signal.astype(float)
            else:
                smoothed_subcarrier_signal = subcarrier_signal.astype(float)
           
            if len(smoothed_subcarrier_signal) < 2: 
                 print(f"Subcarrier signal too short ({len(smoothed_subcarrier_signal)}) for ACF calculation for window starting at index {start_index}, subcarrier {subcarrier_index}. Skipping subcarrier.")
                 continue 


            subcarrier_acf = autocorr(smoothed_subcarrier_signal)

            if len(subcarrier_acf) == window_size_samples:
                 sum_of_subcarrier_acfs += subcarrier_acf
            else:
                 print(f"Warning: Subcarrier ACF length mismatch ({len(subcarrier_acf)} vs {window_size_samples}) for window starting at index {start_index}, subcarrier {subcarrier_index}. Skipping accumulation for this subcarrier.")
                 pass 

        combined_acf = sum_of_subcarrier_acfs
        if num_subcarriers > 0:
            combined_acf = sum_of_subcarrier_acfs / num_subcarriers 

        estimated_bpm_rounded = None
        selected_peak_lag_samples = None
        acf_values_for_plot = np.array([]) 


        if len(combined_acf) < 2 or np.sum(np.abs(combined_acf)) == 0: 
             print(f"Combined ACF is too short or all zeros for window starting at index {start_index}. Skipping peak detection.")
             estimated_bpm_rounded = None
             selected_peak_lag_samples = None
             acf_values_for_plot = combined_acf.copy() 
        else:
             smoothed_combined_acf = combined_acf 
             min_peak_distance_for_breathing = int(sampling_rate * 1.4) 
             if len(smoothed_combined_acf[1:]) == 0:
                  print(f"Combined ACF values (excluding lag 0) are empty for window starting at index {start_index}. Skipping peak detection.")
                  estimated_bpm_rounded = None
                  selected_peak_lag_samples = None
                  acf_values_for_plot = smoothed_combined_acf.copy()
             else:
                  try:
                      peaks, properties = find_peaks(smoothed_combined_acf[1:], distance=min_peak_distance_for_breathing, prominence=0.16) 
                  except ValueError as ve:
                       print(f"Error finding peaks in combined ACF for window starting at index {start_index}: {ve}. Skipping peak detection.")
                       peaks = np.array([])


                  estimated_bpm_rounded = None
                  selected_peak_lag_samples = None 

                  if len(peaks) > 0:
                      peak_lags_samples = peaks + 1

                      
                      if 'prominences' in properties:
                           peak_prominences = properties['prominences']
                      else:
                           peak_prominences = np.ones(len(peaks))

                      valid_peak_indices = np.where(
                          (peak_lags_samples >= typical_period_min_samples) &
                          (peak_lags_samples <= typical_period_max_samples)
                      )[0]

                      if len(valid_peak_indices) > 0:
                          first_valid_peak_index_in_peaks_array = valid_peak_indices[0]
                          selected_peak_lag_samples = peak_lags_samples[first_valid_peak_index_in_peaks_array]

                          tau_b_seconds = selected_peak_lag_samples / sampling_rate
                          if tau_b_seconds > 0:
                               estimated_freq_hz = 1.0 / tau_b_seconds
                               estimated_bpm = estimated_freq_hz * 60
                               estimated_bpm_rounded = estimated_bpm 
                            #    estimated_bpm_rounded = round(estimated_bpm_rounded) for final submission

                  acf_values_for_plot = smoothed_combined_acf.copy()


        estimated_results.append((estimated_bpm_rounded, window_end_timestamp))


    return estimated_results 

# %%
if __name__ == "__main__":
    # csi_file_path = "benchmark/breathing_rate/evaluation/CSI20250227_191018.csv"
    csi_file_path = "./evaluation/CSI20250227_191018.csv" 

    output_plots_directory = "./window_plots_results_2" 

    output_results_directory = "./results" 


    csi_data_df = load_csi_data(csi_file_path)

    if csi_data_df is not None and not csi_data_df.empty:
        print(f"Loaded {len(csi_data_df)} rows of CSI data.")


        sampling_rate = 100 # Hz
        window_duration_sec = 20 # in sec
        step_duration_sec = 0.5 # in sec

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

        ground_truth = './evaluation/gt_20250227_191018.csv'
        ground_truth_data = pd.read_csv(ground_truth)
        
        
    else:
        print("Failed to load CSI data or data is empty.")

# %%
def pad(data):
    bad_indexes = np.isnan(data)
    good_indexes = np.logical_not(bad_indexes)
    good_data = data[good_indexes]
    interpolated = np.interp(bad_indexes.nonzero()[0], good_indexes.nonzero()[0], good_data)
    data[bad_indexes] = interpolated
    return data


def align_and_plot_mae(arr1, arr2, filename, label1, label2):
    if len(arr1) >= len(arr2):
        long_arr = arr1
        short_arr = arr2
    else:
        long_arr = arr2
        short_arr = arr1

    m = len(short_arr)
    n = len(long_arr)
    max_shift = n - m

    if max_shift < 0:
        raise ValueError("The longer array must be at least as long as the shorter array.")
    best_shift = 0
    min_mae = float('inf')
    for shift in range(max_shift + 1):
        window = long_arr[shift : shift + m]
        current_mae = np.mean(np.abs(window - short_arr))
        if current_mae < min_mae:
            min_mae = current_mae
            best_shift = shift
    # Extract the aligned parts
    aligned_long = long_arr[best_shift : best_shift + m]
    aligned_short = short_arr

    # Determine labels based on original arrays
    if len(arr1) >= len(arr2):
        label_long = label1 + " (truncated)"
        label_short = label2
    else:
        label_long = label2 + " (truncated)"
        label_short = label1
    x = np.arange(m)
    plt.clf()
    plt.scatter(x, aligned_long, label=label_long, alpha=0.5, color='blue')
    plt.scatter(x, aligned_short, label=label_short, alpha=0.5, color='red')
    plt.xlabel('Index (Aligned to Shorter Array)')
    plt.ylabel('Value')
    plt.title(f'Optimal Alignment (Shift={best_shift}), MAE={min_mae:.4f}')
    plt.legend()
    plt.ylim(0, max(np.max(aligned_long), np.max(aligned_short)) + 1)
    plt.grid(True)
    plt.savefig(filename)

# %% [markdown]
# Since the time horizon of the retrieved breath rate and ground truth do not coincide, we consider aligning the retrieved breath rate to the ground truth starting from the index $i^*$, which is determined by the following rule:
# $$
# i^* = \arg \min_{i} \text{MAE}\left( pred[i, i + len(pred)], gt \right).
# $$

# %%
prediction = pd.read_csv('./results/estimated_bpms_dataset2.csv')
print(prediction.head())
ground_truth = pd.read_csv('./evaluation/gt_20250227_191018.csv')

print(ground_truth.head())

prediction = prediction['bpm'].to_numpy()

prediction = pad(prediction)
ground_truth = ground_truth['bpm'].to_numpy()

filename = csi_file_path.split('/')[-1].split('.')[0] + '_prediction_vs_ground_truth.png'

align_and_plot_mae(prediction, ground_truth, filename, 'Predicted BPM', 'Ground Truth BPM')


