#include "ManageClusterWindow.h"
#include "ui_ManageClusterWindow.h"

ManageClusterWindow::ManageClusterWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ManageClusterWindow)
{
	ui->setupUi(this);
}

ManageClusterWindow::~ManageClusterWindow()
{
	delete ui;
}
