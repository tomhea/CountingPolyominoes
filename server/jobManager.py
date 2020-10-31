from os import listdir, makedirs, remove, rmdir
from shutil import copyfile
from os.path import isfile, isdir, abspath
from datetime import datetime, timedelta

import defs
from defs import jobs_dir, graphs_dir
from redelServer import jobs_creator
from persistent import Persistent


def update(func):
	def inner(*args, **kwargs):
		self = args[0]
		res = func(*args, **kwargs)
		self._p_changed = True
		return res
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

	def get_result(self, zero_if_not_done=True):
		if not self.done and zero_if_not_done:
			return 0
		return self.result


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

	def delete_all_jobStatus(self):
		for job_id in self.all_jobs:
			jobStatus = self._v_id2job[job_id]
			remove(jobStatus.file_name)
			defs.db_m.unregister_jobStatus(job_id)

	def get_graph(self):
		return self.graph

	def get_percentage(self):
		return 100 * (self.jobs_done / self.totalNumOfJobs)

	def get_percentage_rescheduled(self):
		if self.jobs_done == 0:
			return 0.0
		return 100 * ((self.jobs_done - self.non_rescheduled_jobs_done) / self.jobs_done)

	def is_completed(self):
		return self.jobs_done == self.totalNumOfJobs

	def get_result(self):
		return sum(self._v_id2job[job_id].get_result() for job_id in self.all_jobs)

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

	@update
	def update_job(self, job_id : int, updated_job_path : str):
		jobStatus = self._v_id2job[job_id]
		copyfile(updated_job_path, jobStatus.file_name)
		jobStatus.reschedule()
		self.remaining_jobs.add(job_id)

	def get_long_waiting_jobs(self, max_delta : timedelta):
		return [job_id for job_id in self.active_jobs if \
				self._v_id2job[job_id].active_time() > max_delta]

	def get_average_running_time(self):
		if self.jobs_done < 10:
			return None
		return self.non_rescheduled_total_running_time / self.non_rescheduled_jobs_done

	@update
	# returns how many jobs rescheduled
	def reschedule_long_waiting_jobs(self, minutes_wait_time=None):
		if not minutes_wait_time:
			avg_delta = self.get_average_running_time()
			if not avg_delta:
				return 0
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
		self._v_name2jobGroup = defs.db_m.get_all_jobGroups(reload=True)

	def create_jobGroup(self, graph_name : str, steps : int, approx_num_of_jobs : int, name : str, doubleCheck : bool = False):
		jobs_folder = f"{jobs_dir}{name}/"
		job_base_path = f"{jobs_folder}{name}"
		graph_file_path = defs.db_m.get_graph(graph_name)
		if steps < 0:
			return "Failed. steps should not be negative."
		if approx_num_of_jobs <= 0:
			return "Failed. num should be positive."
		if not graph_file_path:
			return f"Failed. Graph {graph_name} isn't registered."
		if not isfile(graph_file_path):
			return f"Failed. Graph path {abspath(graph_file_path)} doesn't exist."
		if isdir(jobs_folder) and listdir(jobs_folder) != []:
			return f"Failed. Folder {abspath(jobs_folder)} should be deleted or emptied."
		elif not isdir(jobs_folder):	# if folder doesn't exist - create it
			makedirs(jobs_folder)
		num_of_jobs_created = jobs_creator(graph_file_path, steps, approx_num_of_jobs, job_base_path)
		jobGroup = JobGroup(name, graph_name, jobs_folder, self.curr_id, doubleCheck)
		defs.db_m.register_jobGroup(jobGroup)
		self._v_name2jobGroup[name] = jobGroup
		self.curr_id += num_of_jobs_created
		return num_of_jobs_created

	@update
	def delete_jobGroup(self, name : str):
		if name in self.jobGroups:
			return f"Failed! Group {name} is queued, please unqueue it first."
		jobGroup = defs.db_m.get_jobGroup(name)
		if not jobGroup:
			return f"Failed! Group {name} is not found."
		active_jobs_to_remove = [job_id for job_id, jobGroup_name in self.active_jobs.items() if jobGroup_name == name]
		for job_id in active_jobs_to_remove:
			del self.active_jobs[job_id]
		if name in self._v_name2jobGroup:
			del self._v_name2jobGroup[name]
		jobGroup.delete_all_jobStatus()
		jobs_folder = f"{jobs_dir}{name}/"
		rmdir(jobs_folder)
		defs.db_m.unregister_jobGroup(name)

	@update
	# returns error message
	def add_jobGroup(self, name : str):
		jobGroup = defs.db_m.get_jobGroup(name)
		if not jobGroup:
			return f"Failed! Group {name} is not found."
		if jobGroup.is_completed():
			return f"Failed! Group {name} is completed."
		if name in self.jobGroups:
			return f"Failed! Group {name} is already queued."
		self.jobGroups.append(name)
		return None

	@update
	# returns error message
	def remove_jobGroup(self, name: str):
		if name not in self.jobGroups:
			return f"Failed! Group {name} is not queued."
		self.jobGroups.remove(name)
		return None

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

	def update_job(self, job_id : int, updated_job_path : str):
		jobGroup_name = self.active_jobs.get(job_id)
		if not jobGroup_name:
			return False

		jobGroup = self._v_name2jobGroup[jobGroup_name]
		jobGroup.update_job(job_id, updated_job_path)
		return True

	@update
	def set_priority(self, name : str, index : int):
		if index < 1 or index > len(self.jobGroups):
			return f"Failed. index should be between [1, {len(self.jobGroups)}] (number of queued groups)."
		if name not in self._v_name2jobGroup:
			return f"Failed. Name {name} is not found."
		without = [n for n in self.jobGroups if name != n]
		self.jobGroups = without[:index-1] + [name] + without[index-1:]

	def reschedule_long_waiting_jobs(self, minutes_wait_time=None):
		rescheduled = 0
		for name in self.jobGroups:
			rescheduled += self._v_name2jobGroup[name].reschedule_long_waiting_jobs(minutes_wait_time)
		return rescheduled
