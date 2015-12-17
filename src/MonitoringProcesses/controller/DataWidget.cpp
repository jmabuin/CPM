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

#include "controller/DataWidget.h"
#include "ui_DataWidget.h"

#include <QFileDialog>
#include <limits>

DataWidget::DataWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DataWidget)
{
	ui->setupUi(this);

	//ConfigurationWindow confWindow = ConfigurationWindow(this);

	this->configuration = Configuration();

	this->tableHeaders = {"PID","User","RES Mem","CPU%","MEM%","Process","Measure Number","Threads","L1 DCM","L2_DCM","TOTINS","Colour"};


	this->colours = { QBrush(QColor(255,0,0)),QBrush(QColor(0,255,0)),QBrush(QColor(0,0,255)),QBrush(QColor(255,255,0)),QBrush(QColor(255,0,255)),QBrush(QColor(0,255,255)),QBrush(QColor(255,255,255))};


	this->ui->widget_PlotCPU->xAxis->setLabel("Measure number");
	this->ui->widget_PlotCPU->yAxis->setLabel("CPU percentage");

	this->ui->widget_PlotMEM->xAxis->setLabel("Measure number");
	this->ui->widget_PlotMEM->yAxis->setLabel("Memory");

	this->globalMaxCPU = 0.0;
	this->globalMaxMEM = 0.0;

	this->globalMinCPU = std::numeric_limits<double>::max();
	this->globalMinMEM = std::numeric_limits<double>::max();

	this->globalMaxCPUPID = 0;
	this->globalMaxMEMPID = 0;

	this->globalMinCPUPID = 0;
	this->globalMinMEMPID = 0;

	this->numMeasuresCpu = 0;
	this->numMeasuresMem = 0;

	QObject::connect(this,SIGNAL(updateData()),this,SLOT(plotData()));
	QObject::connect(this->ui->pushButton_2CSV_1,SIGNAL(clicked(bool)),this,SLOT(CpuToCSV()));
	QObject::connect(this->ui->pushButton_2CSV_2,SIGNAL(clicked(bool)),this,SLOT(MemToCSV()));


	QObject::connect(this->ui->pushButton_2PDF_1,SIGNAL(clicked(bool)),this,SLOT(CpuToPdf()));
	QObject::connect(this->ui->pushButton_2PDF_2,SIGNAL(clicked(bool)),this,SLOT(MemToPdf()));

	QObject::connect(this->ui->pushButton_2CSV_MainData,SIGNAL(clicked(bool)),this,SLOT(Table2CSV()));

}

DataWidget::~DataWidget()
{
	delete ui;
}


int DataWidget::getAgentId() {
	return this->agentId;
}

void DataWidget::setAgentId(int id) {
	this->agentId = id;
}

void DataWidget::addData(Agent2MasterDataMsg newData) {
	//this->data.push_back(newData);

	//unsigned long int measureNumber = 0;// = newData.measureNumber;
	unsigned int PID = newData.PID;


	if(this->data.count(PID) > 0){

		//measureNumber = this->data.at(PID).size();
		this->data.at(PID).insert(std::pair<unsigned long int,Agent2MasterDataMsg> (newData.measureNumber,newData));

		//printf("Adding measure %lu to PID %u\n",measureNumber,PID);
	}

	else {

		//There is no data associated to this PID. We add it
		std::map<unsigned long int,Agent2MasterDataMsg> newDataToInsert;

		newDataToInsert.insert(std::pair<unsigned long int,Agent2MasterDataMsg>(newData.measureNumber,newData));

		this->data.insert(std::pair<unsigned int,std::map< unsigned long int, Agent2MasterDataMsg> > (PID,newDataToInsert));

		//printf("Inserting new PID %u\n",PID);

	}

	//this->printData();
	emit this->updateData();

}


