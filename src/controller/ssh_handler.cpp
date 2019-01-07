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

#include "ssh_handler.h"


/*!
 * \brief Class to handle ssh connections from the program to the cluster master node
 * \author Jose M. Abuin
 */

/*!
 * \brief SSH_Handler::SSH_Handler
 * \details Empty constructor
 */
SSH_Handler::SSH_Handler() {

    this->verbosity = SSH_LOG_PROTOCOL;
    this->port = 22;
    this->host = "localhost";

}

/*!
 * \brief SSH_Handler::SSH_Handler
 * \param hostname A string containing the hostname to connect with
 * \param port An integer which value is the port to connect to
 * \param verbosity Verbosity level
 * \param username String containing the username
 * \param password String containing the password. If empty, the public-private key method will be used
 * \param keyFileName String containing the path to the private key file. if empty, the password method will be used
 */
SSH_Handler::SSH_Handler(std::string hostname, int port, int verbosity, std::string username, std::string password, std::string keyFileName) {

    this->host = hostname;
    this->port = port;
    this->verbosity = verbosity;
    this->username = username;

    // If the password is not empty, we will use it
    if(!password.empty()) {
        this->password = password;
        this->isKeyConnection = false;
    }
        //Otherwise, if the key file name is not empty, we will use this method
    else if(!keyFileName.empty()) {
        this->keyFileName = keyFileName;
        this->isKeyConnection = true;
    }

    this->pkey = NULL;
}

/*!
 * \brief SSH_Handler::connect
 * \details Stablish a connection with the destination host
 * \return 1 if everything went fine. 0 otherwise
 */
int SSH_Handler::connect() {

    int rc;

    // Creation of a new ssh session
    this->my_ssh_session = ssh_new();

    if (this->my_ssh_session == NULL)
        return 0;

    // We set the ssh options
    ssh_options_set(this->my_ssh_session, SSH_OPTIONS_HOST, this->host.c_str());
    ssh_options_set(this->my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &(this->verbosity));
    ssh_options_set(this->my_ssh_session, SSH_OPTIONS_PORT, &(this->port));

    // Stablish connection
    rc = ssh_connect(this->my_ssh_session);

    // Test the connection
    if (rc != SSH_OK) {
        fprintf(stderr, "Error connecting to localhost: %s\n", ssh_get_error(this->my_ssh_session));
        return 0;
    }

    // Verify the server's identity
    if (this->verify_knownhost() < 0) {
        ssh_disconnect(this->my_ssh_session);
        ssh_free(this->my_ssh_session);
        return 0;
    }

    // Authenticate ourselves
    // With username and password
    if(!this->isKeyConnection) {
        rc = ssh_userauth_password(this->my_ssh_session, this->username.c_str(), this->password.c_str());
    }
        // Or with our private key
    else {
        rc = ssh_pki_import_privkey_file(this->keyFileName.c_str(), NULL, NULL, NULL, &(this->pkey));

        if(rc != SSH_OK) {
            ssh_key_free(this->pkey);
            ssh_disconnect(this->my_ssh_session);
            ssh_free(this->my_ssh_session);
            return 0;
        }
        else {
            rc = ssh_userauth_publickey (this->my_ssh_session, this->username.c_str(), this->pkey);
        }
    }

    // Check the return value
    if (rc != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Error authenticating with password: %s\n", ssh_get_error(my_ssh_session));
        ssh_key_free(this->pkey);
        ssh_disconnect(this->my_ssh_session);
        ssh_free(this->my_ssh_session);
        return 0;
    }

    // If we are here everything went fine
    return 1;
}

/*!
 * \brief SSH_Handler::disconnect
 * \details Function to disconnect from the ssh session
 * \return 1 if everything went fine. 0 otherwise
 * \todo Check return values from functions
 */
int SSH_Handler::disconnect() {

    ssh_key_free(this->pkey);
    ssh_disconnect(this->my_ssh_session);
    ssh_free(this->my_ssh_session);

    return 1;
}

/*!
 * \brief SSH_Handler::verify_knownhost
 * \details Function to verify the identity of the host
 * \return 0 if everything went fine. -1 otherwise
 */
