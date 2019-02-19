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

#include <stdio.h>
#include <stdlib.h>
#include "ssh_handler.h"
#include "Network.h"
#include "ManageClusterWindow.h"
#include "ui_ManageClusterWindow.h"
#include <QFileDialog>
#include <iostream>
#include <sstream>

ManageClusterWindow::ManageClusterWindow(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ManageClusterWindow)
{
    ui->setupUi(this);


    this->projectGitURL = "https://github.com/jmabuin/CPM.git";

    this->initConfiguration();
    this->initWindow();
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

    this->config = conf.getConfiguration();

    if(!this->config.userName.empty()) {
        this->ui->LineEdit_Username->setText(this->config.userName.c_str());
    }

    //if(this->settings->contains(this->passwordKey.c_str())) { //Here we decrypt the value from the configuration file
    if(!this->config.password.empty()) {
        this->ui->LineEdit_Password->setText(this->config.password.c_str());
    }

    if(!this->config.key.empty()) {
        this->ui->LineEdit_KeyFile->setText(this->config.key.c_str());
    }


    char buffer[6];
    sprintf(buffer, "%d", this->config.port);
    this->ui->LineEdit_Port->setText(buffer);

    char bufferSSH_Internal[6];
    sprintf(bufferSSH_Internal, "%d", this->config.internalSSH_Port);
    this->ui->LineEdit_SSH_Internal_Port->setText(bufferSSH_Internal);

    //if(this->settings->contains(this->keyFileKey.c_str())) {
    if(!this->config.key.empty()) {
        this->ui->LineEdit_KeyFile->setText(this->config.key.c_str());
    }


    this->ui->radioButton_userpass->setChecked(this->config.SSH_UsernamePassword);

    this->ui->radioButton_keyfile->setChecked(this->config.SSH_KeyFile);


}

