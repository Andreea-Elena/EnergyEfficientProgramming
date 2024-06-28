import socket
import random

HEADER_SAMPLE_GET = "DELETE /me/you/and/yourself?column_a=unu&column_b=true&column_c=arix&column_d=a HTTP/1.1\r\n\
Content-Type: application/json\r\n\
User-Agent: PostmanRuntime/7.29.0\r\n\
Accept: */*\r\n\
Postman-Token: 1882e221-aaf2-4d7d-bf63-21f143d82b38\r\n\
Host: localhost:8888\r\n\
Accept-Encoding: gzip, deflate, br\r\n\
Connection: keep-alive\r\n\
_epnt: testforme\r\n\
Content-Length: 97\r\n\r\n"


# Define the target server's IP address and port 
target_ip = "127.0.0.1" 
target_port = 8888 

def send_request(): 
    # Define the request to send 
    request = bytes(HEADER_SAMPLE_GET,'UTF-8')

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
    
    print(response.decode("utf-8")) 

# Send only one request
send_request()
