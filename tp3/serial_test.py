#!/usr/bin/env python3

import serial
import threading
import time
import re
import sys
import os

# Configure the serial connection
PORT = "rfc2217://localhost:4000"
BAUD_RATE = 9600

def parse_input(input_str):
    """Parse the input string into bytes to send."""
    tokens = input_str.split()
    data = bytearray()
    
    for token in tokens:
        # Check if token is a hex value with specified byte length
        hex_with_length = re.match(r'0x([0-9A-Fa-f]+)/(\d+)', token)
        if hex_with_length:
            hex_value = hex_with_length.group(1)
            num_bytes = int(hex_with_length.group(2))
            value = int(hex_value, 16)
            # Add value in little endian format with specified byte length
            data.extend(value.to_bytes(num_bytes, byteorder='little'))
        # Check if token is a regular hex value
        elif token.startswith('0x'):
            value = int(token[2:], 16)
            # Default to the minimum number of bytes required for this value
            num_bytes = (value.bit_length() + 7) // 8
            if num_bytes == 0:
                num_bytes = 1  # Ensure at least 1 byte
            data.extend(value.to_bytes(num_bytes, byteorder='little'))
        # Otherwise, treat as ASCII
        else:
            data.extend(token.encode('ascii'))
    
    return data

def print_prompt():
    """Print the input prompt."""
    sys.stdout.write(f"\rSend: {current_input}")
    sys.stdout.flush()

def receive_data(ser):
    """Thread for receiving and displaying data."""
    global current_input
    buffer = ""
    
    while True:
        try:
            if ser.in_waiting > 0:
                byte_data = ser.read(ser.in_waiting)
                if byte_data:
                    # Clear current line
                    sys.stdout.write('\r' + ' ' * (len("Send: ") + len(current_input)) + '\r')
                    
                    # Print each byte in hex format on the same line
                    hex_values = ' '.join([f"{byte:02X}" for byte in byte_data])
                    print(hex_values)
                    
                    # Restore the prompt
                    print_prompt()
                    
            time.sleep(0.01)  # Avoid high CPU usage
        except Exception as e:
            print(f"\rError reading from serial: {e}")
            break

if __name__ == "__main__":
    try:
        # Connect to the serial port
        ser = serial.Serial(port='/dev/ttyACM0', baudrate=9600, bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1,
            xonxoff=False, rtscts=False, dsrdtr=False, inter_byte_timeout=None,
            exclusive=None)
        print(f"Connected to {PORT} at {BAUD_RATE} baud")
        
        # Start the receiver thread
        current_input = ""
        receiver_thread = threading.Thread(target=receive_data, args=(ser,), daemon=True)
        receiver_thread.start()
        
        # Main loop for sending data
        while True:
            current_input = ""
            print_prompt()
            
            # Read input character by character to handle the prompt correctly
            while True:
                char = sys.stdin.read(1)
                if char == '\n':
                    break
                current_input += char
                sys.stdout.write(char)
                sys.stdout.flush()
            
            # Process and send the input
            if current_input.lower() == 'exit':
                break
                
            data_to_send = parse_input(current_input)
            if data_to_send:
                ser.write(data_to_send)
                ser.flush()
                # Print a blank line after sending to separate from potential incoming data
                print()
                
    except KeyboardInterrupt:
        print("\nProgram terminated by user")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serial connection closed")