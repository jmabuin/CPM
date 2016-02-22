/**
  * Copyright 2015 José Manuel Abuín Mosquera <josemanuel.abuin@usc.es>
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

#include "Configuration.h"

/*!
 * \brief Configuration::Configuration Class
 * \details Constructor. This is the class that contains all the configuration parameters. here the keys are set.
 * \author Jose M. Abuin
 *
 */
Configuration::Configuration()
{

	this->configFile		= "Configuration";
	
	this->userKey			= "User";
	this->passwordKey		= "Password";
	this->nodesKey			= "Nodes";
	this->portKey			= "Port";
	this->keyFileKey		= "Key";
	this->nodesBMKey		= "NodesBM";
	this->networkKey		= "NetInterface";
	this->processOwnerKey		= "Owner";
	this->processNameKey		= "ProcessName";
	this->processStartsWithKey	= "ProcessStartsWith";
	this->networkKey		= "NetworkInterface";

	this->measureCPU_Key		= "M_CPU";
	this->measureMEM_Key		= "M_MEM";
	this->measurePapi_Key		= "M_PAPI";

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
			currentConfig.port = this->settings->value(this->portKey.c_str()).toString().toStdString();
		}

		if(this->settings->contains(this->keyFileKey.c_str())) {
			currentConfig.key = this->settings->value(this->keyFileKey.c_str()).toString().toStdString();
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

		if(this->settings->contains(this->nodesBMKey.c_str())) {
			currentConfig.nodesBM = this->settings->value(this->nodesBMKey.c_str()).toString().toStdString();
		}

		if(this->settings->contains(this->networkKey.c_str())) {
			currentConfig.networkInterface = this->settings->value(this->networkKey.c_str()).toString().toStdString();
		}

		if(this->settings->contains(this->measureCPU_Key.c_str())) {
			currentConfig.checkCPU_Status = this->settings->value(this->measureCPU_Key.c_str(), false).toBool();
		}

		if(this->settings->contains(this->measureMEM_Key.c_str())) {
			currentConfig.checkMEM_Status = this->settings->value(this->measureMEM_Key.c_str(), false).toBool();
		}

		if(this->settings->contains(this->measurePapi_Key.c_str())) {
			currentConfig.checkPapi_Status = this->settings->value(this->measurePapi_Key.c_str(), false).toBool();
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
	std::string newPort			= conf.port;
	std::string newKey			= conf.key;
	std::string newProcessOwner		= conf.processOwner;
	std::string newProcessName		= conf.processName;
	std::string newProcessStartsWith	= conf.processStartsWith;
	std::string newNetworkInterface		= conf.networkInterface;
	bool newCheckMEM_Status			= conf.checkMEM_Status;
	bool newCheckCPU_Status			= conf.checkMEM_Status;
	bool newCheckPapi_Status		= conf.checkPapi_Status;


	this->settings->setValue(QString(this->userKey.c_str()),QVariant(QString(newUser.c_str())));
	this->settings->setValue(QString(this->passwordKey.c_str()),QVariant(QString(this->encryptDecrypt(newPassword).c_str())));
	this->settings->setValue(QString(this->nodesKey.c_str()),QVariant(QString(newNodes.c_str())));
	this->settings->setValue(QString(this->nodesBMKey.c_str()),QVariant(QString(newNodesBM.c_str())));
	this->settings->setValue(QString(this->portKey.c_str()),QVariant(QString(newPort.c_str())));
	this->settings->setValue(QString(this->keyFileKey.c_str()),QVariant(QString(newKey.c_str())));
	this->settings->setValue(QString(this->processOwnerKey.c_str()),QVariant(QString(newProcessOwner.c_str())));
	this->settings->setValue(QString(this->processNameKey.c_str()),QVariant(QString(newProcessName.c_str())));
	this->settings->setValue(QString(this->processStartsWithKey.c_str()),QVariant(QString(newProcessStartsWith.c_str())));
	this->settings->setValue(QString(this->networkKey.c_str()),QVariant(QString(newNetworkInterface.c_str())));
	this->settings->setValue(QString(this->measureCPU_Key.c_str()),newCheckMEM_Status);
	this->settings->setValue(QString(this->measureMEM_Key.c_str()),newCheckCPU_Status);
	this->settings->setValue(QString(this->measurePapi_Key.c_str()),newCheckPapi_Status);


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
