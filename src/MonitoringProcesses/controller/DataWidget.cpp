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

/*!
 *  \brief     DataWidget class.
 *  \details   This class is used to store data from Agents and represent them into the plots
 *  \author    Jose M. Abuin
 *  \version   0.1
 *  \date      2015
 *  \copyright GNU Public License.
 */

DataWidget::DataWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DataWidget)
{
	ui->setupUi(this);

	//ConfigurationWindow confWindow = ConfigurationWindow(this);

	this->configuration = Configuration(); /*!< Stores the configuration parameters */

	this->tableHeaders = {"PID","User","RES Mem","CPU%","MEM%","Process","Measure Number","Threads","L1 DCM","L2_DCM","TOTINS","Colour"}; /*!< Headers of table containing processes data */


	this->colours = { QBrush(QColor(255,0,0)),QBrush(QColor(0,255,0)),QBrush(QColor(0,0,255)),QBrush(QColor(255,255,0)),QBrush(QColor(255,0,255)),QBrush(QColor(0,255,255)),QBrush(QColor(255,255,255))}; /*!< Colours to show when plotting */


	this->ui->widget_PlotCPU->xAxis->setLabel("Measure number"); /*!< X Axis legend for CPU plot */
	this->ui->widget_PlotCPU->yAxis->setLabel("CPU percentage"); /*!< Y Axis legend for CPU plot */

	this->ui->widget_PlotMEM->xAxis->setLabel("Measure number"); /*!< X Axis legend for memory plot */
	this->ui->widget_PlotMEM->yAxis->setLabel("Memory (KB)"); /*!< Y Axis legend for memory plot */

	this->ui->comboBox_Plotting->addItem("CPU - Memory");
	this->ui->comboBox_Plotting->addItem("PAPI_L1 - PAPI_L2");

	this->globalMaxCPU = 0.0; /*!< Global maximum CPU percentage in this DataWidget */
	this->globalMaxMEM = 0.0; /*!< Global maximum memory in this DataWidget */

	this->globalMinCPU = std::numeric_limits<double>::max(); /*!< Global minimum CPU percentage in this DataWidget */
	this->globalMinMEM = std::numeric_limits<double>::max(); /*!< Global minimum memory in this DataWidget */

	this->globalMaxCPUPID = 0; /*!< PID of the global maximum CPU percentage in this DataWidget */
	this->globalMaxMEMPID = 0; /*!< PID of the global maximum memory in this DataWidget */

	this->globalMinCPUPID = 0; /*!< PID of the global minimum CPU percentage in this DataWidget */
	this->globalMinMEMPID = 0; /*!< PID of the global minimum memory in this DataWidget */

	this->numMeasuresCpu = 0; /*!< Number of measures taken for CPU percentage */
	this->numMeasuresMem = 0; /*!< Number of measures taken for memory */


	/**
	 * Connect the updateData(); signal with the plotData(); function.
	 */
	QObject::connect(this,SIGNAL(updateData()),this,SLOT(plotData()));

	/**
	 * Connect the updateEnergyData(); signal with the updateEnergyDataInfo(); function.
	 */
	QObject::connect(this,SIGNAL(updateEnergyData()),this,SLOT(updateEnergyDataInfo()));

	/**
	 * Buttons connections with signal clicked();
	 */
	QObject::connect(this->ui->pushButton_2CSV_1,SIGNAL(clicked(bool)),this,SLOT(CpuToCSV()));
	QObject::connect(this->ui->pushButton_2CSV_2,SIGNAL(clicked(bool)),this,SLOT(MemToCSV()));


	QObject::connect(this->ui->pushButton_2PDF_1,SIGNAL(clicked(bool)),this,SLOT(CpuToPdf()));
	QObject::connect(this->ui->pushButton_2PDF_2,SIGNAL(clicked(bool)),this,SLOT(MemToPdf()));

	QObject::connect(this->ui->pushButton_2CSV_MainData,SIGNAL(clicked(bool)),this,SLOT(Table2CSV()));

	/**
	 * Combobox connection with Singal to update plots
	 */
	QObject::connect(this->ui->comboBox_Plotting,SIGNAL(currentIndexChanged(int)),this,SLOT(plotData()));
}

//! DataWidget destructor.
/*!
 * DataWidget destructor.
 */
DataWidget::~DataWidget()
{
	delete ui;
}

