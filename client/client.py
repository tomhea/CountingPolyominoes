#!./python3.8

from sys import argv, stdin
from os import remove
from os.path import isfile, isdir, abspath
from socket import socket
from select import select
from time import time, sleep
from threading import Thread
from multiprocessing import cpu_count, Process, Queue

from defs import *
from redelClient import *

global server_socket
global manager_thread, shutting_down
global process_dict
global prints
global total_counting_contribution, total_jobs_done, total_jobs_failed, total_updates, start_time


# handle annoying whitespaces from start, middle and end
def remove_whitespaces(s: str) -> str:
	return ' '.join(s.split())


def safe_int(x: str):
	try:
		return int(x)
	except ValueError:
		return None


# Non-Deterministic Print
def ndprint(s: str):
	print(f'\n{s}')
	show_prompt()


def show_prompt():
	print(INPUT_PROMPT, end="", flush=True)


def connect_to(ip: str, port: int) -> socket:
	first_try = True
	connect_start_time = time()
	while True:
		if time() - connect_start_time > CONNECT_TIMEOUT:
			return None
		try:
			s = socket()
			s.connect((ip, port))
			return s
		except Exception as e:
			if s:
				s.close()
			if first_try:
				ndprint(f"Can't connect to server ({repr(e)}). I'm keep trying...")
				first_try = False
			sleep(1)


def sendfile(s: socket, path: str) -> bool:
	with open(path, 'rb') as f:
		return send(s, f.read())


def recvfile(s: socket, path: str) -> bool:
	data = recv(s, False)
	if data is None:
		return False
	with open(path, 'wb') as f:
		f.write(data)
	return True


def sendall_timed(s: socket, data: bytes) -> bool:
	send_start_time = time()
	while data:
		if time() - send_start_time > SEND_TIMEOUT:
			return False
		bytes_sent = s.send(data)
		data = data[bytes_sent:]
	return True


# msg can be string or bytes
def send(s: socket, msg) -> bool:
	if not sendall_timed(s, str(len(msg)).zfill(8).encode()):
		return False
	encode = (type(msg) == str)
	for start_chunk in range(0, len(msg), BUFFER_SIZE):
		data = msg[start_chunk:start_chunk + BUFFER_SIZE]
		if not sendall_timed(s, data.encode() if encode else data):
			return False
	return True


def recv(s: socket, decode: bool = True):
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


def print_statistics(finish: bool = False):
	if not finish:
		print(f"You are currently working on {len(process_dict)} jobs.")
	print(f"You helped counting a total of {total_counting_contribution:,} sub-graphs this session,")
	print(f"    while finishing {total_jobs_done:,} different jobs!")
	if total_updates:
		print(f"Moreover, you sent a total of {total_updates} valuable updates!")
	if total_jobs_failed:
		print(f"  Between here and then, {total_jobs_failed} job calculations run into errors.")
	if finish:
		working_hours = (time()-start_time)/3600
		cores = cpu_count()
		print(f"This session was {working_hours:,.02f} hours long,")
		print(f"    which are {working_hours * cores:,.02f} hours considering your {cores} cores :)")
		print("\nThank you :)")


def force_close_by_manager(at_start: bool = False):
	if at_start:
		print(SERVER_DOWN_MSG)

	else:
		for job_id, p in process_dict.items():
			p.terminate()
			p.join()

		print()
		print(SERVER_DOWN_MSG)
		print_statistics(finish=True)

	global shutting_down
	shutting_down = True


def close_and_update(reopen: bool = False):
	close_manager()

	global total_updates, process_dict
	for job_id, p in process_dict.items():
		p.terminate()
		p.join()
		job_path = jobs_dir + job_id
		if isfile(job_path):
			total_updates += 1
			msg = f"{UPDATE_JOB} {str(job_id)}"
			send(server_socket, msg)
			sendfile(server_socket, job_path)
			remove(job_path)
	process_dict = {}

	if reopen:
		start_manager()
	else:
		print_statistics(finish=True)


def handle_request(request: str):
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
			elif op in {'on', '+', '1'}:
				prints = True
			else:
				print("Prints operation isn't valid.")

	elif command in UPDATE:
		if len(args) >= 1:
			print("Update takes no arguments.")
			return
		close_and_update(reopen=True)

	elif command in CLOSE_APP:
		if len(args) >= 1:
			print("Close takes no arguments.")
			return
		close_and_update()
		if server_socket:
			server_socket.close()
		global shutting_down
		shutting_down = True

	else:
		print(f'No such command: "{command}".')


def compute_job(queue: Queue, graph_path: str, job_path: str, job_id: str):
	try:
		result = execute_job(graph_path, job_path)
		queue.put((job_id, result))
	except Exception as e:
		queue.put((job_id, repr(e)))


def execute_manager(max_subprocess: int):
	global server_socket
	global process_dict
	global total_counting_contribution, total_jobs_done, total_jobs_failed, total_updates
	process_dict = {}  # job_id => thread dictionary
	q = Queue()

	server_socket = connect_to(SERVER_IP, SERVER_PORT)
	if not server_socket:
		force_close_by_manager(at_start=True)
		return
	last_job_found = True
	while True:
		try:
			while not q.empty():
				job_id, result = q.get()
				p = process_dict.pop(job_id)
				p.join()
				if type(result) is str:
					total_jobs_done += 1
					if prints:
						ndprint(f"Error occurred while calculating job {job_id}:\n{result}")
				else:
					total_jobs_done += 1
					total_counting_contribution += result
					msg = f'{POST_RES} {job_id} {result}'
					if prints:
						ndprint(msg)
					send(server_socket, msg)
				job_path = jobs_dir + job_id
				if isfile(job_path):
					remove(job_path)

			if not getattr(manager_thread, "do_run"):
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

			####################

			if isfile(job_path):
				if job_id in process_dict:
					# TODO: notify server
					continue
			elif not recvfile(server_socket, job_path):
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

		except Exception as e:
			ndprint(f"Error occurred on manager thread: {repr(e)}\n\t")
			if server_socket:
				server_socket.close()
			server_socket = connect_to(SERVER_IP, SERVER_PORT)
			if not server_socket:
				force_close_by_manager()
				return
			last_job_found = True



def start_manager():
	global manager_thread
	manager_thread = Thread(target=execute_manager, args=(cpu_count(),))
	manager_thread.do_run = True
	manager_thread.start()


def close_manager():
	global manager_thread
	if manager_thread:
		manager_thread.do_run = False
		manager_thread.join()


def main():
	global manager_thread, shutting_down
	global prints
	global total_counting_contribution, total_jobs_done, total_jobs_failed, total_updates, start_time
	manager_thread = None
	shutting_down = False
	total_counting_contribution = total_jobs_done = total_jobs_failed = total_updates = 0
	start_time = time()

	while True:
		try:
			prints = False
			start_manager()

			print(WELCOME_MESSAGE)
			print("Enter 'Help' or 'H' for more details.\n")
			show_prompt()
			while True:
				if shutting_down:
					return 
				r,_,_ = select([stdin], [], [], 1)
				if r:
					handle_request(input())
					show_prompt()				

		except Exception as e:
			ndprint('Error occurred on main thread: \n\t' + repr(e))

		finally:
			close_manager()
			if server_socket:
				server_socket.close()


if __name__ == '__main__':
	main()
