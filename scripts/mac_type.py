from collections.abc import Sequence 

#------------------------------------------------------------------------------
class MAC(Sequence):
    """MAC Address representation class.
    This class provides a versatile way to handle MAC (Media Access Control) addresses in various formats.
    It supports initialization from strings (XX:XX:XX:XX:XX:XX format), bytes, bytearray, or list of integers.
    The class implements sequence protocol and provides common comparison operations.
    Args:
        addr (Union[str, bytes, bytearray, list[int]]): MAC address in one of the supported formats:
            - str: Colon-separated hexadecimal string (e.g., "00:11:22:33:44:55")
            - bytes: Raw bytes object containing MAC address
            - bytearray: Byte array containing MAC address
            - list[int]: List of integers representing MAC address bytes
    Raises:
        ValueError: If the input format is not supported or invalid
    Properties:
        addr (bytes): Internal storage of MAC address as bytes
    """
    def __init__(self, addr: str|bytes|bytearray|list[int]|tuple):
        if(isinstance(addr, str)):
            self.addr = bytes([int(x, 16) for x in addr.split(':')])
        elif(isinstance(addr, bytes)):
            self.addr = addr
        elif(isinstance(addr, bytearray)):
            self.addr = bytes(addr)
        elif(isinstance(addr, list)):
            self.addr = bytes(addr)
        elif(isinstance(addr, tuple)):
            self.addr = bytes(addr)
        else:
            raise ValueError("Invalid MAC Address: ", type(addr))
        super().__init__()
    
    def __str__(self):
        return ':'.join([f"{x:02X}" for x in self.addr])

    def __bytes__(self):
        return self.addr
    
    def __eq__(self, other):
        return self.addr == other.addr
    
    def __ne__(self, other):
        return self.addr != other.addr
    
    def __len__(self):
        return len(self.addr)
    
    def __getitem__(self, key):
        return self.addr[key]
    
    def __repr__(self):
         return f"MAC({self.__str__()})"