void ManageClusterWindow::initWindow() {

    connect(this->ui->pushButton_Execute,SIGNAL(clicked(bool)),this,SLOT(executeSlot()));
    connect(this->ui->pushButton_KeyFile,SIGNAL(clicked(bool)),this,SLOT(openFileDialogKeyFile()));

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
    std::string newUser			    = this->ui->LineEdit_Username->text().toStdString();
    std::string newPassword			= this->ui->LineEdit_Password->text().toStdString();
    int newPort				        = this->ui->LineEdit_Port->text().toInt(); //.toStdString();
    std::string newKey			    = this->ui->LineEdit_KeyFile->text().toStdString();

    bool newSSH_UsernamePassword	= this->ui->radioButton_userpass->isChecked();
    bool newSSH_KeyFile			    = this->ui->radioButton_keyfile->isChecked();

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

        //Config conf = config.getConfiguration();

        this->config.userName			    = newUser;
        this->config.password			    = newPassword;
        this->config.port			        = newPort;
        this->config.key			        = newKey;
        this->config.SSH_UsernamePassword	= newSSH_UsernamePassword;
        this->config.SSH_KeyFile		    = newSSH_KeyFile;

        config.setConfiguration(this->config);

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

void ManageClusterWindow::executeSlot() {

    // Clear output screen
    this->ui->plainTextEdit_MasterOutput->clear();

    //Deploy actions
    if(strcmp(this->ui->comboBox_Actions->currentText().toStdString().c_str(), "Deploy") == 0) {

        if(strcmp(this->ui->comboBox_Locations->currentText().toStdString().c_str(), "All") == 0){

            this->deployAll();
        }
        else if(strcmp(this->ui->comboBox_Locations->currentText().toStdString().c_str(), "Master") == 0){

            this->deployMasterRun();
        }
        else if(strcmp(this->ui->comboBox_Locations->currentText().toStdString().c_str(), "Agents") == 0){

            this->deployAgentsRun();
        }

    }
    else if(strcmp(this->ui->comboBox_Actions->currentText().toStdString().c_str(), "Run") == 0) {

        if(strcmp(this->ui->comboBox_Locations->currentText().toStdString().c_str(), "All") == 0){
            this->runAllRun();
        }
        else if(strcmp(this->ui->comboBox_Locations->currentText().toStdString().c_str(), "Master") == 0){
            this->runMasterRun();
        }
        else if(strcmp(this->ui->comboBox_Locations->currentText().toStdString().c_str(), "Agents") == 0){
            this->runAgentsRun();
        }
    }
    else if(strcmp(this->ui->comboBox_Actions->currentText().toStdString().c_str(), "Check") == 0) {

        if(strcmp(this->ui->comboBox_Locations->currentText().toStdString().c_str(), "All") == 0){
            this->checkAllRun();
        }
        else if(strcmp(this->ui->comboBox_Locations->currentText().toStdString().c_str(), "Master") == 0){
            this->checkMasterRun();
        }
        else if(strcmp(this->ui->comboBox_Locations->currentText().toStdString().c_str(), "Agents") == 0){
            this->checkAgentsRun();
        }
    }
    else if(strcmp(this->ui->comboBox_Actions->currentText().toStdString().c_str(), "Stop") == 0) {

        if(strcmp(this->ui->comboBox_Locations->currentText().toStdString().c_str(), "All") == 0){
            this->stopAllRun();
        }
        else if(strcmp(this->ui->comboBox_Locations->currentText().toStdString().c_str(), "Master") == 0){
            this->stopMasterRun();
        }
        else if(strcmp(this->ui->comboBox_Locations->currentText().toStdString().c_str(), "Agents") == 0){
            this->stopAgentsRun();
        }

    }
    else{
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setStandardButtons(QMessageBox::Ok);

        msgBox.setText("Option not recognized");
        msgBox.exec();

    }

}

void ManageClusterWindow::deployMasterRun() {

    fprintf(stderr, "[%s] Deploying master\n",__func__);


    this->executeCommandInMaster("rm -R CPM");
    this->executeCommandInMaster("git clone "+this->projectGitURL);

    if (!this->config.use_cmake3) {
        fprintf(stderr, "[%s] Not using cmake3\n",__func__);
        this->executeCommandInMaster("cd CPM/ && mkdir build && cd build && cmake -DONLY_MASTER=true .. && make");

    }
    else {
        fprintf(stderr, "[%s] Using cmake3\n",__func__);
        this->executeCommandInMaster("cd CPM/ && mkdir build && cd build && cmake3 -DONLY_MASTER=true .. && make");
    }

}

void ManageClusterWindow::deployAgentsRun() {

    fprintf(stderr, "[%s] Deploying agents\n",__func__);

    this->executeCommandInAgents("rm -R CPM");
    this->copyFromMaster2Agents("CPM/");

    if (!this->config.use_cmake3) {
        this->executeCommandInAgents("cd CPM/build/ && rm -Rf ./* && cmake -DONLY_AGENT=true .. && make");
    }
    else {
        this->executeCommandInAgents("cd CPM/build/ && rm -Rf ./* && cmake3 -DONLY_AGENT=true .. && make");
    }


}

void ManageClusterWindow::deployAll() {

    this->deployMasterRun();
    this->deployAgentsRun();
}

void ManageClusterWindow::runAgentsRun() {

    fprintf(stderr, "[%s] Starting agents\n",__func__);

    Configuration config = Configuration();

    Config conf = config.getConfiguration();


    if(conf.agentPort != DAEMON_AGENT_PORT) {
        this->executeCommandInAgents("cd CPM/build/ && ./MonitoringAgent -a "+ std::to_string(conf.agentPort));
    }
    else{
        this->executeCommandInAgents("cd CPM/build/ && ./MonitoringAgent");
    }

}

void ManageClusterWindow::runAllRun() {
    this->runMasterRun();
    this->runAgentsRun();
}

void ManageClusterWindow::runMasterRun() {

    Configuration config = Configuration();

    Config conf = config.getConfiguration();

    std::string command_to_execute = "cd CPM/build/ && ./MonitoringMaster";

    std::string agent_command_to_execute = "cd CPM/build/ && ./MonitoringAgent";


    if(conf.agentPort != DAEMON_AGENT_PORT) {

        command_to_execute = command_to_execute + " -a " + std::to_string(conf.agentPort);

        agent_command_to_execute = agent_command_to_execute + " -a " + std::to_string(conf.agentPort);
        // agent_command_to_execute = agent_command_to_execute + bufferAgentPort;


    }



    if(conf.masterPort != MASTER_BASE_PORT) {

        command_to_execute = command_to_execute + " -m " + std::to_string(conf.masterPort);

    }

    if(conf.clientPort != CLIENT_BASE_PORT) {


        command_to_execute = command_to_execute + " -c " + std::to_string(conf.clientPort);

    }

    if(conf.masterInInterface != "eth0") {
        command_to_execute = command_to_execute + " -i "+conf.masterInInterface;
    }

    if(conf.masterOutInterface != "eth0") {
        command_to_execute = command_to_execute + " -o "+conf.masterOutInterface;
    }

    this->executeCommandInMaster(command_to_execute);
    this->executeCommandInMaster(agent_command_to_execute);


}

void ManageClusterWindow::checkAgentsRun() {


    this->executeCommandInMaster("ps -ef  | grep \"MonitoringAgent\" | grep -v grep");
    this->executeCommandInAgents("ps -ef  | grep \"MonitoringAgent\" | grep -v grep");

}

void ManageClusterWindow::checkMasterRun() {
    this->executeCommandInMaster("ps -ef  | grep \"MonitoringMaster\" | grep -v grep");
}

void ManageClusterWindow::checkAllRun() {
    this->checkMasterRun();
    this->checkAgentsRun();
}

void ManageClusterWindow::stopMasterRun() {

    this->executeCommandInMaster("cd CPM/script/ && ./stop-agents.sh");
    this->executeCommandInMaster("cd CPM/script/ && ./stop-master.sh");
}

void ManageClusterWindow::stopAgentsRun() {
    this->executeCommandInAgents("cd CPM/script/ && ./stop-agents.sh");
}

void ManageClusterWindow::stopAllRun() {
    this->stopMasterRun();
    this->stopAgentsRun();
}


void ManageClusterWindow::executeCommandInMaster(std::string command) {

    Configuration config = Configuration();

    Config conf = config.getConfiguration();

    std::string hostname	= conf.nodes;
    int port		= this->ui->LineEdit_Port->text().toInt();
    int verbosity		= SSH_LOG_PROTOCOL;
    std::string username	= this->ui->LineEdit_Username->text().toStdString();
    std::string password;
    std::string keyFileName;

    if(this->ui->radioButton_userpass->isChecked()) {
        password	= this->ui->LineEdit_Password->text().toStdString();
        keyFileName	= "";
    }
    else if (this->ui->radioButton_keyfile->isChecked()) {
        password	= "";
        keyFileName	= this->ui->LineEdit_KeyFile->text().toStdString();

    }


    int connectResult = 0;
    unsigned int buffer_length_command = 4096;


    SSH_Handler sshHandler = SSH_Handler(hostname, port, verbosity, username, password, keyFileName);
    //sshHandler.sftp_allocate();

    connectResult = sshHandler.connect();

    if(!connectResult) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setStandardButtons(QMessageBox::Ok);

        msgBox.setText("Could not open SSH session");
        msgBox.exec();
    }



    this->ui->plainTextEdit_MasterOutput->setPlainText(this->ui->plainTextEdit_MasterOutput->toPlainText() + QString("=====Executing command in master=====\n"));
    this->ui->plainTextEdit_MasterOutput->setPlainText(this->ui->plainTextEdit_MasterOutput->toPlainText() + QString(command.c_str()) + "\n");

    char *result_command = (char *)calloc(buffer_length_command, sizeof(char));
    sshHandler.execute_remote_command(command.c_str(), result_command);
    this->ui->plainTextEdit_MasterOutput->setPlainText(this->ui->plainTextEdit_MasterOutput->toPlainText() + QString(result_command) + "\n");

    this->ui->plainTextEdit_MasterOutput->repaint();

    free(result_command);

    //sshHandler.sftp_deallocate();
    sshHandler.disconnect();

}

