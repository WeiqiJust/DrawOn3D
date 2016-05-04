#ifndef VTKWIDGET_H
#define VTKWIDGET_H

#include <QtGui>
#include <QVTKWidget.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageTracerWidget.h>

#include <vtkCallbackCommand.h>
#include <vtkImageActor.h>
#include <vtkActor.h>
#include <vtkSphereSource.h>
#include <vtkImageMapper3D.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkInteractorStyleImage.h>
#include <vtkProperty.h>
#include <vtkJPEGReader.h>
#include <vtkWindowToImageFilter.h> // Screenshot
#include <vtkJPEGWriter.h> // Screenshot
#include <vtkInteractorObserver.h>

#include <vtkVertexGlyphFilter.h>
#include <vtkPointData.h>
#include <vtkCellLocator.h>
#include <vtkCamera.h>
#include <vtkImageResize.h>
#include <vtkCleanPolyData.h>
#include <vtkDataSetMapper.h>
#include <vtkDelaunay2D.h>
#include <vtkSelectVisiblePoints.h>
#include <vtkCellPicker.h>
#include <vtkGlyph3D.h>
#include <vtkPoints.h>
#include <vtkIdTypeArray.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkObjectFactory.h>
#include <algorithm>
#include <vtkRendererCollection.h>
#include <vtkExtractSelection.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkInformation.h>
#include <vtkUnstructuredGrid.h>
#include <vtkOBJReader.h>
#include <vtkInteractorStyleTrackballCamera.h>

#include <vtkImageCanvasSource2D.h>

#include "vtkView.h"

enum OBJECTTYPE{EMPTY=0, IMAGE2D, MODEL3D};

class VtkWidget : public QWidget
{
  Q_OBJECT

public:
	VtkWidget(VtkView *mvcont);
	VtkWidget(QWidget *parent = 0);
	~VtkWidget();

	VtkView* mvc();

	int getId() {return id;}

	OBJECTTYPE render(QString filename);

	void screenshot(QString file);

	void renderUpdate();

private:
	void initializeMainWindow();

	void copyExistingPointers(VtkView *vcont);

	bool read3D(QString filename);

	void rendering3D();

	bool read2D(QString filename);

	void rendering2D();

	static void CallbackFunction (vtkObject* caller,
                       long unsigned int vtkNotUsed(eventId),
                       void* vtkNotUsed(clientData),
                       void* vtkNotUsed(callData));

signals:
	void selectedSurface(vtkSmartPointer<vtkPolyData> path);
	void test();

public:
	QVTKWidget* mQVTKWidget;
	QVBoxLayout* mLayout;
	vtkSmartPointer<vtkRenderer> mRenderer;
	vtkImageData* mVtkImageData;
	vtkPolyData* mVtkPolyData; 
	vtkSmartPointer<vtkPolyDataMapper> mMapper;
	vtkSmartPointer<vtkActor> mActor;
	vtkSmartPointer<vtkImageActor> mActor2D;
	vtkSmartPointer<vtkImageTracerWidget> tracer;
	vtkSmartPointer<QVTKInteractor> mInteractor;
	static VtkWidget *vwidget;

private:
	int id;
};



#endif 