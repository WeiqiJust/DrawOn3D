#include "mainWindow.h"
#include <QDesktopWidget>
#include <QMdiSubWindow>
#include <QDir>

#include <vtkCellLocator.h>
#include <vtkOBJExporter.h>


MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	mdiArea = new QMdiArea;
	mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setCentralWidget(mdiArea);

	createActions(); 
	menuBar = new QMenuBar(0);
	this->setMenuBar(menuBar);
	createMenus();
	showMaximized();

	windowMapper = new QSignalMapper(this);
	connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));
	connect(windowMapper, SIGNAL(mapped(QWidget*)),this, SLOT(wrapSetActiveSubWindow(QWidget *)));

	setAcceptDrops(true);
	mdiArea->setAcceptDrops(true);

	QDesktopWidget qdw;
	setMaximumSize(qdw.size());
	setWindowFlags(Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint | Qt::CustomizeWindowHint);
	moveStyle = NULL;
	updateMenu();
}

MainWindow::~MainWindow()
{

}

void MainWindow::selectOnOnject(vtkSmartPointer<vtkPolyData> path)
{
	qDebug() << "There are " << path->GetNumberOfPoints() << " points in the path." ;
	vtkSmartPointer<vtkPoints> pt = vtkSmartPointer<vtkPoints>::New();
	double point[3];
	double displayPt[4];
	
	// get points on the drawing path
	for (int i = 0; i < path->GetNumberOfPoints(); i++)
	{
		path->GetPoint(i, point);
		vtkInteractorObserver::ComputeDisplayToWorld(VTKAObject()->mRenderer, point[0], point[1], point[2], displayPt);
		pt->InsertPoint(i, displayPt[0], displayPt[1], displayPt[2]);
		//qDebug()<<"path point"<<displayPt[0]<<displayPt[1]<<displayPt[2];
	}

	// create 2D convex hull for the points on the path
	vtkSmartPointer<vtkPolyData> pointdata = vtkSmartPointer<vtkPolyData>::New();
	pointdata->SetPoints(pt);

	vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	vertexFilter->SetInputConnection(pointdata->GetProducerPort());
	vertexFilter->Update();

	vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
	cleaner->SetInputConnection (vertexFilter->GetOutputPort());

	vtkSmartPointer<vtkDelaunay2D> delaunay = vtkSmartPointer<vtkDelaunay2D>::New();
	delaunay->SetInputConnection (cleaner->GetOutputPort());
	delaunay->Update();

	vtkSmartPointer<vtkPolyData> selected = vtkSmartPointer<vtkPolyData>::New();
	selected->ShallowCopy(delaunay->GetOutput());

	/*
	vtkSmartPointer<vtkDataSetMapper> mapper1 = vtkSmartPointer<vtkDataSetMapper>::New();
	mapper1->SetInput(selected);
	vtkSmartPointer<vtkActor> actor1 = vtkSmartPointer<vtkActor>::New();
	actor1->SetMapper(mapper1);
	actor1->GetProperty()->SetColor(1,0,0);
	actor1->GetProperty()->SetLineWidth(10);
	actor1->GetProperty()->SetPointSize(5);
	VTKAObject()->mRenderer->AddActor(actor1);*/

	// do ray tracing to project the convex hull on to the object
	vtkSmartPointer<vtkCellLocator> cellLocator = vtkSmartPointer<vtkCellLocator>::New();
	cellLocator->SetDataSet(selected);
	cellLocator->BuildLocator();
	double intersectPoints[3], pcoord[3], t, projection[3];
	int sub, result;
	vtkSmartPointer<vtkPoints> ptOnObject = vtkSmartPointer<vtkPoints>::New();

	vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
	ids->SetNumberOfComponents(1);
	typeIds.clear();
	double* direction = new double[3];
	double* camera = new double[3];
	direction = VTKAObject()->mRenderer->GetActiveCamera()->GetDirectionOfProjection();
	camera = VTKAObject()->mRenderer->GetActiveCamera()->GetPosition();
	qDebug()<<"get here"<<camera[0]<<camera[1]<<camera[2];
	for (int i = 0; i < VTKAObject()->mVtkPolyData->GetNumberOfPoints(); i++)
	{
		VTKAObject()->mVtkPolyData->GetPoint(i, point);	// reuse double point[3] here
		if (!isVisible(point, VTKAObject()->mRenderer))	// select only visible point in current camera position
			continue;
		projection[0] = point[0] - direction[0]*10000;
		projection[1] = point[1] - direction[1]*10000;
		projection[2] = point[2] - direction[2]*10000;
		result = cellLocator->IntersectWithLine(point, projection, 0.0001, t, intersectPoints, pcoord, sub);
		//qDebug()<<"result = "<<result<<point[0]<<point[1]<<point[2];
		if (result != 0)
		{
			ptOnObject->InsertNextPoint(point[0], point[1], point[2]);
			typeIds.push_back(i);
			ids->InsertNextValue(i);
		}
	}

	pointdata = vtkSmartPointer<vtkPolyData>::New();
	pointdata->SetPoints(ptOnObject);

	
	vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	vertexFilter->SetInputConnection(pointdata->GetProducerPort());
	vertexFilter->Update();
/*
	vtkSmartPointer<vtkSelectionNode> selectionNode = vtkSmartPointer<vtkSelectionNode>::New();
	selectionNode->SetFieldType(vtkSelectionNode::POINT);
	selectionNode->SetContentType(vtkSelectionNode::INDICES);
	selectionNode->SetSelectionList(ids);
	selectionNode->GetProperties()->Set(vtkSelectionNode::CONTAINING_CELLS(), 1);

	vtkSmartPointer<vtkSelection> selection =
	vtkSmartPointer<vtkSelection>::New();
	selection->AddNode(selectionNode);

	vtkSmartPointer<vtkExtractSelection> extractSelection =
	vtkSmartPointer<vtkExtractSelection>::New();

	extractSelection->SetInput(0, VTKAObject()->mVtkPolyData);

	extractSelection->SetInput(1, selection);

	extractSelection->Update();
	vtkSmartPointer<vtkUnstructuredGrid> selecteddata = vtkSmartPointer<vtkUnstructuredGrid>::New();
	selecteddata->ShallowCopy(extractSelection->GetOutput());*/


	vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
	mapper->SetInputConnection(vertexFilter->GetOutputPort());
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(1,0,0);
	actor->GetProperty()->SetLineWidth(10);
	actor->GetProperty()->SetPointSize(5);
	VTKAObject()->mRenderer->AddActor(actor);

	//VTKAObject()->mInteractor->Start();	

	moveStyle = new InteractorStyleMoveGlyph;
	
	VTKAObject()->mInteractor->SetInteractorStyle( moveStyle );
	moveStyle->Data = VTKAObject()->mVtkPolyData;
	//moveStyle->ids = ids;
	moveStyle->MoveData = pointdata;
	moveStyle->surfaceMapper = mapper;
	moveStyle->widget = VTKAObject();

	moveStyle->typeId = typeIds;
	
	moveStyle->renderMoveData();
	
	mdiArea->setActiveSubWindow(mdiArea->subWindowList()[0]);
	updateMenu();
}

