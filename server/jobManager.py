from redelServer import jobs_creator
from os import listdir
from datetime import datetime, timedelta
from persistent import Persistent
from server import db_m


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
		self.done = False
		self.result = -1

	def activate(self):
		self.active = True
		self.active_since = datetime.now()

	def deactivate(self):
		self.active = False

	def post_result(self, result):
		self.done = True
		self.result = result


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
			job = JobStatus(file_name, curr_id)
			db_m.register_jobStatus(job)

			self.all_jobs.add(curr_id)
			self.remaining_jobs.add(curr_id)
			self._v_id2job[curr_id] = job

			curr_id += 1

		self.active_jobs = set()

		self.totalNumOfJobs = len(self.all_jobs)
		self.jobs_done = 0

	def reload_dict(self):
		self._v_id2job = {job_id: db_m.get_jobStatus(job_id) for job_id in self.all_jobs}

	def get_graph(self):
		return self.graph

	def get_percentage(self):
		return self.jobs_done / self.totalNumOfJobs

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
		self._v_id2job[job_id].post_result(result)
		self.active_jobs.remove(job_id)
		self.jobs_done += 1

	def get_long_waiting_jobs(self, minutes_wait_time : int):
		return [job_id for job_id in self.active_jobs if \
				self._v_id2job[job_id].active_since + timedelta(minutes=minutes_wait_time) < datetime.now()]

	@update
	def reschedule_long_waiting_jobs(self, minutes_wait_time : int):
		long_waiting_jobs = self.get_long_waiting_jobs(minutes_wait_time)
		for job_id in long_waiting_jobs:
			self._v_id2job[job_id].activate()
			self.remaining_jobs.add(job_id)


class JobManager(Persistent):
	def __init__(self):
		self.jobGroups = []		# jobGroup names
		self._v_name2jobGroup = {}		# _v_ prefix volatiles the dict, so it's not saved in the db
		self.active_jobs = {}	# job_id -> jobGroup name
		self.curr_id = 0

	def reload_dict(self):
		self._v_name2jobGroup = {name: db_m.get_jobGroup(name) for name in self.jobGroups}

	def create_jobGroup(self, graph_file_path : str, steps : int, approx_num_of_jobs : int, job_base_path : str, name : str, doubleCheck : bool = False):
		num_of_jobs_created = jobs_creator(graph_file_path, steps, approx_num_of_jobs, job_base_path)
		jobs_folder = '/'.join(job_base_path.split('/')[:-1])
		jobGroup = JobGroup(name, graph_file_path, jobs_folder, self.curr_id, doubleCheck)
		db_m.register_jobGroup(jobGroup)
		self.curr_id += num_of_jobs_created

	@update
	def add_jobGroup(self, name : str):
		jobGroup = db_m.get_jobGroup(name)
		self.name2jobGroup[name] = jobGroup
		self.jobGroups.append(name)

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
		jobGroup_name = self.active_jobs.pop(job_id, default=None)
		if jobGroup_name == None:
			return False, None

		jobGroup = self._v_name2jobGroup[jobGroup_name]
		jobGroup.post_result(job_id, result)
		if jobGroup.is_completed():
			self.jobGroups.remove(jobGroup_name)
			del self._v_name2jobGroup[jobGroup_name]
			return True, jobGroup.get_result()
		return True, None
