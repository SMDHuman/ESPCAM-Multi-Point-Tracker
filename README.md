# ESPCAM Multi-Point Tracker
A budget-friendly experimental project for 3D position tracking using infrared light points and ESP32-CAM modules. Multiple cameras communicate via ESP-NOW protocol to calculate 2D vectors of tracked points, enabling 3D positioning capabilities on further use.

## Features

- Multi-point infrared tracking using ESP32-CAM modules
- Seamless inter-camera communication via ESP-NOW protocol
- Real-time performance at ~10 FPS under default configuration settings

## Hardware Requirements

- ESP32-CAM Board(s)
- IR LED targets or markers
- USB-TTL programmer
- 5V power supply (or Batteries)

## Software Dependencies

- PlatformIO (on VSCode Recommended)
- Python 3.11 >=

## How Does It Work?
The main focus of this project is the firmware to utilize the image processing capabilities of ESP32-CAM module. The firmware utilizes both cores of the ESP32. One is for all communication between peers, wifi and serial. User request or set values from serial communication channel. All other operations between peers done seamless by the ESP32. 

Other core process just frames that camera captures. The main 'seperation' process of bright points to track is splits to 6 steps. 

1. **Image Capture**
Raw image acquisition from ESP32-CAM. The configuration variables, such as `cam_brightness` or `cam_contrast`, determine the result of this image. The image buffer uses a 1-byte-per-pixel grayscale format for simplicity and efficiency.
2. **Light Level Filter**
Threshold filtering to identify bright spots. 
3. **Erode** 
Noise reduction and spot isolation. Small and lone pixels cleaned here.
4. **Dilate**: 
Inflate remaining valid spots. Also near spots merge here.
5. **Flood Fill Seperation**: 
Identify separate bright regions with floodfill algorithm. Each island has a unique number.
6. **Vectorizaton**: 
Calculate the size occupation of each island to get a rectangle vectors of it.

<img src="/images/Filter_Layers.png" width="400" title="hover text">

---
## Configuration
### Camera Settings
- Brightness, Contrast, Saturation
- IR Filter Mode
- White Balance
- Resolution and Frame Rate

### Tracker Settings
- IR Threshold Values
- Noise Reduction Filters
- Image Orientation
- Point Detection Parameters

---
## API Reference
To Communicate with the tracker modules, you can use `tracker_interface.py` module with your python codes. 
> **Warning:** This Python module depends on other files in this repository, such as some header files from firmware. To use it, ensure your Python import path includes the cloned repository. For example:
> ```python
> import sys
> sys.path.append("path/to/cloned/repository")
> ```

### Dependent Libraries To tracker_interface.py
- pyserial
- pyparsing

### Usage
To use tracker_interface.py to utilize all tracker's capabilities, you'll need to create a `Tracker_Interface()` object first, which serves as the main interface for communicating with and configuring the tracker modules.
```python 
tracker = Tracker_Interface("Port Name", baudrate = 115200, timeout = 1)
```
After that you are ready to go.
### Methods
```python
# Get total processed frame count from specified peer
tracker.get_frame_count(frm = 0) -> int

# Get number of connected peers
tracker.get_peer_count() -> int

# Get list of peers with their IDs and MAC addresses
tracker.get_peer_list() -> list[tuple[int, MAC]]

# Get detected point coordinates as rectangles => [(x1, y1, x2, y2), ...]
tracker.get_points(frm = 0) -> list[tuple[int, int, int, int]]
# Returns a list of rectangles representing detected points, where each tuple contains:
# (x1, y1) - Top-left corner coordinates of the rectangle
# (x2, y2) - Bottom-right corner coordinates of the rectangle

# Set configuration parameters for specific peer
tracker.set_config(peer_id: int = 0, **configs) -> bool

# Reload configuration for specific peer
tracker.reload_config(peer_id = 0) -> bool

# Get specific configuration value
tracker.get_config(key_name, peer_id = 0) -> int

# Reboot the tracker system
tracker.reboot() -> None
```

#### Example Usage
```python
from tracker_interface import Tracker_Interface

tracker = Tracker_Interface("COM3")  # "COM3" refers to the serial port of the tracker device

# Configure every peer with same settings
# Note: Normally, all configuration parameters persist in their flash memory, so you only need to perform this step
#       once for every new peer. However, it is not a resource-heavy process.
peers = tracker.get_peer_list()
for peer_id, peer_mac in peers:
  tracker.set_config(peer_id, cam_brightness = -1, cam_contrast = 1)
  tracker.reload_config(peer_id)

# Get points from every peer
while(True):
  for peer_id, peer_mac in tracker.get_peer_list():
    print("====")
    print("Points from ", peer_id)
    for rect in tracker.get_points(peer_id):
      print("----")
      print(f"x: {rect[0]}, y: {rect[1]}")
      print(f"size: {rect[2]-rect[0]}x{rect[3]-rect[1]}")
```

### Config Variables
  - Camera Variables
    - `cam_brightness`  = 0 (-2 to 2)
    - `cam_contrast`    = 0 (-2 to 2)
    - `cam_saturation`  = 0 (-2 to 2)
    - `cam_spec_effect` = 2 (0 to 6 [0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia])
    - `cam_whitebal`    = 1 (0 = disable , 1 = enable)
    - `cam_awb_gain`    = 1 (0 = disable , 1 = enable)
    - `cam_wb_mode`     = 0 (0 to 4 !if awb_gain enabled! [0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home])
    - `cam_expo_ctrl`   = 1 (0 = disable , 1 = enable)
    - `cam_aec2`        = 0 (0 = disable , 1 = enable)
    - `cam_ae_level`    = 0 (-2 to 2)
  - Tracker Variables
    - `trk_filter_min`  = 235
    - `trk_erode`       = 1
    - `trk_erode_mul`   = 3
    - `trk_erode_div`   = 10
    - `trk_dilate`      = 5
    - `trk_flip_x`      = 0 (0 = disable , 1 = enable)
    - `trk_flip_y`      = 0 (0 = disable , 1 = enable)
  - Module Variables
    - `cfg_restore`     = false (If True, device restore all variables to default on next reboot)
    - `serial_baudrate` = 115200
    - `espnet_mode`     = 0  (0 = BOTH, 1 = HOST ONLY, 2 = CLIENT ONLY)


---
## Troubleshooting

### Common Issues
- Camera Not Detected
  - Check power supply stability:
    - Ensure the power supply provides a stable 5V output.
- Poor Point Detection
  - Adjust IR threshold
  - Check ambient lighting
  - Ensure IR markers are functioning correctly
  - Adjust the camera's position for better visibility of the markers
    - Confirm that the PSRAM is enabled in the firmware settings (e.g., in `sdkconfig` for ESP-IDF or `platformio.ini` for PlatformIO).
    - Check the ESP32-CAM datasheet to ensure the PSRAM is compatible and properly connected.
    - Reflash the firmware if necessary, ensuring the correct PSRAM settings are applied.

---
## License

This project is licensed under the MIT License. See the [LICENSE](https://opensource.org/licenses/MIT) file for details.

The MIT License permits anyone to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the software, provided that the original copyright notice and permission notice are included in all copies or substantial portions of the software.
