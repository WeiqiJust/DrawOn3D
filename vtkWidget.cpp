#include <omp.h>
#include <QVTKInteractor.h>

#include "vtkWidget.h"

VtkWidget* VtkWidget::vwidget = NULL;

void VtkWidget::CallbackFunction (vtkObject* caller,
                       long unsigned int vtkNotUsed(eventId),
                       void* vtkNotUsed(clientData),
                       void* vtkNotUsed(callData))
{	
	vtkImageTracerWidget* tracerWidget = static_cast<vtkImageTracerWidget*>(caller);

	vtkSmartPointer<vtkPolyData> path = vtkSmartPointer<vtkPolyData>::New();

	tracerWidget->GetPath(path);

	emit vwidget->selectedSurface(path);
}

VtkWidget::VtkWidget(QWidget * parent)
  :  QWidget()
{
  this->setParent(parent);
  initializeMainWindow();
  vwidget = this;
}

// Multi view constructor
VtkWidget::VtkWidget(VtkView *mvcont)
  :  QWidget()
{
  this->setParent(mvcont);
  id = mvcont->getNextViewerId();

  if (id > 0)
  {
    copyExistingPointers(mvcont);
  }
  else 
  {
    initializeMainWindow();
  }
  vwidget = this;
}

// NB: vtkSmartPointer and ITK Pointer will be deleted automatically when out-of-scope.
VtkWidget::~VtkWidget(){
  // Widgets
  mQVTKWidget = NULL;
  mLayout = NULL;

}

void VtkWidget::initializeMainWindow()
{
	mVtkImageData = NULL; // RGB for 2D images (including stack)
	mVtkPolyData = NULL; // for 3D surface polygon data

	mQVTKWidget = NULL;
	mLayout = NULL;

	setAttribute(Qt::WA_DeleteOnClose);

	int width = this->frameGeometry().width();
	int height = this->frameGeometry().height();

	this->setMinimumHeight(height/2);
	this->setMinimumWidth(width/2);
	
}

void VtkWidget::copyExistingPointers(VtkView *vcont)
{
	mVtkPolyData = vcont->getViewer(0)->mVtkPolyData;
	rendering3D();
}

OBJECTTYPE VtkWidget::render(QString filename)
{
	QFileInfo fi(filename);

	QString extension = fi.suffix().toLower();
	std::string fnstr = filename.toLocal8Bit().constData();

	this->setObjectName(filename);

	if (extension==tr("obj")) 
	{
		if(read3D(filename)) return MODEL3D;
		else	return EMPTY;
	}
	else if (extension==tr("jpg") || extension==tr("jpeg"))
	{
		if(read2D(filename)) return IMAGE2D;
		else	return EMPTY;
	}
}

bool VtkWidget::read3D(QString filename)
{
	vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();
	reader->SetFileName(filename.toStdString().c_str());
	reader->Update();
	mVtkPolyData = reader->GetOutput();
	if (mVtkPolyData == NULL)
		return false;
	rendering3D();
	return true;
}


void VtkWidget::rendering3D()
{
	if (mQVTKWidget != NULL)
		delete mQVTKWidget;

	mQVTKWidget = new QVTKWidget(this);

	if (mLayout!=NULL)
		delete mLayout;
	mLayout = new QVBoxLayout( this ); // size stretching trick
	mLayout->setMargin( 0 );
	mLayout->addWidget( mQVTKWidget, 1 );
	this->setLayout(mLayout);

	mMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mMapper->SetInputConnection(mVtkPolyData->GetProducerPort());
	mMapper->ScalarVisibilityOff();

	mActor = vtkSmartPointer<vtkActor>::New();
	mActor->SetMapper(mMapper);
	mActor->GetProperty()->SetInterpolationToFlat();

    vtkSmartPointer<vtkProperty> property = vtkSmartPointer<vtkProperty>::New();
    property = mActor->GetProperty();
    double ambient[3] = {0.2, 0.2, 0.2};
    property->SetAmbientColor(ambient); // for surface
    double diffuse[3] = {1., 1., 1.};
    property->SetDiffuseColor(diffuse); // for surface
    double specular[3] = {1., 1., 1.};
    property->SetSpecularColor(specular); // for surface

    mActor->SetTexture(NULL);

	mRenderer = vtkSmartPointer<vtkRenderer>::New();
	mActor->PickableOn();
	mRenderer->AddActor(mActor);// Add actor to renderer

	mQVTKWidget->GetRenderWindow()->AddRenderer(mRenderer);

	mRenderer->ResetCamera();

	mRenderer->ResetCameraClippingRange();

	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New(); // 3D image
	//vtkSmartPointer<vtkInteractorStyleTrackballActor> style = vtkSmartPointer<vtkInteractorStyleTrackballActor>::New(); // 3D image
	
	mInteractor = vtkSmartPointer<QVTKInteractor>::New();
	mInteractor->SetRenderWindow(mQVTKWidget->GetRenderWindow());
	mInteractor->GetInteractorStyle()->SetDefaultRenderer(mRenderer);
	mInteractor->SetInteractorStyle(style);
	mInteractor->Initialize();

	mQVTKWidget->show();
	mQVTKWidget->update();
}

