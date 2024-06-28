# python 
import socket 
import threading 
 
# Define the target server's IP address and port 
target_ip = "127.0.0.1" 
target_port = 8888 
 
# Define the number of connections/threads to create 
num_connections = 32

 
# Define the request to send 
request = b"Hello World" 
 
def send_request(): 
    # Create a socket connection
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
    s.connect((target_ip, target_port)) 
    s.settimeout(30)
    print(s.getsockname())
 
    # Send the request 
    s.send(request) 
 
    # Receive the response (optional) 
    response = s.recv(4096) 
    # Close the socket connection 
    s.close() 
    
    print(response) 

 

 
# Create multiple threads to send requests concurrently 
threads = [] 
for _ in range(num_connections): 
    t = threading.Thread(target=send_request) 
    t.start() 
    threads.append(t) 
 
# Wait for all threads to finish 
for t in threads: 
    t.join() 