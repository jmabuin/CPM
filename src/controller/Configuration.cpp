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

#include <stdlib.h>
#include "Configuration.h"
#include "Network.h"

/*!
 * \brief Configuration::Configuration Class
 * \details Constructor. This is the class that contains all the configuration parameters. here the keys are set.
 * \author Jose M. Abuin
 *
 */
Configuration::Configuration()
{

	this->configFile			= "Configuration";
	
	this->userKey				= "User";
	this->passwordKey			= "Password";
	this->nodesKey				= "Nodes";
	this->portKey				= "Port";
	this->internalSSH_PortKey	= "InternalSSHPort";
	this->clientPortKey			= "ClientPort";
	this->masterPortKey			= "MasterPort";
	this->agentPortKey			= "AgentPort";
	this->keyFileKey			= "Key";
	this->nodesBMKey			= "NodesBM";
	this->networkKey			= "NetInterface";
	this->processOwnerKey		= "Owner";
	this->processNameKey		= "ProcessName";
	this->processStartsWithKey	= "ProcessStartsWith";
	this->cpu_thresholdKey		= "CPU_Threshold";
	this->networkKey			= "NetworkInterface";
	this->masterInInterfaceKey	= "MasterInInterface";
	this->masterOutInterfaceKey	= "MasterOutInterface";
	this->measureCPU_Key		= "M_CPU";
	this->measureMEM_Key		= "M_MEM";
	this->measurePapi_Key		= "M_PAPI";
	this->measureEnergy_Key		= "M_PAPI_RAPL";

	this->SSH_UsernamePassword	= "SSH_UserPass";
	this->SSH_KeyFile			= "SSH_KeyFile";

	this->use_cmake3Key			= "cmake3";

}

/*!
 * \brief Configuration::getConfiguration This function gets the current configuration
 * \return A struct Config that contains the current configuration
 */
