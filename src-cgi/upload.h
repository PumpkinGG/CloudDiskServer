#pragma once

#include <string>
#include <stdio.h>
#include "utils_cfg.h"

struct FileInfo {
	std::string user;
	std::string fileName;
	std::string md5;
	std::string type;
	int size;
};

/*
 * Upload working function
 */
void doWork();

/*
 * Pack reply post json data
 */
std::string setReplyData(const Utils::ReplyStatus stat);

/*
 * Upload File to FastDFS
 * @tempFileName -- received file from client
 * @fdfsConfigFile -- fdfs client.conf path
 * Return: fileId on FastDFS
 */
std::string uploadFileToFastdfs(const std::string& tempFileName, 
	Utils::FdfsInfo& fdfsInfo);

/*
 * Save fileInfo to Mysql
 * @mysqlInfo -- configure info to access MySQL
 * @fdfsInfo -- configure info to access FastDFS
 * @fileInfo -- upload file info
 * @fileId -- fileid return from FastDFS
 * Return Status
 */
Utils::ReplyStatus dealWithMySQL(const Utils::MysqlInfo& mysqlInfo, Utils::FdfsInfo& fdfsInfo,
	const FileInfo& fileInfo, const std::string& fileId);

/*
 * Receive file and save as tempfile to server
 * Return tempfile name if success, else throw exception
 */
std::string recvUploadFile(const int contentLen, FileInfo& fileInfo);

/*
 * fcgiParseHead
 *     -- parsed boundary and file name from message
 * return read length from stdin
 */
int fcgiParseHead(std::string& boundaryStr, FileInfo& fileInfo);

/*
 * fcgiParseContent
 *     -- parsed file content
 * @mainContentLen -- main content length to read here
 * @out -- output FILE* to fileName@fcgiParseHead
 * return read length from stdin
 */
int fcgiParseContent(const int mainContentLen, FILE* out);

/*
 * fcgiParseTailor
 *     -- check and deal file end
 * @boundaryStr -- boundary from @fcgiParseHead
 * @out -- output FILE* to fileName@fcgiParseHead
 * return read length from stdin
 */
int fcgiParseTailor(const std::string& boundaryStr, FILE* out);