//! Procedure to add a new package from an Agent into the DataWidget data.
/*!
 * \param newData Data package coming from an Agent
 */
void DataWidget::addData(Agent2MasterDataMsg newData) {


	if(newData.packageId == PACKAGE_ID_DATAMSG){
		//The PID of the new data that comes from the Agent
		unsigned int PID = newData.PID;

		//If the PID does exists in the stored data, it only must be added.
		if(this->data.count(PID) > 0){

			//measureNumber = this->data.at(PID).size();
			this->data.at(PID).insert(std::pair<unsigned long int,Agent2MasterDataMsg> (newData.measureNumber,newData));

			//printf("Adding measure %lu to PID %u\n",measureNumber,PID);
		}
		//Otherwise it must be created
		else {

			//There is no data associated to this PID. We add it
			std::map<unsigned long int,Agent2MasterDataMsg> newDataToInsert; //<Measure Number, Data>

			//We insert Pair<Measure Number, Data> into the new Map
			newDataToInsert.insert(std::pair<unsigned long int,Agent2MasterDataMsg>(newData.measureNumber,newData));

			//Finally we add the <PID,Map> to the stored Data
			this->data.insert(std::pair<unsigned int,std::map< unsigned long int, Agent2MasterDataMsg> > (PID,newDataToInsert));

		}

		//The signal of new data received is emitted, so the plots can be updated
		emit this->updateData();
	}


}

//! Procedure to add a new energy data package from an Agent into the DataWidget data.
/*!
 * \param newData Data energy package coming from an Agent
 */
void DataWidget::addDataEnergy(Agent2MasterEnergyMsg newData) {


	if(newData.packageId == PACKAGE_ID_ENERGY){

		this->energyData.insert(std::pair<unsigned long int, Agent2MasterEnergyMsg> (newData.measureNumber,newData));

		emit this->updateEnergyData();
	}


}

//! Procedure to update the energy data.
void DataWidget::updateEnergyDataInfo() {

	std::map<unsigned long int, Agent2MasterEnergyMsg>::iterator itEnergyData;

	Agent2MasterEnergyMsg receivedData;

	for(itEnergyData = this->energyData.begin();itEnergyData != this->energyData.end(); itEnergyData++){

		receivedData = itEnergyData->second;
	}

	//QString(std::to_string(this->globalMaxMEMPID).c_str())
	this->ui->label_InfoEnergyMeasure->setText(QString(std::to_string(receivedData.measureNumber).c_str()));

	std::string energyData1;
	std::string energyData2;


	QLabel *labelsCPU[12];

	labelsCPU[0] = this->ui->label_PPt_CPU1;
	labelsCPU[1] = this->ui->label_PP1_CPU1;
	labelsCPU[2] = this->ui->label_PP0_CPU1;
	labelsCPU[3] = this->ui->label_PPt_CPU2;
	labelsCPU[4] = this->ui->label_PP1_CPU2;
	labelsCPU[5] = this->ui->label_PP0_CPU2;
	labelsCPU[6] = this->ui->label_PPt_CPU3;
	labelsCPU[7] = this->ui->label_PP1_CPU3;
	labelsCPU[8] = this->ui->label_PP0_CPU3;
	labelsCPU[9] = this->ui->label_PPt_CPU4;
	labelsCPU[10] = this->ui->label_PP1_CPU4;
	labelsCPU[11] = this->ui->label_PP0_CPU4;


	//It is not possible in the Agent to take RAPL measures

	int i = 0;
	int j = 0;

	for(i = 0; i< 24; i+=2) {
		if(receivedData.energyMeasures[i] == 0.0){
			energyData1 = "0.0";
			energyData2 = "0.0";
		}

		else {
			energyData1 = std::to_string(receivedData.energyMeasures[i+1]);
			energyData2 = std::to_string((double)receivedData.energyMeasures[i+1]/(double)receivedData.energyMeasures[i]);
		}

		labelsCPU[j]->setText(QString((energyData1+" J  -- "+energyData2+" W").c_str()));

		j++;
	}


}

