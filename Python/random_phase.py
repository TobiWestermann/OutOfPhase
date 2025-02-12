import numpy as np
import scipy.io.wavfile as wav
import os

def process_wav(filename, output_filename):
    # Read WAV file
    sample_rate, data = wav.read(filename)
    
    # If stereo, convert to mono
    if len(data.shape) > 1:
        data = np.mean(data, axis=1)
    
    # Perform FFT
    spectrum = np.fft.fft(data)
    magnitude = np.abs(spectrum)
    phase = np.angle(spectrum)
    
    # Randomize phase
    random_phase = np.random.uniform(-np.pi, np.pi, size=phase.shape)
    new_spectrum = magnitude * np.exp(1j * random_phase)
    
    # Inverse FFT
    new_signal = np.fft.ifft(new_spectrum).real
    
    # Normalize signal to int16 range
    new_signal = np.int16(new_signal / np.max(np.abs(new_signal)) * 32767)
    
    # Save output
    wav.write(output_filename, sample_rate, new_signal)
    print(f"Processed file saved as {output_filename}")

# Example usage
input_wav = "Test.wav"  # Change this to your input file
output_wav = "random_phase.wav"
if os.path.exists(input_wav):
    process_wav(input_wav, output_wav)
else:
    print(f"File {input_wav} not found!")
