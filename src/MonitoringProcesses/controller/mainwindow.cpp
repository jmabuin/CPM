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

#include "mainwindow.h"
#include "ui_mainwindow.h"

/*!
 * \brief MainWindow::MainWindow Constructor
 * \param parent The parent Widget
 */
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	//Call to vcarious procedures to init the Window
	this->initWindow();
	this->createActions();
	this->createMenus();
	this->initButtonsActions();

	this->setWindowIcon(QIcon(":/icons/controller/CPM.ico"));
}

/*!
 * \brief MainWindow::~MainWindow Destructor
 */
MainWindow::~MainWindow()
{
	delete ui;
}

/*!
 * \brief MainWindow::initWindow Procedure to init various items from the MainWindow
 */
void MainWindow::initWindow() {
	this->ui->pushButton_StopMeasures->setEnabled(false);
	this->receivingData = false;
}

/*!
 * \brief MainWindow::createActions Procedure to create actions for this window
 */
void MainWindow::createActions() {

	this->configureAction = new QAction(tr("&Configure program..."), this);
	this->configureAction->setShortcuts(QKeySequence::Preferences);
	this->configureAction->setStatusTip(tr("Configure program preferences"));
	connect(this->configureAction, SIGNAL(triggered()), this, SLOT(configureRun()));


	this->manageSettingsSSH = new QAction(tr("&Manage cluster..."), this);
	this->manageSettingsSSH->setShortcuts(QKeySequence::Preferences);
	this->manageSettingsSSH->setStatusTip(tr("Manage the computing cluster"));
	connect(this->manageSettingsSSH, SIGNAL(triggered()), this, SLOT(manageSSHRun()));

	this->exitProgram = new QAction(tr("&Exit"), this);
	this->exitProgram->setShortcuts(QKeySequence::Quit);
	this->exitProgram->setStatusTip(tr("Quit program"));
	connect(this->exitProgram, SIGNAL(triggered()), this, SLOT(exitRun()));
}


/*!
 * \brief MainWindow::createMenus Procedure to create menus for this window
 */
void MainWindow::createMenus() {

	this->fileMenu = this->menuBar()->addMenu(tr("&File"));
	this->fileMenu->addAction(this->configureAction);
	this->fileMenu->addAction(this->manageSettingsSSH);
	this->fileMenu->addSeparator();
	this->fileMenu->addAction(this->exitProgram);
}

/*!
 * \brief MainWindow::initButtonsActions Procedure do create actions for the buttons in this window
 */
void MainWindow::initButtonsActions() {

	connect(this->ui->pushButton_StartMeasures,SIGNAL(clicked(bool)),this,SLOT(startMeasures()));
	connect(this->ui->pushButton_StopMeasures,SIGNAL(clicked(bool)),this,SLOT(stopMeasures()));

	//connect(self.ui.pushButton_StopMeasures, QtCore.SIGNAL("clicked()"), self.stopMeasures)

}

/*!
 * \brief MainWindow::configureRun Procedure to open the configuration window
 */
void MainWindow::configureRun() {
	//printf("Configuring program\n");

	this->configurationWindow = new ConfigurationWindow(this);
	this->configurationWindow->setModal(true);
	this->configurationWindow->show();
}

void MainWindow::manageSSHRun() {
	this->manageClusterWindow = new ManageClusterWindow(this);
	this->manageClusterWindow->setModal(true);
	this->manageClusterWindow->show();
}

/*!
 * \brief MainWindow::exitRun Procedure to exit the program
 */
void MainWindow::exitRun() {
	this->stopMeasures();
	QApplication::quit();
}

/*!
 * \brief MainWindow::startMeasures Procedure that starts the measures in the master and slaves nodes. It launches the getData thread
 */
void MainWindow::startMeasures() {

	pthread_t dataThread;
	int thread;

	//The tabs are cleared
	this->ui->nodesTab->clear();

	//The configuration parameters are obtained
	Configuration cnf = Configuration();

	Config currentConf = cnf.getConfiguration();

	char *nodes;

	//We get the master and slave nodes
	char *tmpNodes = (char *)malloc(currentConf.nodes.length()*sizeof(char));
	strcpy(tmpNodes,currentConf.nodes.c_str());

	char *tmpNodesBM = (char *)malloc(currentConf.nodesBM.length()*sizeof(char));
	strcpy(tmpNodesBM,currentConf.nodesBM.c_str());



	nodes = strtok(tmpNodes,"\n");
	while(nodes != NULL){
		this->ui->nodesTab->addTab(new DataWidget(this),QString(nodes));
		nodes = strtok(NULL,"\n");
	}


	nodes = strtok(tmpNodesBM,"\n");
	while(nodes != NULL){
		this->ui->nodesTab->addTab(new DataWidget(this),QString(nodes));
		nodes = strtok(NULL,"\n");
	}

	free(tmpNodes);
	free(tmpNodesBM);
	free(nodes);


	//The buttons change their state
	this->ui->pushButton_StopMeasures->setEnabled(true);
	this->ui->pushButton_StartMeasures->setEnabled(false);

	//Launch getting data thread
	this->receivingData = true;

	//pthread_create...
	thread = pthread_create(&dataThread,NULL,MainWindow::getData,this);
	fprintf(stderr,"[%s] Thread is %d\n",__func__, thread);
	//All the agents must start here, and also the bridge in the cluster master node


}