void DataWidget::plotData() {

	this->ui->tableWidget_Data->clear();


	this->ui->tableWidget_Data->setColumnCount(this->tableHeaders.size());
	this->ui->tableWidget_Data->setRowCount(this->data.size());


	QList<QString> dataHeaders;

	for (unsigned int i=0; i< this->tableHeaders.size(); i++){
		dataHeaders.append(QString(this->tableHeaders.at(i).c_str()));
	}

	this->ui->tableWidget_Data->setHorizontalHeaderLabels(dataHeaders);
	this->ui->tableWidget_Data->horizontalHeader()->setStretchLastSection(true);

	this->currentTableRow = 0;


	int j = 0;
	//unsigned int k = 0;

	std::map<unsigned int,std::map<unsigned long int,Agent2MasterDataMsg> >::iterator it;

	std::map<unsigned long int,Agent2MasterDataMsg>::iterator it2;

	//std::map<unsigned long int,Agent2MasterDataMsg>::iterator itInternal;

	this->ui->widget_PlotCPU->clearGraphs();
	this->ui->widget_PlotMEM->clearGraphs();

	double MaxMem = 0.0;
	double MaxCPU = 0.0;
	double Max_X_Mem = 0.0;
	double Max_X_CPU = 0.0;

	unsigned int numGraph = 0;

	std::map<unsigned int,QVector<double> > X_Mem;
	std::map<unsigned int,QVector<double> > Y_Mem;

	std::map<unsigned int,QVector<double> > X_CPU;
	std::map<unsigned int,QVector<double> > Y_CPU;

	//Iterate over PIDS
	for(it = this->data.begin(); it != this->data.end(); it++){

		std::map<unsigned long int,Agent2MasterDataMsg> currentProcesses_PID = it->second;
		unsigned int currentPID = it->first;

		Agent2MasterDataMsg currentData;
		std::vector<std::string> availableData;

		for(it2 = currentProcesses_PID.begin(); it2 != currentProcesses_PID.end(); it2 ++) {
			currentData = it2->second;//currentPID.at(currentPID.size()-1);



			if(X_Mem.count(currentPID) > 0) {


				Y_Mem.at(currentPID).append((double)currentData.memory);
				//X_Mem.at(currentPID).append((double)Y_Mem.at(currentData.PID).size());
				X_Mem.at(currentPID).append((double)currentData.measureNumber);

				//printf("Adding data to existing memory map %d %f \n",currentData.PID,(double)this->Y_Mem.at(currentData.PID).size());

			}
			else{

				//printf("Creating memory map\n");

				QVector<double> newMeasuresMem;
				QVector<double> newMeasuresNumber;

				newMeasuresMem.append(currentData.memory);
				newMeasuresNumber.append(currentData.measureNumber);

				X_Mem.insert(std::pair<unsigned int,QVector<double> >(currentPID,newMeasuresNumber));
				Y_Mem.insert(std::pair<unsigned int,QVector<double> > (currentPID,newMeasuresMem));
			}

			if(MaxMem < (double)currentData.memory) {
				MaxMem = (double)currentData.memory;
			}

			if(Max_X_Mem < (double)currentData.measureNumber) {
				Max_X_Mem = (double)currentData.measureNumber + 1.0;
			}

			//Max_X_Mem = (double)Y_Mem.at(currentPID).size() + 1.0;


			if(this->globalMaxMEM < (double)currentData.memory) {
				this->globalMaxMEM = (double)currentData.memory;
				this->globalMaxMEMPID = currentPID;

			}
			if (this->globalMinMEM > (double)currentData.memory) {
				this->globalMinMEM = (double)currentData.memory;
				this->globalMinMEMPID = currentPID;
			}

			//printf("Max_X_Mem %f\n",Max_X_Mem);

			if(X_CPU.count(currentPID) > 0) {

				//printf("Adding data to existing CPU map\n");


				Y_CPU.at(currentPID).append((double)currentData.cpuPercentage);
				//X_CPU.at(currentPID).append((double)Y_CPU.at(currentPID).size());
				X_CPU.at(currentPID).append((double)currentData.measureNumber);
			}
			else{

				//printf("Creating CPU map\n");

				QVector<double> newMeasuresCPU;
				QVector<double> newMeasuresNumber;

				newMeasuresCPU.append(currentData.cpuPercentage);
				newMeasuresNumber.append(currentData.measureNumber);

				X_CPU.insert(std::pair<unsigned int,QVector<double> >(currentPID,newMeasuresNumber));
				Y_CPU.insert(std::pair<unsigned int,QVector<double> > (currentPID,newMeasuresCPU));
			}

			if(MaxCPU < (double)currentData.cpuPercentage) {
				MaxCPU = (double)currentData.cpuPercentage;

			}

			if(Max_X_CPU < (double) currentData.measureNumber){
				Max_X_CPU = (double)currentData.measureNumber + 1.0;
			}
			//Max_X_CPU = (double)Y_CPU.at(currentData.PID).size() + 1.0;

			if(this->globalMaxCPU < (double)currentData.cpuPercentage) {
				this->globalMaxCPU = (double)currentData.cpuPercentage;
				this->globalMaxCPUPID = currentPID;

			}
			if (this->globalMinCPU > (double)currentData.cpuPercentage) {
				this->globalMinCPU = (double)currentData.cpuPercentage;
				this->globalMinCPUPID = currentPID;
			}


			//printf("Max_X_CPU %f\n",Max_X_CPU);



		}

		//Insert current data in current Row
		this->insertDataInTable(j,currentData);


		j++;
	}

	this->setTableColours();


	//Labels with statictics
	this->ui->label_InfoMaxMemData->setText(QString(std::to_string(this->globalMaxMEMPID).c_str()));
	this->ui->label_InfoMaxMemProcessData->setText(QString(std::to_string(this->globalMaxMEM).c_str()));

	this->ui->label_InfoMinMemData->setText(QString(std::to_string(this->globalMinMEMPID).c_str()));
	this->ui->label_InfoMinMemProcessData->setText(QString(std::to_string(this->globalMinMEM).c_str()));

	this->ui->label_InfoMaxCpuData->setText(QString(std::to_string(this->globalMaxCPUPID).c_str()));
	this->ui->label_InfoMaxCpuProcessData->setText(QString(std::to_string(this->globalMaxCPU).c_str()));

	this->ui->label_InfoMinCpuData->setText(QString(std::to_string(this->globalMinCPUPID).c_str()));
	this->ui->label_InfoMinCpuProcessData->setText(QString(std::to_string(this->globalMinCPU).c_str()));



	//Iterate over plots data

	std::map<unsigned int,QVector<double> >::iterator iter;

	std::map<unsigned int,QVector<double> >::iterator iter2 = Y_CPU.begin();

	QBrush tmpColour;

	for(iter = X_CPU.begin(); iter != X_CPU.end(); iter++){
		//printf("Plotting CPU %u\n",iter->first);

		this->ui->widget_PlotCPU->addGraph();


		QVector<double> tmpX = iter->second;
		QVector<double> tmpY = iter2->second;


		this->ui->widget_PlotCPU->graph(numGraph)->setData(tmpX,tmpY);

		tmpColour = this->colours.at(numGraph % this->colours.size());

		this->ui->widget_PlotCPU->graph(numGraph)->setPen(QPen(tmpColour,1));

		iter2++;

		numGraph++;

	}

	iter2 = Y_Mem.begin();
	numGraph = 0;

	for(iter = X_Mem.begin(); iter != X_Mem.end(); iter++){

		this->ui->widget_PlotMEM->addGraph();
		//printf("Plotting Mem %u\n",iter->first);

		QVector<double> tmpX = iter->second;
		QVector<double> tmpY = iter2->second;



		this->ui->widget_PlotMEM->graph(numGraph)->setData(tmpX,tmpY);

		tmpColour = this->colours.at(numGraph % this->colours.size());

		this->ui->widget_PlotMEM->graph(numGraph)->setPen(QPen(tmpColour,1));

		iter2++;

		numGraph++;
	}

	this->ui->widget_PlotMEM->xAxis->setRange(0, Max_X_Mem);
	this->ui->widget_PlotMEM->yAxis->setRange(0, MaxMem);
	this->ui->widget_PlotMEM->replot();

	this->ui->widget_PlotCPU->xAxis->setRange(0, Max_X_CPU);
	this->ui->widget_PlotCPU->yAxis->setRange(0, MaxCPU);
	this->ui->widget_PlotCPU->replot();

	//printf("NUmber of data: %lu\n",this->data.size());

}

