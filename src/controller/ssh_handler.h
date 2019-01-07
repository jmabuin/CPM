/**
  * Copyright 2016 José Manuel Abuín Mosquera <josemanuel.abuin@usc.es>
  *
  * This file is part of CPM.
  *
  * CPM is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * CPM is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with CPM. If not, see <http://www.gnu.org/licenses/>.
  */

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

/*!
 * \brief Class to handle ssh connections from the program to the cluster master node
 * \author Jose M. Abuin
 */
class SSH_Handler
{
public:
	SSH_Handler();
	SSH_Handler(std::string hostname, int port, int verbosity, std::string username, std::string password, std::string keyFileName);
	//SSH_Handler(std::string hostname, int port, int verbosity, std::string username, std::string password);

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
	std::string keyFileName;
	ssh_key pkey;
	bool isKeyConnection;

	int verify_knownhost();


};

#endif // SSH_HANDLER_H
