import socket
import sys
import thread
import threading

lock = threading.Lock()
server_address = './server_file/_server_file_'
nt = 10

def connection(thname, index):
	# Create a UDS socket
	sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	ss = thname+str(index);
	pp = ss;
	try:
		sock.connect(server_address)
	except:
		print "fail to connect to server"
		return 0
	try:
		sock.sendall(ss)
		data = sock.recv(1024)
		if "got some msg" in data:
			pp = pp+data
			lock.acquire()
			print pp, "time", i
			lock.release()
		sock.close()
	except: 
		lock.acquire()
		print "error ", pp
		##pp = pp+msg
		##print pp
		lock.release()
# Connect the socket to the port where the server is listening
#print >>sys.stderr, 'connecting to %s' % server_address

for i in range(nt):
	#thread.start_new_thread(connection, ("python-", i, ));
	#print "doing"
	s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	s.connect(server_address)
	spp = "jfklsd"+str(i)
	s.sendall(spp)
	data = s.recv(1024)
	print data
	s.close()
