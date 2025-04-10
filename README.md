# I2S Digital Audio Transmission: Raspberry Pi → ESP32

This document explains how to transmit a WAV audio file from a Raspberry Pi to an ESP32 via the **I2S (Inter-IC Sound)** interface. The data flow is completely digital, and no actual analog audio playback is required.

---

## Project Overview

- **Goal**: Send audio data from Raspberry Pi to ESP32 over I2S for real-time digital signal processing (DSP).
- **Key Steps**:  
  1. Configure Raspberry Pi’s I2S interface.  
  2. Convert a WAV file to a compatible format.  
  3. Use `aplay` on the Pi to output PCM data through the I2S pins.  
  4. Configure the ESP32 in I2S Slave Receive mode.  
  5. Verify that the ESP32 is receiving valid PCM samples for further analysis (e.g., FFT or level detection).

---

## Hardware Used

- **Raspberry Pi 3B+** (Any modern Pi with GPIO and I2S should work)
- **ESP32** development board (DevKit v1 or similar)
- **Jumper wires** for connections
- **Common GND** shared between both boards

---

## Hardware Wiring

| Raspberry Pi GPIO | Signal            | ESP32 Pin | Purpose            |
|-------------------|-------------------|----------|--------------------|
| GPIO18 (Pin 12)   | BCLK (Bit Clock)  | GPIOxx   | Clock line         |
| GPIO19 (Pin 35)   | LRCK (Word Select)| GPIOyy   | L/R channel timing |
| GPIO21 (Pin 40)   | DATA (PCM)        | GPIOzz   | Audio data line    |
| GND               | Ground            | GND      | Common reference   |

> Note: Replace `GPIOxx`, `GPIOyy`, and `GPIOzz` with the actual GPIOs you use on the ESP32.

---

## Raspberry Pi Configuration

1. **Enable I2S in `config.txt`:**

   - Open the firmware configuration file:
     ```bash
     sudo nano /boot/firmware/config.txt
     ```
   - Add (or uncomment) the lines:
     ```ini
     dtparam=i2s=on
     dtoverlay=hifiberry-dac
     ```
   - Make sure these lines are placed outside of any `[cm4]` or `[cm5]` blocks.  
   - Save and exit, then reboot:
     ```bash
     sudo reboot
     ```

2. **Convert the WAV File (Optional if you already have the correct format):**

   Use `ffmpeg` to convert audio into a standard PCM format. Example:
   ```bash
   ffmpeg -i input.wav -ar 44100 -ac 2 -acodec pcm_s16le fixed.wav

3. **Play the WAV File Through I2S
    ```bash
    aplay -D hw:2,0 fixed.wav

Where:
- **44.1 kHz sample rate**
- **16-bit samples**
- **Stereo (2 channels)**


## ESP32 Setup

1. **I2S Slave Mode**  
   Configure the ESP32 to receive bit clock (BCLK) and word select (LRCK) signals from the Raspberry Pi. The ESP32 operates as **Slave** and **RX** in this scenario.

2. **Bit Depth & Sample Rate**  
   - The ESP32’s I2S driver must match the same sample rate used on the Raspberry Pi (e.g., 44.1 kHz).
   - If the Pi outputs 16-bit data in a 32-bit frame, you may need to shift bits in your ESP32 driver code to properly extract the samples.

3. **Channels & Buffering**  
   - Typically set to stereo for a stereo WAV file.
   - Allocate enough DMA buffers to accommodate the incoming audio data rate without overflow.

4. **Reading the Data**  
   - The ESP32 continuously reads PCM samples from its I2S DMA buffer.
   - These samples can then be processed in real time for tasks such as RMS level detection, FFT, logging, or other DSP operations.

---

## Short Explanation of the I2S Code

Although the detailed code is not included here, the general steps for setting up the ESP32 I2S peripheral are:

1. **Create an I2S channel**  
   - Specify the role as **Slave**.  
   - Configure DMA buffer count and length.
2. **Configure Standard Mode**  
   - Define clock settings (sample rate, bit width).  
   - Set the slot configuration (stereo, 16-bit or 32-bit slot width).  
   - Assign GPIO pins for BCLK, WS, and DATA.
3. **Initialize and Enable**  
   - Initialize the channel with the selected standard mode settings.  
   - Call the function to enable the I2S RX channel.
4. **Read Samples**  
   - Continuously call `i2s_channel_read` to obtain audio samples from the DMA buffer.  
   - Once retrieved, these samples can be used for further processing, such as frequency analysis or logging.

---

## Common Issues & Debugging

1. **All Zero Samples**  
   - Make sure the Raspberry Pi is actively sending data via `aplay`.  
   - Verify `dtoverlay=hifiberry-dac` in `/boot/firmware/config.txt`.  
   - Confirm that 16-bit data is not padded into a 32-bit frame without proper handling on the ESP32.  
   - Use a logic analyzer on the Pi’s GPIO21 (DATA) to ensure data is changing.

2. **Mismatch in Sample Rate**  
   - Both the Pi and ESP32 must be set to the same rate (e.g., 44100 Hz).  
   - Any mismatch can cause distorted or meaningless data.

3. **Noisy or Garbled Data**  
   - Check for interference or poor connections at high clock speeds.  
   - Ensure stereo vs. mono settings match your WAV file.

---

## Potential Next Steps

- **Real-Time DSP**  
  Implement an FFT to visualize frequencies, detect audio peaks, or measure RMS levels in real time.

- **Data Logging**  
  Write incoming samples to an SD card or internal flash for offline analysis or archiving.

- **Visualization**  
  Expose a small web server or use a serial plotting tool to display amplitude or spectrogram data from the PCM stream.

- **Multi-Channel Audio**  
  Investigate I2S TDM (Time-Division Multiplexing) if you need more channels than standard stereo.