Config Configuration::getConfiguration() {


		Config currentConfig;

		//Settingst are got from file
		this->settings = new QSettings(QString(this->configFile.c_str()),QSettings::NativeFormat);


		//By using the keys, the current settings are obtained
		if(this->settings->contains(this->userKey.c_str())) {
			currentConfig.userName = this->settings->value(this->userKey.c_str()).toString().toStdString();
		}

		if(this->settings->contains(this->passwordKey.c_str())) { //Here we decrypt the value from the configuration file
			currentConfig.password = this->encryptDecrypt(this->settings->value(this->passwordKey.c_str()).toString().toStdString()).c_str();
		}

		if(this->settings->contains(this->nodesKey.c_str())){
			currentConfig.nodes = this->settings->value(this->nodesKey.c_str()).toString().toStdString();
		}

		if(this->settings->contains(this->portKey.c_str())) {
			currentConfig.port = this->settings->value(this->portKey.c_str()).toInt(); //.toString().toStdString();
		}
		else{
			currentConfig.port = 22;
		}

		if(this->settings->contains(this->internalSSH_PortKey.c_str())) {
			currentConfig.internalSSH_Port = this->settings->value(this->internalSSH_PortKey.c_str()).toInt(); //.toString().toStdString();
		}
		else{
			currentConfig.internalSSH_Port = 22;
		}


		if(this->settings->contains(this->clientPortKey.c_str())) {
			// currentConfig.clientPort = this->settings->value(this->clientPortKey.c_str()).toString().toStdString();
			currentConfig.clientPort = this->settings->value(this->clientPortKey.c_str()).toInt();
		}
		else{
			currentConfig.clientPort = CLIENT_BASE_PORT;
		}

		if(this->settings->contains(this->masterPortKey.c_str())) {
			// currentConfig.masterPort = this->settings->value(this->masterPortKey.c_str()).toString().toStdString();
			currentConfig.masterPort = this->settings->value(this->masterPortKey.c_str()).toInt();
		}
		else{
			currentConfig.masterPort = MASTER_BASE_PORT;
		}

		if(this->settings->contains(this->agentPortKey.c_str())) {
			// currentConfig.masterPort = this->settings->value(this->masterPortKey.c_str()).toString().toStdString();
			currentConfig.agentPort = this->settings->value(this->agentPortKey.c_str()).toInt();
		}
		else{
			currentConfig.agentPort = DAEMON_AGENT_PORT;
		}

		if(this->settings->contains(this->keyFileKey.c_str())) {
			currentConfig.key = this->settings->value(this->keyFileKey.c_str()).toString().toStdString();
		}
		else{
			currentConfig.key = "";
		}

		if(this->settings->contains(this->processOwnerKey.c_str())) {
			currentConfig.processOwner = this->settings->value(this->processOwnerKey.c_str()).toString().toStdString();
		}

		if(this->settings->contains(this->processNameKey.c_str())) {
			currentConfig.processName = this->settings->value(this->processNameKey.c_str()).toString().toStdString();
		}

		if(this->settings->contains(this->processStartsWithKey.c_str())) {
			currentConfig.processStartsWith = this->settings->value(this->processStartsWithKey.c_str()).toString().toStdString();
		}

		if(this->settings->contains(this->cpu_thresholdKey.c_str())) {
			currentConfig.cpuThreshold = this->settings->value(this->cpu_thresholdKey.c_str()).toUInt();
		}
		else{
			currentConfig.cpuThreshold = 0;
		}

		if(this->settings->contains(this->nodesBMKey.c_str())) {
			currentConfig.nodesBM = this->settings->value(this->nodesBMKey.c_str()).toString().toStdString();
		}

		if(this->settings->contains(this->networkKey.c_str())) {
			currentConfig.networkInterface = this->settings->value(this->networkKey.c_str()).toString().toStdString();
		}

		if(this->settings->contains(this->masterInInterfaceKey.c_str())) {
			currentConfig.masterInInterface = this->settings->value(this->masterInInterfaceKey.c_str()).toString().toStdString();
		}

		if(this->settings->contains(this->masterOutInterfaceKey.c_str())) {
			currentConfig.masterOutInterface = this->settings->value(this->masterOutInterfaceKey.c_str()).toString().toStdString();
		}

		if(this->settings->contains(this->measureCPU_Key.c_str())) {
			currentConfig.checkCPU_Status = this->settings->value(this->measureCPU_Key.c_str(), false).toBool();
		}
		else{
			currentConfig.checkCPU_Status = false;
		}

		if(this->settings->contains(this->measureMEM_Key.c_str())) {
			currentConfig.checkMEM_Status = this->settings->value(this->measureMEM_Key.c_str(), false).toBool();
		}
		else{
			currentConfig.checkMEM_Status = false;
		}

		if(this->settings->contains(this->measurePapi_Key.c_str())) {
			currentConfig.checkPapi_Status = this->settings->value(this->measurePapi_Key.c_str(), false).toBool();
		}
		else{
			currentConfig.checkPapi_Status = false;
		}

		if(this->settings->contains(this->measureEnergy_Key.c_str())) {
			currentConfig.checkEnergy_Status = this->settings->value(this->measureEnergy_Key.c_str(), false).toBool();
		}
		else{
			currentConfig.checkEnergy_Status = false;
		}

		if(this->settings->contains(this->SSH_UsernamePassword.c_str())) {
			currentConfig.SSH_UsernamePassword = this->settings->value(this->SSH_UsernamePassword.c_str(), false).toBool();
		}
		else{
			currentConfig.SSH_UsernamePassword = true;
		}

		if(this->settings->contains(this->SSH_KeyFile.c_str())) {
			currentConfig.SSH_KeyFile = this->settings->value(this->SSH_KeyFile.c_str(), false).toBool();
		}
		else{
			currentConfig.SSH_KeyFile = false;
		}

		if(this->settings->contains(this->use_cmake3Key.c_str())) {
			currentConfig.use_cmake3 = this->settings->value(this->use_cmake3Key.c_str(), false).toBool();
		}
		else{
			currentConfig.use_cmake3 = false;
		}

		return currentConfig;

	}

/*!
 * \brief Configuration::setConfiguration This procedure allows to set a configuration that is passed as argument
 * \param conf The struct Config with the configuration parameters to be set
 */