int SSH_Handler::verify_knownhost() {
    char *hexa;
    int state;
    char buf[10];
    unsigned char *hash = NULL;
    size_t hlen;
    ssh_key srv_pubkey;
    int rc;

    state = ssh_is_server_known(this->my_ssh_session);

    rc = ssh_get_server_publickey(this->my_ssh_session, &srv_pubkey);
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
                if (ssh_write_knownhost(this->my_ssh_session) < 0) {
                    ssh_clean_pubkey_hash(&hash);
                    fprintf(stderr, "error %s\n", strerror(errno));
                    return -1;
                }
            }

            break;

        case SSH_SERVER_ERROR:
            ssh_clean_pubkey_hash(&hash);
            fprintf(stderr,"%s",ssh_get_error(this->my_ssh_session));
            return -1;
    }

    ssh_clean_pubkey_hash(&hash);
    return 0;
}

/*!
 * \brief SSH_Handler::execute_remote_command
 * \details Function to execute a command in the remote host
 * \param command The command to be executed
 * \param buffer The output from the executed command
 * \return SSH_OK if everuthing went fine. Other libssh values otherwise
 */
int SSH_Handler::execute_remote_command(std::string command, char *buffer) {

    ssh_channel channel;
    int rc;
    unsigned int tmpBufferSize = 512;
    char *tmpBuffer = (char *)calloc(tmpBufferSize, sizeof(char));
    int nbytes;
    unsigned int resultLineNumber = 0;
    channel = ssh_channel_new(this->my_ssh_session);

    if (channel == NULL)
        return SSH_ERROR;

    rc = ssh_channel_open_session(channel);

    if (rc != SSH_OK) {
        fprintf(stderr,"[%s] %s",__func__,ssh_get_error(this->my_ssh_session));
        ssh_channel_free(channel);
        return rc;
    }

    rc = ssh_channel_request_exec(channel, command.c_str());

    if (rc != SSH_OK) {
        fprintf(stderr,"[%s] %s",__func__,ssh_get_error(this->my_ssh_session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return rc;
    }

    nbytes = ssh_channel_read(channel, tmpBuffer, tmpBufferSize*sizeof(char), 0);
    //nbytes = ssh_channel_read(channel, buffer,sizeof(buffer), 0);

    //fprintf(stderr,"[%s] Number of received bytes: %d %lu\n",__func__, nbytes, strlen(buffer));


    while (nbytes > 0) {

        // Result to stdout
        if (write(1, tmpBuffer, nbytes) != (unsigned int) nbytes) {
            fprintf(stderr,"[%s] %s",__func__,ssh_get_error(this->my_ssh_session));
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            return SSH_ERROR;
        }

        //Fills total buffer
        if(resultLineNumber == 0){
            sprintf(buffer,"%s", tmpBuffer);
        }
        else{
            strcat(buffer, tmpBuffer);
        }

        free(tmpBuffer);
        tmpBuffer = (char *)calloc(tmpBufferSize, sizeof(char));

        nbytes = ssh_channel_read(channel, tmpBuffer, tmpBufferSize*sizeof(char), 0);
        resultLineNumber++;

    }

    if (nbytes < 0) {
        fprintf(stderr,"[%s] %s",__func__,ssh_get_error(this->my_ssh_session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return SSH_ERROR;
    }


    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return SSH_OK;

}

/*!
 * \brief SSH_Handler::sftp_allocate
 * \details Function to allocate a sftp session
 * \return SSH_OK if everything went fine. Other libssh values otherwise
 */
int SSH_Handler::sftp_allocate() {

    int rc;
    this->sftp = sftp_new(this->my_ssh_session);

    if (this->sftp == NULL) {
        fprintf(stderr, "Error allocating SFTP session: %s\n", ssh_get_error(this->my_ssh_session));
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

/*!
 * \brief SSH_Handler::sftp_deallocate
 * \details Function to dealocate a sftp session
 * \return 1 if everything went fine
 */
int SSH_Handler::sftp_deallocate() {

    sftp_free(this->sftp);
    return 1;
}

/*!
 * \brief SSH_Handler::sftp_ownmkdir
 * \details Function to create a remote directory
 * \param dirName A string conaining the name of the directory to be created
 * \return SSH_OK if everything went fine. Other libssh values otherwise
 */
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

/*!
 * \brief SSH_Handler::sftp_copyFileToRemote
 * \details Function to copy a local file to a remote destination
 * \param localFileName The path to the local filename
 * \param remoteFileName The path to the remote filename
 * \return SSH_OK if everything went fine. Other libssh values otherwise
 */
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

/*!
 * \brief SSH_Handler::sftp_ownrmdir
 * \details Function to delete a remote directory
 * \param dirName The directory name to be deleted
 * \return SSH_OK if everything went fine. Other libssh values otherwise
 */
int SSH_Handler::sftp_ownrmdir(std::string dirName) {

    return sftp_rmdir(this->sftp, dirName.c_str());

}
