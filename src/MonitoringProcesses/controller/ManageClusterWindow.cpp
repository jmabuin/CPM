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

#include "ManageClusterWindow.h"
#include "ui_ManageClusterWindow.h"

ManageClusterWindow::ManageClusterWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ManageClusterWindow)
{
	ui->setupUi(this);

	this->initConfiguration();
}

ManageClusterWindow::~ManageClusterWindow()
{
	delete ui;
}

/*!
 * /brief ConfigurationWindow::initConfiguration Procedure to init Configuration Window with the stored values or the default ones (if there are none stored).
 * /param filename File where the configuration is stored.
 */
void ManageClusterWindow::initConfiguration(){

	Configuration conf = Configuration();

	Config cnf = conf.getConfiguration();

	if(!cnf.userName.empty()) {
		this->ui->LineEdit_Username->setText(cnf.userName.c_str());
	}

	//if(this->settings->contains(this->passwordKey.c_str())) { //Here we decrypt the value from the configuration file
	if(!cnf.password.empty()) {
		this->ui->LineEdit_Password->setText(cnf.password.c_str());
	}

	char buffer[6];
	sprintf(buffer, "%d", cnf.port);
	this->ui->LineEdit_Port->setText(buffer);


	//if(this->settings->contains(this->keyFileKey.c_str())) {
	if(!cnf.key.empty()) {
		this->ui->LineEdit_KeyFile->setText(cnf.key.c_str());
	}


	this->ui->radioButton_userpass->setChecked(cnf.SSH_UsernamePassword);

	this->ui->radioButton_keyfile->setChecked(cnf.SSH_KeyFile);


}

void ManageClusterWindow::done(int result) {
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


bool ManageClusterWindow::saveAndClose() {

	//Save settings and close

	//1.- Check values
	std::string newUser			= this->ui->LineEdit_Username->text().toStdString();
	std::string newPassword			= this->ui->LineEdit_Password->text().toStdString();
	int newPort				= this->ui->LineEdit_Port->text().toInt(); //.toStdString();
	std::string newKey			= this->ui->LineEdit_KeyFile->text().toStdString();

	bool newSSH_UsernamePassword		= this->ui->radioButton_userpass->isChecked();
	bool newSSH_KeyFile			= this->ui->radioButton_keyfile->isChecked();

	QMessageBox msgBox;
	msgBox.setWindowTitle("Error");
	//msgBox.setText("Question");
	msgBox.setStandardButtons(QMessageBox::Ok);
	//msgBox.setDefaultButton(QMessageBox::No);


	//We check inserted values
	if(!this->isNumeric(this->ui->LineEdit_Port->text().toStdString())){
		msgBox.setText("Port must be a numeric value");
		msgBox.exec();

		return false;
	}

	else{

		Configuration config = Configuration();

		Config conf = config.getConfiguration();

		conf.userName			= newUser;
		conf.password			= newPassword;
		conf.port			= newPort;
		conf.key			= newKey;
		conf.SSH_UsernamePassword	= newSSH_UsernamePassword;
		conf.SSH_KeyFile		= newSSH_KeyFile;

		config.setConfiguration(conf);

		return true;

	}


}


/**
 * Procedure to check if a given string value is a number.
 * @param String to check.
 * @return true if the given string is numeric, false otherwise.
 */
bool ManageClusterWindow::isNumeric(std::string cadea){

	unsigned int i = 0;

	for(i = 0;i< cadea.length();i++){
		if(!std::isdigit(cadea.c_str()[i])){
			return false;
		}
	}

	return true;

}