//! Procedure to plot the data.
void DataWidget::plotData() {

	//First the data showed in the table are reset
	this->resetTable();


	int j = 0;
	//unsigned int k = 0;

	//Iterator to iterate over the data. <PID,Map<Measure Number,Data>>
	std::map<unsigned int,std::map<unsigned long int,Agent2MasterDataMsg> >::iterator it;

	//Iterator with the measures for each one of the PIDs. <Measure Number, Data>
	std::map<unsigned long int,Agent2MasterDataMsg>::iterator it2;

	//std::map<unsigned long int,Agent2MasterDataMsg>::iterator itInternal;

	//The two plots are cleared
	this->ui->widget_PlotCPU->clearGraphs();
	this->ui->widget_PlotMEM->clearGraphs();


	//To store the Max CPU and Mem
	double MaxMem = 0.0;
	double MaxCPU = 0.0;
	double Max_X_Mem = 0.0;
	double Max_X_CPU = 0.0;

	//Max values for PAPI L1 and L2
	long long int Max_L1 = 0;
	long long int Max_L2 = 0;
	double Max_X_L1 = 0.0;
	double Max_X_L2 = 0.0;

	//To use inside the plot as the number of graphic (PID) inside the plot
	unsigned int numGraph = 0;

	//Maps where data to plot are going to be stored
	std::map<unsigned int,QVector<double> > X_Mem;
	std::map<unsigned int,QVector<double> > Y_Mem;

	std::map<unsigned int,QVector<double> > X_CPU;
	std::map<unsigned int,QVector<double> > Y_CPU;

	std::map<unsigned int,QVector<double> > X_PAPI_L1;
	//std::map<unsigned int,QVector<long long int> > Y_PAPI_L1;
	std::map<unsigned int,QVector<double> > Y_PAPI_L1;

	std::map<unsigned int,QVector<double> > X_PAPI_L2;
	//std::map<unsigned int,QVector<long long int> > Y_PAPI_L2;
	std::map<unsigned int,QVector<double> > Y_PAPI_L2;

	//Iterate over PIDS
	for(it = this->data.begin(); it != this->data.end(); it++){

		//We get the Map that corresponds to this PID
		std::map<unsigned long int,Agent2MasterDataMsg> currentProcesses_PID = it->second;

		//The PID that is being processed
		unsigned int currentPID = it->first;

		//Current data to process
		Agent2MasterDataMsg currentData;


		std::vector<std::string> availableData;

		//Iterate over the Measure Numbers from the current PID
		for(it2 = currentProcesses_PID.begin(); it2 != currentProcesses_PID.end(); it2 ++) {

			//We get the data
			currentData = it2->second;


			if (this->ui->comboBox_Plotting->currentIndex() == 0){

				/*
				 * First we process the memory information
				 */

				//In X_Mem we store memory values. If X_Mem already has the current PID, the memory value and measure number are added at the end of the corresponding vector.
				if(X_Mem.count(currentPID) > 0) {

					//Add memory value
					Y_Mem.at(currentPID).append((double)currentData.memory);

					//Add measure number
					X_Mem.at(currentPID).append((double)currentData.measureNumber);

					//printf("Adding data to existing memory map %d %f \n",currentData.PID,(double)this->Y_Mem.at(currentData.PID).size());

				}
				//Otherwise, vectors need to be created and added.
				else{

					//printf("Creating memory map\n");

					//Vector creation
					QVector<double> newMeasuresMem;
					QVector<double> newMeasuresNumber;

					//Data are added at the end of vectors
					newMeasuresMem.append(currentData.memory);
					newMeasuresNumber.append(currentData.measureNumber);

					//And vectors are stored in the PID position of the maps
					X_Mem.insert(std::pair<unsigned int,QVector<double> > (currentPID,newMeasuresNumber));
					Y_Mem.insert(std::pair<unsigned int,QVector<double> > (currentPID,newMeasuresMem));
				}

				//Now we check if the max values for this PID are the ones from the data being processed.
				if(MaxMem < (double)currentData.memory) {
					MaxMem = (double)currentData.memory;
				}

				//To set the limit of the right side of the plot
				if(Max_X_Mem < (double)currentData.measureNumber) {
					Max_X_Mem = (double)currentData.measureNumber + 1.0;
				}





				//printf("Max_X_Mem %f\n",Max_X_Mem);

				/*
				* Second we process the CPU information
				*/

				//In X_CPU we store CPU percentage values. If X_CPU already has the current PID, the CPU value and measure number are added at the end of the corresponding vector.
				if(X_CPU.count(currentPID) > 0) {

					//Add CPU value
					Y_CPU.at(currentPID).append((double)currentData.cpuPercentage);

					//Add measure number
					X_CPU.at(currentPID).append((double)currentData.measureNumber);
				}
				//Otherwise, vectors need to be created and added.
				else{

					//Vector creation
					QVector<double> newMeasuresCPU;
					QVector<double> newMeasuresNumber;

					//Data are added at the end of vectors
					newMeasuresCPU.append(currentData.cpuPercentage);
					newMeasuresNumber.append(currentData.measureNumber);

					//And vectors are stored in the PID position of the maps
					X_CPU.insert(std::pair<unsigned int,QVector<double> > (currentPID,newMeasuresNumber));
					Y_CPU.insert(std::pair<unsigned int,QVector<double> > (currentPID,newMeasuresCPU));
				}

				//Now we check if the max values for this PID are the ones from the data being processed.
				if(MaxCPU < (double)currentData.cpuPercentage) {
					MaxCPU = (double)currentData.cpuPercentage;
				}

				//To set the limit of the right side of the plot
				if(Max_X_CPU < (double) currentData.measureNumber){
					Max_X_CPU = (double)currentData.measureNumber + 1.0;
				}


			}
			else if(this->ui->comboBox_Plotting->currentIndex() == 1){
				/*
				 * Third, Papi_L1
				 */

				//In X_CPU we store CPU percentage values. If X_CPU already has the current PID, the CPU value and measure number are added at the end of the corresponding vector.
				if(X_PAPI_L1.count(currentPID) > 0) {

					//Add L1 value
					Y_PAPI_L1.at(currentPID).append((double)currentData.papiMeasures[0]);

					//Add measure number
					X_PAPI_L1.at(currentPID).append((double)currentData.measureNumber);
				}
				//Otherwise, vectors need to be created and added.
				else{

					//Vector creation
					QVector<double> newMeasuresL1;
					QVector<double> newMeasuresNumber;

					//Data are added at the end of vectors
					newMeasuresL1.append(currentData.papiMeasures[0]);
					newMeasuresNumber.append(currentData.measureNumber);

					//And vectors are stored in the PID position of the maps
					X_PAPI_L1.insert(std::pair<unsigned int,QVector<double> > (currentPID,newMeasuresNumber));
					Y_PAPI_L1.insert(std::pair<unsigned int,QVector<double> > (currentPID,newMeasuresL1));
				}

				//Now we check if the max values for this PID are the ones from the data being processed.
				if(Max_L1 < currentData.papiMeasures[0]) {
					Max_L1 = currentData.papiMeasures[0];
				}

				//To set the limit of the right side of the plot
				if(Max_X_L1 < (double) currentData.measureNumber){
					Max_X_L1 = (double)currentData.measureNumber + 1.0;
				}


				/*
				 * Fourth, Papi_L2
				 */

				//In X_CPU we store CPU percentage values. If X_CPU already has the current PID, the CPU value and measure number are added at the end of the corresponding vector.
				if(X_PAPI_L2.count(currentPID) > 0) {

					//Add L1 value
					Y_PAPI_L2.at(currentPID).append((double)currentData.papiMeasures[1]);

					//Add measure number
					X_PAPI_L2.at(currentPID).append((double)currentData.measureNumber);
				}
				//Otherwise, vectors need to be created and added.
				else{

					//Vector creation
					QVector<double> newMeasuresL2;
					QVector<double> newMeasuresNumber;

					//Data are added at the end of vectors
					newMeasuresL2.append(currentData.papiMeasures[1]);
					newMeasuresNumber.append(currentData.measureNumber);

					//And vectors are stored in the PID position of the maps
					X_PAPI_L2.insert(std::pair<unsigned int,QVector<double> > (currentPID,newMeasuresNumber));
					Y_PAPI_L2.insert(std::pair<unsigned int,QVector<double> > (currentPID,newMeasuresL2));
				}

				//Now we check if the max values for this PID are the ones from the data being processed.
				if(Max_L2 < currentData.papiMeasures[1]) {
					Max_L2 = currentData.papiMeasures[1];
				}

				//To set the limit of the right side of the plot
				if(Max_X_L2 < (double) currentData.measureNumber){
					Max_X_L2 = (double)currentData.measureNumber + 1.0;
				}


			}


			//Max and min global values for Memory and CPU
			if(this->globalMaxMEM < (double)currentData.memory) {
				this->globalMaxMEM = (double)currentData.memory;
				this->globalMaxMEMPID = currentPID;

			}
			if (this->globalMinMEM > (double)currentData.memory) {
				this->globalMinMEM = (double)currentData.memory;
				this->globalMinMEMPID = currentPID;
			}

			//And the same for the global values among all the PIDs
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

		//Insert current data in current Row. Note that currentData is going to be the last of the measures for the processed PID and j is the row that corresponds to the PID in the table
		this->insertDataInTable(j,currentData);

		j++;
	}

	//Table colours are set
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

	if(this->ui->comboBox_Plotting->currentIndex() == 0){
		//Iterator for the X axis data
		std::map<unsigned int,QVector<double> >::iterator iter;

		//Iterator for the Y axis data
		std::map<unsigned int,QVector<double> >::iterator iter2 = Y_CPU.begin();

		QBrush tmpColour;

		//CPU plotting (Left plot)
		for(iter = X_CPU.begin(); iter != X_CPU.end(); iter++){ //Iterate over CPU X axis data

			//We add a new graph, this is, a process PID
			this->ui->widget_PlotCPU->addGraph();

			//X and Y data that corresponds to this PID
			QVector<double> tmpX = iter->second;
			QVector<double> tmpY = iter2->second;

			//We set the current data into the current graph
			this->ui->widget_PlotCPU->graph(numGraph)->setData(tmpX,tmpY);

			//Colour election
			tmpColour = this->colours.at(numGraph % this->colours.size());
			//Colour set
			this->ui->widget_PlotCPU->graph(numGraph)->setPen(QPen(tmpColour,1));

			//Advance of the Y data
			iter2++;

			//Advance of the numGraph parameter
			numGraph++;

		}

		//The same process for the Memory plot
		//Now iter2 is going to be the Y data for the memory
		iter2 = Y_Mem.begin();
		//The numGraph parameter is reset
		numGraph = 0;

		//Memory plotting
		for(iter = X_Mem.begin(); iter != X_Mem.end(); iter++){

			//We add a new graph, this is, a process PID
			this->ui->widget_PlotMEM->addGraph();

			//X and Y data that corresponds to this PID
			QVector<double> tmpX = iter->second;
			QVector<double> tmpY = iter2->second;

			//We set the current data into the current graph
			this->ui->widget_PlotMEM->graph(numGraph)->setData(tmpX,tmpY);

			//Colour election
			tmpColour = this->colours.at(numGraph % this->colours.size());
			//Colour set
			this->ui->widget_PlotMEM->graph(numGraph)->setPen(QPen(tmpColour,1));

			//Advance of the Y data
			iter2++;

			//Advance of the numGraph parameter
			numGraph++;
		}


		this->ui->widget_PlotCPU->xAxis->setLabel("Measure number"); /*!< X Axis legend for CPU plot */
		this->ui->widget_PlotCPU->yAxis->setLabel("CPU percentage"); /*!< Y Axis legend for CPU plot */

		this->ui->widget_PlotMEM->xAxis->setLabel("Measure number"); /*!< X Axis legend for memory plot */
		this->ui->widget_PlotMEM->yAxis->setLabel("Memory (KB)"); /*!< Y Axis legend for memory plot */

		//replot of both graphics with axis limits
		this->ui->widget_PlotMEM->xAxis->setRange(0, Max_X_Mem);
		this->ui->widget_PlotMEM->yAxis->setRange(0, MaxMem);
		this->ui->widget_PlotMEM->replot();

		this->ui->widget_PlotCPU->xAxis->setRange(0, Max_X_CPU);
		this->ui->widget_PlotCPU->yAxis->setRange(0, MaxCPU);
		this->ui->widget_PlotCPU->replot();
	}
	else if (this->ui->comboBox_Plotting->currentIndex() == 1){
		//Iterator for the X axis data
		std::map<unsigned int,QVector<double> >::iterator iter;

		//Iterator for the Y axis data
		std::map<unsigned int,QVector<double> >::iterator iter2 = Y_PAPI_L1.begin();

		QBrush tmpColour;

		//L1 plotting (Left plot)
		for(iter = X_PAPI_L1.begin(); iter != X_PAPI_L1.end(); iter++){ //Iterate over L1 misses X axis data

			//We add a new graph, this is, a process PID
			this->ui->widget_PlotCPU->addGraph();

			//X and Y data that corresponds to this PID
			QVector<double> tmpX = iter->second;
			QVector<double> tmpY = iter2->second;

			//We set the current data into the current graph
			this->ui->widget_PlotCPU->graph(numGraph)->setData(tmpX,tmpY);

			//Colour election
			tmpColour = this->colours.at(numGraph % this->colours.size());
			//Colour set
			this->ui->widget_PlotCPU->graph(numGraph)->setPen(QPen(tmpColour,1));

			//Advance of the Y data
			iter2++;

			//Advance of the numGraph parameter
			numGraph++;

		}

		//The same process for the Memory plot
		//Now iter2 is going to be the Y data for the memory
		iter2 = Y_PAPI_L2.begin();
		//The numGraph parameter is reset
		numGraph = 0;

		//Memory plotting
		for(iter = X_PAPI_L2.begin(); iter != X_PAPI_L2.end(); iter++){

			//We add a new graph, this is, a process PID
			this->ui->widget_PlotMEM->addGraph();

			//X and Y data that corresponds to this PID
			QVector<double> tmpX = iter->second;
			QVector<double> tmpY = iter2->second;

			//We set the current data into the current graph
			this->ui->widget_PlotMEM->graph(numGraph)->setData(tmpX,tmpY);

			//Colour election
			tmpColour = this->colours.at(numGraph % this->colours.size());
			//Colour set
			this->ui->widget_PlotMEM->graph(numGraph)->setPen(QPen(tmpColour,1));

			//Advance of the Y data
			iter2++;

			//Advance of the numGraph parameter
			numGraph++;
		}


		this->ui->widget_PlotCPU->xAxis->setLabel("Measure number"); /*!< X Axis legend for L1 plot */
		this->ui->widget_PlotCPU->yAxis->setLabel("L1 Misses"); /*!< Y Axis legend for L1 plot */

		this->ui->widget_PlotMEM->xAxis->setLabel("Measure number"); /*!< X Axis legend for L2 plot */
		this->ui->widget_PlotMEM->yAxis->setLabel("L2 Misses"); /*!< Y Axis legend for L2 plot */

		//replot of both graphics with axis limits
		this->ui->widget_PlotMEM->xAxis->setRange(0, Max_X_L2);
		this->ui->widget_PlotMEM->yAxis->setRange(0, Max_L2);
		this->ui->widget_PlotMEM->replot();

		this->ui->widget_PlotCPU->xAxis->setRange(0, Max_X_L1);
		this->ui->widget_PlotCPU->yAxis->setRange(0, Max_L1);
		this->ui->widget_PlotCPU->replot();





	}
	//printf("NUmber of data: %lu\n",this->data.size());

}

//! Procedure to reset the table data.
void DataWidget::resetTable() {

	//The data currently showed at the table is cleared
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
}

//! Procedure to reset the plots.
void DataWidget::resetPlots() {

}

//! Function to pass from Agent2MasterDataMsg to a vector of sctrings.
/*!
 * \param data Agent2MasterDataMsg data to be processed.
 * \return A vector of strings with the data.
 */
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

	return dataReturned;
}

