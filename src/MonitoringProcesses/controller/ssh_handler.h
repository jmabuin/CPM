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

private:
	ssh_session my_ssh_session;
	sftp_session sftp;
	int verbosity ;
	int port;
	std::string host;
	std::string username;
	std::string password;

	int connect();
	int disconnect();
	int verify_knownhost(ssh_session session);
	int execute_remote_command(ssh_session session, std::string command);
	int sftp_allocate(ssh_session session);
	int sftp_deallocate();
	int sftp_ownmkdir(std::string dirName);
	int sftp_copyFileToRemote(std::string localFileName, std::string remoteFileName);
};

#endif // SSH_HANDLER_H