void ManageClusterWindow::executeCommandInAgents(std::string command) {

    Configuration config = Configuration();

    Config conf = config.getConfiguration();

    std::string hostnames		= conf.nodesBM;
    std::string hostname		= conf.nodes;
    int port			        = this->ui->LineEdit_Port->text().toInt();
    std::string internal_port	= this->ui->LineEdit_SSH_Internal_Port->text().toStdString();
    int verbosity			    = SSH_LOG_PROTOCOL;
    std::string username		= this->ui->LineEdit_Username->text().toStdString();
    std::string password;
    std::string keyFileName;

    if(this->ui->radioButton_userpass->isChecked()) {
        password	= this->ui->LineEdit_Password->text().toStdString();
        keyFileName	= "";
    }
    else if (this->ui->radioButton_keyfile->isChecked()) {
        password	= "";
        keyFileName	= this->ui->LineEdit_KeyFile->text().toStdString();

    }

    int connectResult = 0;
    unsigned int buffer_length_command = 4096;
    std::string ssh_command;

    std::string current_node;

    std::vector<std::string> nodes;


    std::stringstream ss(hostnames.c_str());

    while(std::getline(ss, current_node, '\n')){
        nodes.push_back(current_node);
    }


    this->ui->plainTextEdit_MasterOutput->setPlainText(this->ui->plainTextEdit_MasterOutput->toPlainText() + QString("=====Executing remote command=====\n"));

    SSH_Handler sshHandler = SSH_Handler(hostname, port, verbosity, username, password, keyFileName);
    //sshHandler.sftp_allocate();

    connectResult = sshHandler.connect();

    if(!connectResult) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setStandardButtons(QMessageBox::Ok);

        msgBox.setText("Could not open SSH session");
        msgBox.exec();
    }

    for(int i = 0; i< nodes.size(); ++i) {

        current_node = nodes[i];
        fprintf(stderr, "[%s] Deploying agent in %s\n",__func__, current_node.c_str());

        this->ui->plainTextEdit_MasterOutput->setPlainText(this->ui->plainTextEdit_MasterOutput->toPlainText() + QString("==========================\n"));
        this->ui->plainTextEdit_MasterOutput->setPlainText(this->ui->plainTextEdit_MasterOutput->toPlainText() + QString(current_node.c_str())+"\n");


        char *result_command = (char *)calloc(buffer_length_command, sizeof(char));

        if(internal_port != "22"){
            ssh_command = "ssh -p "+internal_port+" "+current_node+" \""+command.c_str()+"\"";
            //std::cout << ssh_command << std::endl;
        }
        else {
            ssh_command = "ssh "+current_node+" \""+command.c_str()+"\"";
            std::cout << ssh_command << std::endl;
        }

        this->ui->plainTextEdit_MasterOutput->setPlainText(this->ui->plainTextEdit_MasterOutput->toPlainText() + QString(ssh_command.c_str())+"\n");

        sshHandler.execute_remote_command(ssh_command.c_str(), result_command);

        this->ui->plainTextEdit_MasterOutput->setPlainText(this->ui->plainTextEdit_MasterOutput->toPlainText() + QString(result_command));
        this->ui->plainTextEdit_MasterOutput->repaint();

        free(result_command);

    }

    sshHandler.disconnect();
}

