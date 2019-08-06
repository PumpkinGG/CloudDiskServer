CREATE TABLE user (
  id bigint PRIMARY KEY AUTO_INCREMENT,
  name varchar(128) UNIQUE,
  nickname varchar(128) UNIQUE,
  password varchar(128) NOT NULL,
  phone varchar(15),
  createtime timestamp DEFAULT CURRENT_TIMESTAMP,
  email varchar(100),
);

CREATE TABLE file_info (
  md5 varchar(200) PRIMARY KEY,
  url varchar(512) NOT NULL,
  size bigint,
  type varchar(20),
  count int DEFAULT 1,
);

CREATE TABLE user_file_list (
  user_name varchar(128) NOT NULL,
  md5 varchar(200) NOT NULL,
  createtime timestamp DEFAULT CURRENT_TIMESTAMP,
  file_name varchar(128),
  shared_status int DEFAULT 0,
  download_count int DEFAULT 0,
  CONSTRAINT fk_user_file_list_md5 FOREIGN KEY (md5) REFERENCES file_info (md5),
  CONSTRAINT fk_user_file_list_user_name FOREIGN KEY (user_name) REFERENCES user (name)
);

CREATE TABLE shared_file_list (
  user_name varchar(128) NOT NULL,
  md5 varchar(200) NOT NULL,
  sharedtime timestamp DEFAULT CURRENT_TIMESTAMP,
  file_name varchar(128),
  download_count int DEFAULT 1,
  CONSTRAINT fk_shared_file_list_md5 FOREIGN KEY (md5) REFERENCES file_info (md5),
  CONSTRAINT fk_shared_file_list_user_name FOREIGN KEY (user_name) REFERENCES user (name)
);

CREATE VIEW AS
  SELECT user_name, COUNT(md5) as count
  FROM user_file_list
  GROUP BY user_name
  ORDER BY count DESC;