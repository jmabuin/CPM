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

#ifndef CONFIGURATIONWINDOW_H
#define CONFIGURATIONWINDOW_H

#include <QDialog>
#include <QSettings>
#include <QString>
#include <QMessageBox>
#include "Configuration.h"
#include "Network.h"

namespace Ui {
class ConfigurationWindow;
}


/*!
 * \brief The ConfigurationWindow class
 * \details   This class is the Configuration Window class
 * \author    Jose M. Abuin
 * \version   0.1
 * \date      2015
 * \copyright GNU Public License.
 */
class ConfigurationWindow : public QDialog
{
	Q_OBJECT

public:
	explicit ConfigurationWindow(QWidget *parent = 0);
	~ConfigurationWindow();

	//Configuration getConfiguration();

private slots:

	void done(int result);

private:
	Ui::ConfigurationWindow *ui;

	//QSettings *settings;
	Config config;



	void initConfiguration();
	bool isAlnum(std::string cadea);
	bool isNumeric(std::string cadea);
	bool saveAndClose();



};

#endif // CONFIGURATIONWINDOW_H
