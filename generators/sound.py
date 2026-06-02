import wave
import math
import struct
import random

# Standard CD-quality sample rate
SAMPLE_RATE = 44100

def write_wav(filename, audio_data):
    """Packs floating point audio data into a 16-bit PCM WAV file."""
    with wave.open(filename, 'w') as w:
        w.setnchannels(1) # Mono audio
        w.setsampwidth(2) # 2 bytes = 16-bit resolution
        w.setframerate(SAMPLE_RATE)
        for sample in audio_data:
            # Clamp values to [-1.0, 1.0] to prevent integer overflow/digital clipping
            sample = max(-1.0, min(1.0, sample))
            # Convert float float to 16-bit signed integer
            w.writeframesraw(struct.pack('<h', int(sample * 32767)))

def generate_hoe():
    """Generates a digging sound (white noise + low thud)."""
    samples = []
    duration = 0.15
    total_samples = int(SAMPLE_RATE * duration)
    for i in range(total_samples):
        t = i / total_samples
        env = 1.0 - t # Linear decay
        noise = random.uniform(-1, 1)
        low_thud = math.sin(2 * math.pi * 100 * i / SAMPLE_RATE)
        samples.append((noise * 0.6 + low_thud * 0.4) * env)
    write_wav("hoe.wav", samples)
    print("Generated: hoe.wav")

def generate_water():
    """Generates a liquid 'bloop' using Exponential FM (Frequency Modulation)."""
    samples = []
    total_samples = int(SAMPLE_RATE * 0.25)
    for i in range(total_samples):
        t = i / SAMPLE_RATE
        env = math.exp(-10 * t)
        noise = random.uniform(-1, 1) * 0.15
        freq = 400 + 600 * math.exp(-20 * t) # Pitch drops rapidly from 1000Hz to 400Hz
        droplet = math.sin(2 * math.pi * freq * t) * 0.8
        samples.append((noise + droplet) * env)
    write_wav("water.wav", samples)
    print("Generated: water.wav")

def generate_plant():
    """Generates a tight 'thud/pop' for placing items or planting seeds."""
    samples = []
    duration = 0.1
    total_samples = int(SAMPLE_RATE * duration)
    for i in range(total_samples):
        env = math.exp(-15 * i / total_samples) # Sharp exponential decay
        freq = 150 - 50 * (i / total_samples)
        samples.append(math.sin(2 * math.pi * freq * i / SAMPLE_RATE) * env)
    write_wav("plant.wav", samples)
    print("Generated: plant.wav")

def generate_harvest():
    """Generates an earthy, sandy sound using Subtractive Synthesis concepts."""
    samples = []
    total_samples = int(SAMPLE_RATE * 0.2)
    for i in range(total_samples):
        t = i / SAMPLE_RATE
        env = math.exp(-15 * t)
        noise = random.uniform(-1, 1) * 0.8 # High noise content for grit
        thud = math.sin(2 * math.pi * 60 * t) * 0.6 # Low frequency rumble
        samples.append((noise + thud) * env)
    write_wav("harvest.wav", samples)
    print("Generated: harvest.wav")

def generate_buy():
    """Generates a descending double-klunk to psychologically indicate money loss."""
    samples = []
    total_samples = int(SAMPLE_RATE * 0.2)
    for i in range(total_samples):
        t = i / SAMPLE_RATE
        env = 1.0 - (i / total_samples)
        # Shift frequency down halfway through
        val = math.sin(2 * math.pi * 300 * t) if t < 0.1 else math.sin(2 * math.pi * 200 * t)
        samples.append(val * env)
    write_wav("buy.wav", samples)
    print("Generated: buy.wav")

def generate_sell():
    """Generates a musical 'Ka-Ching' to indicate gaining money."""
    samples = []
    for i in range(int(SAMPLE_RATE * 0.1)):
        samples.append(math.sin(2 * math.pi * 987 * i / SAMPLE_RATE))
    for i in range(int(SAMPLE_RATE * 0.3)):
        env = 1.0 - (i / (SAMPLE_RATE * 0.3))
        samples.append(math.sin(2 * math.pi * 1318 * i / SAMPLE_RATE) * env)
    write_wav("sell.wav", samples)
    print("Generated: sell.wav")

def generate_step():
    """Generates a very short, muffled footstep synced with the animation."""
    samples = []
    total_samples = int(SAMPLE_RATE * 0.1)
    for i in range(total_samples):
        t = i / SAMPLE_RATE
        env = math.exp(-35 * t) # Extremely fast fadeout
        noise = random.uniform(-1, 1) * 0.5
        thud = math.sin(2 * math.pi * 40 * t) * 0.5
        samples.append((noise + thud) * env)
    write_wav("step.wav", samples)
    print("Generated: step.wav")

def generate_click():
    """Generates a heavy mechanical keyboard switch sound (Multi-transient)."""
    samples = []
    total_samples = int(SAMPLE_RATE * 0.15)
    for i in range(total_samples):
        t = i / SAMPLE_RATE
        val = 0.0
        
        # 1. Key Press ("Thock")
        if t < 0.07:
            noise = random.uniform(-1, 1)
            snap = noise * math.exp(-300 * t)
            thock = math.sin(2 * math.pi * 350 * t) * math.exp(-80 * t)
            val += (snap * 0.7) + (thock * 0.6)
            
        # 2. Key Release ("Clack" + Spring ping)
        if t >= 0.08:
            tr = t - 0.08
            noise = random.uniform(-1, 1)
            rel_snap = noise * math.exp(-200 * tr)
            spring = math.sin(2 * math.pi * 1200 * tr) * math.exp(-40 * tr)
            val += (rel_snap * 0.4) + (spring * 0.2)
            
        samples.append(val * 0.8)
    write_wav("click.wav", samples)
    print("Generated: click.wav")

if __name__ == "__main__":
    generate_hoe()
    generate_water()
    generate_plant()
    generate_harvest()
    generate_buy()
    generate_sell()
    generate_step()
    generate_click()
    print("\nSUCCESS: All 8 audio files are ready for your Raylib engine!")