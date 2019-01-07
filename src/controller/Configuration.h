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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QSettings>

/*!
 * \brief Struct that contains all parameters from configuration
 */
typedef struct Config{
	std::string userName;		/*!< String containing the username */
	std::string password;		/*!< String containing the password */
	std::string nodes;		/*!< String containing the master nodes */
	std::string nodesBM;		/*!< String containing the nodes behind master */
	int port;			/*!< Int containing the port for SSH */
	int internalSSH_Port;		/*!< Int containing the port for SSH internal to the cluster */
	int clientPort;			/*!< int containing the port where this client listens */
	int masterPort;			/*!< Int containing the port where the master agent listens */
	int agentPort;			/*!< Int containing the port where the agent listens */
	std::string key;		/*!< String containing the key file */
	std::string processOwner;	/*!< String containing the process owner */
	std::string processName;	/*!< String containing the process name */
	std::string processStartsWith;	/*!< String containing the start part of the process name */
	unsigned int cpuThreshold;	/*!< Unsigned int containing the minimun CPU percentage od the processes to search for */
	std::string networkInterface;	/*!< String containing the network interface to use */
	std::string masterInInterface;	/*!< String containing the input network interface to use in master agent */
	std::string masterOutInterface;	/*!< String containing the output network interface to use in master agent */
	bool checkMEM_Status;		/*!< Boolean value used to measure or not the memory */
	bool checkCPU_Status;		/*!< Boolean value used to measure or not the CPU percentage */
	bool checkPapi_Status;		/*!< Boolean value used to measure or not PAPI counters */
	bool checkEnergy_Status;	/*!< Boolean value used to measure energy using PAPI and RAPL */

	bool SSH_UsernamePassword;
	bool SSH_KeyFile;


} Config;

/*!
 * \brief The Configuration class. Class that contains all the configuration parameters
 * \author Jose M. Abuin
 */

class Configuration
{
public:
	Configuration();			/*!< Constructor */
	Config getConfiguration();		/*!< Returns the Config struct filled with the current configuration parameters */
	void setConfiguration(Config conf);	/*!< Sets the current configuration parameters */

private:
	QSettings *settings;			/*!< QSettings pointer used to get and set the configuration parameters */

	std::string configFile;			/*!< String containing the configuration file name */

	std::string userKey;			/*!< Key for the username */
	std::string passwordKey;		/*!< Key for the password */
	std::string nodesKey;			/*!< Key for nodes */
	std::string portKey;			/*!< Key for port in SSH */
	std::string internalSSH_PortKey;	/*!< Key for port in SSH internal to the cluster*/
	std::string clientPortKey;		/*!< Key for client port */
	std::string masterPortKey;		/*!< Key for master port */
	std::string agentPortKey;		/*!< Key for agent port */
	std::string keyFileKey;			/*!< Key for file containing public key */
	std::string nodesBMKey;			/*!< Key for nodes behind master */
	std::string networkKey;			/*!< Key for the network interface */
	std::string masterInInterfaceKey;	/*!< Key for the master in network interface */
	std::string masterOutInterfaceKey;	/*!< Key for the master out network interface */
	std::string processOwnerKey;		/*!< Key for the process owner */
	std::string processNameKey;		/*!< Key for the process name */
	std::string processStartsWithKey;	/*!< key for the string that contains the start of the process */
	std::string cpu_thresholdKey;		/*!< Key for the CPU threshold value*/

	std::string measureCPU_Key;		/*!< Key to measure or not CPU */
	std::string measureMEM_Key;		/*!< Key to measure or not Memory */
	std::string measurePapi_Key;		/*!< Key to measure or not PAPI counters */
	std::string measureEnergy_Key;		/*!< Key to measure or not energy */

	std::string SSH_UsernamePassword;
	std::string SSH_KeyFile;

	std::string encryptDecrypt(std::string toEncrypt);	/*!< Function to encrypt or decrypt a string */
};

#endif // CONFIGURATION_H
