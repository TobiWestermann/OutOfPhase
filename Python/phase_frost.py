import numpy as np
import scipy.io.wavfile as wav
import os

def process_wav_fixed_phase(filename, output_filename, block_size=1024, overlap=512):
    # Read WAV file
    sample_rate, data = wav.read(filename)
    
    # If stereo, convert to mono
    if len(data.shape) > 1:
        data = np.mean(data, axis=1)
    
    # Ensure float processing
    data = data.astype(np.float64)
    
    # Process in overlapping blocks
    step_size = block_size - overlap
    num_blocks = (len(data) - overlap) // step_size
    output_signal = np.zeros(len(data))
    window = np.hanning(block_size)  # Smooth transitions with a window function
    window_sum = np.zeros(len(data))
    
    # Get phase from the first block
    first_block = data[:block_size] * window
    first_spectrum = np.fft.fft(first_block)
    fixed_phase = np.angle(first_spectrum)
    
    for i in range(num_blocks):
        start = i * step_size
        end = start + block_size
        block = data[start:end] * window  # Apply window
        
        # Perform FFT
        spectrum = np.fft.fft(block)
        magnitude = np.abs(spectrum)
        
        # Apply fixed phase from first block
        new_spectrum = magnitude * np.exp(1j * fixed_phase)
        
        # Inverse FFT
        new_block = np.fft.ifft(new_spectrum).real * window
        
        # Overlap-add to reconstruct signal
        output_signal[start:end] += new_block
        window_sum[start:end] += window
    
    # Normalize by the sum of window functions to avoid amplitude artifacts
    output_signal = np.divide(output_signal, window_sum, where=window_sum != 0)
    
    # Normalize signal to int16 range
    output_signal = np.int16(output_signal / np.max(np.abs(output_signal)) * 32767)
    
    # Save output
    wav.write(output_filename, sample_rate, output_signal)
    print(f"Processed file saved as {output_filename}")

# Example usage
input_wav = "Test.wav"  # Change this to your input file
output_wav = "frost_blockwise.wav"
if os.path.exists(input_wav):
    process_wav_fixed_phase(input_wav, output_wav)
else:
    print(f"File {input_wav} not found!")