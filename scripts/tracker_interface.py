import serial
from serial.threaded import ReaderThread, Protocol
import time, os, sys, struct
import queue
import serial.threaded
from enum_parser import enum_parser
from pyslip import SLIP
from mac_type import MAC

class Tracker_Interface(Protocol):
    def _self_(self):
        return self

    def __init__(self, port: str, baudrate: int = 115200, timeout: int = 1):
        # import the enums from the header file
        path = os.path.join(sys.path[0], "..", "include", "command_handler.h")
        with open(path) as f:
            enums = enum_parser(f.read())
        self.__dict__.update(enums["CMD_PACKET_TYPE_E"])

        # Initialize the SLIP protocol and serial port
        self.slip = SLIP()
        self.checksum = 0
        self.esp = serial.Serial(port, baudrate = baudrate, timeout=timeout)
        self.esp_thread = ReaderThread(self.esp, self._self_)
        self.esp_thread.start()
        self.rx_buffer = queue.Queue(1)

    # Called when data is received By Serial.ReaderThread
    def data_received(self, data: bytes):
        for byte in data:
            self.slip.push(byte)
            #...
            if(self.slip.in_wait() > 0):
                packet = self.slip.get()
                self.handle_response(packet)

    # Sends data to the ESP32-CAM using SLIP protocol
    def slip_send(self, data: int|bytes|bytearray|list[int], check_checksum: bool = True):
        if(isinstance(data, bytes) or isinstance(data, list) or isinstance(data, bytearray)):
            for byte in data:
                self.slip_send(byte)
        elif(isinstance(data, int)):
            self.checksum += (data + 1)*check_checksum
            if(data == SLIP.END):
                self.esp.write(bytes([SLIP.ESC, SLIP.ESC_END]))
            elif(data == SLIP.ESC):
                self.esp.write(bytes([SLIP.ESC, SLIP.ESC_ESC]))
            else:
                self.esp.write(bytes([data]))
        self.checksum %= 2**32

    # Sends the end of the SLIP packet
    # and the checksum to the ESP32-CAM
    def slip_end(self):
        self.slip_send(self.checksum & 0xFF, False)
        self.slip_send((self.checksum >> 8) & 0xFF, False)
        self.slip_send((self.checksum >> 16) & 0xFF, False)
        self.slip_send((self.checksum >> 24) & 0xFF, False)
        self.checksum = 0
        self.esp.write(bytes([SLIP.END]))
    
    # Handles the response from the ESP32-CAM
    # and puts it in the RX buffer
    def handle_response(self, packet: bytes):
        if(len(packet) == 0): return
        tag = packet[0]
        data = packet[1:]
        self.rx_buffer.put((tag, data))

    def pop_rx_buffer(self, timeout: int = 0.2) -> tuple[int, bytes]:
        """Pop the next response from the RX buffer.
        Returns:
            tuple[int, bytes]: The tag and data of the response.
        """
        try:
            return self.rx_buffer.get(timeout=timeout)
        except queue.Empty:
            return None, None

    #--------------------------------------------------------------------------
    # Command functions
    #--------------------------------------------------------------------------
    def get_frame_count(self, frm: int = 0) -> int:
        """Get the frame count from the tracker.
        Args:
            frm (int): The frame number to get the count from. Default is 0."
            "Returns:"
            "int: The frame count from the tracker."
            "If the frame count is not available, returns -1."
        """
        self.slip_send(self.CMD_REQ_FCOUNT)
        self.slip_send(frm)
        self.slip_end()
        #...
        tag, data = self.pop_rx_buffer()
        #...
        if(tag == self.CMD_RSP_FCOUNT):
            rsp_frm = data[0]
            if(rsp_frm != frm): return -1
            fcount = struct.unpack("Q", data[1:])[0]
            return fcount
        else:
            return -1
    #--------------------------------------------------------------------------
    def get_peer_count(self) -> int:
        """Get the peer count from the tracker.
        Returns:
            int: The peer count from the tracker.
            If the peer count is not available, returns -1.
        """
        self.slip_send(self.CMD_REQ_PEERCOUNT)
        self.slip_end()
        #...
        tag, data = self.pop_rx_buffer()
        #...
        if(tag == self.CMD_RSP_PEERCOUNT):
            peer_count = data[0]
            return peer_count
        else:
            return -1
    #--------------------------------------------------------------------------
    def get_peer_list(self) -> list[tuple[int, MAC]]:
        """Get the peer list from the tracker.
        Returns:
            list[tuple[int, bytes]]: The peer list from the tracker.
            If the peer list is not available, returns an empty list.
        """
        self.slip_send(self.CMD_REQ_PEERLIST)
        self.slip_end()
        #...
        tag, data = self.pop_rx_buffer()
        #...
        if(tag == self.CMD_RSP_PEERLIST):
            peer_count = len(data) // 7
            peer_list = []
            for i in range(peer_count):
                peer_info = struct.unpack("B6B", data[i*7:(i+1)*7])
                peer_list.append((peer_info[0], MAC(peer_info[1:])))
            return peer_list
        else:
            return []
    #--------------------------------------------------------------------------
    def get_points(self, frm: int = 0) -> list[tuple[int, int, int, int]]:
        """Get the points from the tracker.
        Args:
            frm (int): The frame number to get the points from. Default is 0.
        Returns:
            list[tuple[int, int, int, int]]: The points as [x1, y1, x2, y2] rectangles from the tracker.
            If the points are not available, returns an empty list.
        """
        self.slip_send(self.CMD_REQ_POINTS)
        self.slip_send(frm)
        self.slip_end()
        #...
        tag, data = self.pop_rx_buffer()
        #...
        if(tag == self.CMD_RSP_POINTS):
            if(data[0] != frm):
                return []
            point_count = (len(data)-1) // 16
            points = []
            for i in range(point_count):
                point_info = struct.unpack("4I", data[1+i*16:1+(i+1)*16])
                points.append(point_info)
            return points
        else:
            return []
    #--------------------------------------------------------------------------
    def set_config(self, peer_id: int = 0, **configs) -> bool:
        """Set the configuration of the tracker.
        Args:
            configs (dict): The configuration to set.
            `cfg_restore     = false`;
            `cam_brightness  = 0` (-2 to 2);
            `cam_contrast    = 0` (-2 to 2);
            `cam_saturation  = 0` (-2 to 2);
            `cam_spec_effect = 2` (0 to 6 [0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia]);
            `cam_whitebal    = 1` (0 = disable , 1 = enable);
            `cam_awb_gain    = 1` (0 = disable , 1 = enable);
            `cam_wb_mode     = 0` (0 to 4 !if awb_gain enabled! [0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home]);
            `cam_expo_ctrl   = 1` (0 = disable , 1 = enable);
            `cam_aec2        = 0` (0 = disable , 1 = enable);
            `cam_ae_level    = 0` (-2 to 2);
            `trk_filter_min  = 235`;
            `trk_erode       = 1`;
            `trk_erode_mul   = 3`;
            `trk_erode_div   = 10`;
            `trk_dilate      = 5`;
            `serial_baudrate = 115200`;
        Returns:
            bool: True if the configuration was set successfully, False otherwise.
        """
        success = True
        for key, value in configs.items():
            self.slip_send(self.CMD_SET_CONFIG)
            self.slip_send(peer_id)
            self.slip_send(key.encode("ascii"))
            self.slip_send(struct.pack("I", value))
            self.slip_end()
            tag, data = self.pop_rx_buffer(0.5)
            if(tag == self.CMD_RSP_ERROR):
                print(f"Couln't set `{key}` config")
                success = False
        return(success)
    
    def get_config(self, key_name: str, peer_id: int = 0) -> int:
        """Get the configuration of the tracker.
        Returns:
            int: The configuration value.
        """
        self.slip_send(self.CMD_REQ_CONFIG)
        self.slip_send(peer_id)
        self.slip_send(key_name.encode("ascii"))
        self.slip_end()
        tag, data = self.pop_rx_buffer()
        #print(tag, data)
        
        if(tag != self.CMD_RSP_CONFIG): return -1
        if(data[0] != peer_id): return -1

        rsp_key = data[1:-4].decode()
        if(rsp_key != key_name): return -1

        return struct.unpack("i", data[-4:])[0]

    def reboot(self):
        """Reboot the ESP32-CAM."""
        self.slip_send(self.CMD_REBOOT)
        self.slip_end()