bool MainWindow::isVisible(double *point, vtkSmartPointer<vtkRenderer> render)
{
	vtkSmartPointer<vtkPoints> vtkPoint = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkPolyData> pointData = vtkSmartPointer<vtkPolyData>::New();
	vtkPoint->SetNumberOfPoints(1);
	vtkPoint->SetPoint(0, point[0], point[1], point[2]);
	pointData->SetPoints(vtkPoint);
	vtkSmartPointer<vtkSelectVisiblePoints> selectVisiblePoints = vtkSmartPointer<vtkSelectVisiblePoints>::New();
	selectVisiblePoints->SetInput(pointData);
	selectVisiblePoints->SetRenderer(render);
	selectVisiblePoints->Update();
	if (selectVisiblePoints->GetOutput()->GetNumberOfPoints() != 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void MainWindow::updateMenu()
{
	int size = mdiArea->subWindowList().size();
	switch(size)
	{
	case 0:
		openAct->setEnabled(true);
		drawAct->setEnabled(false);
		selectAct->setEnabled(false);
		rotateAct->setEnabled(false);
		exportAct->setEnabled(false);
		break;
	case 1:
		openAct->setEnabled(true);
		drawAct->setEnabled(true);
		selectAct->setEnabled(false);
		rotateAct->setEnabled(false);
		exportAct->setEnabled(true);
		break;
	case 2:
		openAct->setEnabled(true);
		drawAct->setEnabled(true);
		selectAct->setEnabled(true);
		rotateAct->setEnabled(true);
		exportAct->setEnabled(true);
		break;
	default:
		openAct->setEnabled(true);
		drawAct->setEnabled(false);
		selectAct->setEnabled(false);
		rotateAct->setEnabled(false);
		exportAct->setEnabled(false);
		break;
	}
}

void MainWindow::createActions()
{
	openAct = new QAction(tr("&Open"), this);
	openAct->setStatusTip(tr("Open a 3D model"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

	drawAct = new QAction(tr("&Draw"), this);
	drawAct->setStatusTip(tr("Draw on 2D image"));
	connect(drawAct, SIGNAL(triggered()), this, SLOT(draw()));

	selectAct = new QAction(tr("&Select"), this);
	selectAct->setStatusTip(tr("Select and move the points"));
	connect(selectAct, SIGNAL(triggered()), this, SLOT(select()));

	rotateAct = new QAction(tr("&Rotate"), this);
	rotateAct->setStatusTip(tr("Rotate the 3D model"));
	connect(rotateAct, SIGNAL(triggered()), this, SLOT(rotate()));

	exportAct = new QAction(tr("&Export"), this);
	exportAct->setStatusTip(tr("Export the 3D model"));
	connect(exportAct, SIGNAL(triggered()), this, SLOT(exportModel())); 
}

void MainWindow::createMenus()
{
	menuBar->addAction(openAct);
	menuBar->addAction(drawAct);
	menuBar->addAction(selectAct);
	menuBar->addAction(rotateAct);
	menuBar->addAction(exportAct);
}


void MainWindow::open(QString fileName)
{
	QStringList filters;
	filters.push_back("*.obj");
	filters.push_back("*.jpg");
	QString file;
	if (fileName.isEmpty())
	{
		if (mdiArea->subWindowList().size() > 0)
		{
			mdiArea->closeAllSubWindows();
		}
		file = QFileDialog::getOpenFileName(this,tr("Open a file"), QDir::currentPath(), filters.join(";;"));
	}
	else 
	{
		file = fileName;
	}

	if (!file.isEmpty())
	{
		file = QDir::toNativeSeparators(file);
		QFileInfo fi(file);
		if (!fi.isFile())
		{
			QMessageBox::critical(this, tr("Opening Object Error"), "Unable to open " + fi.fileName() + ": It is not a File!");
			return;
		}
		QStringList nameElement = file.split(QDir::separator());
		QString fileNameElement = nameElement[nameElement.size() - 1];
		QString path = file;
		path.truncate(path.lastIndexOf(QDir::separator())); // PC

		newImage();
		qDebug()<<file;
		OBJECTTYPE open = VTKA()->render(file);
		int windows = VTKA()->getId();
		if(open == EMPTY)
		{
			if(mdiArea->currentSubWindow())
			{
				mdiArea->closeActiveSubWindow();
			}

			if(fi.exists()) 
			{
				QMessageBox::critical(this, tr("Opening Error"), "Unable to open " + fi.fileName() + ". ");
			} 
			else 
			{
				QMessageBox::critical(this, tr("Opening Error"), "Unable to open " + fi.fileName() + ": File not found. ");
			}
			updateAllViews();
			updateMenu();
			return;
		}
		else if (open == MODEL3D)
		{
		}

		currentVtkView()->prjName = fi.fileName();

		currentVtkView()->setWindowTitle(currentVtkView()->prjName);
		updateAllViews();
		if (mdiArea->subWindowList().size() == 1)
			updateMenu();
	}
}

void MainWindow::draw()
{
	if (mdiArea->subWindowList().size() == 2)
		mdiArea->removeSubWindow(mdiArea->subWindowList()[1]);
	VTKA()->renderUpdate();
	QString file = QDir::currentPath();
	file.append(QDir::separator() + QString("screenshot.jpg"));
	//QSize size = mdiArea->subWindowList()[0]->frameSize();
	VTKA()->mvc()->showNormal();
	//VTKA()->mvc()->setMaximumSize(size);
	VTKA()->screenshot(file);
	VTKA()->mvc()->showMaximized();
	open(file);
	if (mdiArea->subWindowList().size() != 2)
	{
		QMessageBox::critical(this, tr("Opening Error"), "The screenshot is not correctly generated/loaded!");
		return;
	}
	connect(VTKAImage(), SIGNAL(selectedSurface(vtkSmartPointer<vtkPolyData>)), this, SLOT(selectOnOnject(vtkSmartPointer<vtkPolyData>)));
}

void MainWindow::select()
{
	if (moveStyle == NULL)
		return;
	moveStyle->renderMoveData();
	VTKAObject()->mInteractor->SetInteractorStyle( moveStyle );
	
}

void MainWindow::rotate()
{
	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	VTKAObject()->mInteractor->SetInteractorStyle( style );
}

void MainWindow::exportModel()
{
	if (VTKAObject() == 0)
		return;
	QString file = QFileDialog::getSaveFileName((QWidget* )0, "Export Object", QString(), "");
	vtkSmartPointer<vtkOBJExporter> writer = vtkSmartPointer<vtkOBJExporter>::New();
	writer->SetRenderWindow(VTKAObject()->mRenderer->GetRenderWindow());
	writer->SetFilePrefix(file.toStdString().c_str());
	writer->Write();
}

void MainWindow::setActiveSubWindow(QWidget *window)
{
	if (!window)
		return;
	mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void MainWindow::wrapSetActiveSubWindow(QWidget* window)
{
	QMdiSubWindow* subwindow;
	subwindow = dynamic_cast<QMdiSubWindow*>(window);
	if(subwindow!= NULL)
	{
		mdiArea->setActiveSubWindow(subwindow);
	}
	else
	{
		qDebug("Type of window is not a QMdiSubWindow*");
	}
}

QMdiSubWindow* MainWindow::newImage()
{
    VtkView *mvcont = new VtkView(mdiArea);
    QMdiSubWindow* subWindow = mdiArea->addSubWindow(mvcont);
	subWindow->installEventFilter(this);
    VtkWidget *gla = new VtkWidget(mvcont);
	vtkSmartPointer<vtkRenderer> render;
    mvcont->addView(gla, Qt::Horizontal);// this fuction may destroy the renderer on mdiArea
    connect(mvcont,SIGNAL(updateMainWindowMenus()),this,SLOT(updateAllViews()));
    gla->mvc()->showMaximized(); // make the mdi window a full screen view
    updateAllViews();
    return subWindow;
}

void MainWindow::setHandleMenu(QPoint point, Qt::Orientation orientation, QSplitter *origin)
{
    VtkView *mvc =  currentVtkView();
    int epsilon =10;
    splitMenu->clear();
    unSplitMenu->clear();
    //the viewer to split/unsplit is chosen through picking

    //Vertical handle allows to split horizontally
    if(orientation == Qt::Vertical)
    {
      splitUpAct->setData(point);
      splitDownAct->setData(point);

      //check if the viewer on the top is splittable according to its size
      int pickingId = mvc->getViewerByPicking(QPoint(point.x(), point.y()-epsilon));
      if(pickingId>=0)
        splitUpAct->setEnabled(mvc->getViewer(pickingId)->size().width()/2 > mvc->getViewer(pickingId)->minimumSizeHint().width());

      //the viewer on top can be closed only if the splitter over the handle that orginated the event has one child
      bool unSplittabilityUp = true;
      VtkSplitter * upSplitter = qobject_cast<VtkSplitter *>(origin->widget(0));
      if(upSplitter)
        unSplittabilityUp = !(upSplitter->count()>1);
      unsplitUpAct->setEnabled(unSplittabilityUp);

      //check if the viewer below is splittable according to its size
      pickingId = mvc->getViewerByPicking(QPoint(point.x(), point.y()+epsilon));
      if(pickingId>=0)
        splitDownAct->setEnabled(mvc->getViewer(pickingId)->size().width()/2 > mvc->getViewer(pickingId)->minimumSizeHint().width());

      //the viewer below can be closed only if the splitter ounder the handle that orginated the event has one child
      bool unSplittabilityDown = true;
      VtkSplitter * downSplitter = qobject_cast<VtkSplitter *>(origin->widget(1));
      if(downSplitter)
        unSplittabilityDown = !(downSplitter->count()>1);
      unsplitDownAct->setEnabled(unSplittabilityDown);

      splitMenu->addAction(splitUpAct);
      splitMenu->addAction(splitDownAct);

      unsplitUpAct->setData(point);
      unsplitDownAct->setData(point);

      unSplitMenu->addAction(unsplitUpAct);
      unSplitMenu->addAction(unsplitDownAct);
    }
    //Horizontal handle allows to split vertically
    else if (orientation == Qt::Horizontal)
    {
      splitRightAct->setData(point);
      splitLeftAct->setData(point);

      //check if the viewer on the right is splittable according to its size
      int pickingId =mvc->getViewerByPicking(QPoint(point.x()+epsilon, point.y()));
      if(pickingId>=0)
        splitRightAct->setEnabled(mvc->getViewer(pickingId)->size().height()/2 > mvc->getViewer(pickingId)->minimumSizeHint().height());

      //the viewer on the rigth can be closed only if the splitter on the right the handle that orginated the event has one child
      bool unSplittabilityRight = true;
      VtkSplitter * rightSplitter = qobject_cast<VtkSplitter *>(origin->widget(1));
      if(rightSplitter)
        unSplittabilityRight = !(rightSplitter->count()>1);
      unsplitRightAct->setEnabled(unSplittabilityRight);

      //check if the viewer on the left is splittable according to its size
      pickingId =mvc->getViewerByPicking(QPoint(point.x()-epsilon, point.y()));
      if(pickingId>=0)
        splitLeftAct->setEnabled(mvc->getViewer(pickingId)->size().height()/2 > mvc->getViewer(pickingId)->minimumSizeHint().height());

      //the viewer on the left can be closed only if the splitter on the left of the handle that orginated the event has one child
      bool unSplittabilityLeft = true;
      VtkSplitter * leftSplitter = qobject_cast<VtkSplitter *>(origin->widget(0));
      if(leftSplitter)
        unSplittabilityLeft = !(leftSplitter->count()>1);
      unsplitLeftAct->setEnabled(unSplittabilityLeft);

      splitMenu->addAction(splitRightAct);
      splitMenu->addAction(splitLeftAct);

      unsplitRightAct->setData(point);
      unsplitLeftAct->setData(point);

      unSplitMenu->addAction(unsplitRightAct);
      unSplitMenu->addAction(unsplitLeftAct);
    }

    handleMenu->popup(point);
}

void MainWindow::updateAllViews()
{
	VtkView *mvc = currentVtkView();
	if(mvc)
	{
		mvc->updateAllViewer();
	}
	if(VTKA())
	{
	  VTKA()->update();
	} 

}
