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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include "ConfigurationWindow.h"
#include "ManageClusterWindow.h"
#include <stdio.h>
#include "DataWidget.h"
#include "Network.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:

	void configureRun();
	void manageSSHRun();
	void exitRun();

	void startMeasures();
	void stopMeasures();

private:

	void createActions();
	void createMenus();
	void initWindow();
	void initButtonsActions();
	static void *getData(void *param);
	void closeEvent(QCloseEvent *event);

	Ui::MainWindow *ui;

	QMenu *fileMenu;
	QAction *configureAction;
	QAction *manageSettingsSSH;
	QAction *exitProgram;

	ConfigurationWindow *configurationWindow;
	ManageClusterWindow *manageClusterWindow;
	bool receivingData;

	struct in_addr masterNodeIp;
	unsigned int rxPortInMaster;

};

#endif // MAINWINDOW_H
