import matplotlib
matplotlib.use('Agg')
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from datetime import datetime, timedelta 
from scipy.signal import savgol_filter, find_peaks, butter, sosfiltfilt 
import os 
matplotlib.use('Agg')
import sys


'''
The theories are based on the IMWUT 2019 paper, with citation
Feng Zhang, Chenshu Wu, Beibei Wang, Hung-Quoc Lai, Yi Han, and K. J. Ray Liu. 2019. WiDetect: Robust Motion Detection with a Statistical Electromagnetic Model. Proc. ACM Interact. Mob. Wearable Ubiquitous Technol. 3, 3, Article 122 (September 2019), 24 pages. https://doi.org/10.1145/3351280
'''



def load_csi_data(file_path: str) -> pd.DataFrame:
    try:
        df = pd.read_csv(file_path)

        return df
    except FileNotFoundError:
        print(f"Error: File not found at {file_path}")
        return None
    except Exception as e:
        print(f"Error loading or processing CSV file: {e}")
        return None
    
def retrieve_float(data_list: str) -> str:
    data_list = data_list.split(',')
    data_list[0] = data_list[0].replace('[','')
    data_list[-1] = data_list[-1].replace(']','')
    data_list = [float(i) for i in data_list]
    return np.array(data_list)
    
def csi_data_parsing(csi: pd.DataFrame) -> pd.DataFrame:
    csi_data_length = csi['len'][0]
    csi['real'] = csi['data'].apply(lambda x: retrieve_float(x)[0:csi_data_length-1:2])
    csi['imaginary'] = csi['data'].apply(lambda x: retrieve_float(x)[1:csi_data_length:2])
    csi['amplitude'] = csi['real']**2 + csi['imaginary']**2
    # c.f. equation (1) in paper, we use square of CSI magnitude
    return csi

def get_subcarrier(csi_data: pd.DataFrame, subcarrier: int) -> np.ndarray:
    try:
        extracted_subcarrier_signal = csi_data['amplitude'].apply(lambda x: x[subcarrier - 1])
    except IndexError:
        print(f"Error: Invalid subcarrier index {subcarrier} is out of bounds for the data.")
        return None
    return extracted_subcarrier_signal.to_numpy()

def sample_auto_covariance(data: np.ndarray, lag: int) -> float:
    T = len(data)
    mean = np.mean(data)
    data_1 = data[lag:T]
    data_2 = data[0:T - lag]
    acf = np.sum((data_1 - mean) * (data_2 - mean)) / T  # equation (2) in paper
    return acf

# Note that the CSI amplitude |H(t, f)|^2 is denoted as \mu(t, f) in the paper following equation (1).

def rho_G(data: np.ndarray, lag: int) -> float:
    ACF_G = sample_auto_covariance(data, lag)
    ACF_0 = sample_auto_covariance(data, 0)
    if ACF_0 == 0:
        return None
    return ACF_G / ACF_0  # Mentioned in Section 3.2, assumption (2)


def motion_detection(
        csi_data: pd.DataFrame,
        threshold: float = 0.2,
        window_size: int = 100,
        step_size: int = 50,
) -> np.ndarray:
    csi_data = csi_data_parsing(csi_data)
    lag = 1
    time_horizon = len(csi_data)
    num_of_windows = (time_horizon - window_size) // step_size + 1
    detection_outcome = np.zeros(num_of_windows)
    num_of_subcarriers = csi_data['len'][0] // 2  # assuming len is even, 234 in our test case
    subacarrier_csi = [
        get_subcarrier(csi_data, i) for i in range(1, num_of_subcarriers + 1)
    ]
    for i in range(num_of_windows):
        start = i * step_size
        end = min(i * step_size + window_size, time_horizon)
        sum_of_subcarrier_estimator = 0
        valid_subcarriers = 0
        estimator_i = 0
        for j in range(num_of_subcarriers):
            rho_j = rho_G(subacarrier_csi[j][start:end], lag)
            if rho_j is not None:
                valid_subcarriers += 1
                sum_of_subcarrier_estimator += rho_j
        if valid_subcarriers == 0:
            print(f"Warning: No valid subcarriers found for window {i}.")
            continue
        if valid_subcarriers > 0:
            estimator_i = sum_of_subcarrier_estimator / num_of_subcarriers
        print(estimator_i)
        if estimator_i > threshold:
            detection_outcome[i] = 1
        else:
            detection_outcome[i] = 0
    return detection_outcome
    


if __name__ == '__main__':
    # Load the CSI data
    try:
        path_name = sys.argv[1]
    except IndexError:
        print("Error: Please provide the file name as a command line argument.")
        sys.exit(1)

    file_path = path_name

    csi_data = load_csi_data(file_path)

    if csi_data is not None:
        # Perform motion detection
        detection_outcome = motion_detection(csi_data, threshold=0.4)

        print('Testing file: {}'.format(file_path))
        print("Detection Outcome:")
        print(detection_outcome)
        print(len(detection_outcome))
