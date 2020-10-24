from jobManager import JobStatus, JobGroup, JobManager

from ZODB import FileStorage, DB


class MyZODB(object):
	def __init__(self, path):
		self.storage = FileStorage.FileStorage(path)
		self.db = DB(self.storage)
		self.connection = self.db.open()
		self.dbroot = self.connection.root()

	def close(self):
		if self.connection:
			self.connection.close()
		if self.db:
			self.db.close()
		if self.storage:
			self.storage.close()


class DatabaseManager:
	def __init__(self):
		self.jobsDB = MyZODB('./db/jobs.fs')				# id   -> jobStatus
		self.graphsDB = MyZODB('./db/graphs.fs')			# name -> graph_path
		self.jobGroupsDB = MyZODB('./db/jobGroups.fs')		# name -> jobGroup
		self.jobManagerDB = MyZODB('./db/jobManager.fs')	# self is jobManager

	def close(self):
		self.jobsDB.close()
		self.graphsDB.close()
		self.jobGroupsDB.close()
		self.jobManagerDB.close()

	def register_jobStatus(self, jobStatus: JobStatus):
		self.jobsDB.dbroot[jobStatus.id] = jobStatus

	def get_jobStatus(self, job_id: int):
		if job_id not in list(self.jobsDB.dbroot.keys()):
			return None
		return self.jobsDB.dbroot[job_id]

	def register_graph(self, graph_path : str, graph_name : str):
		self.graphsDB.dbroot[graph_name] = graph_path

	def get_graph(self, graph_name):
		if graph_name not in list(self.graphsDB.dbroot.keys()):
			return None
		return self.graphsDB.dbroot[graph_name]

	def register_jobGroup(self, jobGroup: JobGroup):
		self.jobGroupsDB.dbroot[jobGroup.name] = jobGroup

	def get_jobGroup(self, name: str):
		if name not in list(self.jobGroupsDB.dbroot.keys()):
			return None
		jobGroup = self.jobGroupsDB.dbroot[name]
		jobGroup.reload_dict()
		return jobGroup

	def register_jobManager(self, jobManager : JobManager):
		self.jobManagerDB.dbroot['self'] = jobManager

	def get_jobManager(self):
		if 'self' not in list(self.jobManagerDB.dbroot.keys()):
			return None
		jobManager = self.jobManagerDB.dbroot['self']
		jobManager.reload_dict()
		return jobManager

	def get_all_graphs(self):
		graphs = {}
		for graph_name, graph_path in self.graphsDB.dbroot.items():
			graphs[graph_name] = graph_path
		return graphs

	def get_all_jobGroups(self, reload=False):
		groups = {}
		for name, jobGroup in self.jobGroupsDB.dbroot.items():
			groups[name] = jobGroup
			if reload:
				jobGroup.reload_dict()
		return groups
