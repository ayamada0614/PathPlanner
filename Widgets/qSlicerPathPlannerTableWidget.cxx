/*==============================================================================

  Program: Point-based Registration User Interface for 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Junichi Tokuda, Brigham and Women's
  Hospital. The project was supported by NIH P41EB015898.

==============================================================================*/

// PathPlannerPanel Widgets includes
#include "qSlicerPathPlannerTableWidget.h"
#include "ui_qSlicerPathPlannerTableWidget.h"

// VTK includes
#include "vtkNew.h"
#include "vtkSmartPointer.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLSelectionNode.h"

#include "vtkSlicerAnnotationModuleLogic.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"

#include "qSlicerPathPlannerFiducialItem.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PathPlanner
class Q_SLICER_MODULE_PATHPLANNER_WIDGETS_EXPORT qSlicerPathPlannerTableWidgetPrivate
  : public Ui_qSlicerPathPlannerTableWidget
{
  Q_DECLARE_PUBLIC(qSlicerPathPlannerTableWidget);
 protected:
  qSlicerPathPlannerTableWidget * const q_ptr;
  vtkMRMLAnnotationHierarchyNode* selectedHierarchyNode;
  vtkSlicerAnnotationModuleLogic* annotationLogic;

 public:
  qSlicerPathPlannerTableWidgetPrivate(
    qSlicerPathPlannerTableWidget& object);
  virtual void setupUi(qSlicerPathPlannerTableWidget*);
};

//-----------------------------------------------------------------------------
qSlicerPathPlannerTableWidgetPrivate
::qSlicerPathPlannerTableWidgetPrivate(
  qSlicerPathPlannerTableWidget& object)
  : q_ptr(&object)
{
  this->selectedHierarchyNode = NULL;
  this->annotationLogic = NULL;
}

//-----------------------------------------------------------------------------
void qSlicerPathPlannerTableWidgetPrivate
::setupUi(qSlicerPathPlannerTableWidget* widget)
{
  this->Ui_qSlicerPathPlannerTableWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
qSlicerPathPlannerTableWidget
::qSlicerPathPlannerTableWidget(QWidget *parentWidget)
  : Superclass (parentWidget)
    , d_ptr( new qSlicerPathPlannerTableWidgetPrivate(*this) )
{
  Q_D(qSlicerPathPlannerTableWidget);
  d->setupUi(this);  

  qSlicerAbstractCoreModule* annotationModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Annotations");
  if (annotationModule)
    {
    d->annotationLogic = 
      vtkSlicerAnnotationModuleLogic::SafeDownCast(annotationModule->logic());
    }

  connect(d->AddButton, SIGNAL(clicked()),
	  this, SLOT(onAddButtonClicked()));

  connect(d->DeleteButton, SIGNAL(clicked()),
	  this, SLOT(onDeleteButtonClicked()));

  connect(d->ClearButton, SIGNAL(clicked()),
	  this, SLOT(onClearButtonClicked()));
}

//-----------------------------------------------------------------------------
qSlicerPathPlannerTableWidget
::~qSlicerPathPlannerTableWidget()
{
}

//-----------------------------------------------------------------------------
QTableWidget* qSlicerPathPlannerTableWidget
::getTableWidget()
{
  Q_D(qSlicerPathPlannerTableWidget);
  if (d->TableWidget)
    {
    return d->TableWidget;
    }
  return NULL;
}

//-----------------------------------------------------------------------------
void qSlicerPathPlannerTableWidget
::setSelectedHierarchyNode(vtkMRMLAnnotationHierarchyNode* selectedNode)
{
  Q_D(qSlicerPathPlannerTableWidget);

  if (!selectedNode)
    {
    return;
    }

  d->selectedHierarchyNode = selectedNode;
}

//-----------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode* qSlicerPathPlannerTableWidget
::selectedHierarchyNode()
{
  Q_D(qSlicerPathPlannerTableWidget);

  return d->selectedHierarchyNode;
}

//-----------------------------------------------------------------------------
void qSlicerPathPlannerTableWidget
::onAddButtonClicked()
{
  Q_D(qSlicerPathPlannerTableWidget);

  if (!d->annotationLogic | !d->selectedHierarchyNode)
    {
    return;
    }

  // if active hierarchy node is different from selected one
  // set selected one as active
  if (d->annotationLogic->GetActiveHierarchyNode() != d->selectedHierarchyNode)
    {
    d->annotationLogic->SetActiveHierarchyNodeID(d->selectedHierarchyNode->GetID());
    }

  // Set fiducial as annotation to drop in selection node
  vtkMRMLApplicationLogic *mrmlAppLogic = d->annotationLogic->GetMRMLApplicationLogic();
  if (mrmlAppLogic)
    {
    vtkMRMLInteractionNode *inode = mrmlAppLogic->GetInteractionNode();
    if (inode)
      {
      vtkMRMLSelectionNode *snode = mrmlAppLogic->GetSelectionNode();
      if (snode)
	{
	snode->SetActiveAnnotationID ("vtkMRMLAnnotationFiducialNode");
	}
      }
    }

  // Place fiducial (non persistent)
  d->annotationLogic->StartPlaceMode();  
}

//-----------------------------------------------------------------------------
void qSlicerPathPlannerTableWidget
::onDeleteButtonClicked()
{
  Q_D(qSlicerPathPlannerTableWidget);

  int selectedRow = d->TableWidget->currentRow();
  if (selectedRow < 0)
    {
    return;
    }

  // Remove fiducial from scene
  qSlicerPathPlannerFiducialItem* itemToRemove =
    dynamic_cast<qSlicerPathPlannerFiducialItem*>(d->TableWidget->item(selectedRow, 0));

  if (itemToRemove)
    {
    d->annotationLogic->GetMRMLScene()->RemoveNode(itemToRemove->getFiducialNode());
    }

  // Remove row from widget
  d->TableWidget->removeRow(selectedRow);
}

//-----------------------------------------------------------------------------
void qSlicerPathPlannerTableWidget
::onClearButtonClicked()
{
  Q_D(qSlicerPathPlannerTableWidget);

  if (!d->selectedHierarchyNode)
    {
    return;
    }

  d->selectedHierarchyNode->RemoveAllChildrenNodes();
  d->TableWidget->clearContents();
  d->TableWidget->setRowCount(0);
}



