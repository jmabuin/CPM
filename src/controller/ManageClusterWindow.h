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

#ifndef MANAGECLUSTERWINDOW_H
#define MANAGECLUSTERWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include "Configuration.h"

namespace Ui {
class ManageClusterWindow;
}

class ManageClusterWindow : public QDialog
{
	Q_OBJECT

public:
	explicit ManageClusterWindow(QWidget *parent = 0);
	~ManageClusterWindow();

private slots:

	void done(int result);
	void executeSlot();
	void openFileDialogKeyFile();


private:
	Ui::ManageClusterWindow *ui;

	Config config;

	void initConfiguration();
	void initWindow();
	bool isAlnum(std::string cadea);
	bool isNumeric(std::string cadea);
	bool saveAndClose();

	std::string projectGitURL;

	void deployMasterRun();
	void deployAgentsRun();
	void deployAll();

	void stopMasterRun();
	void stopAgentsRun();
	void stopAllRun();

	void checkMasterRun();
	void checkAgentsRun();
	void checkAllRun();

	void runMasterRun();
	void runAgentsRun();
	void runAllRun();

	void executeCommandInMaster(std::string command);
	void executeCommandInAgents(std::string command);
	void copyFromMaster2Agents(std::string path);

};

#endif // MANAGECLUSTERWINDOW_H
