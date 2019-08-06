#include <iostream>
#include <string>
#include <vector>

#include <mysqlx/xdevapi.h>
#include <glog/logging.h>

using namespace mysqlx;
using namespace std;

namespace {
	const std::string kHost = "localhost";
	const std::string kUser = "guo";
	const std::string kPassWord = "930201@Gkh";
	const int kPort = 33060;
	const std::string kDefaultDB = "cloud_disk";
	const std::string kTable = "user";
	const std::string kClientUser = "guokh888";

} // namespace

int main(int argc, char* argv[]) {
	google::InitGoogleLogging(argv[0]);
	FLAGS_log_dir = "./logs";

	Session sess(kHost, kPort, kUser, kPassWord, kDefaultDB);
	// An alternative way of Connecting to MySQL Server on a network machine
	/*Session sess(SessionOption::HOST, kHost,
		SessionOption::PORT, kPort,
		SessionOption::USER, kUser,
		SessionOption::PWD, kPassWord);
	Schema db = sess.getSchema("mydb0");*/

	vector<Schema> schemas = sess.getSchemas();
	cout << "MySQL> Show databases: " << endl;
	for (const auto& s : schemas) {
		cout << s.getName() << endl;
		DLOG(INFO) << s.getName();
	}
	cout << "MySQL> Default database: " << endl;
	cout << sess.getDefaultSchemaName() << endl;

	Schema myDB = sess.getDefaultSchema();
	vector<Table> tables = myDB.getTables();
	cout << "MySQL> Show tables: " << endl;
	for (const auto& t : tables) {
		cout << t.getName() << endl;
	}

	Table userTable = myDB.getTable(kTable, true);
	RowResult users = userTable.select("password", "nickname")
		.where("name = :name")
		.bind("name", "guokh666")
		.execute();
	Row user = users.fetchOne();
	std::string nickname = (std::string) user[1];
	for (int i = 0; i < nickname.size(); i++) {
		printf("\\x%02X", (unsigned char)nickname[i]);
	}
	printf("\n");
	printf("MySql读取数据最后一位为\\x00? ");
	printf("%s\n", ((unsigned char)nickname.back() == 0 ? "Yes" : "No"));
	//DLOG(INFO) << row[0] << " " << endl;

	// Test join
	RowResult files;
	try {
		//sess.sql("use cloud_disk");
		std::string sqlStat;
		sqlStat.append("SELECT u.user_name, u.file_name, DATE_FORMAT(createtime, \"%Y/%m/%d %T\"), u.shared_status, u.download_count, ")
			.append("f.url, f.type, f.size ")
			.append("FROM file_info f, user_file_list u ")
			.append("WHERE u.user_name = '").append(kClientUser).append("' ")
			.append("AND f.md5 = u.md5");
		files = sess.sql(sqlStat).execute();
	}
	catch (const Error& err) {
		cout << "sql " << err.what() << endl;
	}

	try {
		for (const auto& file : files) {
			cout << file.colCount() << " ";
			cout << "User: " << file[0]
				<< " FileName: " << file[1]
				<< " Createtime: " << file[2]
				<< " Shared_status: " << file[3]
				<< " Download_count " << file[4] << endl
				<< " Url " << file[5]
				<< " Type " << file[6]
				<< " Size " << file[7] << endl;
		}
	}
	catch (const Error& err) {
		cout << "result " << err.what() << endl;
	}

	google::ShutdownGoogleLogging();
	return 0;
}