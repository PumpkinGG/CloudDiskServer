## A server on CentOS of my cloud disk.

- Nginx + FastDFS + FastCGI + MySQL + Redis
  - Nginx as http server
  - FastDFS as file storage
  - FastCGI as a tool to deal post request for Nginx
  - MySQL + Redis as database

- Support
  - User log up && login
  - Redis to save & check token
  - Upload && Download && Fetch user file list for client user