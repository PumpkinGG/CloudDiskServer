/*
 *@Description FastDFS API
 *@Auther Guo
 *@Create time 2019年7月1日21:51:15
 */

#ifndef _FASTDFS_H_
#define _FASTDFS_H_

#include <fastdfs/fdfs_client.h>
#include <string>

class FastDFS {
public:
	explicit FastDFS();
	~FastDFS();
	void init(std::string configFile);

public:
	std::string uploadFile(std::string localFileName);
	void downloadFile(std::string fileId, std::string localFileName);
	void deleteFile(std::string fileId);

private:
	std::string configFile_;
	ConnectionInfo* pTrackerServer_;
	
};





#endif // !_FASTDFS_H_
