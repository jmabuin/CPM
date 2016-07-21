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

private:
	Ui::ManageClusterWindow *ui;

	Config config;

	void initConfiguration();
	bool isAlnum(std::string cadea);
	bool isNumeric(std::string cadea);
	bool saveAndClose();
};

#endif // MANAGECLUSTERWINDOW_H
