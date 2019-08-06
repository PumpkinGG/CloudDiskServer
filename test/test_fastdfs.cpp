#include <string>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fastdfs_api.h"
#include <glog/logging.h>

using namespace std;

int main(int argc, char* argv[])
{
	google::InitGoogleLogging(argv[0]);
	FLAGS_log_dir = "/home/guo/FastDFS/client/logs";

	FastDFS fastDFS;

	string fileName = "/home/guo/Workspace/Init.sql";
	string saveFileName = "init.sql";
	string fileId;

	try {
		fastDFS.init("/etc/fdfs/client.conf");
		fileId = fastDFS.uploadFile(fileName);
		fastDFS.downloadFile(fileId, saveFileName);
		fastDFS.deleteFile(fileId);
	}
	catch (int ex) {
		LOG(ERROR) << "error: " << ex;
	}
	catch (...) {
		LOG(ERROR) << "other exceptions.";
	}

	google::ShutdownGoogleLogging();

	return 0;
}
