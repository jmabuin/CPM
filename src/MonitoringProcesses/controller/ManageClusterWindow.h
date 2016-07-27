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
