from jobManager import JobStatus, JobGroup, JobManager


class DatabaseManager:
	def __init__(self):
		self.jobGroups = {}	# name -> jobGroup
		self.jobs = {}		# id   -> jobStatus
		self.curr_id = 0


	def register_job(self, file_name : str, jobGroup : JobGroup):
		id = self.curr_id
		self.jobs[id] = JobStatus(file_name, jobGroup)
		self.curr_id += 1
		return id

	def job_activate(self, job_id : int):
		if job_id not in self.jobs:
			return
		job = self.jobs[job_id]
		job.activate()

	def job_deactivate(self, job_id : int):
		if job_id not in self.jobs:
			return
		job = self.jobs[job_id]
		job.deactivate()

	def job_get_result(self, job_id : int, zero_if_not_done = True):
		if job_id not in self.jobs:
			return None
		job = self.jobs[job_id]
		if not job.done and zero_if_not_done:
			return 0
		return job.result

	def job_post_result(self, job_id : int, result : int):
		if job_id not in self.jobs:
			return
		job = self.jobs[job_id]
		job.post_result(result)

	def job_live_longer_than(self, job_id : int, minutes_wait_time : int):
		if job_id not in self.jobs:
			return
		job = self.jobs[job_id]
		return job.active_since + timedelta(minutes=minutes_wait_time) < datetime.now()



	def register_jobGroup(self, name : str, graph : str, jobs_folder : str):
		self.jobGroups[name] = JobGroup(name, graph, jobs_folder)

	def get_next_job(self, jobGroup_name : str):
		if name not in self.jobGroups:
			return None
		jobGroup = self.jobGroups[name]
		return jobGroup.get_next_job()

	def get_graph(self, jobGroup_name : str):
		if name not in self.jobGroups:
			return None
		jobGroup = self.jobGroups[name]
		return jobGroup.get_graph()

	def is_completed(self, jobGroup_name : str):
		if name not in self.jobGroups:
			return None
		jobGroup = self.jobGroups[name]
		return jobGroup.is_completed()

	def get_result(self, jobGroup_name : str):
		if name not in self.jobGroups:
			return None
		jobGroup = self.jobGroups[name]
		return sum(job_get_result(job_id) for job_id in jobGroup.all_jobs)
		# return jobGroup.get_result()

	def post_result(self, jobGroup_name : str, job_id : int, result : int):
		if jobGroup_name not in self.jobGroups:
			return None
		jobGroup = self.jobGroups[jobGroup_name]
		jobGroup.post_result(job_id, result)


