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

#include "ConfigurationWindow.h"
#include "ui_ConfigurationWindow.h"

/*!
 * \brief ConfigurationWindow::ConfigurationWindow Constructor
 * \param parent The parent widget
 */
ConfigurationWindow::ConfigurationWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ConfigurationWindow)
{
	ui->setupUi(this);


	this->initConfiguration();

	//connect(this->ui->pushButton_KeyFile, SIGNAL("clicked()"), this->openKeyFileRun);
	//connect(this->ui->pushButton_NodesInfo, SIGNAL("clicked()"), this->showNodesInfo);

}

/*!
 * \brief ConfigurationWindow::~ConfigurationWindow Destructor
 */
ConfigurationWindow::~ConfigurationWindow()
{
	delete ui;
}


/*!
 * \brief ConfigurationWindow::done This method oberwrites the done method. Its code is executed when the Accept or Cancel buttons are clicked
 * \param result An integer that depends on the clicked button
 */
void ConfigurationWindow::done(int result) {
	if(result == QDialog::Accepted) {
		
		//printf("Accepted configuration\n");
		if(this->saveAndClose()){
			QDialog::done(result);
			return;
		}


	}
	else {
		QDialog::done(result);
		return;
	}

}

/*!
 * /brief ConfigurationWindow::initConfiguration Procedure to init Configuration Window with the stored values or the default ones (if there are none stored).
 * /param filename File where the configuration is stored.
 */
void ConfigurationWindow::initConfiguration(){

	Configuration conf = Configuration();

	Config cnf = conf.getConfiguration();

	//this->settings = new QSettings(QString(this->configFile.c_str()),QSettings::NativeFormat);

	//Default checkboxes status
	this->ui->checkBox_CPU->setChecked(true);

	this->ui->checkBox_CPU->setEnabled(false);
	this->ui->checkBox_MEM->setChecked(true);
	this->ui->checkBox_MEM->setEnabled(false);


	//Get the network interfaces and list them in the combobox
	Network networkObject = Network();

	std::vector<std::string> interfaces = networkObject.getInterfaces();

	for (unsigned int i = 0; i< interfaces.size(); i++) {
		this->ui->comboBox_NetworkInterfaces->addItem(QString(interfaces.at(i).c_str()));
	}

	if(!cnf.networkInterface.empty()){
		for (int i = 0; i< this->ui->comboBox_NetworkInterfaces->count(); i++) {
			if(strcmp(cnf.networkInterface.c_str(),this->ui->comboBox_NetworkInterfaces->itemText(i).toStdString().c_str())==0){
				this->ui->comboBox_NetworkInterfaces->setCurrentIndex(i);
			}
		}
	}



	//if(this->settings->contains(this->userKey.c_str())) {
	if(!cnf.userName.empty()) {
		this->ui->LineEdit_Username->setText(cnf.userName.c_str());
	}

	//if(this->settings->contains(this->passwordKey.c_str())) { //Here we decrypt the value from the configuration file
	if(!cnf.password.empty()) {
		this->ui->LineEdit_Password->setText(cnf.password.c_str());
	}

	if(!cnf.nodes.empty()){
		this->ui->plainTextEdit_ClusterNodeList->setPlainText(cnf.nodes.c_str());
	}

	//if(this->settings->contains(this->portKey.c_str())) {
	if(!cnf.port.empty()) {
		this->ui->LineEdit_Port->setText(cnf.port.c_str());
	}

	//if(this->settings->contains(this->keyFileKey.c_str())) {
	if(!cnf.key.empty()) {
		this->ui->LineEdit_KeyFile->setText(cnf.key.c_str());
	}

	if(!cnf.processOwner.empty()) {
		this->ui->LineEdit_ProcessOwner->setText(cnf.processOwner.c_str());
	}

	if(!cnf.processName.empty()) {
		this->ui->LineEdit_ProcessName->setText(cnf.processName.c_str());
	}

	if(!cnf.processStartsWith.empty()) {
		this->ui->LineEdit_ProcessStartsWith->setText(cnf.processStartsWith.c_str());
	}

	if(!cnf.nodesBM.empty()) {
		this->ui->plainTextEdit_ClusterNodeListBM->setPlainText(cnf.nodesBM.c_str());
	}


	this->ui->checkBox_CPU->setChecked(cnf.checkCPU_Status);



	this->ui->checkBox_MEM->setChecked(cnf.checkMEM_Status);



	this->ui->checkBox_PapiCounters->setChecked(cnf.checkPapi_Status);



}

/*!
 * \brief ConfigurationWindow::saveAndClose Function to save the current configuration settings and close the window
 * \return A boolean value that is true if the configuration parameters are correctly stored
 */
