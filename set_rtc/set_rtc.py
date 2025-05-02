import serial
import datetime
import time
import sys

# --- Configuration ---
# Try to auto-detect or set manually
SERIAL_PORT = 'COM7' 
# Common ports:
# Windows: 'COM1', 'COM3', 'COM5', etc.
# Linux: '/dev/ttyACM0', '/dev/ttyUSB0', etc.
# macOS: '/dev/cu.usbmodemXXXX', '/dev/cu.usbserial-XXXX', etc.

BAUD_RATE = 9600     # Must match Arduino sketch
TIMEOUT_SEC = 5      # Max time to wait for response
CONNECT_DELAY_SEC = 2 # Allow time for Arduino reset after connection

# --- Auto-detect Serial Port (Optional but Recommended) ---
def find_arduino_port():
    """Tries to find a likely Arduino serial port."""
    import serial.tools.list_ports
    ports = serial.tools.list_ports.comports()
    arduino_ports = []
    print("Available serial ports:")
    for port, desc, hwid in sorted(ports):
        print(f"- {port}: {desc} [{hwid}]")
        # Look for common Arduino identifiers in description or hardware ID
        if 'arduino' in desc.lower() or 'usb-serial' in desc.lower() or 'ch340' in desc.lower() \
           or 'cp210x' in desc.lower() or 'ttyacm' in port.lower() or 'usbmodem' in port.lower():
            arduino_ports.append(port)
    
    if not arduino_ports:
        print("\nWarning: Could not auto-detect an Arduino port.")
        return None
    elif len(arduino_ports) == 1:
        print(f"\nAuto-detected Arduino port: {arduino_ports[0]}")
        return arduino_ports[0]
    else:
        print("\nMultiple possible Arduino ports found:")
        for i, p in enumerate(arduino_ports):
            print(f"  {i+1}: {p}")
        while True:
            try:
                choice = input(f"Please select port number (1-{len(arduino_ports)}): ")
                index = int(choice) - 1
                if 0 <= index < len(arduino_ports):
                    print(f"Selected: {arduino_ports[index]}")
                    return arduino_ports[index]
                else:
                    print("Invalid selection.")
            except ValueError:
                print("Invalid input. Please enter a number.")
            except EOFError:
                print("\nOperation cancelled by user.")
                return None


# --- Main Script ---
if __name__ == "__main__":
    print("--- Arduino RTC Sync Script ---")

    if SERIAL_PORT is None:
        SERIAL_PORT = find_arduino_port()

    if SERIAL_PORT is None:
        print("\nError: Serial port not specified or detected. Exiting.")
        # Optional: Prompt user to enter port manually
        # SERIAL_PORT = input("Please enter the Arduino serial port (e.g., COM3 or /dev/ttyACM0): ")
        # if not SERIAL_PORT:
        sys.exit(1) # Exit if no port is found/selected


    try:
        print(f"Attempting to connect to {SERIAL_PORT} at {BAUD_RATE} baud...")
        # Use 'with' statement for automatic resource management (closes port)
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT_SEC) as ser:
            print(f"Connected. Waiting {CONNECT_DELAY_SEC}s for Arduino to initialize...")
            time.sleep(CONNECT_DELAY_SEC) # Give Arduino time to reset

            # Optional: Read initial message from Arduino (if it sends one)
            if ser.in_waiting > 0:
                 initial_message = ser.read(ser.in_waiting).decode('utf-8', errors='ignore').strip()
                 print(f"Received from Arduino: {initial_message}")
                 # You could check here if the message indicates readiness

            # Get current system time
            now = datetime.datetime.now()

            # Format time string for Arduino (T<YYYY>,<MM>,<DD>,<hh>,<mm>,<ss>\n)
            # Use :02 to ensure two digits for month, day, hour, minute, second
            time_string = f"T{now.year},{now.month:02},{now.day:02},{now.hour:02},{now.minute:02},{now.second:02}\n"
            
            print(f"Sending time: {time_string.strip()}")

            # Send data (encode string to bytes)
            ser.write(time_string.encode('utf-8'))
            ser.flush() # Ensure data is sent immediately

            print("Time sent. Waiting for confirmation from Arduino...")

            # Wait for response (e.g., "OK")
            response_bytes = ser.readline() # Reads until '\n' or timeout
            
            if response_bytes:
                response = response_bytes.decode('utf-8', errors='ignore').strip()
                print(f"Arduino response: {response}")
                if "OK" in response:
                    print("Successfully synchronized RTC!")
                else:
                    print("Warning: Received unexpected response or failure message from Arduino.")
            else:
                print("Warning: No response received from Arduino within timeout.")

        print(f"Serial port {SERIAL_PORT} closed.")

    except serial.SerialException as e:
        print(f"\nError: Could not open or communicate on serial port {SERIAL_PORT}.")
        print(f"Details: {e}")
        print("Please check:")
        print(" - If the Arduino is connected.")
        print(" - If the correct serial port is selected.")
        print(" - If another program (like Arduino IDE Serial Monitor) is using the port.")
        sys.exit(1)
    except Exception as e:
        print(f"\nAn unexpected error occurred: {e}")
        sys.exit(1)

    print("--- Script finished ---")