void DataWidget::resetPlots() {

}

std::vector<std::string> DataWidget::Data2Vector(Agent2MasterDataMsg data) {

	std::vector<std::string> dataReturned;

	//We follow the order: {"PID","User","RES Mem","CPU%","MEM%","Process","Threads","L1 DCM","L2_DCM","TOTINS","Colour"};
	//printf("%d %llu\n",data.PID, data.memory);

	dataReturned.push_back(std::to_string(data.PID));
	dataReturned.push_back(data.userName);
	dataReturned.push_back(std::to_string(data.memory));
	dataReturned.push_back(std::to_string(data.cpuPercentage));
	dataReturned.push_back("0");

	dataReturned.push_back(data.processName);
	dataReturned.push_back(std::to_string(data.measureNumber));
	dataReturned.push_back("0");
	dataReturned.push_back("0");
	dataReturned.push_back("0");
	dataReturned.push_back("0");
	dataReturned.push_back("");

	/*
	dataReturned.push_back("PID");
	dataReturned.push_back("User");
	dataReturned.push_back("12");
	dataReturned.push_back("50");
	dataReturned.push_back("0");


	dataReturned.push_back("Process");
	dataReturned.push_back("Measure");
	dataReturned.push_back("0");
	dataReturned.push_back("0");
	dataReturned.push_back("0");
	dataReturned.push_back("0");
	dataReturned.push_back("");
*/
	return dataReturned;
}

