import numpy as np
import scipy.io.wavfile as wav
import os

def process_wav_blockwise(filename, output_filename, block_size=1024*8):
    # Read WAV file
    sample_rate, data = wav.read(filename)
    
    # If stereo, convert to mono
    if len(data.shape) > 1:
        data = np.mean(data, axis=1)
    
    # Process in blocks
    new_signal = np.zeros_like(data, dtype=np.float64)
    num_blocks = len(data) // block_size
    
    for i in range(num_blocks):
        start = i * block_size
        end = start + block_size
        block = data[start:end]
        
        # Perform FFT
        spectrum = np.fft.fft(block)
        magnitude = np.abs(spectrum)
        phase = np.angle(spectrum)
        
        # Randomize phase
        random_phase = np.random.uniform(-np.pi, np.pi, size=phase.shape)
        new_spectrum = magnitude * np.exp(1j * random_phase)
        
        # Inverse FFT
        new_block = np.fft.ifft(new_spectrum).real
        new_signal[start:end] = new_block
    
    # Normalize signal to int16 range
    new_signal = np.int16(new_signal / np.max(np.abs(new_signal)) * 32767)
    
    # Save output
    wav.write(output_filename, sample_rate, new_signal)
    print(f"Processed file saved as {output_filename}")

# Example usage
input_wav = "Test.wav"  # Change this to your input file
output_wav = "random_blockwise.wav"
if os.path.exists(input_wav):
    process_wav_blockwise(input_wav, output_wav)
else:
    print(f"File {input_wav} not found!")