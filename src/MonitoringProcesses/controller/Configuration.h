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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QSettings>

typedef struct Config{
	std::string userName;
	std::string password;
	std::string nodes;
	std::string nodesBM;
	std::string port;
	std::string key;
	std::string processOwner;
	std::string processName;
	std::string processStartsWith;
	std::string networkInterface;
	bool checkMEM_Status;
	bool checkCPU_Status;
	bool checkPapi_Status;


} Config;

class Configuration
{
public:
	Configuration();
	Config getConfiguration();
	void setConfiguration(Config conf);

private:
	QSettings *settings;

	std::string configFile;

	std::string userKey;
	std::string passwordKey;
	std::string nodesKey;
	std::string portKey;
	std::string keyFileKey;
	std::string nodesBMKey;
	std::string networkKey;
	std::string processOwnerKey;
	std::string processNameKey;
	std::string processStartsWithKey;

	std::string measureCPU_Key;
	std::string measureMEM_Key;
	std::string measurePapi_Key;

	std::string encryptDecrypt(std::string toEncrypt);
};

#endif // CONFIGURATION_H
