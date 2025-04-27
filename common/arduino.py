import serial
import time
import os.path
import threading


def get_arduino_port():
    if os.path.exists('/dev/arduino'):
        return '/dev/arduino'
    
    # Puertos posibles
    ports = ['/dev/ttyACM0', '/dev/ttyACM1', '/dev/ttyUSB0', '/dev/ttyUSB1']
    for port in ports:
        if os.path.exists(port):
            return port
    
    # Si no está conectado, intentamos el simulador
    return 'rfc2217://localhost:4000'


class SerialConnection:
    def __init__(self, baudrate=115200, timeout=1, reconnect_interval=2, verbose=False):
        self.baudrate = baudrate
        self.timeout = timeout
        self.serial = None
        self.port = None
        self.verbose = verbose
        self.reconnect_interval = reconnect_interval
        self.is_connected = False
        self.lock = threading.RLock()
        self._stop_monitor = threading.Event()
        self._monitor_thread = None
        self.connect()
    
    def log(self, message):
        if self.verbose:
            print(f"Arduino: {message}")
    
    def connect(self):
        with self.lock:
            connected = self._connect()
            
            # Comenzar el monitoreo de la conexión
            if not self._monitor_thread:
                self._stop_monitor.clear()
                self._monitor_thread = threading.Thread(
                    target=self._connection_monitor,
                    daemon=True
                )
                self._monitor_thread.start()
                self.log("Connection monitoring started")
                
            return connected
    
    def _connect(self):
        try:
            port = get_arduino_port()
            
            if self.serial and self.serial.is_open:
                self.serial.close()
            
            if port.startswith("rfc2217://"):
                self.log(f"No Arduino found, attempting connection to Wokwi simulator at {port}")
            
            self.serial = serial.serial_for_url(port, self.baudrate, timeout=self.timeout)
            self.port = port
            self.is_connected = True
            self.log(f"Connected to {self.port}")
            return True
        except (serial.SerialException, PermissionError) as e:
            self.log(f"Connection error: {e}")
            self.is_connected = False
            return False
    
    # monitorea el estado de la conexión en otro hilo
    def _connection_monitor(self):
        while not self._stop_monitor.is_set():
            if not self.is_connected:
                self.log("Connection monitor attempting reconnection...")
                with self.lock:
                    self._connect()
            
            elif self.serial and self.serial.is_open:
                try:
                    self.serial.in_waiting
                except (serial.SerialException, OSError) as e:
                    self.log(f"Connection monitor detected issue: {e}")
                    self.is_connected = False
            
            self._stop_monitor.wait(self.reconnect_interval)
    
    def stop_monitoring(self):
        if self._monitor_thread:
            self._stop_monitor.set()
            self._monitor_thread.join(timeout=1)
            self._monitor_thread = None
    
    def read(self, bytes=1):
        try:
            return self.serial.read(bytes)
        except Exception as e:
            self.is_connected = False
            raise Exception("Failed reading data from Arduino")
    
    def write(self, data):
        try:
            self.serial.write(data)
        except Exception as e:
            self.is_connected = False
            raise Exception("Failed writing data to Arduino")
    
    def in_waiting(self):
        try:
            return self.serial.in_waiting
        except Exception as e:
            self.is_connected = False
            raise Exception("Failed checking if there's data available from Arduino")
    
    def close(self):
        self.stop_monitoring()
        
        with self.lock:
            if self.serial and self.serial.is_open:
                try:
                    self.serial.close()
                except Exception as e:
                    self.log(f"Error closing connection: {e}")
                self.log("Connection closed")
            
            self.is_connected = False
    
    # Para utilizar "with" de tal forma que se utilice el lock
    def __enter__(self):
        self.lock.acquire()
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.lock.release()
        return False


# prueba
if __name__ == "__main__":
    arduino = SerialConnection(baudrate=115200, verbose=True)
    
    try:
        while True:
            try:
                if arduino.in_waiting() > 0:
                    with arduino:
                        data = arduino.read()
                        if data:
                            print(f"Received: {data}")
                time.sleep(0.1)
            except Exception as e:
                print(e)
                time.sleep(1)
    

    except KeyboardInterrupt:
        print("Program terminated")
    finally:
        arduino.close()
