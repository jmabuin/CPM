#include "controller/ssh_handler.h"

// Constructors
SSH_Handler::SSH_Handler() {

	this->verbosity = SSH_LOG_PROTOCOL;
	this->port = 22;
	this->host = "localhost";

}

SSH_Handler::SSH_Handler(std::string hostname, int port, int verbosity, std::string username, std::string password) {

	this->host = hostname;
	this->port = port;
	this->verbosity = verbosity;
	this->username = username;
	this->password = password;

}

//Functions to connect and disconnect
int SSH_Handler::connect() {

	int rc;
	this->my_ssh_session = ssh_new();

	if (this->my_ssh_session == NULL)
		return 0;

	ssh_options_set(this->my_ssh_session, SSH_OPTIONS_HOST, this->host.c_str());
	ssh_options_set(this->my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &(this->verbosity));
	ssh_options_set(this->my_ssh_session, SSH_OPTIONS_PORT, &(this->port));

	rc = ssh_connect(this->my_ssh_session);

	if (rc != SSH_OK)
	{
	  fprintf(stderr, "Error connecting to localhost: %s\n", ssh_get_error(this->my_ssh_session));
	  return 0;
	}

	// Verify the server's identity
	// For the source code of verify_knowhost(), check previous example
	if (this->verify_knownhost(this->my_ssh_session) < 0) {
		ssh_disconnect(this->my_ssh_session);
		ssh_free(this->my_ssh_session);
		return 0;
	}

	// Authenticate ourselves
	// password = getpass("Password: ");
	rc = ssh_userauth_password(this->my_ssh_session, this->username.c_str(), this->password.c_str());

	if (rc != SSH_AUTH_SUCCESS) {
		fprintf(stderr, "Error authenticating with password: %s\n", ssh_get_error(my_ssh_session));
		ssh_disconnect(this->my_ssh_session);
		ssh_free(this->my_ssh_session);
		return 0;
	}

	return 1;
}

int SSH_Handler::disconnect() {

	ssh_disconnect(this->my_ssh_session);
	ssh_free(this->my_ssh_session);

	return 1;
}


int SSH_Handler::verify_knownhost(ssh_session session) {
	char *hexa;
	int state;
	char buf[10];
	unsigned char *hash = NULL;
	size_t hlen;
	ssh_key srv_pubkey;
	int rc;

	state = ssh_is_server_known(session);

	rc = ssh_get_publickey(session, &srv_pubkey);
	if (rc < 0) {
		return -1;
	}

	rc = ssh_get_publickey_hash(srv_pubkey,
				      SSH_PUBLICKEY_HASH_SHA1,
				      &hash,
				      &hlen);
	ssh_key_free(srv_pubkey);

	if (rc < 0) {
		return -1;
	}

	switch(state){

		case SSH_SERVER_KNOWN_OK:
			break; /* ok */

		case SSH_SERVER_KNOWN_CHANGED:
			fprintf(stderr,"Host key for server changed : server's one is now :\n");
			ssh_print_hexa("Public key hash",hash, hlen);
			ssh_clean_pubkey_hash(&hash);
			fprintf(stderr,"For security reason, connection will be stopped\n");
			return -1;

		case SSH_SERVER_FOUND_OTHER:
			fprintf(stderr,"The host key for this server was not found but an other type of key exists.\n");
			fprintf(stderr,"An attacker might change the default server key to confuse your client"
			"into thinking the key does not exist\n"
			"We advise you to rerun the client with -d or -r for more safety.\n");
			return -1;

		case SSH_SERVER_FILE_NOT_FOUND:
			fprintf(stderr,"Could not find known host file. If you accept the host key here,\n");
			fprintf(stderr,"the file will be automatically created.\n");
			/* fallback to SSH_SERVER_NOT_KNOWN behavior */

		case SSH_SERVER_NOT_KNOWN:
			hexa = ssh_get_hexa(hash, hlen);
			fprintf(stderr,"The server is unknown. Do you trust the host key ?\n");
			fprintf(stderr, "Public key hash: %s\n", hexa);
			ssh_string_free_char(hexa);

			if (fgets(buf, sizeof(buf), stdin) == NULL) {
				ssh_clean_pubkey_hash(&hash);
				return -1;
			}

			if(strncasecmp(buf,"yes",3)!=0){
				ssh_clean_pubkey_hash(&hash);
				return -1;
			}

			fprintf(stderr,"This new key will be written on disk for further usage. do you agree ?\n");

			if (fgets(buf, sizeof(buf), stdin) == NULL) {
				ssh_clean_pubkey_hash(&hash);
				return -1;
			}

			if(strncasecmp(buf,"yes",3)==0){
				if (ssh_write_knownhost(session) < 0) {
					ssh_clean_pubkey_hash(&hash);
					fprintf(stderr, "error %s\n", strerror(errno));
					return -1;
				}
			}

			break;

		case SSH_SERVER_ERROR:
			ssh_clean_pubkey_hash(&hash);
			fprintf(stderr,"%s",ssh_get_error(session));
			return -1;
	}

	ssh_clean_pubkey_hash(&hash);
	return 0;
}