bool ConfigurationWindow::saveAndClose() {

	//Save settings and close

	//1.- Check values
	std::string newUser			= this->ui->LineEdit_Username->text().toStdString();
	std::string newPassword			= this->ui->LineEdit_Password->text().toStdString();
	std::string newNodes			= this->ui->plainTextEdit_ClusterNodeList->toPlainText().toStdString();
	std::string newNodesBM			= this->ui->plainTextEdit_ClusterNodeListBM->toPlainText().toStdString();
	std::string newPort			= this->ui->LineEdit_Port->text().toStdString();
	std::string newKey			= this->ui->LineEdit_KeyFile->text().toStdString();
	std::string newProcessOwner		= this->ui->LineEdit_ProcessOwner->text().toStdString();
	std::string newProcessName		= this->ui->LineEdit_ProcessName->text().toStdString();
	std::string newProcessStartsWith	= this->ui->LineEdit_ProcessStartsWith->text().toStdString();
	std::string newNetworkInterface		= this->ui->comboBox_NetworkInterfaces->currentText().toStdString();
	bool newCheckMEM_Status			= this->ui->checkBox_MEM->isChecked();
	bool newCheckCPU_Status			= this->ui->checkBox_CPU->isChecked();
	bool newCheckPapi_Status		= this->ui->checkBox_PapiCounters->isChecked();

	QMessageBox msgBox;
	msgBox.setWindowTitle("Error");
	//msgBox.setText("Question");
	msgBox.setStandardButtons(QMessageBox::Ok);
	//msgBox.setDefaultButton(QMessageBox::No);


	//We check inserted values
	if(!this->isNumeric(newPort)){
		msgBox.setText("Port must be a numeric value");
		msgBox.exec();

		return false;
	}

	else{

		Configuration config = Configuration();

		Config conf = config.getConfiguration();

		conf.userName		= newUser;
		conf.password		= newPassword;
		conf.nodes		= newNodes;
		conf.nodesBM		= newNodesBM;
		conf.port		= newPort;
		conf.key		= newKey;
		conf.processOwner	= newProcessOwner;
		conf.processName	= newProcessName;
		conf.processStartsWith	= newProcessStartsWith;
		conf.networkInterface	= newNetworkInterface;
		conf.checkMEM_Status	= newCheckMEM_Status;
		conf.checkCPU_Status	= newCheckCPU_Status;
		conf.checkPapi_Status	= newCheckPapi_Status;


		/*
		this->settings->setValue(QString(this->userKey.c_str()),QVariant(QString(newUser.c_str())));
		this->settings->setValue(QString(this->passwordKey.c_str()),QVariant(QString(this->encryptDecrypt(newPassword).c_str())));
		this->settings->setValue(QString(this->nodesKey.c_str()),QVariant(QString(newNodes.c_str())));
		this->settings->setValue(QString(this->nodesBMKey.c_str()),QVariant(QString(newNodesBM.c_str())));
		this->settings->setValue(QString(this->portKey.c_str()),QVariant(QString(newPort.c_str())));
		this->settings->setValue(QString(this->keyFileKey.c_str()),QVariant(QString(newKey.c_str())));
		this->settings->setValue(QString(this->processOwnerKey.c_str()),QVariant(QString(newProcessOwner.c_str())));
		this->settings->setValue(QString(this->processNameKey.c_str()),QVariant(QString(newProcessName.c_str())));
		this->settings->setValue(QString(this->measureCPU_Key.c_str()),newCheckMEM_Status);
		this->settings->setValue(QString(this->measureMEM_Key.c_str()),newCheckCPU_Status);
		this->settings->setValue(QString(this->measurePapi_Key.c_str()),newCheckPapi_Status);
*/

		config.setConfiguration(conf);

		return true;

	}


}

/**
 * Procedure to check if a given string value is alphanumeric.
 * @param String to check.
 * @return true if the given string is alphanumeric, false otherwise.
 */
bool ConfigurationWindow::isAlnum(std::string cadea){

	unsigned int i = 0;

	for(i = 0;i< cadea.length();i++){
		if(!std::isalnum(cadea.c_str()[i])){
			return false;
		}
	}

	return true;

}

/**
 * Procedure to check if a given string value is a number.
 * @param String to check.
 * @return true if the given string is numeric, false otherwise.
 */
bool ConfigurationWindow::isNumeric(std::string cadea){

	unsigned int i = 0;

	for(i = 0;i< cadea.length();i++){
		if(!std::isdigit(cadea.c_str()[i])){
			return false;
		}
	}

	return true;

}



/*
Configuration ConfigurationWindow::getConfiguration() {

	Configuration currentConfig;

	this->settings = new QSettings(QString(this->configFile.c_str()),QSettings::NativeFormat);



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

	if(this->settings->contains(this->nodesBMKey.c_str())) {
		currentConfig.nodesBM = this->settings->value(this->nodesBMKey.c_str()).toString().toStdString();
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
*/
