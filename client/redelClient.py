from ctypes import *
import pathlib


dlldir = "C:/Users/tomhe/OneDrive - Technion/Documents/CountingPolyominoes/cmake-build-release"
# libname = str(pathlib.Path().absolute() / "cmake-build-debug" / "libRedelClient.dll").replace('\\', '/')
libname = dlldir + "/libRedelClient.dll"
print(libname)
redel_client_lib = cdll.LoadLibrary(libname)

def cpp_str(s):
	return create_string_buffer(s.encode('utf-8'))


# u64 executeJob(const char* graphFilePath, const char* jobPath);
def execute_job(graph_file_path : str, job_path : str) -> int:
	redel_client_lib.executeJob.restype = c_ulonglong
	count = redel_client_lib.executeJob(cpp_str(graph_file_path), cpp_str(job_path))
	return count
