#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMdiArea>
#include <QtGui/QMainWindow>
#include <QSignalMapper>
#include "ui_fabrication.h"
#include "vtkWidget.h"
#include "vtkView.h"
#include "InteractorStyleMoveGlyph.h"


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

	void setHandleMenu(QPoint point, Qt::Orientation orientation, QSplitter *origin);

	inline VtkWidget *VTKA() const 
	{
		if(mdiArea->currentSubWindow()==0) return 0;
		VtkView *mvc = currentVtkView();
		if(!mvc) return 0;
		VtkWidget *glw =  qobject_cast<VtkWidget*>(mvc->currentView());
		if(!glw) return 0;
		return glw;
	}

	inline VtkView* currentVtkView() const 
	{
		if(mdiArea->currentSubWindow()==0) return 0;
		VtkView *mvc = qobject_cast<VtkView *>(mdiArea->currentSubWindow());
		if(!mvc)
		{
			mvc = qobject_cast<VtkView *>(mdiArea->currentSubWindow()->widget());
			return mvc;
		}
		else return 0;
	}

	inline VtkWidget *VTKAObject()
	{
		if(mdiArea->currentSubWindow()==0) return 0;
		QList<QMdiSubWindow*> windows = mdiArea->subWindowList();
		VtkView* mvc = qobject_cast<VtkView *>(windows[0]->widget());
		if(!mvc) return 0;
		VtkWidget *glw =  qobject_cast<VtkWidget*>(mvc->currentView());
		return glw;
	}

	inline VtkWidget *VTKAImage() const 
	{
		if(mdiArea->currentSubWindow()==0) return 0;
		QList<QMdiSubWindow*> windows = mdiArea->subWindowList();
		VtkView* mvc = qobject_cast<VtkView *>(windows[1]->widget());
		if(!mvc) return 0;
		VtkWidget *glw =  qobject_cast<VtkWidget*>(mvc->currentView());
		return glw;
	}

public slots:
	void selectOnOnject(vtkSmartPointer<vtkPolyData> path);

	void updateAllViews();

private:
	void createActions();

	void createMenus();

	QMdiSubWindow* newImage();

	bool isVisible(double *point, vtkSmartPointer<vtkRenderer> render);

	void updateMenu();

private slots:
	void open(QString fileName = QString());

	void draw();

	void select();

	void rotate();

	void exportModel();

	void setActiveSubWindow(QWidget *window);

	void wrapSetActiveSubWindow(QWidget* window);

private:
	Ui::fabricationClass ui;
	QMdiArea* mdiArea;
	QMenuBar *menuBar;
	QSignalMapper *windowMapper;

	QAction* openAct;
	QAction* drawAct;
	QAction* selectAct;
	QAction* rotateAct;
	QAction* exportAct;

	QMenu *handleMenu;
	QMenu *splitMenu;
	QMenu *unSplitMenu;

	QAction *setSplitHAct;
	QAction *setSplitVAct;
	QActionGroup *setSplitGroupAct;
	QAction *setUnsplitAct;
	QActionGroup *splitGroupAct;
	QActionGroup *unsplitGroupAct;
	QAction *splitUpAct;
	QAction *splitDownAct;
	QAction *unsplitUpAct;
	QAction *unsplitDownAct;
	QAction *splitRightAct;
	QAction *splitLeftAct;
	QAction *unsplitRightAct;
	QAction *unsplitLeftAct;

	vtkSmartPointer<vtkUnstructuredGrid> selecteddata;
	std::vector<int> typeIds;
	InteractorStyleMoveGlyph* moveStyle;
};

#endif // FABRICATION_H
