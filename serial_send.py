import serial
import psutil
import time
import serial.tools.list_ports
import platform
from datetime import datetime

# Detect the available serial ports
def detect_serial_port():
    system_platform = platform.system().lower()
    available_ports = list(serial.tools.list_ports.comports())

    if system_platform == 'windows':
        # On Windows, the COM ports will be listed as "COMx"
        for port, desc, _ in available_ports:
            if 'usb' in desc.lower():
                return port  # Return the first USB serial port found

    elif system_platform == 'linux' or system_platform == 'darwin':
        # On Linux and macOS, the serial ports will typically be like /dev/ttyUSBx or /dev/ttyACMx
        for port, desc, _ in available_ports:
            if 'usb' in desc.lower():
                return port  # Return the first USB serial port found

    # If no serial port is found, return None
    return None

# Get system stats (CPU usage, free memory, total memory, RAM usage percentage, disk usage, time, and date)
def get_system_stats():
    # Get CPU usage percentage
    cpu_usage = psutil.cpu_percent(interval=1)

    # Get free memory in MB
    mem_free = psutil.virtual_memory().available / 1024 / 1024

    # Get total memory in MB
    mem_total = psutil.virtual_memory().total / 1024 / 1024

    # Get RAM usage percentage
    ram_usage = psutil.virtual_memory().percent

    # Get disk usage percentage
    disk_usage = psutil.disk_usage('/').percent

    # Get current time and date
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    return cpu_usage, mem_free, mem_total, ram_usage, disk_usage, current_time

# Send data to Arduino
def send_data_to_arduino(ser, cpu_usage, mem_free, mem_total, ram_usage, disk_usage, current_time):
    data = f"{cpu_usage},{mem_free},{mem_total},{ram_usage},{disk_usage},{current_time}\n"  # Format the data
    ser.write(data.encode())  # Send the data to the Arduino over serial

def main():
    # Detect the serial port automatically
    port = detect_serial_port()
    
    if port is None:
        print("No compatible serial device found.")
        return

    print(f"Found device on {port}.")
    
    # Open the serial port
    ser = serial.Serial(port, 9600, timeout=1)
    time.sleep(2)  # Wait for the connection to establish

    try:
        while True:
            cpu_usage, mem_free, mem_total, ram_usage, disk_usage, current_time = get_system_stats()
            print(f"Sending Data: CPU Usage: {cpu_usage}%, Free Mem: {mem_free} MB, Total Mem: {mem_total} MB, RAM Usage: {ram_usage}%, Disk Usage: {disk_usage}%, Time: {current_time}")
            send_data_to_arduino(ser, cpu_usage, mem_free, mem_total, ram_usage, disk_usage, current_time)
            time.sleep(1)  # Send data every 1 second
    except KeyboardInterrupt:
        print("\nProgram interrupted by user.")
    finally:
        ser.close()  # Close the serial port when done

if __name__ == "__main__":
    main()
