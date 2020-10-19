from redelClient import *
from sys import argv, stdin
from select import select
from socket import socket
from threading import Thread


RUN_COMPUTE = ("run",)
STOP_COMPUTE = ("stop",)
# CHECK_PROGRESS = ("progress", )
HELP = ("help", 'h')		# print every command possible
EXIT = ("exit",)

GET_JOB = "Get"
POST_RES = "Post"
GET_GRAPH = "Graph"

def is_graph_available(graph_file_name : str):
	pass

def compute_jobs_thread(amount : int):
	global server_socket
	global computing_thread

	for i in range(amount):
		if getattr(computing_thread, "do_run", False):
			break

		server_socket.send(GET_JOB.encode())
		graph_file_name = server_socket.recv(1024).decode()
		job_file_name = server_socket.recv(1024).decode()
		#TODO get job file content

		if not is_graph_available(graph_file_name):
			msg = f"{GET_GRAPH} {graph_file_name}"
			server_socket.send(msg.encode())
			#TODO get graph file content

		counted = execute_job(graphs_dir + graph_file_name, jobs_dir + job_file_name)
		msg = f'{POST_RES} {job_file_name} {counted}'
		print(msg)
		server_socket.send(msg.encode())

	print(f"Finished computing {amount} jobs.")


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
		amount = int(request.split(' ')[1])
		computing_thread = Thread(target = compute_jobs_thread, args = (amount,))
		computing_thread.start()
	elif command in STOP_COMPUTE:
		if not computing_thread:
			print("No computing jobs to stop.")
			return
		computing_thread.do_run = False
	elif command in EXIT:
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
	timeout = 5
	while True:
		r,_,_ = select([stdin], [], [], timeout)
		if computing_thread and not computing_thread.is_alive():
			computing_thread.join()
			computing_thread = None
		if stdin in r:
			handle_request(input())


if __name__ == '__main__':
	main()