bool VtkWidget::read2D(QString filename)
{
	mRenderer = vtkSmartPointer<vtkRenderer>::New();

	vtkSmartPointer<vtkJPEGReader> reader = vtkSmartPointer<vtkJPEGReader>::New();
	reader->SetFileName(filename.toStdString().c_str());
	reader->Update();
	mVtkImageData = reader->GetOutput();
	if (mVtkImageData==NULL)
		return false;
	rendering2D();
	return true;
}

void VtkWidget::rendering2D()
{
	if (mQVTKWidget != NULL)
    delete mQVTKWidget;

	mQVTKWidget = new QVTKWidget(this);

	if (mLayout!=NULL)
	delete mLayout;
	mLayout = new QVBoxLayout( this ); // size stretching trick
	mLayout->setMargin( 0 );
	mLayout->addWidget( mQVTKWidget, 1 );
	this->setLayout(mLayout);

	mActor2D = vtkSmartPointer<vtkImageActor>::New();
	mActor2D->GetMapper()->SetInput(mVtkImageData);

	mRenderer = vtkSmartPointer<vtkRenderer>::New();
	mRenderer->AddActor(mActor2D);// Add actor to renderer

	mQVTKWidget->GetRenderWindow()->AddRenderer(mRenderer);
	mRenderer->SetLayer(0);

	vtkCamera* camera = mRenderer->GetActiveCamera();
	
	// reset camer to maximize the screenshot in window
	double origin[3];
	double spacing[3];
	int extent[6];
	mVtkImageData->GetOrigin( origin );
	mVtkImageData->GetSpacing( spacing );
	mVtkImageData->GetExtent( extent );
	camera->ParallelProjectionOn();

	double xc = origin[0] + 0.5*(extent[0] + extent[1])*spacing[0];
	double yc = origin[1] + 0.5*(extent[2] + extent[3])*spacing[1];
	//double xd = (extent[1] - extent[0] + 1)*spacing[0];
	double yd = (extent[3] - extent[2] + 1)*spacing[1];
	double d = camera->GetDistance();
	camera->SetParallelScale(0.5*yd);
	camera->SetFocalPoint(xc,yc,0.0);
	camera->SetPosition(xc,yc,d);

	vtkSmartPointer<vtkInteractorStyleImage> style =  vtkSmartPointer<vtkInteractorStyleImage>::New();
	mInteractor = vtkSmartPointer<QVTKInteractor>::New();
	mInteractor->SetRenderWindow(mQVTKWidget->GetRenderWindow());
	mInteractor->GetInteractorStyle()->SetDefaultRenderer(mRenderer);
	mInteractor->SetInteractorStyle(style);

	mInteractor->Initialize();

	tracer = vtkSmartPointer<vtkImageTracerWidget>::New();
	tracer->GetLineProperty()->SetLineWidth(5);
	tracer->SetInteractor(mInteractor);
	tracer->SetViewProp(mActor2D);
	tracer->SetCaptureRadius(0.001);
	vtkSmartPointer<vtkCallbackCommand> callback = vtkSmartPointer<vtkCallbackCommand>::New();
	callback->SetCallback(VtkWidget::CallbackFunction);
	tracer->AddObserver(vtkCommand::EndInteractionEvent, callback);

	tracer->On();
	vwidget = this;

	mQVTKWidget->show(); // no Render() is required for 3D
	mQVTKWidget->update(); //MK: this is important!
}

VtkView * VtkWidget::mvc()
{
  QObject * curParent = this->parent();
  while(qobject_cast<VtkView *>(curParent) == 0)
  {
    curParent = curParent->parent();
  }
  return qobject_cast<VtkView *>(curParent);
}

void VtkWidget::screenshot(QString file)
{
    vtkSmartPointer<vtkRenderWindow> renderWindow = mRenderer->GetRenderWindow();

    vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
    windowToImageFilter->SetInput(renderWindow);

    windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel
    windowToImageFilter->Update();

    vtkSmartPointer<vtkJPEGWriter> writer = vtkSmartPointer<vtkJPEGWriter>::New();
	writer->SetFileName(file);
    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
    writer->Write();  
}

void VtkWidget::renderUpdate()
{
	mQVTKWidget->show(); 
	mQVTKWidget->update(); 
}