# --------------------------------------------------------------------------
# Example usage with matplotlib
# --------------------------------------------------------------------------
if(__name__ == "__main__"):
    import matplotlib.pyplot as plt
    from matplotlib.patches import Rectangle

    # Example usage
    tracker = Tracker_Interface("COM6")
    print("Tracker interface initialized")
    time.sleep(1)

    plt.ion()  # Enable interactive mode
    fig, ax = plt.subplots()
    ax.set_xlim(0, 240)  # ESP32-CAM typical resolution
    ax.set_ylim(176, 0)  # Inverted Y-axis for image coordinates
    
    for peers in tracker.get_peer_list():
        tracker.set_config(peers[0],
                           trk_erode = 1,
                           trk_erode_mul = 4,
                           trk_erode_div = 9, 
                           trk_dilate = 3, 
                           trk_filter_min = 230)
    tracker.reboot()


    last_fcount_time = time.time()
    last_fcount = 0
    try:
        while plt.fignum_exists(fig.number):  # Check if figure window exists
            #print("-"*80)
            peer_count = tracker.get_peer_count()
            #print("peer count: ", peer_count)
            #print("peer list: ", tracker.get_peer_list())

            ax.clear()
            ax.set_xlim(0, 240)
            ax.set_ylim(176, 0)
            # Keep track of which peers we've already labeled
            labeled_peers = set()

            for i in range(peer_count+1):
                #print(f" |-----------------")
                #print(f" | peer #{i} ")
                #print("  +-total frames: ", tracker.get_frame_count(i))
                if(i == 0):
                    fcount = tracker.get_frame_count(i)
                    current_fps = round((fcount-last_fcount)/(time.time()-last_fcount_time))
                    #print("fps: ", current_fps)
                    ax.set_title(f'FPS: {current_fps}')
                    last_fcount_time = time.time()
                    last_fcount = fcount
                points = tracker.get_points(i)
                #print("  +-points: ", points)
                #print(f"id{i} trk_erode:", tracker.get_config("trk_erode", i))

                # Draw rectangles for each point
                for rect in points:
                    x1, y1, x2, y2 = rect
                    width = x2 - x1
                    height = y2 - y1
                    # Only add label if this peer hasn't been labeled yet
                    label = f'Peer {i}' if i not in labeled_peers else ""
                    ax.add_patch(Rectangle((x1, y1), width, height, 
                                fill=False, color=f'C{i}', 
                                label=label))
                    labeled_peers.add(i)

            ax.legend()
            plt.draw()
            plt.pause(0.1)
    except KeyboardInterrupt:
        plt.close(fig)