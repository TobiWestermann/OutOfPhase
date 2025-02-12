import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft, fftfreq
import wave

def plot_spectrum_improved(signal, fs, title="Improved FFT Spectrum"):
    """
    Compute and plot the magnitude and phase spectrum with phase thresholding.
    """
    N = len(signal)
    X = fft(signal)
    freqs = fftfreq(N, 1/fs)  # Frequency axis

    magnitude_spectrum = np.abs(X)
    phase_spectrum = np.angle(X)

    # Apply phase thresholding to remove noise
    threshold = 0.000001 * np.max(magnitude_spectrum)
    phase_spectrum[magnitude_spectrum < threshold] = np.nan  # Mask unreliable phase

    # Take only positive frequencies for visualization
    pos_mask = freqs >= 0
    freqs = freqs[pos_mask]
    magnitude_spectrum = magnitude_spectrum[pos_mask]
    phase_spectrum = phase_spectrum[pos_mask]

    # Plot results
    plt.figure(figsize=(10, 5))
    
    plt.subplot(2, 1, 1)
    plt.plot(freqs, magnitude_spectrum, 'b')
    plt.title(f"Magnitude Spectrum - {title}")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.grid()

    plt.subplot(2, 1, 2)
    plt.plot(freqs, phase_spectrum, 'r', marker='o', linestyle='None', markersize=3)
    plt.title(f"Phase Spectrum - {title} (Thresholded)")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Phase (radians)")
    plt.grid()

    plt.tight_layout()

def load_wave_file(file_path):
    """
    Load a wave file and return the signal and sampling frequency.
    """
    with wave.open(file_path, 'rb') as wf:
        fs = wf.getframerate()
        n_frames = wf.getnframes()
        signal = np.frombuffer(wf.readframes(n_frames), dtype=np.int16)
    return signal, fs

file_path = 'Test.wav'
signal, fs = load_wave_file(file_path)
plot_spectrum_improved(signal[0:2048], fs)
plt.show()