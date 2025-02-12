import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft, fftfreq

def plot_spectrum_basic(signal, fs, title="Basic FFT Spectrum"):
    """
    Compute and plot the magnitude and phase spectrum of a signal without modifications.
    """
    N = len(signal)
    X = fft(signal)
    freqs = fftfreq(N, 1/fs)  # Frequency axis

    magnitude_spectrum = np.abs(X)
    phase_spectrum = np.angle(X)

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
    plt.title(f"Phase Spectrum - {title}")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Phase (radians)")
    plt.grid()

    plt.tight_layout()

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


if __name__ == "__main__":
    # Sampling parameters
    fs = 1000  # Sampling frequency (Hz)
    T = 2      # Signal duration (seconds)
    t = np.linspace(0, T, fs * T, endpoint=False)  # Time vector

    # Example 1: Pure sinusoid with phase shift
    f0 = 50
    phi = np.pi / 4
    x1 = np.cos(2 * np.pi * f0 * t + phi)
    plot_spectrum_basic(x1, fs, title="Pure Sinusoid (50Hz, Phase π/4)")
    plot_spectrum_improved(x1, fs, title="Pure Sinusoid (50Hz, Phase π/4)")

    # Example 2: Sinusoid with a non-integer number of cycles (spectral leakage)
    f1 = 57.3  # Not an exact integer multiple of sampling interval
    x2 = np.cos(2 * np.pi * f1 * t)
    plot_spectrum_basic(x2, fs, title="Sinusoid with Spectral Leakage (57.3Hz)")
    plot_spectrum_improved(x2, fs, title="Sinusoid with Spectral Leakage (57.3Hz)")

    # Example 3: Sum of two sinusoids (multiple frequency components)
    f2 = 150
    x3 = np.cos(2 * np.pi * f0 * t + phi) + 0.5 * np.cos(2 * np.pi * f2 * t + phi)
    plot_spectrum_basic(x3, fs, title="Sum of Sinusoids (50Hz + 150Hz)")
    plot_spectrum_improved(x3, fs, title="Sum of Sinusoids (50Hz + 150Hz)")

    plt.show()