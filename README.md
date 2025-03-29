# ESPCAM-Multi-Point-Tracker
Multi point tracker system using multiple ESP32-Cam modules for real-time bright point detection and tracking. This project still developed under different V2 branch. This main branch is older version of this project and not maintained. 

## Features
- Real-time tracking of multiple bright points
- Distributed camera system using ESP32-CAM modules
- Advanced image processing pipeline
- 3D positioning and calibration

## Hardware Requirements
- ESP32-CAM modules

## How Does It Work?
The system uses a sophisticated image processing pipeline consisting of 6 steps:

1. Image Capture (1/6)
2. Light Level Filter (2/6)
3. Erode (3/6)
4. Dilate (4/6)
5. Flood Fill and Find Separate Spots (5/6)
6. Center Point Calculation (6/6)

<img src="/images/Filter_Layers.png" width="500" alt="Image Processing Pipeline">

### Image Processing Pipeline Details
Each frame goes through the following processing steps:
1. **Image Capture**: Raw image acquisition from ESP32-CAM
2. **Light Level Filter**: Threshold filtering to identify bright spots
3. **Erode**: Noise reduction and spot isolation
4. **Dilate**: Enhance remaining valid spots
5. **Flood Fill**: Identify separate bright regions
6. **Center Detection**: Calculate center points using extrema pixels

## License
MIT License

## Contributing
Contributions are welcome! Please feel free to submit a Pull Request.