/*!
 * \brief MainWindow::stopMeasures Procedure that stops taking measures in master and slaves nodes
 */
void MainWindow::stopMeasures() {
	this->receivingData = false;

	this->ui->pushButton_StartMeasures->setEnabled(true);
	this->ui->pushButton_StopMeasures->setEnabled(false);

	//Stop all the agents and the bridge master node
	ProcessesInfo stopAgents;
	Configuration cnf = Configuration();

	Config currentConf = cnf.getConfiguration();
	stopAgents.packageId = PACKAGE_ID_STOP;
	strcpy(stopAgents.processName,currentConf.processName.c_str());
	strcpy(stopAgents.userName,currentConf.processOwner.c_str());

	Network networkObject = Network();
	//struct in_addr masterNodeIp;

	char tmpHostName[256];

	strcpy(tmpHostName,this->ui->nodesTab->tabText(0).toStdString().c_str());
	strcpy(stopAgents.nodeName,this->ui->nodesTab->tabText(0).toStdString().c_str());



	networkObject.sendMsgTo((void *)&stopAgents,PACKAGE_ID_STOP,this->rxPortInMaster,inet_ntoa(this->masterNodeIp));



}

/*!
 * \brief MainWindow::getData Procedure that starts the thread that gets the data
 * \param param A pointer to the MainWindow object
 * \return
 */