void Configuration::setConfiguration(Config conf){

	//this->configFile	   = "Configuration";
	this->settings = new QSettings(QString(this->configFile.c_str()),QSettings::NativeFormat);

	std::string newUser			= conf.userName;
	std::string newPassword			= conf.password;
	std::string newNodes			= conf.nodes;
	std::string newNodesBM			= conf.nodesBM;
	int newPort				= conf.port;
	int newInternalSSH_Port			= conf.internalSSH_Port;
	int newClientPort			= conf.clientPort;
	int newMasterPort			= conf.masterPort;
	int newAgentPort			= conf.agentPort;
	std::string newKey			= conf.key;
	std::string newProcessOwner		= conf.processOwner;
	std::string newProcessName		= conf.processName;
	std::string newProcessStartsWith	= conf.processStartsWith;
	unsigned int newCpu_Threshold		= conf.cpuThreshold;
	std::string newNetworkInterface		= conf.networkInterface;
	std::string newMasterInInterface	= conf.masterInInterface;
	std::string newMasterOutInterface	= conf.masterOutInterface;
	bool newCheckMEM_Status			= conf.checkMEM_Status;
	bool newCheckCPU_Status			= conf.checkMEM_Status;
	bool newCheckPapi_Status		= conf.checkPapi_Status;
	bool newCheckEnergy_Status		= conf.checkEnergy_Status;

	bool newSSH_UsernamePassword		= conf.SSH_UsernamePassword;
	bool newSSH_KeyFile			= conf.SSH_KeyFile;

	bool newUse_cmake3					= conf.use_cmake3;

	this->settings->setValue(QString(this->userKey.c_str()),QVariant(QString(newUser.c_str())));
	this->settings->setValue(QString(this->passwordKey.c_str()),QVariant(QString(this->encryptDecrypt(newPassword).c_str())));
	this->settings->setValue(QString(this->nodesKey.c_str()),QVariant(QString(newNodes.c_str())));
	this->settings->setValue(QString(this->nodesBMKey.c_str()),QVariant(QString(newNodesBM.c_str())));

	char *buffer = (char *) calloc(6,sizeof(char));
	sprintf(buffer, "%d",newPort);
	this->settings->setValue(QString(this->portKey.c_str()),QVariant(QString(buffer)));
	free(buffer);

	buffer = (char *) calloc(6,sizeof(char));
	sprintf(buffer, "%d",newInternalSSH_Port);
	this->settings->setValue(QString(this->internalSSH_PortKey.c_str()),QVariant(QString(buffer)));
	free(buffer);

	buffer = (char *) calloc(6,sizeof(char));
	sprintf(buffer, "%d",newClientPort);
	this->settings->setValue(QString(this->clientPortKey.c_str()),QVariant(QString(buffer)));
	free(buffer);

	buffer = (char *) calloc(6,sizeof(char));
	sprintf(buffer, "%d",newMasterPort);
	this->settings->setValue(QString(this->masterPortKey.c_str()),QVariant(QString(buffer)));
	free(buffer);

	buffer = (char *) calloc(6,sizeof(char));
	sprintf(buffer, "%d",newAgentPort);
	this->settings->setValue(QString(this->agentPortKey.c_str()),QVariant(QString(buffer)));
	free(buffer);

	this->settings->setValue(QString(this->keyFileKey.c_str()),QVariant(QString(newKey.c_str())));
	this->settings->setValue(QString(this->processOwnerKey.c_str()),QVariant(QString(newProcessOwner.c_str())));
	this->settings->setValue(QString(this->processNameKey.c_str()),QVariant(QString(newProcessName.c_str())));
	this->settings->setValue(QString(this->processStartsWithKey.c_str()),QVariant(QString(newProcessStartsWith.c_str())));

	buffer = (char *) calloc(4,sizeof(char));
	sprintf(buffer, "%u",newCpu_Threshold);
	this->settings->setValue(QString(this->cpu_thresholdKey.c_str()),QVariant(QString(buffer)));
	free(buffer);

	this->settings->setValue(QString(this->networkKey.c_str()),QVariant(QString(newNetworkInterface.c_str())));
	this->settings->setValue(QString(this->masterInInterfaceKey.c_str()),QVariant(QString(newMasterInInterface.c_str())));
	this->settings->setValue(QString(this->masterOutInterfaceKey.c_str()),QVariant(QString(newMasterOutInterface.c_str())));
	this->settings->setValue(QString(this->measureCPU_Key.c_str()),newCheckMEM_Status);
	this->settings->setValue(QString(this->measureMEM_Key.c_str()),newCheckCPU_Status);
	this->settings->setValue(QString(this->measurePapi_Key.c_str()),newCheckPapi_Status);
	this->settings->setValue(QString(this->measureEnergy_Key.c_str()),newCheckEnergy_Status);
	this->settings->setValue(QString(this->SSH_UsernamePassword.c_str()),newSSH_UsernamePassword);
	this->settings->setValue(QString(this->SSH_KeyFile.c_str()),newSSH_KeyFile);

	this->settings->setValue(QString(this->use_cmake3Key.c_str()),newUse_cmake3);


}

/*!
 * \brief Configuration::encryptDecrypt Function that is used to encrypt or decrypt a string.
 * \param toEncrypt The string to be encrypted/decrypted
 * \return The String encrypted or decrypted
 */
std::string Configuration::encryptDecrypt(std::string toEncrypt) {
	char key[3] = {'K', 'C', 'Q'}; //Any chars will work
	std::string output = toEncrypt;

	for (unsigned int i = 0; i < toEncrypt.size(); i++)
		output[i] = toEncrypt[i] ^ key[i % (sizeof(key) / sizeof(char))];

	return output;
}
