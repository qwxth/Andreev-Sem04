import usb.core
import usb.util
import time
import os
import struct
from functools import reduce

IMAGE_WIDTH = 64
IMAGE_HEIGHT = 80
HEADER_OFFSET = 4
PIXEL_STRIDE = 2 
# Sensor returns 2 bytes per pixel: 64*80*2 = 10240 + 6 header

VENDOR_ID = 0x2808
PRODUCT_ID = 0xc652

OUT_ENDPOINT = 0x03
IN_ENDPOINT = 0x81

RESPONSE_ACK_LENGTH = 7
IMAGE_DATA_LENGTH = 10246

PAYLOAD_STATUS_CHECK = bytes.fromhex("37010101")
PAYLOAD_PREPARE_SENSOR = bytes.fromhex("800201")
PAYLOAD_TRIGGER_CAPTURE = bytes.fromhex("827301")
PAYLOAD_REQUEST_IMAGE = bytes.fromhex("81")

class FocaltechDevice:
    def __init__(self, vendor_id=VENDOR_ID, product_id=PRODUCT_ID):
        self.vendor_id = vendor_id
        self.product_id = product_id
        self.dev = None

    def open(self):
        self.dev = usb.core.find(idVendor=self.vendor_id, idProduct=self.product_id)
        if self.dev is None:
            raise ValueError(f"Device not found: Vendor ID {hex(self.vendor_id)}, Product ID {hex(self.product_id)}")
        
        print(f"Focaltech device found: {self.dev.product}")

        self.dev.set_configuration()
        
        try:
            self.dev.reset()
        except usb.core.USBError as e:
            print(f"Error resetting device: {e}")

        self._detach_kernel_driver()
        usb.util.claim_interface(self.dev, 0)
        print("Interface 0 claimed.")

    def close(self):
        if self.dev:
            usb.util.release_interface(self.dev, 0)
            usb.util.dispose_resources(self.dev)
            print("Interface 0 released and resources disposed.")
            self.dev = None

    def _detach_kernel_driver(self):
        for cfg in self.dev:
            for intf in cfg:
                if self.dev.is_kernel_driver_active(intf.bInterfaceNumber):
                    try:
                        self.dev.detach_kernel_driver(intf.bInterfaceNumber)
                    except usb.core.USBError as e:
                        print(f"Could not detach kernel driver from interface {intf.bInterfaceNumber}: {e}")

    def _build_command(self, payload):
        length = len(payload)
        checksum = reduce(lambda x, y: x ^ y, [length] + list(payload))
        command = bytes([0x02, 0x00, length]) + payload + bytes([checksum])
        return command

    def send_command(self, payload):
        command = self._build_command(payload)
        try:
            self.dev.write(OUT_ENDPOINT, command)
        except usb.core.USBError as e:
            print(f"Error sending command: {e}")
            raise

    def read_response(self, expected_length, timeout=1000):
        try:
            response = self.dev.read(IN_ENDPOINT, expected_length, timeout=timeout)
            return bytes(response)
        except usb.core.USBError as e:
            if e.errno != 110:
                print(f"Error reading response: {e}")
            return None

    def capture_image_data(self):
        self.send_command(PAYLOAD_STATUS_CHECK)
        self.read_response(RESPONSE_ACK_LENGTH)
        self.send_command(PAYLOAD_PREPARE_SENSOR)
        self.read_response(RESPONSE_ACK_LENGTH)
        self.send_command(PAYLOAD_TRIGGER_CAPTURE)
        self.read_response(RESPONSE_ACK_LENGTH)
        self.send_command(PAYLOAD_REQUEST_IMAGE)
        image_data = self.read_response(IMAGE_DATA_LENGTH)
        return image_data

def hsv_to_rgb(hue, sat=1.0, val=1.0):
    """
    Convert HSV color to RGB.
    hue: 0..360, sat: 0..1, val: 0..1
    Returns (r, g, b) each in 0..255
    """
    h = hue / 60.0
    c = val * sat
    x = c * (1 - abs(h % 2 - 1))
    m = val - c

    if 0 <= h < 1:
        r, g, b = c, x, 0
    elif 1 <= h < 2:
        r, g, b = x, c, 0
    elif 2 <= h < 3:
        r, g, b = 0, c, x
    elif 3 <= h < 4:
        r, g, b = 0, x, c
    elif 4 <= h < 5:
        r, g, b = x, 0, c
    elif 5 <= h < 6:
        r, g, b = c, 0, x
    else:
        r, g, b = 0, 0, 0

    return int((r + m) * 255), int((g + m) * 255), int((b + m) * 255)

def colorize_8bit(value_8bit):
    """
    Map 0..255 intensity to RGB using HSV gradient:
    low (0) -> blue (240°), mid (128) -> green (120°), high (255) -> red (0°)
    """
    t = value_8bit / 255.0          # 0=low, 1=high
    hue = 240 * (1 - t)             # 240° (blue) -> 0° (red)
    return hsv_to_rgb(hue, sat=1.0, val=1.0)

def save_ppm_from_rgb(rgb_bytes, width, height, filename):
    """Save RGB raw bytes as PPM file (P6 format)."""
    header = f"P6\n{width} {height}\n255\n"
    try:
        with open(filename, "wb") as f:
            f.write(header.encode('ascii'))
            f.write(rgb_bytes)
        print(f"Color image saved as {filename}")
    except Exception as e:
        print(f"Error saving PPM {filename}: {e}")

def main():
    device = FocaltechDevice()
    try:
        device.open()

        while True:
            print("Capturing image...")
            image_data = device.capture_image_data()
            if not image_data:
                time.sleep(0.1)
                continue
            
            width = IMAGE_WIDTH
            height = IMAGE_HEIGHT
            num_pixels = width * height
            
            # Extract 16-bit raw pixel values (little-endian)
            raw_16bit = []
            for i in range(num_pixels):
                offset = HEADER_OFFSET + i * PIXEL_STRIDE
                if offset + 1 < len(image_data):
                    # Combine two bytes into 16-bit value (little-endian)
                    pixel = image_data[offset] | (image_data[offset + 1] << 8)
                    raw_16bit.append(pixel)
                else:
                    raw_16bit.append(0)
            
            # Normalize 16-bit to 8-bit using full range (or just shift right 8 bits)
            # Here we use shift to keep faster, but min/max normalization would be better
            # For simplicity shift right 8 bits (assume sensor uses 16-bit range)
            # If you want better contrast, you can compute min/max of the frame and map.
            min_val = min(raw_16bit)
            max_val = max(raw_16bit)
            if max_val == min_val:
                max_val = min_val + 1
            # Normalize to 0..255
            gray_8bit = [int((p - min_val) * 255 / (max_val - min_val)) for p in raw_16bit]
            
            # Apply colormap to get RGB
            rgb_data = bytearray()
            for gray in gray_8bit:
                r, g, b = colorize_8bit(gray)
                rgb_data.extend([r, g, b])
            
            save_ppm_from_rgb(rgb_data, width, height, "live_view_color.ppm")
            print("Color image updated.")
            time.sleep(0.1)

    except KeyboardInterrupt:
        print("\nStopping...")
    except (ValueError, usb.core.USBError) as e:
        print(f"An error occurred: {e}")
    finally:
        device.close()

if __name__ == "__main__":
    main()