//! Procedure to print all the data stored in this widget
void DataWidget::printData() {

	std::map<unsigned int,std::map<unsigned long int,Agent2MasterDataMsg> >::iterator it;

	std::map<unsigned long int,Agent2MasterDataMsg>::iterator it2;


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

//! Procedure to print data from a Agent2MasterDataMsg item
/*!
 * \brief DataWidget::printDataFromAgent
 * \param data Data to be printed
 */
void DataWidget::printDataFromAgent(Agent2MasterDataMsg data){

	printf("%u %d %u %lu %lu %f %f %s %s %llu\n",data.packageId,data.PID,data.agentId,data.messageNumber,data.measureNumber,data.cpuPercentage,data.totalCpuPercentage,data.userName,data.processName,data.memory);

}

//! Procedure to insert data into the table
/*!
 * \brief DataWidget::insertDataInTable
 * \param j Row where the data is going to be inserted
 * \param data Data to be inserted
 */
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
	newItem = new QTableWidgetItem(QString(std::to_string(data.papiMeasures[0]).c_str()));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//L2_DCM
	newItem = new QTableWidgetItem(QString(std::to_string(data.papiMeasures[1]).c_str()));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//TOT_INS
	newItem = new QTableWidgetItem(QString(std::to_string(data.papiMeasures[2]).c_str()));
	this->ui->tableWidget_Data->setItem(j,col,newItem);

	col++;

	//Colour
	newItem = new QTableWidgetItem(QString(""));
	this->ui->tableWidget_Data->setItem(j,col,newItem);


}
//! Procedure to set the colour in the last column of the table
/*!
 * \brief DataWidget::setTableColours
 */