//Function to execute a remote command
int SSH_Handler::execute_remote_command(ssh_session session, std::string command) {

	ssh_channel channel;
	int rc;
	char buffer[256];
	int nbytes;

	channel = ssh_channel_new(session);

	if (channel == NULL)
		return SSH_ERROR;

	rc = ssh_channel_open_session(channel);

	if (rc != SSH_OK) {
		ssh_channel_free(channel);
		return rc;
	}

	rc = ssh_channel_request_exec(channel, command.c_str());

	if (rc != SSH_OK) {
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return rc;
	}

	nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);

	while (nbytes > 0) {

		if (write(1, buffer, nbytes) != (unsigned int) nbytes) {
			ssh_channel_close(channel);
			ssh_channel_free(channel);
			return SSH_ERROR;
		}

		nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
	}

	if (nbytes < 0) {
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return SSH_ERROR;
	}

	ssh_channel_send_eof(channel);
	ssh_channel_close(channel);
	ssh_channel_free(channel);
	return SSH_OK;

}

// Allocate SFTP session
int SSH_Handler::sftp_allocate(ssh_session session) {

	int rc;
	this->sftp = sftp_new(session);

	if (this->sftp == NULL) {
		fprintf(stderr, "Error allocating SFTP session: %s\n", ssh_get_error(session));
		return SSH_ERROR;
	}

	rc = sftp_init(this->sftp);

	if (rc != SSH_OK) {
		fprintf(stderr, "Error initializing SFTP session: %d.\n", sftp_get_error(this->sftp));
		sftp_free(this->sftp);
		return rc;
	}

	return SSH_OK;
}

// Deallocate SFTP session
int SSH_Handler::sftp_deallocate() {

	sftp_free(this->sftp);
	return 1;
}

//Creates a remote directory
int SSH_Handler::sftp_ownmkdir(std::string dirName) {

	int rc;
	rc = sftp_mkdir(this->sftp, dirName.c_str(), S_IRWXU);

	if (rc != SSH_OK) {
		if (sftp_get_error(this->sftp) != SSH_FX_FILE_ALREADY_EXISTS) {
			fprintf(stderr, "Can't create directory: %s\n", ssh_get_error(this->my_ssh_session));
			return rc;
		}
	}

	return SSH_OK;
}

int SSH_Handler::sftp_copyFileToRemote(std::string localFileName, std::string remoteFileName) {

	int access_type = O_WRONLY | O_CREAT | O_TRUNC;

	int rc;
	unsigned int nwritten;

	sftp_file remoteFile = sftp_open(this->sftp, remoteFileName.c_str(), access_type, S_IRWXU);

	if (remoteFile == NULL) {
		fprintf(stderr, "Can't open file for writing: %s\n", ssh_get_error(this->my_ssh_session));
		return SSH_ERROR;
	}

	FILE *file = fopen ( localFileName.c_str(), "r" );

	if (file == NULL) {

		fprintf(stderr, "Can't read local file: %s\n", localFileName.c_str());
		return 0;

	}

	char line[MAX_LINE_SIZE];

	while(fgets(line, sizeof(line), file)) {

		nwritten = sftp_write(remoteFile, line, strlen(line));

		if (nwritten != strlen(line)) {
			fprintf(stderr, "Can't write data to file: %s\n", ssh_get_error(this->my_ssh_session));
			sftp_close(remoteFile);
			return SSH_ERROR;
		}


		  // fputs ( line, stdout ); /* write the line */
	}

	fclose(file);

	rc = sftp_close(remoteFile);

	if (rc != SSH_OK) {
		fprintf(stderr, "Can't close the written file: %s\n", ssh_get_error(this->my_ssh_session));
		return rc;
	}

	return SSH_OK;

}
