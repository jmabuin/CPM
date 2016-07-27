#ifndef SSH_HANDLER_H
#define SSH_HANDLER_H

#include <libssh/sftp.h>
#include <libssh/libssh.h>
#include <string>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LINE_SIZE 2048

class SSH_Handler
{
public:
	SSH_Handler();
	SSH_Handler(std::string hostname, int port, int verbosity, std::string username, std::string password);

	int connect();
	int disconnect();
	int execute_remote_command(std::string command, char *buffer);
	int sftp_allocate();
	int sftp_deallocate();
	int sftp_ownmkdir(std::string dirName);
	int sftp_ownrmdir(std::string dirName);
	int sftp_copyFileToRemote(std::string localFileName, std::string remoteFileName);

private:
	ssh_session my_ssh_session;
	sftp_session sftp;
	int verbosity ;
	int port;
	std::string host;
	std::string username;
	std::string password;


	int verify_knownhost();


};

#endif // SSH_HANDLER_H
