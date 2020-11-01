#!./python3.8
from sys import argv, stdin
from select import select
from socket import socket
from threading import Thread
from multiprocessing import cpu_count, Process, Queue
from os.path import isfile, isdir, abspath
from os import remove
from time import sleep

from defs import *
from redelClient import *

global server_socket
global manager_thread


# handle annoying whitespaces from start, middle and end
def remove_whitespaces(s : str):
	return ' '.join(s.split())


def safe_int(x):
	try:
		return int(x)
	except ValueError:
		return None


# Non-Deterministic Print
def ndprint(s):
	print(f'\r{s}{" " * 20}')
	print("⛟ ", end="")


def connect_to(ip, port):
	first_try = True
	while True:
		try:
			s = socket()
			s.connect((ip, port))
			return s
		except Exception as e:
			if first_try:
				ndprint("Can't connect to server. I'm keep trying...")
				first_try = False
		sleep(3)


def sendfile(s : socket, path : str):
	with open(path, 'rb') as f:
		send(s, f.read())


def recvfile(s : socket, path : str):
	data = recv(s, False)
	if data is None:
		return False
	with open(path, 'wb') as f:
		f.write(data)
	return True


# msg can be string or bytes
def send(s : socket, msg):
	s.sendall(str(len(msg)).zfill(8).encode())
	encode = (type(msg) == str)
	for start_chunk in range(0, len(msg), BUFFER_SIZE):
		if encode:
			s.sendall(msg[start_chunk:start_chunk + BUFFER_SIZE].encode())
		else:
			s.sendall(msg[start_chunk:start_chunk + BUFFER_SIZE])


def recv(s : socket, decode = True):
	meta_data = s.recv(8).decode()
	if not meta_data:
		return None
	size = safe_int(meta_data)
	if size is None:
		return None
	msg = b""
	while len(msg) < size:
		msg += s.recv(min(BUFFER_SIZE, size - len(msg)))
	if decode:
		msg = msg.decode()
	return msg


def print_statistics(finish : bool = False):
	print("Statistics are coming...")
	# TODO print statistics


def update():
	pass
	# TODO send server all current working files. using queue?


def handle_request(request : str):
	request = remove_whitespaces(request)
	command, *args = request.split(' ')
	command = command.lower()

	if not command:
		pass

	elif command in GET_STATISTICS:
		if len(args) >= 1:
			print("Statistics takes no arguments.")
			return
		print_statistics()

	elif command in HELP:
		if len(args) > 1:
			print("Help takes exactly 0 or 1 arguments.")
			return
		elif args:
			cmd = args[0]
			possible_cmds = [key for key in cmd_dict.keys() if cmd in key]
			if not possible_cmds:
				print("No such command.")
			else:
				print(cmd_dict[possible_cmds[0]])
		else:
			print(WELCOME_MESSAGE)
			print(HELP_MSG1)
			print(HELP_MSG2)
			print(HELP_MSG3)
			print('\n'.join([cmd_dict[cmd] for cmd in CMD_ORDER]))

	elif command in SET_PRINTS:
		global prints
		if len(args) > 1:
			print("Prints takes exactly 0 or 1 arguments.")
			return
		elif not args:
			print(f"prints are {'on' if prints else 'off'}.")
		else:
			op = args[0].lower()
			if   op in {'off', '-', '0'}:
				prints = False
			elif op in {'on',  '+', '1'}:
				prints = True
			else:
				print("Prints operation isn't valid.")

	elif command in UPDATE:
		if len(args) >= 1:
			print("Update takes no arguments.")
			return
		update()

	elif command in CLOSE_APP:
		if len(args) >= 1:
			print("Close takes no arguments.")
			return
		global manager_thread
		if manager_thread:
			manager_thread.do_run = False
			manager_thread.join()
		if server_socket:
			server_socket.close()
		exit()

	else:
		print(f'No such command: "{command}".')


def compute_job(queue : Queue, graph_path : str, job_path : str, job_id : str):
	result = execute_job(graph_path, job_path)
	queue.put((job_id, result))


def execute_manager(max_subprocess : int):
	q = Queue()
	process_dict = {}	# job_id => thread dictionary
	jobs_finished = 0
	last_job_found = True
	while True:
		while not q.empty():
			job_id, result = q.get()
			p = process_dict.pop(job_id)
			p.join()
			jobs_finished += 1
			msg = f'{POST_RES} {job_id} {result}'
			global prints
			if prints:
				ndprint(msg)
			send(server_socket, msg)
			job_path = jobs_dir + job_id
			if isfile(job_path):
				remove(job_path)

		if not getattr(manager_thread, "do_run"):
			for job_id, p  in process_dict.items():
				p.terminate()
				p.join()
				job_path = jobs_dir + job_id
				if isfile(job_path):
					msg = f"{UPDATE_JOB} {str(job_id)}"
					send(server_socket, msg)
					sendfile(server_socket, job_path)
					remove(job_path)
			break

		if len(process_dict) >= max_subprocess:
			sleep(1)
			continue

		send(server_socket, GET_JOB)
		response = recv(server_socket)
		if response is None:
			ndprint('Weird server response (conversation start).')
			sleep(1)
			continue
		if response == NO_JOB_FOUND:
			if last_job_found:
				ndprint("No job available.")
				last_job_found = False
			sleep(5)
			continue
		else:
			last_job_found = True

		graph_name = recv(server_socket)
		if graph_name is None:
			ndprint('Weird server response (graph name).')
			sleep(1)
			continue
		graph_path = graphs_dir + graph_name

		job_id = recv(server_socket)
		if job_id is None:
			ndprint('Weird server response (job id).')
			sleep(1)
			continue
		job_path = jobs_dir + job_id

		if not recvfile(server_socket, job_path):
			ndprint('Weird server response (job file).')
			sleep(1)
			continue

		if not isfile(graph_path):
			msg = f"{GET_GRAPH} {graph_name}"
			send(server_socket, msg)
			if not recvfile(server_socket, graph_path):
				ndprint('Weird server response (graph file).')
				sleep(1)
				continue

		p = Process(target=compute_job, args=(q, graph_path, job_path, job_id))
		p.start()
		process_dict[job_id] = p

	print_statistics(finish=True)
	print(f"Finished computing {jobs_finished} jobs.")


def main():
	global server_socket
	global manager_thread
	global prints
	server_socket, manager_thread = None, None

	while True:
		try:
			prints = True
			server_socket = connect_to(SERVER_IP, SERVER_PORT)
			manager_thread = Thread(target=execute_manager, args=(cpu_count(),))
			manager_thread.do_run = True
			manager_thread.start()

			while True:
				print("⛟ ", end="")
				handle_request(input())

		except Exception as e:
			print('Error Occurred: \n\t' + repr(e))

		finally:
			if manager_thread:
				manager_thread.do_run = False
				manager_thread.join()
			if server_socket:
				server_socket.close()


if __name__ == '__main__':
	main()
