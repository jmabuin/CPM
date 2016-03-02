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

/*!
 *  \brief     DataWidget class.
 *  \details   This class is used to store data from Agents and represent them into the plots
 *  \author    Jose M. Abuin
 *  \version   0.1
 *  \date      2015
 *  \copyright GNU Public License.
 */
class DataWidget : public QWidget
{
	Q_OBJECT

public:
	explicit DataWidget(QWidget *parent = 0);
	~DataWidget();

	void addData(Agent2MasterDataMsg newData);
	void addDataEnergy(Agent2MasterEnergyMsg newData);

private slots:
	void plotData();
	void CpuToCSV();
	void MemToCSV();
	void CpuToPdf();
	void MemToPdf();
	void Table2CSV();
	void updateEnergyDataInfo();

signals:
	void updateData();
	void updateEnergyData();


private:
	Ui::DataWidget *ui;/*!< Ui that stores the graphic items */
	Configuration configuration; /*!< Stores the configuration parameters */

	int currentTableRow; /*!< To store the number of the row currently selected in the table */

	double globalMaxCPU; /*!< Global maximum CPU percentage in this DataWidget */
	double globalMaxMEM; /*!< Global maximum memory in this DataWidget */

	double globalMinCPU; /*!< Global minimum CPU percentage in this DataWidget */
	double globalMinMEM; /*!< Global minimum memory in this DataWidget */

	unsigned int globalMaxCPUPID; /*!< PID of the global maximum CPU percentage in this DataWidget */
	unsigned int globalMaxMEMPID; /*!< PID of the global maximum memory in this DataWidget */

	unsigned int globalMinCPUPID; /*!< PID of the global minimum CPU percentage in this DataWidget */
	unsigned int globalMinMEMPID; /*!< PID of the global minimum memory in this DataWidget */

	unsigned long int numMeasuresMem; /*!< Number of measures taken for CPU percentage */
	unsigned long int numMeasuresCpu; /*!< Number of measures taken for memory */

	double energyTimes[4];		/*!< Stored time for calculate average Watts */

	std::map<unsigned int,std::map<unsigned long int,Agent2MasterDataMsg> > data; /*!< Map that stores measures in the format <PID,<Measure_Number,Process_Data>> */

	std::map<unsigned long int,Agent2MasterEnergyMsg> energyData; /*!< Map that stores energy measurements in the format <Measure Number, Process_Data> */

	std::vector<std::string> tableHeaders; /*!< Headers of table containing processes data */
	std::vector<std::string> tableHeadersPapi; /*!< Headers of table containing processes data for future versions*/

	std::vector<QBrush> colours; /*!< Colours to show when plotting */

	void resetPlots();
	void resetTable();
	std::vector<std::string> Data2Vector(Agent2MasterDataMsg data);
	void printData();
	void printDataFromAgent(Agent2MasterDataMsg data);
	void insertDataInTable(int j, Agent2MasterDataMsg data);
	void setTableColours();




};

#endif // DATAWIDGET_H