void ManageClusterWindow::copyFromMaster2Agents(std::string path) {

    //fprintf(stderr, "Performing SCP command\n");
    Configuration config = Configuration();

    Config conf = config.getConfiguration();

    std::string hostnames		= conf.nodesBM;
    std::string hostname		= conf.nodes;
    int port			        = this->ui->LineEdit_Port->text().toInt();
    std::string internal_port	= this->ui->LineEdit_SSH_Internal_Port->text().toStdString();
    int verbosity			    = SSH_LOG_PROTOCOL;
    std::string username		= this->ui->LineEdit_Username->text().toStdString();
    std::string password;
    std::string keyFileName;

    if(this->ui->radioButton_userpass->isChecked()) {
        password	= this->ui->LineEdit_Password->text().toStdString();
        keyFileName	= "";
    }
    else if (this->ui->radioButton_keyfile->isChecked()) {
        password	= "";
        keyFileName	= this->ui->LineEdit_KeyFile->text().toStdString();

    }

    int connectResult = 0;
    unsigned int buffer_length_command = 4096;
    std::string ssh_command;

    std::string current_node;

    std::vector<std::string> nodes;
    
    std::stringstream ss(hostnames.c_str());

    while(std::getline(ss,current_node,'\n')){
        nodes.push_back(current_node);
    }

    this->ui->plainTextEdit_MasterOutput->clear();
    this->ui->plainTextEdit_MasterOutput->setPlainText(this->ui->plainTextEdit_MasterOutput->toPlainText() + QString("=====Executing remote command=====\n"));

    SSH_Handler sshHandler = SSH_Handler(hostname, port, verbosity, username, password, keyFileName);
    //sshHandler.sftp_allocate();

    connectResult = sshHandler.connect();

    if(!connectResult) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setStandardButtons(QMessageBox::Ok);

        msgBox.setText("Could not open SSH session");
        msgBox.exec();
    }

    for(int i = 0; i< nodes.size(); ++i) {

        current_node = nodes[i];

        //fprintf(stderr, "[%s] Deploying agent in %s\n",__func__, nodes);
        std::cout << "Performing SCP in node: " << current_node << std::endl;
        this->ui->plainTextEdit_MasterOutput->setPlainText(this->ui->plainTextEdit_MasterOutput->toPlainText() + QString("==========================\n"));
        this->ui->plainTextEdit_MasterOutput->setPlainText(this->ui->plainTextEdit_MasterOutput->toPlainText() + QString(current_node.c_str())+"\n");

        char *result_command = (char *)calloc(buffer_length_command, sizeof(char));

        //std::string command_to_execute = "scp -r CPM/ "+std::string(nodes)+":CPM/";

        if(internal_port != "22"){
            ssh_command = "scp -p "+internal_port+" -r "+path+" "+current_node+":"+path;

        }
        else {
            ssh_command = "scp -r "+path+" "+current_node+":"+path;
            fprintf(stderr, "SCP command is %s\n", ssh_command.c_str());

        }

        this->ui->plainTextEdit_MasterOutput->setPlainText(this->ui->plainTextEdit_MasterOutput->toPlainText() + QString(ssh_command.c_str())+"\n");

        sshHandler.execute_remote_command(ssh_command.c_str(), result_command);

        this->ui->plainTextEdit_MasterOutput->setPlainText(this->ui->plainTextEdit_MasterOutput->toPlainText() + QString(result_command));
        this->ui->plainTextEdit_MasterOutput->repaint();

        free(result_command);

    }

    sshHandler.disconnect();

}

void ManageClusterWindow::openFileDialogKeyFile(){


    std::string currentKey = this->ui->LineEdit_KeyFile->text().toStdString();

    QString fileName;

    if(currentKey.empty()) {
        fileName = QFileDialog::getOpenFileName(this,"Select key file...","./");
    }
    else {
        size_t found;

        found = currentKey.find_last_of("/\\");

        fileName = QFileDialog::getOpenFileName(this,"Select key file...",currentKey.substr(0,found).c_str());

    }


    if(!fileName.isEmpty()) {

        this->ui->LineEdit_KeyFile->setText(fileName);

    }

}
