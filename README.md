# ESPCAM-Multi-Point-Tracker
 Multi point tracker system with multiple ESP32-Cam modules. 

## How Does It Work?
The method we using here to locate and track the bright points on camera is consist of 4 steps. 
1. Light Level Filter (2/6)
2. Erode (3/6)
3. Dilate (4/6)
4. Fool Fill and Find Separate Spots (5/6)
<img src="/images/Filter_Layers.png" width="500" title="hover text">
In this example, step 1 is to capture image and step 6 is to locate center point by finding most left-top and right-bottom pixels of that seperated spot. 

---
### Readme WIP...
