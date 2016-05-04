#ifndef InteractorStyleMoveGlyph_H
#define InteractorStyleMoveGlyph_H

#include "vtkWidget.h"
#include <vtkVertexGlyphFilter.h>

class InteractorStyleMoveGlyph : public vtkInteractorStyleTrackballActor
{
  public:
    //static InteractorStyleMoveGlyph* New();
    vtkTypeMacro(InteractorStyleMoveGlyph,vtkInteractorStyleTrackballActor);
 
    InteractorStyleMoveGlyph()
    {
		this->Move = false;
		this->MoveData = vtkSmartPointer<vtkPolyData>::New();
		this->Data = vtkSmartPointer<vtkPolyData>::New();
    }

	void renderMoveData()
	{
		
		vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
		vertexFilter->SetInputConnection(this->MoveData->GetProducerPort());
		vertexFilter->Update();

		

		// In selection
		//this->MoveData = vtkSmartPointer<vtkUnstructuredGrid>::New();

		this->MoveMapper = vtkSmartPointer<vtkDataSetMapper>::New();
		this->MoveMapper->SetInputConnection(vertexFilter->GetOutputPort());

		this->MoveActor = vtkSmartPointer<vtkActor>::New();
		this->MoveActor->SetMapper(this->MoveMapper);
		this->MoveActor->GetProperty()->SetColor(1,0,0);
	}
 
    void OnMouseMove()
    {
      if(!this->Move)	return;
 
      vtkInteractorStyleTrackballActor::OnMouseMove();
 
    }

	virtual void OnLeftButtonDown()
    {
      vtkInteractorStyleTrackballActor::OnLeftButtonDown();
    }

	virtual void OnLeftButtonUp()
    {
      vtkInteractorStyleTrackballActor::OnLeftButtonUp();
 
	}
 
    void OnMiddleButtonUp()
    {
      // Forward events
		vtkInteractorStyleTrackballActor::OnMiddleButtonUp();
		this->MoveActor->VisibilityOff();
		if (!this->Move)	
		{
			return;
		}
		
		this->Move = false;
		this->EndPan();
		
		double *end = new double[3];
		double displayPt[4];
		 int currPos[2];
		this->widget->mInteractor->GetEventPosition(currPos);
		end = this->MoveActor->GetPosition();
		double distance[3];
		distance[0] = end[0] - start[0];
		distance[1] = end[1] - start[1];
		distance[2] = end[2] - start[2];
		double pointPos[3];
		vtkSmartPointer<vtkPoints> ptOnObject = vtkSmartPointer<vtkPoints>::New();
		for (int i = 0; i < typeId.size(); i++)
		{
			this->Data->GetPoint(typeId[i], pointPos);
			pointPos[0] += distance[0];
			pointPos[1] += distance[1];
			pointPos[2] += distance[2];
			this->Data->GetPoints()->SetPoint(typeId[i], pointPos);
			MoveData->GetPoints()->SetPoint(i, pointPos);
			ptOnObject->InsertNextPoint(pointPos);
		}
		vtkSmartPointer<vtkPolyData> pointdata = vtkSmartPointer<vtkPolyData>::New();
		pointdata->SetPoints(ptOnObject);

		vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
		vertexFilter->SetInputConnection(pointdata->GetProducerPort());
		vertexFilter->Update();
		surfaceMapper->SetInputConnection(vertexFilter->GetOutputPort());

		MoveData->Modified();
		this->Data->Modified();
		this->GetCurrentRenderer()->Render();
		this->GetCurrentRenderer()->GetRenderWindow()->Render();
		widget->renderUpdate();
    }
    void OnMiddleButtonDown()
    {
		// Forward events

		vtkInteractorStyleTrackballActor::OnMiddleButtonDown();
		widget->renderUpdate();
		if(static_cast<vtkCellPicker*>(this->InteractionPicker)->GetPointId() >= 0)
		{
			int id = static_cast<vtkCellPicker*>(this->InteractionPicker)->GetPointId();

			if (std::find(typeId.begin(), typeId.end(), id) == typeId.end())	// the selected point is not on the selected mesh
			{
				this->Move = false;
				return;
			}
			this->StartPan();
			this->MoveActor->VisibilityOn();
			qDebug() << "Id: " << id;

			this->Move = true;
			this->Data->GetPoint(id, start);
			
			this->MoveActor->SetPosition(start);
		}

		//this->GetCurrentRenderer()->AddActor(this->MoveActor);
		this->InteractionProp = this->MoveActor;
 
    }

	double start[3];
	vtkSmartPointer<vtkDataSetMapper> MoveMapper;
	vtkSmartPointer<vtkDataSetMapper> surfaceMapper;
	vtkSmartPointer<vtkActor> MoveActor;
	vtkSmartPointer<vtkPolyData> Data;
	std::vector<int> typeId;
	//vtkSmartPointer<vtkIdTypeArray> ids;
	vtkSmartPointer<vtkPolyData> MoveData;
	VtkWidget* widget;

	bool Move;
};
//vtkStandardNewMacro(InteractorStyleMoveGlyph);
#endif 
