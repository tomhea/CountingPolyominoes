from redelClient import *
from sys import argv, stdin
from select import select
from socket import socket
from threading import Thread
import os.path
from time import sleep

NO_JOB_FOUND = "None"
JOB_FOUND = "Job"

RUN_COMPUTE = ("run",)
# CHECK_PROGRESS = ("progress", )
HELP = ("help", 'h')		# print every command possible
CLOSE_APP = ("exit",'close','quit','q')

GET_JOB = "GET"
POST_RES = "POST"
GET_GRAPH = "GRAPH"
UPDATE_JOB = "UPDATE"

graphs_dir = "./Graphs/"
jobs_dir = "./Jobs/"


BUFFER_SIZE = 1024
def sendfile(s : socket, path : str):
	with open(path, 'rb') as f:
		send(s, f.read(), False)

def recvfile(s : socket, path : str):
	with open(path, 'wb') as f:
		f.write(recv(s, False))

def send(s : socket, msg : str, encode = True):
	s.sendall(str(len(msg)).zfill(8).encode())
	for start_chunk in range(0, len(msg), BUFFER_SIZE):
		if encode:
			s.sendall(msg[start_chunk:start_chunk + BUFFER_SIZE].encode())
		else:
			s.sendall(msg[start_chunk:start_chunk + BUFFER_SIZE])

def recv(s : socket, decode = True):
	meta_data = s.recv(8).decode()
	if not meta_data:
		return None
	size = int(meta_data)
	msg = b""
	while len(msg) < size:
		msg += s.recv(min(BUFFER_SIZE, size - len(msg)))
	if decode:
		msg = msg.decode()
	return msg


def is_graph_available(graph_name : str):
	return os.path.isfile(graphs_dir + graph_name)

def compute_jobs_thread():
	global server_socket
	global computing_thread

	jobs_finished = 0
	while True:
		if getattr(computing_thread, "do_run", False):
			break

		send(server_socket, GET_JOB)
		response = recv(server_socket)
		if response == NO_JOB_FOUND:
			sleep(3)
			continue

		graph_name = recv(server_socket)
		job_id = int(recv(server_socket))
		recvfile(server_socket, jobs_dir + str(job_id))

		if not is_graph_available(graph_name):
			msg = f"{GET_GRAPH} {graph_name}"
			send(server_socket, msg)
			recvfile(server_socket, graphs_dir + graph_name)

		counted = execute_job(graphs_dir + graph_name, jobs_dir + str(job_id))
		jobs_finished += 1
		msg = f'{POST_RES} {job_id} {counted}'
		print(msg)
		send(server_socket, msg)

	print(f"Finished computing {jobs_finished} jobs.")


def connect_to(ip, port):
	s = socket()
	s.connect((ip, port))
	return s


def handle_request(request : str):
	global server_socket
	global computing_thread

	command = request.split(' ')[0].lower()
	if command in RUN_COMPUTE:
		if computing_thread:
			print("Already computing.")
			return
		computing_thread = Thread(target = compute_jobs_thread)
		computing_thread.start()
	elif command in CLOSE_APP:
		# later to be kill() ==> update job files ==> exit()
		if computing_thread:
			computing_thread.do_run = False
			computing_thread.join()
		server_socket.close()
		exit()
	elif command in HELP:
		print("""Welcome to SubgraphCounter Client-App!
Commands:
""")


def main():
	global server_socket
	global computing_thread
	server_socket = connect_to('127.0.0.1', 36446)
	computing_thread = None
	while True:
		handle_request(input())
		# r,_,_ = select([stdin], [], [])
		# if stdin in r:
		# 	handle_request(input())


if __name__ == '__main__':
	main()
