#ifndef VTKVIEW_H
#define VTKVIEW_H

#include <QSplitter>

class VtkWidget;
class VtkView;
class MainWindow;

class VtkSplitter : public QSplitter
{
  Q_OBJECT

public:
  VtkSplitter ( QWidget * parent = 0 );
  VtkSplitter(Qt::Orientation orientation, QWidget *parent = 0);

  VtkView *getRootContainer();

protected:
  QSplitterHandle *createHandle();
};

class VtkSplitterHandle : public QSplitterHandle
{
  Q_OBJECT

public:
  VtkSplitterHandle(Qt::Orientation orientation, QSplitter *parent);

protected:
  void mousePressEvent ( QMouseEvent * e );
};

class VtkView : public VtkSplitter
{
  Q_OBJECT

public:
  VtkView(QWidget *parent = 0);
    ~VtkView();

  VtkWidget *copyCurrentView();
  void addView(VtkWidget* viewer,  Qt::Orientation);
  void addOrphanView(VtkWidget* viewer);
  void initializeView(VtkWidget* viewer);
  void removeView(int);

  VtkWidget* currentView();

  int getNextViewerId();
  int viewerCounter();

  void updateAllViewer();
  void update(int id);

  VtkWidget* getViewer(int id);
  int getViewerByPicking(QPoint);
  VtkView * mvc();

  int currentId;
  QString prjName;

protected:
  void closeEvent(QCloseEvent *event);

signals:
    void updateMainWindowMenus(); //updates the menus of the MainWindow

public slots:
  // Called when we change viewer, set the current viewer
    void updateCurrent(int current);

private:
  QList<VtkWidget *> viewerList; // widgets for the OpenGL contexts and images
  MainWindow * findmw();

};


#endif // VTKVIEW_H
