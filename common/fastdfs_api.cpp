#include "fastdfs_api.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdexcept>

#include <glog/logging.h>

FastDFS::FastDFS()
{
}

FastDFS::~FastDFS()
{
	tracker_disconnect_server_ex(pTrackerServer_, true);
	fdfs_client_destroy();
}

void FastDFS::init(std::string configFile)
{
	configFile_ = configFile;

	int result;
	result = fdfs_client_init(configFile_.c_str());
	if (result != 0) {
		LOG(ERROR) << "fdfs client init fail, error no: "
			<< result << ", error info: " << STRERROR(result);
		throw std::runtime_error("fdfs client init fail");
	}

	pTrackerServer_ = tracker_get_connection();
	if (pTrackerServer_ == NULL) {
		LOG(ERROR) << "fdfs tracker conn fail, error no: "
			<< result << ", error info: " << STRERROR(result);
		throw std::runtime_error("fdfs tracker conn fail");
	}
}

std::string FastDFS::uploadFile(std::string localFileName)
{
	char groupName[FDFS_GROUP_NAME_MAX_LEN + 1];
	char fileId[128];
	int result;
	int storePathIndex;
	ConnectionInfo storageServer;

	*groupName = '\0';

	if ((result = tracker_query_storage_store(pTrackerServer_, \
		&storageServer, groupName, &storePathIndex)) != 0) {
		LOG(ERROR) << "tracker_query_storage fail, error no: " 
			<< result << ", error info: " << STRERROR(result);
		throw std::runtime_error("tracker_query_storage fail");
	}

	result = storage_upload_by_filename1(pTrackerServer_, \
		&storageServer, storePathIndex, \
		localFileName.c_str(), NULL, \
		NULL, 0, groupName, fileId);
	if (result == 0) {
		DLOG(INFO) << "upload file " << localFileName 
			<< " successful, file id: " << fileId;
	}
	else {
		LOG(ERROR) << "upload file fail, error no: "
			<< result << ", error info: " << STRERROR(result);
		throw std::runtime_error("upload file fail");
	}

	return fileId;
}

void FastDFS::downloadFile(std::string fileId, std::string localFileName)
{
	int result;
	int64_t file_size;
	int64_t file_offset;
	int64_t download_bytes;
	ConnectionInfo storageServer;

	file_offset = 0;
	download_bytes = 0;

	result = tracker_query_storage_fetch1(pTrackerServer_, \
		&storageServer, fileId.c_str());
	if (result != 0) {
		LOG(ERROR) << "file does not exist, error no: "
			<< result << ", error info: " << STRERROR(result);
		throw std::runtime_error("file does not exist");
	}

	result = storage_download_file_to_file1(pTrackerServer_, \
		&storageServer, fileId.c_str(), localFileName.c_str(), &file_size);
	if (result != 0) {
		LOG(ERROR) << "download file fail, error no: "
			<< result << ", error info: " << STRERROR(result);
		throw std::runtime_error("download file fail");
	}

}

void FastDFS::deleteFile(std::string fileId)
{
	int result;
	if ((result = storage_delete_file1(pTrackerServer_, NULL, fileId.c_str())) != 0) {
		LOG(ERROR) << "delete file fail, error no: "
			<< result << ", error info: " << STRERROR(result);
		throw std::runtime_error("delete file fail");
	}
	DLOG(INFO) << "delete file, file id: " << fileId;
}
