#ifndef MANAGECLUSTERWINDOW_H
#define MANAGECLUSTERWINDOW_H

#include <QDialog>

namespace Ui {
class ManageClusterWindow;
}

class ManageClusterWindow : public QDialog
{
	Q_OBJECT

public:
	explicit ManageClusterWindow(QWidget *parent = 0);
	~ManageClusterWindow();

private:
	Ui::ManageClusterWindow *ui;
};

#endif // MANAGECLUSTERWINDOW_H