void DataWidget::setTableColours() {

	QBrush tmpColour;

	for ( int i = 0; i< this->ui->tableWidget_Data->rowCount(); i++) {

		tmpColour = this->colours.at(i % (int)this->colours.size());

		this->ui->tableWidget_Data->item(i,this->ui->tableWidget_Data->columnCount()-1)->setBackground(tmpColour);


	}

}

//! Procedure to export data from CPU plot into a CSV file
/*!
 * \brief DataWidget::CpuToCSV
 */
void DataWidget::CpuToCSV() {

	QString fileName = QFileDialog::getSaveFileName(this,"Save file","./","CSV (*.csv)");

	if(!fileName.isEmpty() && !fileName.isNull()){
		FILE *fp = fopen(fileName.toStdString().c_str(),"w");

		std::map<unsigned int,std::map<unsigned long int,Agent2MasterDataMsg> >::iterator it;

		std::map<unsigned long int,Agent2MasterDataMsg>::iterator it2;

		unsigned long int max_time = 0;

		//We calculate the maximum time
		for ( it = this->data.begin(); it != this->data.end(); it++) {

			for( it2 = it->second.begin(); it2 != it->second.end(); it2++){
				if(it2->first>=max_time){
					max_time = it2->first;
				}
			}

		}

		//We print the table header
		fprintf(fp,"PID;");

		unsigned long int tmp_time = 0;

		for(tmp_time = 0; tmp_time<=max_time; tmp_time+=SLEEP_NUM_SECS){
			fprintf(fp,"%lu;",tmp_time);
		}

		fprintf(fp,"\n");

		//Now we print the data
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

//! Procedure to export data from Memory plot into a CSV file
/*!
 * \brief DataWidget::MemToCSV
 */
void DataWidget::MemToCSV() {

	QString fileName = QFileDialog::getSaveFileName(this,"Save file","./","CSV (*.csv)");

	if(!fileName.isEmpty() && !fileName.isNull()){
		FILE *fp = fopen(fileName.toStdString().c_str(),"w");

		std::map<unsigned int,std::map<unsigned long int,Agent2MasterDataMsg> >::iterator it;

		std::map<unsigned long int,Agent2MasterDataMsg>::iterator it2;

		unsigned long int max_time = 0;

		//We calculate the maximum time
		for ( it = this->data.begin(); it != this->data.end(); it++) {

			for( it2 = it->second.begin(); it2 != it->second.end(); it2++){
				if(it2->first>=max_time){
					max_time = it2->first;
				}
			}

		}

		//We print the table header
		fprintf(fp,"PID;");

		unsigned long int tmp_time = 0;

		for(tmp_time = 0; tmp_time<=max_time; tmp_time+=SLEEP_NUM_SECS){
			fprintf(fp,"%lu;",tmp_time);
		}

		fprintf(fp,"\n");

		//Now we print the data
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

//! Procedure to export data from Memory plot into a PDF file
/*!
 * \brief DataWidget::MemToPdf
 */
void DataWidget::MemToPdf(){

	QString fileName = QFileDialog::getSaveFileName(this,"Save file","./","PDF (*.pdf)");

	if(!fileName.isEmpty() && !fileName.isNull()){

		this->ui->widget_PlotMEM->savePdf(fileName);
	}

}

//! Procedure to export data from CPU plot into a PDF file
/*!
 * \brief DataWidget::CpuToPdf
 */
void DataWidget::CpuToPdf(){

	QString fileName = QFileDialog::getSaveFileName(this,"Save file","./","PDF (*.pdf)");

	if(!fileName.isEmpty() && !fileName.isNull()){
		this->ui->widget_PlotCPU->savePdf(fileName);
	}


}

//! Procedure to export data from the table into a CSV file
/*!
 * \brief DataWidget::Table2CSV
 */
void DataWidget::Table2CSV(){

	QString fileName = QFileDialog::getSaveFileName(this,"Save file","./","CSV (*.csv)");
	//printf("Gardando CSV\n");
	if(!fileName.isEmpty() && !fileName.isNull()){
		FILE *fp = fopen(fileName.toStdString().c_str(),"w");

		//printf("Gardando CSV en %s\n",fileName.toStdString().c_str());
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
