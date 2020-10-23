from redelServer import jobs_creator
from os import listdir
from datetime import datetime, timedelta
from persistent import Persistent
import defs

def update(func):
	def inner(*args, **kwargs):
		self = args[0]
		self._p_changed = True
		return func(*args, **kwargs)
	return inner


class JobStatus(Persistent):
	def __init__(self, file_name : str, job_id : int):
		self.file_name = file_name
		self.id = job_id
		self.active = False
		self.active_since = datetime.now()
		self.was_rescheduled = False
		self.done = False
		self.result = -1

	def activate(self):
		self.active = True
		self.active_since = datetime.now()

	def reschedule(self):
		self.active_since = datetime.now()
		self.was_rescheduled = True

	def deactivate(self):
		self.active = False

	def active_time(self):
		return datetime.now() - self.active_since

	# return total running time (if wasn't rescheduled)
	def post_result(self, result):
		self.done = True
		self.result = result

		if self.was_rescheduled:
			return None
		return self.active_time()


class JobGroup(Persistent):
	@update
	def __init__(self, name : str, graph : str, jobs_folder : str, curr_id : int, doubleCheck : bool = False):
		self.name = name
		self.graph = graph
		
		self.jobs_folder = jobs_folder

		self.all_jobs = set()
		self.remaining_jobs = set()
		self._v_id2job = {}		# _v_ prefix volatiles the dict, so it's not saved in the db
		for file_name in listdir(self.jobs_folder):
			job = JobStatus(self.jobs_folder+file_name, curr_id)
			defs.db_m.register_jobStatus(job)

			self.all_jobs.add(curr_id)
			self.remaining_jobs.add(curr_id)
			self._v_id2job[curr_id] = job

			curr_id += 1

		self.active_jobs = set()

		self.totalNumOfJobs = len(self.all_jobs)
		self.jobs_done = 0
		self.non_rescheduled_total_running_time = timedelta()
		self.non_rescheduled_jobs_done = 0

	def reload_dict(self):
		self._v_id2job = {job_id: defs.db_m.get_jobStatus(job_id) for job_id in self.all_jobs}

	def get_graph(self):
		return self.graph

	def get_percentage(self):
		return 100*(self.jobs_done / self.totalNumOfJobs)

	def get_percentage_rescheduled(self):
		if self.jobs_done == 0:
			return 0.0
		return 100 * ((self.jobs_done - self.non_rescheduled_jobs_done) / self.jobs_done)

	def is_completed(self):
		return self.jobs_done == self.totalNumOfJobs

	def get_result(self):
		return sum(self._v_id2job[job_id].result for job_id in self.all_jobs if self._v_id2job[job_id].done)

	@update
	def get_next_job(self):
		if self.remaining_jobs == set():
			return None
		job_id = self.remaining_jobs.pop()
		job = self._v_id2job[job_id]
		job.activate()
		self.active_jobs.add(job_id)
		return job

	@update
	def post_result(self, job_id : int, result : int):
		job = self._v_id2job[job_id]
		running_time_delta = job.post_result(result)
		if running_time_delta:
			self.non_rescheduled_total_running_time += running_time_delta
			self.non_rescheduled_jobs_done += 1

		if job_id in self.remaining_jobs:
			self.remaining_jobs.remove(job_id)
		self.active_jobs.remove(job_id)
		self.jobs_done += 1

	def get_long_waiting_jobs(self, max_delta : timedelta):
		return [job_id for job_id in self.active_jobs if \
				self._v_id2job[job_id].active_time() > max_delta]

	def get_average_running_time(self):
		if self.jobs_done < 10:
			return None
		return self.non_rescheduled_total_running_time / self.non_rescheduled_jobs_done

	@update
	def reschedule_long_waiting_jobs(self, minutes_wait_time=None):
		if not minutes_wait_time:
			avg_delta = self.get_average_running_time()
			if not avg:
				return
			max_delta = avg_delta * 10
		else:
			max_delta = timedelta(minutes=minutes_wait_time)
		long_waiting_jobs = self.get_long_waiting_jobs(max_delta)
		for job_id in long_waiting_jobs:
			self._v_id2job[job_id].reschedule()
			self.remaining_jobs.add(job_id)

		return len(long_waiting_jobs)


class JobManager(Persistent):
	def __init__(self):
		self.jobGroups = []		# jobGroup names
		self._v_name2jobGroup = {}		# _v_ prefix volatiles the dict, so it's not saved in the db
		self.active_jobs = {}	# job_id -> jobGroup name
		self.curr_id = 0

	def reload_dict(self):
		self._v_name2jobGroup = defs.db_m.get_all_jobGroups()

	def create_jobGroup(self, graph_name : str, steps : int, approx_num_of_jobs : int, jobs_folder : str, name : str, doubleCheck : bool = False):
		job_base_path = f"{jobs_folder}{name}"
		graph_file_path = defs.db_m.get_graph(graph_name)
		num_of_jobs_created = jobs_creator(graph_file_path, steps, approx_num_of_jobs, job_base_path)
		jobGroup = JobGroup(name, graph_name, jobs_folder, self.curr_id, doubleCheck)
		defs.db_m.register_jobGroup(jobGroup)
		self._v_name2jobGroup[name] = jobGroup
		self.curr_id += num_of_jobs_created

	@update
	def add_jobGroup(self, name : str):
		jobGroup = defs.db_m.get_jobGroup(name)
		if not jobGroup:
			print(f"Adding {name} failed! jobGroup is not found.")
			return False
		if jobGroup.is_completed():
			print(f"Adding {name} failed! jobGroup is completed.")
			return False
		if name in self.jobGroups:
			print(f"Adding {name} failed! jobGroup is already queued.")
			return False
		self.jobGroups.append(name)
		return True

	@update
	def remove_jobGroup(self, name: str):
		if name not in self.jobGroups:
			print(f"Stoping {name} failed! jobGroup is not queued.")
			return False

		self.jobGroups.remove(name)
		return True

	@update
	def get_next_job(self, name : str = None):
		if not self.jobGroups:
			return None

		if name:
			if name not in self.jobGroups:
				return None
			jobGroup = self._v_name2jobGroup[name]
			job = jobGroup.get_next_job()
			if not job:
				return None
			graph = jobGroup.get_graph()
			self.active_jobs[job.id] = name
			return job.id, job.file_name, graph

		for jobGroup_name in self.jobGroups:
			jobGroup = self._v_name2jobGroup[jobGroup_name]
			job = jobGroup.get_next_job()
			if not job:
				continue
			graph = jobGroup.get_graph()
			self.active_jobs[job.id] = jobGroup_name
			return job.id, job.file_name, graph
		return None

	@update
	def post_result(self, job_id : str, result : int):
		jobGroup_name = self.active_jobs.pop(job_id, None)
		if jobGroup_name == None:
			return False, None

		jobGroup = self._v_name2jobGroup[jobGroup_name]
		jobGroup.post_result(job_id, result)
		if jobGroup.is_completed():
			if jobGroup_name in self.jobGroups:
				self.jobGroups.remove(jobGroup_name)
			return True, (jobGroup_name, jobGroup.get_result())
		return True, None

	def get_queued_jobGroups(self):
		return self.jobGroups

	def set_priority(self, name : str, new_prio : int):
		jobGroup = self._v_name2jobGroup[name]
		pass
		# Todo write function

	def reschedule_long_waiting_jobs(self, minutes_wait_time=None):
		for name in self.jobGroups:
			self._v_name2jobGroup[name].reschedule_long_waiting_jobs(minutes_wait_time)
