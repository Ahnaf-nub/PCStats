import serial
import psutil
import time
from datetime import datetime
import serial.tools.list_ports

# Detect available serial port
def detect_serial_port():
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        if "USB" in port.description or "COM" in port.description:
            return port.device
    return None

# Get system stats
def get_system_stats():
    cpu_usage = psutil.cpu_percent(interval=1)
    mem_free = psutil.virtual_memory().available / 1024 / 1024  # in MB
    ram_usage = psutil.virtual_memory().percent
    disk_usage = psutil.disk_usage('/').percent
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M")
    current_day = datetime.now().strftime("%A")
    return cpu_usage, mem_free, ram_usage, disk_usage, current_time, current_day

# Send data over serial
def send_data(ser):
    cpu_usage, mem_free, ram_usage, disk_usage, current_time, current_day = get_system_stats()
    data = f"{cpu_usage},{mem_free},{ram_usage},{disk_usage},{current_time},{current_day}\n"
    print(f"Sending: {data.strip()}")  # Debug output
    ser.write(data.encode())  # Send data to Arduino

def main():
    port = detect_serial_port()
    if not port:
        print("No serial port found!")
        return

    ser = serial.Serial(port, 9600, timeout=1)
    time.sleep(2)  # Allow time for the connection to stabilize

    print(f"Connected to {port}")
    try:
        while True:
            send_data(ser)
            time.sleep(2)  # Send data every 2 seconds
    except KeyboardInterrupt:
        print("\nExiting program.")
    finally:
        ser.close()

if __name__ == "__main__":
    main()
