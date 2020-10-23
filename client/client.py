from redelClient import *
from sys import argv, stdin
from select import select
from socket import socket
from threading import Thread
import threading
import os.path
from time import sleep
from queue import Queue
from multiprocessing import cpu_count


SERVER_IP = '127.0.0.1'
SERVER_PORT = 36446


global server_socket
global manager_thread

NO_JOB_FOUND = "NONE"
JOB_FOUND = "JOB"

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


def is_graph_available(graph_path : str):
	return os.path.isfile(graph_path)


def connect_to(ip, port):
	s = socket()
	s.connect((ip, port))
	return s


def handle_request(request : str):
	global manager_thread
	command = request.split(' ')[0].lower()
	if command in CLOSE_APP:
		# later to be kill() ==> update job files ==> exit()
		if manager_thread:
			manager_thread.do_run = False
			manager_thread.join()
		server_socket.close()
		exit()
	elif command in HELP:
		print("""Welcome to SubgraphCounter Client-App!
Commands:
""")


def compute_job(queue : Queue, graph_path : str, job_path : str, job_id : str):
	result = execute_job(graph_path, job_path)
	queue.put((job_id, result))


def execute_manager(max_threads : int):
	q = Queue()
	threads_dict = {}	# job_id => thread dictionary
	jobs_finished = 0
	last_job_found = True
	while True:
		if not q.empty():
			job_id, result = q.get()
			t = threads_dict.pop(job_id)
			t.join()
			jobs_finished += 1
			msg = f'{POST_RES} {job_id} {result}'
			print(msg)
			send(server_socket, msg)
		if len(threads_dict) >= max_threads:
			sleep(1)
			continue

		if not getattr(manager_thread, "do_run"):
			# join() all
			# or: send updates of job files
			break

		send(server_socket, GET_JOB)
		response = recv(server_socket)
		if response == NO_JOB_FOUND:
			if last_job_found:
				print("No job available.")
				last_job_found = False
			sleep(5)
			continue
		else:
			last_job_found = True

		graph_name = recv(server_socket)
		graph_path = graphs_dir + graph_name
		job_id = recv(server_socket)
		job_path = jobs_dir + job_id
		recvfile(server_socket, job_path)

		if not is_graph_available(graph_path):
			msg = f"{GET_GRAPH} {graph_name}"
			send(server_socket, msg)
			recvfile(server_socket, graph_path)

		t = Thread(target=compute_job, args=(q, graph_path, job_path, job_id))
		t.start()
		threads_dict[job_id] = t

	print(f"Finished computing {jobs_finished} jobs.")


def main():
	global server_socket
	global manager_thread
	server_socket = connect_to(SERVER_IP, SERVER_PORT)
	manager_thread = Thread(target=execute_manager, args=(cpu_count(),))
	manager_thread.do_run = True
	manager_thread.start()

	while True:
		handle_request(input())


if __name__ == '__main__':
	main()
