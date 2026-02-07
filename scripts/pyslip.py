import struct

#------------------------------------------------------------------------------
class SLIP:
    END = 0xC0
    ESC = 0xDB
    ESC_END = 0xDC
    ESC_ESC = 0xDD
    def __init__(self, checksum_enable = True):
        self.buffer: list[int] = []
        self.packages: list[bytearray] = []
        self.esc_flag: bool= False
        self.wait_ack: bool = False
        self.checksum_enable: bool = checksum_enable
        self.checksum = 0
    #...
    def push(self, value: int):
        #...
        if(self.esc_flag):
            if(value == self.ESC_END):
                self.buffer.append(self.END)
                self.checksum += self.END + 1
            elif(value == self.ESC_ESC):
                self.buffer.append(self.ESC)
                self.checksum += self.ESC + 1
            elif(value == self.END):
                self.wait_ack = True
            self.esc_flag = False
        #...
        elif(value == self.ESC):
            self.esc_flag = True
        #...
        elif(value == self.END):
            if(self.checksum_enable):
                #...
                if(len(self.buffer) < 4):
                    self.reset_buffer()
                    return
                #...
                for byte in self.buffer[-4:]:
                    self.checksum -= byte+1
                checksum = struct.unpack("I", bytes(self.buffer[-4:]))[0]
                #...
                if(self.checksum != checksum): 
                    self.reset_buffer()
                    return
                self.buffer = self.buffer[:-4]
            self.packages.append(bytearray(self.buffer))
            self.reset_buffer()
        #...
        else:
            self.buffer.append(value)
            self.checksum += value + 1
        self.checksum %= 2**32
    #...
    def get(self) -> bytearray:
        if(len(self.packages) > 0):
            return(self.packages.pop(0))
        return(bytearray())
    #...
    def in_wait(self) -> int:
        return(len(self.packages))
    #...
    def reset_buffer(self):
        self.buffer.clear()
        self.esc_flag = False
        self.wait_ack = False
        self.checksum = 0