from os import listdir
from datetime import datetime, timedelta


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
	def __init__(self, name : str, graph : str, jobs_folder : str, doubleCheck = False : bool):
		self.name = name
		self.graph = graph
		
		self.jobs_folder = jobs_folder
		self.all_jobs = [JobStatus(file_name) for file_name in listdir(self.jobs_folder)]
		self.remaining_jobs = self.all_jobs[:]
		self.active_jobs = []
		self.name2job = {job.file_name:job for job in self.all_jobs}

		self.totalNumOfJobs = len(self.jobs)
		self.jobs_done = 0

	def get_graph(self):
		return self.graph

	def get_percentage(self):
		return self.jobs_done / self.totalNumOfJobs

	def is_completed(self):
		return self.jobs_done == self.totalNumOfJobs

	def get_result(self):
		if self.jobs_done != self.totalNumOfJobs:
			return -1
		return sum(jobs_status.values())

	def get_next_job(self):
		if self.remaining_jobs == []:
			return None
		job = self.remaining_jobs.pop()
		job.activate()
		self.active_jobs.append(job)
		return job.file_name

	def post_result(self, file_name, result):
		job = name2job[file_name]
		job.post_result(result)
		self.active_jobs.remove(job)
		self.jobs_done += 1

	def get_long_waiting_jobs(self, minutes_wait_time : int, deactivate = False):
		long_waiting_jobs = [job for job in self.active_jobs if job.active_since + timedelta(minutes=minutes_wait_time) < datetime.now()]
		if deactivate:
			for job in long_waiting_jobs:
				job.deactivate()
				self.active_jobs.remove(job)
				self.remaining_jobs.append(job)
		return long_waiting_jobs


class jobManager:
	def __init__(self):
		self.JobGroups = []
		self.active_jobs = {}	# file_name -> group
		self.completed_JobGroups = []

	def add_JobGroup(self, name : str, graph : str, jobs_folder : str):
		self.JobGroups.append(JobGroup(name, graph, jobs_folder))

	def get_next_job(self, JobGroup_name=None : str):
		if self.JobGroups == []:
			return None

		if JobGroup_name:
			for jobGroup in self.JobGroups:
				if jobGroup.name == JobGroup_name:
					job_file = jobGroup.get_next_job()
					if job_file != None:
						active_jobs[job_file] = jobGroup
						return job_file, jobGroup.get_graph()
					return None
			return None

		for jobGroup in self.JobGroups:
			job_file = jobGroup.get_next_job()
			if job_file != None:
				active_jobs[job_file] = jobGroup
				return job_file, jobGroup.get_graph()
		return None

	def post_result(self, job_file : str, result : int):
		jobGroup = active_jobs.pop(job_file, default=None)
		if jobGroup == None:
			return False, None

		jobGroup.post_result(job_file, result)
		if jobGroup.is_completed():
			self.jobGroups.remove(jobGroup)
			result = (jobGroup.name, jobGroup.get_result)
			self.completed_JobGroups.append(result)
			return True, result
		return True, None
