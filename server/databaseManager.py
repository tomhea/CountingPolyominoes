from jobManager import JobStatus, JobGroup, JobManager

from ZODB import FileStorage, DB


class MyZODB(object):
	def __init__(self, path):
		self.storage = FileStorage.FileStorage(path)
		self.db = DB(self.storage)
		self.connection = self.db.open()
		self.dbroot = self.connection.root()

	def close(self):
		self.connection.close()
		self.db.close()
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
		self.jobsDB.dbroot[jobStatus.job_id] = jobStatus

	def get_jobStatus(self, job_id: int):
		return self.jobsDB.dbroot[job_id]

	def register_graph(self, graph_path : str, graph_name : str):
		self.graphsDB.dbroot[graph_name] = graph_path

	def get_graph(self, graph_name):
		return self.graphsDB.dbroot[graph_name]

	def register_jobGroup(self, jobGroup: JobGroup):
		self.jobGroupsDB.dbroot[jobGroup.name] = jobGroup

	def get_jobGroup(self, name: str):
		jobGroup = self.jobsDB.dbroot[name]
		jobGroup.reload_dict()
		return jobGroup

	def register_jobManager(self, jobManager : JobManager):
		self.jobManagerDB.dbroot['self'] = jobManager

	def get_jobManager(self):
		jobManager = self.jobManagerDB.dbroot['self']
		jobManager.reload_dict()
		return jobManager

	def get_all_graphs(self):
		graphs = {}
		for graph_name, graph_path in self.graphsDB.dbroot.items():
			graphs[graph_name] = graph_path
		return graphs

	def get_all_jobGroups(self):
		groups = {}
		for name, jobGroup in self.jobGroupsDB.dbroot.items():
			groups[graph_name] = jobGroup
		return groups
