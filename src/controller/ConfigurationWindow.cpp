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

	this->ui->checkBox_cmake3->setChecked(false);

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

	if(!cnf.masterInInterface.empty()) {
		this->ui->LineEdit_MasterIn->setText(cnf.masterInInterface.c_str());
	}

	if(!cnf.masterOutInterface.empty()) {
		this->ui->LineEdit_MasterOut->setText(cnf.masterOutInterface.c_str());
	}

	//if(this->settings->contains(this->userKey.c_str())) {
	/*if(!cnf.userName.empty()) {
		this->ui->LineEdit_Username->setText(cnf.userName.c_str());
	}

	//if(this->settings->contains(this->passwordKey.c_str())) { //Here we decrypt the value from the configuration file
	if(!cnf.password.empty()) {
		this->ui->LineEdit_Password->setText(cnf.password.c_str());
	}
*/
	if(!cnf.nodes.empty()){
		this->ui->plainTextEdit_ClusterNodeList->setPlainText(cnf.nodes.c_str());
	}

	//if(this->settings->contains(this->portKey.c_str())) {


	// char buffer[6];
	char *buffer = (char *) calloc(6,sizeof(char));

	sprintf(buffer,"%d",cnf.clientPort);
	this->ui->LineEdit_Port->setText(buffer);
	free(buffer);

	buffer = (char *) calloc(6,sizeof(char));
	sprintf(buffer,"%d",cnf.masterPort);
	this->ui->LineEdit_MasterPort->setText(buffer);
	free(buffer);

	buffer = (char *) calloc(6,sizeof(char));
	sprintf(buffer,"%d",cnf.agentPort);
	this->ui->LineEdit_AgentsPort->setText(buffer);
	free(buffer);

	if(!cnf.processOwner.empty()) {
		this->ui->LineEdit_ProcessOwner->setText(cnf.processOwner.c_str());
	}

	if(!cnf.processName.empty()) {
		this->ui->LineEdit_ProcessName->setText(cnf.processName.c_str());
	}

	if(!cnf.processStartsWith.empty()) {
		this->ui->LineEdit_ProcessStartsWith->setText(cnf.processStartsWith.c_str());
	}

	this->ui->spinBox_CPU_Threshold->setValue(cnf.cpuThreshold);


	if(!cnf.nodesBM.empty()) {
		this->ui->plainTextEdit_ClusterNodeListBM->setPlainText(cnf.nodesBM.c_str());
	}

	//this->ui->checkBox_CPU->setChecked(cnf.checkCPU_Status);


	//this->ui->checkBox_MEM->setChecked(cnf.checkMEM_Status);


	this->ui->checkBox_PapiCounters->setChecked(cnf.checkPapi_Status);

	this->ui->checkBox_Energy->setChecked(cnf.checkEnergy_Status);

	this->ui->checkBox_cmake3->setChecked(cnf.use_cmake3);

}

/*!
 * \brief ConfigurationWindow::saveAndClose Function to save the current configuration settings and close the window
 * \return A boolean value that is true if the configuration parameters are correctly stored
 */
bool ConfigurationWindow::saveAndClose() {

	//Save settings and close

	//1.- Check values
	//std::string newUser			= this->ui->LineEdit_Username->text().toStdString();
	//std::string newPassword			= this->ui->LineEdit_Password->text().toStdString();
	std::string newNodes			= this->ui->plainTextEdit_ClusterNodeList->toPlainText().toStdString();
	std::string newNodesBM			= this->ui->plainTextEdit_ClusterNodeListBM->toPlainText().toStdString();
	int newClientPort			= this->ui->LineEdit_Port->text().toInt(); //.toStdString();
	int newMasterPort			= this->ui->LineEdit_MasterPort->text().toInt(); //.toStdString();
	int newAgentPort			= this->ui->LineEdit_AgentsPort->text().toInt();
	std::string newProcessOwner		= this->ui->LineEdit_ProcessOwner->text().toStdString();
	std::string newProcessName		= this->ui->LineEdit_ProcessName->text().toStdString();
	std::string newProcessStartsWith	= this->ui->LineEdit_ProcessStartsWith->text().toStdString();
	unsigned int newCPU_threshold		= this->ui->spinBox_CPU_Threshold->value();
	std::string newNetworkInterface		= this->ui->comboBox_NetworkInterfaces->currentText().toStdString();
	std::string newMasterInInterface	= this->ui->LineEdit_MasterIn->text().toStdString();
	std::string newMasterOutInterface	= this->ui->LineEdit_MasterOut->text().toStdString();

	bool newCheckMEM_Status			= this->ui->checkBox_MEM->isChecked();
	bool newCheckCPU_Status			= this->ui->checkBox_CPU->isChecked();
	bool newCheckPapi_Status		= this->ui->checkBox_PapiCounters->isChecked();
	bool newCheckEnergy_Status		= this->ui->checkBox_Energy->isChecked();

	bool newUse_cmake3                  = this->ui->checkBox_cmake3->isChecked();

	QMessageBox msgBox;
	msgBox.setWindowTitle("Error");
	//msgBox.setText("Question");
	msgBox.setStandardButtons(QMessageBox::Ok);
	//msgBox.setDefaultButton(QMessageBox::No);


	if(!this->isNumeric(this->ui->LineEdit_Port->text().toStdString())){
		msgBox.setText("Client port must be a numeric value");
		msgBox.exec();

		return false;
	}
	else if(!this->isNumeric(this->ui->LineEdit_MasterPort->text().toStdString())){
		msgBox.setText("Master port must be a numeric value");
		msgBox.exec();

		return false;
	}
	else if(!this->isNumeric(this->ui->LineEdit_AgentsPort->text().toStdString())){
		msgBox.setText("Agents port must be a numeric value");
		msgBox.exec();

		return false;
	}
	else{

		Configuration config = Configuration();

		Config conf = config.getConfiguration();

		//conf.userName		= newUser;
		//conf.password		= newPassword;
		conf.nodes		= newNodes;
		conf.nodesBM		= newNodesBM;
		conf.clientPort		= newClientPort;
		conf.masterPort		= newMasterPort;
		conf.agentPort		= newAgentPort;
		conf.processOwner	= newProcessOwner;
		conf.processName	= newProcessName;
		conf.processStartsWith	= newProcessStartsWith;
		conf.cpuThreshold	= newCPU_threshold;
		conf.networkInterface	= newNetworkInterface;
		conf.masterInInterface	= newMasterInInterface;
		conf.masterOutInterface	= newMasterOutInterface;
		conf.checkMEM_Status	= newCheckMEM_Status;
		conf.checkCPU_Status	= newCheckCPU_Status;
		conf.checkPapi_Status	= newCheckPapi_Status;
		conf.checkEnergy_Status	= newCheckEnergy_Status;
        conf.use_cmake3         = newUse_cmake3;

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
