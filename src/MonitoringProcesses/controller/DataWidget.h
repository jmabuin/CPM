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

#ifndef DATAWIDGET_H
#define DATAWIDGET_H

#include <QWidget>
#include "Configuration.h"
#include "Network.h"

namespace Ui {
class DataWidget;
}

class DataWidget : public QWidget
{
	Q_OBJECT

public:
	explicit DataWidget(QWidget *parent = 0);
	~DataWidget();

	void addData(Agent2MasterDataMsg newData);

private slots:
	void plotData();
	void CpuToCSV();
	void MemToCSV();
	void CpuToPdf();
	void MemToPdf();
	void Table2CSV();

signals:
	void updateData();


private:
	Ui::DataWidget *ui;
	Configuration configuration;

	int currentTableRow;

	double globalMaxCPU;
	double globalMaxMEM;

	double globalMinCPU;
	double globalMinMEM;

	unsigned int globalMaxCPUPID;
	unsigned int globalMaxMEMPID;

	unsigned int globalMinCPUPID;
	unsigned int globalMinMEMPID;



	unsigned long int numMeasuresMem;
	unsigned long int numMeasuresCpu;

	//Map that stores measures in the format <PID,<Measure_Number,Process_Data>>
	std::map<unsigned int,std::map<unsigned long int,Agent2MasterDataMsg> > data;




	std::vector<std::string> tableHeaders;
	std::vector<std::string> tableHeadersPapi;

	std::vector<QBrush> colours;

	void resetPlots();
	void resetTable();
	std::vector<std::string> Data2Vector(Agent2MasterDataMsg data);
	void printData();
	void printDataFromAgent(Agent2MasterDataMsg data);
	void insertDataInTable(int j, Agent2MasterDataMsg data);
	void setTableColours();




};

#endif // DATAWIDGET_H