void DataWidget::printData() {

	std::map<unsigned int,std::map<unsigned long int,Agent2MasterDataMsg> >::iterator it;

	std::map<unsigned long int,Agent2MasterDataMsg>::iterator it2;

	//MEM
	for(it = this->data.begin(); it != this->data.end(); it++){

		std::map<unsigned long int,Agent2MasterDataMsg> currentPID = it->second;



		printf("PID: %u  ==============\n",it->first);

		for(it2 = currentPID.begin(); it2 != currentPID.end(); it2++){

			unsigned long measureNumber = it2->first;
			Agent2MasterDataMsg currentData = it2->second;
			printf("Measure: %lu ",measureNumber);

			std::vector<std::string> availableData = this->Data2Vector(currentData);

			for( unsigned int i = 0; i< availableData.size(); i++){

				printf(" %s ",availableData.at(i).c_str());

			}
			printf("\n");
		}


	}

}


void DataWidget::printDataFromAgent(Agent2MasterDataMsg data){

	printf("%u %d %u %lu %lu %f %f %s %s %llu\n",data.packageId,data.PID,data.agentId,data.messageNumber,data.measureNumber,data.cpuPercentage,data.totalCpuPercentage,data.userName,data.processName,data.memory);

}


void DataWidget::insertDataInTable(int j, Agent2MasterDataMsg data) {

	//We follow the order: {"PID","User","RES Mem","CPU%","MEM%","Process","Threads","L1 DCM","L2_DCM","TOTINS","Colour"};
	int col = 0;

	//PID
	QTableWidgetItem *newItem = new QTableWidgetItem(QString(std::to_string(data.PID).c_str()));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//Username
	newItem = new QTableWidgetItem(QString(data.userName));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//Memory
	newItem = new QTableWidgetItem(QString(std::to_string(data.memory).c_str()));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//CPU
	newItem = new QTableWidgetItem(QString(std::to_string(data.cpuPercentage).c_str()));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//MEM%
	newItem = new QTableWidgetItem(QString(std::to_string(0.0).c_str()));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//ProcessName
	newItem = new QTableWidgetItem(QString(data.processName));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//Measure number
	newItem = new QTableWidgetItem(QString(std::to_string(data.measureNumber).c_str()));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//Threads
	newItem = new QTableWidgetItem(QString(std::to_string(0).c_str()));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//L1_DCM
	newItem = new QTableWidgetItem(QString(std::to_string(0).c_str()));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//L2_DCM
	newItem = new QTableWidgetItem(QString(std::to_string(0).c_str()));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//TOT_INS
	newItem = new QTableWidgetItem(QString(std::to_string(0).c_str()));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//Colour
	newItem = new QTableWidgetItem(QString(""));
	this->ui->tableWidget_Data->setItem(j,col,newItem);


}

