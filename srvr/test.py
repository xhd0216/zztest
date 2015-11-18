import socket
import sys
import thread
import threading

lock = threading.Lock()
server_address = './server_file/_server_file_'
nt = 200

def connection(thname, index):
	# Create a UDS socket
	#print "working thread ", index
	sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	ss = thname+str(index);
	pp = ss;
	try:
	    sock.connect(server_address)
	except socket.error, msg:
	    ##print >>sys.stderr, ss, msg
		pp = pp+msg
		lock.acquire()
		print pp
		lock.release()
		sys.exit(1)
	sock.sendall(ss)
	data = sock.recv(1024)
	pp = pp+data
	lock.acquire()
	print pp
	lock.release()
# Connect the socket to the port where the server is listening
#print >>sys.stderr, 'connecting to %s' % server_address

for i in range(nt):
	thread.start_new_thread(connection, ("python-", i, ));
	print "doing"