void *MainWindow::getData(void *param) {

	MainWindow *receivedObject = (MainWindow *) param;

	Configuration cnf = Configuration();

	Config currentConf = cnf.getConfiguration();

	Network networkObject = Network();

	int rxPort = CLIENT_BASE_PORT;

	if (currentConf.clientPort != rxPort) {

		rxPort = currentConf.clientPort;
	}

	int master_base_port = MASTER_BASE_PORT;

	if (currentConf.masterPort != master_base_port) {
		master_base_port = currentConf.masterPort;
	}


	//Creamos dirección de recepción
	struct in_addr rxAddr = { INADDR_ANY };
	struct sockaddr_in myRxAddr = networkObject.buildAddr(AF_INET, rxPort, rxAddr);

	int nRxBytes;
	socklen_t addrLen;
	char rxBuffer[MAX_BUF_SIZE];//Nowadays 20480. En Network.h
	Agent2MasterDataMsg rxMsg;


	//Creamos socket de recepción.
	int rxSocket = networkObject.createUDPSocket(1, 1, myRxAddr); //0=no broadcast, 0=no tx socket, myRxAddr especifies an addr to bind to

	printf("I am client App at %s\n", networkObject.strAddr(myRxAddr));


	//First we need to send a message per each one of our agents

	ProcessesInfo packageAgent;
	ProcessesInfo receivedData;



	//Start package info
	packageAgent.packageId = PACKAGE_ID_DATAPROCESS;
	strcpy(packageAgent.processName,currentConf.processName.c_str());
	strcpy(packageAgent.processStartsWith,currentConf.processStartsWith.c_str());

	packageAgent.cpuThreshold = currentConf.cpuThreshold;

	//strcpy(packageAgent.userName,currentConf.userName.c_str());
	strcpy(packageAgent.userName,currentConf.processOwner.c_str());

	//Check if PAPI counters have to be monitorized
	if(currentConf.checkPapi_Status){
		packageAgent.measurePapi = 1;
	}
	else{
		packageAgent.measurePapi = 0;
	}

	//Check if energy has to be monitorized
	if(currentConf.checkEnergy_Status){
		packageAgent.measureEnergy = 1;
	}
	else{
		packageAgent.measureEnergy = 0;
	}


	char tmpInterface[currentConf.networkInterface.size()];
	strcpy(tmpInterface,currentConf.networkInterface.c_str());

	packageAgent.from.sin_addr = networkObject.getOwnIp(tmpInterface);
	packageAgent.from.sin_port = rxPort; //CLIENT_BASE_PORT;

	int i = 0;

	char tmpHostName[256];
	struct addrinfo *result;

	int error;

	//printf("About to send packages\n");

	for (i = 0; i < receivedObject->ui->nodesTab->count(); i++) {
		//receivedObject->ui->nodesTab->widget(index)->
		//printf("%s\n",receivedObject->ui->nodesTab->tabText(i).toStdString().c_str());
		packageAgent.agentId = i;

		//tmpHostName = receivedObject->ui->nodesTab->tabText(i).toStdString().c_str();
		strcpy(tmpHostName,receivedObject->ui->nodesTab->tabText(i).toStdString().c_str());
		strcpy(packageAgent.nodeName,receivedObject->ui->nodesTab->tabText(i).toStdString().c_str());
		//printf("Iteration %d\n",i);


		if (i == 0){
			error = getaddrinfo(tmpHostName, NULL, NULL, &result);
			if(error == 0) {
				//packageAgent.from = ((struct sockaddr_in *)result->ai_addr)->sin_addr;
				//transSocket.sin_addr = ((struct sockaddr_in *)result->ai_addr)->sin_addr;
				//printf("Destination: %s \n",inet_ntoa(transSocket.sin_addr));

				receivedObject->masterNodeIp = ((struct sockaddr_in *)result->ai_addr)->sin_addr;

				printf("Sending message to %s:%u\n",inet_ntoa(receivedObject->masterNodeIp),master_base_port);

				networkObject.sendMsgTo((void *)&packageAgent,PACKAGE_ID_DATAPROCESS,master_base_port,inet_ntoa(receivedObject->masterNodeIp));
				//printf("Message sent\n");
				nRxBytes = recvfrom(rxSocket, rxBuffer, sizeof(rxBuffer), 0,(struct sockaddr *) &myRxAddr, &addrLen);
				if(nRxBytes <= 0) {
					fprintf(stderr,"[%s] Error in recvfrom()\n",__func__);
				}
				//printf("Message received\n");
				memcpy(&receivedData, rxBuffer, sizeof(ProcessesInfo));
				//memcpy(&packageAgent,&receivedData,sizeof(ProcessesInfo));
				receivedObject->rxPortInMaster = receivedData.port;
				receivedData.from = packageAgent.from;


				//Send message to start agentMonitor also in the master node
				//printf("Sending message to %s:%u\n",inet_ntoa(receivedObject->masterNodeIp),receivedObject->rxPortInMaster);
				networkObject.sendMsgTo((void *)&packageAgent,PACKAGE_ID_DATAPROCESS,receivedObject->rxPortInMaster,inet_ntoa(receivedObject->masterNodeIp));

			}
		}

		//The daemon in the master has been started. Now, send the info about the agents to start
		//We assume that masterNodeIp and rxPortInMaster had been init in the first iteration
		else {
			printf("Sending message to %s:%u\n",inet_ntoa(receivedObject->masterNodeIp),receivedObject->rxPortInMaster);
			networkObject.sendMsgTo((void *)&packageAgent,PACKAGE_ID_DATAPROCESS,receivedObject->rxPortInMaster,inet_ntoa(receivedObject->masterNodeIp));

		}

	}


	//while (receivingData) { //Recepción hasta el infinito!
	while(receivedObject->receivingData) {
		//printf("Receiving data\n");
		//Nos quedamos bloqueados esperando por un nuevo mensaje
		nRxBytes = recvfrom(rxSocket, rxBuffer, sizeof(rxBuffer), 0,(struct sockaddr *) &myRxAddr, &addrLen);
		if(nRxBytes <= 0) {
			fprintf(stderr,"[%s] Error in recvfrom()\n",__func__);
		}
		//printf("Received message\n");
		//Parseamos el mensaje
		memcpy(&rxMsg, rxBuffer, sizeof(Agent2MasterDataMsg));


		if(rxMsg.packageId == PACKAGE_ID_DATAMSG){
			//printf("%d %u %lu %2f%c %2f%c %s %s %llu Kb\n",rxMsg.PID, rxMsg.agentId, rxMsg.measureNumber,rxMsg.cpuPercentage,'%', rxMsg.totalCpuPercentage,'%', rxMsg.userName,rxMsg.processName,rxMsg.memory);
			QWidget *currentWidget = receivedObject->ui->nodesTab->widget(rxMsg.agentId);
			DataWidget *currentDataWidget;

			//if(currentWidget->metaObject()->className() == "DataWidget"){
				currentDataWidget = (DataWidget *)currentWidget;

				currentDataWidget->addData(rxMsg);

			//}
		}
		else if (rxMsg.packageId == PACKAGE_ID_ENERGY){
			Agent2MasterEnergyMsg energyMsg;

			memcpy(&energyMsg, rxBuffer, sizeof(Agent2MasterEnergyMsg));

			QWidget *currentWidget = receivedObject->ui->nodesTab->widget(rxMsg.agentId);
			DataWidget *currentDataWidget;

			currentDataWidget = (DataWidget *)currentWidget;

			currentDataWidget->addDataEnergy(energyMsg);

		}


	}


	printf("End of thread\n");

	return NULL;
}


void MainWindow::closeEvent(QCloseEvent *event) {

	this->stopMeasures();

	event->accept();
}