void DataWidget::setTableColours() {

	QBrush tmpColour;

	for ( int i = 0; i< this->ui->tableWidget_Data->rowCount(); i++) {

		tmpColour = this->colours.at(i % (int)this->colours.size());

		this->ui->tableWidget_Data->item(i,this->ui->tableWidget_Data->columnCount()-1)->setBackground(tmpColour);


	}

}

void DataWidget::CpuToCSV() {

	//QFileDialog.getSaveFileName(self, QString.fromLocal8Bit(u"Save File"), filter=u'CSV (*.csv)')

	QString fileName = QFileDialog::getSaveFileName(this,"Save file","./","CSV (*.csv)");

	if(!fileName.isEmpty() && !fileName.isNull()){
		FILE *fp = fopen(fileName.toStdString().c_str(),"w");

		//int numGraphs = this->ui->widget_PlotCPU->graphCount();

		std::map<unsigned int,std::map<unsigned long int,Agent2MasterDataMsg> >::iterator it;

		std::map<unsigned long int,Agent2MasterDataMsg>::iterator it2;

		for ( it = this->data.begin(); it != this->data.end(); it++) {

			fprintf(fp,"%u;",it->first);

			for( it2 = it->second.begin(); it2 != it->second.end(); it2++){
				fprintf(fp,"%f;",it2->second.cpuPercentage);
			}

			fprintf(fp,"\n");

		}


		fclose(fp);
	}



}

void DataWidget::MemToCSV() {

	//QFileDialog.getSaveFileName(self, QString.fromLocal8Bit(u"Save File"), filter=u'CSV (*.csv)')

	QString fileName = QFileDialog::getSaveFileName(this,"Save file","./","CSV (*.csv)");

	if(!fileName.isEmpty() && !fileName.isNull()){
		FILE *fp = fopen(fileName.toStdString().c_str(),"w");

		//int numGraphs = this->ui->widget_PlotCPU->graphCount();

		std::map<unsigned int,std::map<unsigned long int,Agent2MasterDataMsg> >::iterator it;

		std::map<unsigned long int,Agent2MasterDataMsg>::iterator it2;

		for ( it = this->data.begin(); it != this->data.end(); it++) {

			fprintf(fp,"%u;",it->first);

			for( it2 = it->second.begin(); it2 != it->second.end(); it2++){
				fprintf(fp,"%llu;",it2->second.memory);
			}

			fprintf(fp,"\n");

		}


		fclose(fp);
	}



}

void DataWidget::MemToPdf(){

	QString fileName = QFileDialog::getSaveFileName(this,"Save file","./","PDF (*.pdf)");

	if(!fileName.isEmpty() && !fileName.isNull()){

		this->ui->widget_PlotMEM->savePdf(fileName);
	}

}

void DataWidget::CpuToPdf(){

	QString fileName = QFileDialog::getSaveFileName(this,"Save file","./","PDF (*.pdf)");

	if(!fileName.isEmpty() && !fileName.isNull()){
		this->ui->widget_PlotCPU->savePdf(fileName);
	}


}


void DataWidget::Table2CSV(){

	QString fileName = QFileDialog::getSaveFileName(this,"Save file","./","CSV (*.csv)");
	printf("Gardando CSV\n");
	if(!fileName.isEmpty() && !fileName.isNull()){
		FILE *fp = fopen(fileName.toStdString().c_str(),"w");
		printf("Gardando CSV en %s\n",fileName.toStdString().c_str());
		//int numGraphs = this->ui->widget_PlotCPU->graphCount();

		for(unsigned int i = 0; i< this->tableHeaders.size();i++){

			fprintf(fp,"%s;",this->tableHeaders.at(i).c_str());

		}

		fprintf(fp,"\n");

		for (int row = 0; row < this->ui->tableWidget_Data->rowCount(); row++) {

			for (int col = 0; col< this->ui->tableWidget_Data->columnCount(); col++){
				fprintf(fp,"%s;",this->ui->tableWidget_Data->item(row,col)->text().toStdString().c_str());
			}

			fprintf(fp,"\n");
		}

		fclose(fp);
	}

}
