from os import listdir
from datetime import datetime, timedelta

global db_m


class JobStatus:
	def __init__(self, file_name : str):
		self.file_name = file_name
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


class JobGroup:
	def __init__(self, name : str, graph : str, jobs_folder : str, doubleCheck : bool = False):
		self.name = name
		self.graph = graph
		
		self.jobs_folder = jobs_folder
		self.all_jobs = [db_m.register_job(file_name) for file_name in listdir(self.jobs_folder)]
		self.remaining_jobs = self.all_jobs[:]
		self.active_jobs = []
		# self.name2job = {job.file_name:job for job in self.all_jobs}

		self.totalNumOfJobs = len(self.jobs)
		self.jobs_done = 0

	def get_graph(self):
		return self.graph

	def get_percentage(self):
		return self.jobs_done / self.totalNumOfJobs

	def is_completed(self):
		return self.jobs_done == self.totalNumOfJobs

	# def get_result(self):
	# 	# if self.jobs_done != self.totalNumOfJobs:
	# 	# 	return -1
	# 	return sum(job.result for job in self.all_jobs if job.done)

	def get_next_job(self):
		if self.remaining_jobs == []:
			return None
		job_id = self.remaining_jobs.pop()
		db_m.job_activate(job_id)
		self.active_jobs.append(job_id)
		return job_id

	def post_result(self, job_id : int, result : int):
		db_m.job_post_result(job_id, result)
		self.active_jobs.remove(job_id)
		self.jobs_done += 1

	def get_long_waiting_jobs(self, minutes_wait_time : int, deactivate = False):
		long_waiting_jobs = [job_id for job_id in self.active_jobs if db_m.job_live_longer_than(job_id, minutes_wait_time)]
		if deactivate:
			for job_id in long_waiting_jobs:
				db_m.deactivate(job_id)
				self.active_jobs.remove(job_id)
				self.remaining_jobs.append(job_id)
		return long_waiting_jobs


class JobManager:
	def __init__(self):
		self.JobGroups = []		# jobGroup names
		# self.name2jobGroup = {}
		self.active_jobs = {}	# job_id -> jobGroup name
		self.completed_JobGroups = []

	def add_JobGroup(self, name : str):
		# jobGroup = db_m.get_jobGroup(name)
		# self.name2jobGroup[name] = jobGroup
		self.JobGroups.append(name)

	def get_next_job(self, jobGroup_name : str = None):
		if self.JobGroups == []:
			return None

		if jobGroup_name:
			if jobGroup_name not in self.jobGroups:
				return None
			job_id = db_m.get_next_job(jobGroup_name)
			graph = db_m.get_graph(jobGroup_name)
			if None in (job_id, graph):
				return None
			return (job_id, graph)


			# for jobGroup in self.JobGroups:
			# 	if jobGroup.name == jobGroup_name:
			# 		job_file = jobGroup.get_next_job()
			# 		if job_file != None:
			# 			active_jobs[job_file] = jobGroup
			# 			return job_file, jobGroup.get_graph()
			# 		return None
			# return None

		for jobGroup_name in self.JobGroups:
			job_id = db_m.get_next_job(jobGroup_name)
			graph = db_m.get_graph(jobGroup_name)
			if None in (job_id, graph):
				continue
			self.active_jobs[job_id] = jobGroup_name
			return (job_id, graph)
		return None

	def post_result(self, job_id : str, result : int):
		jobGroup_name = self.active_jobs.pop(job_id, default=None)
		if jobGroup_name == None:
			return False, None

		db_m.post_result(jobGroup_name, job_id, result)
		# jobGroup.post_result(job_file, result)
		if db_m.is_completed(jobGroup_name):
			self.jobGroups.remove(jobGroup_name)
			self.completed_JobGroups[jobGroup_name](db_m.get_result(jobGroup_name))
			return True, result
		return True, None
