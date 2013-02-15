/*==============================================================================
 
 Program: Path Planner User Interface for 3D Slicer
 
 Copyright (c) Brigham and Women's Hospital
 
 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 
 This file was developed by Atsushi Yamada, Brigham and Women's
 Hospital. The project was supported by NIH P41EB015898.
 
 ==============================================================================*/

// Panel Widgets includes
#include "qSlicerPathPlannerPanelWidget.h"
#include "ui_qSlicerPathPlannerPanelWidget.h"

#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLCommandLineModuleNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PathPlanner
class qSlicerPathPlannerPanelWidgetPrivate
  : public Ui_qSlicerPathPlannerPanelWidget
{
  Q_DECLARE_PUBLIC(qSlicerPathPlannerPanelWidget);
protected:
  qSlicerPathPlannerPanelWidget* const q_ptr;

public:
  qSlicerPathPlannerPanelWidgetPrivate(
    qSlicerPathPlannerPanelWidget& object);
  virtual void setupUi(qSlicerPathPlannerPanelWidget*);
};

// --------------------------------------------------------------------------
qSlicerPathPlannerPanelWidgetPrivate
::qSlicerPathPlannerPanelWidgetPrivate(
  qSlicerPathPlannerPanelWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidgetPrivate
::setupUi(qSlicerPathPlannerPanelWidget* widget)
{
  this->Ui_qSlicerPathPlannerPanelWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPathPlannerPanelWidget methods

//-----------------------------------------------------------------------------
qSlicerPathPlannerPanelWidget
::qSlicerPathPlannerPanelWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerPathPlannerPanelWidgetPrivate(*this) )
{
  Q_D(qSlicerPathPlannerPanelWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
/*
void qSlicerPathPlannerPanelWidget
::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerPathPlannerPanelWidget);
  
  if (d->ImagePointsAnnotationNodeSelector)
  {
    d->ImagePointsAnnotationNodeSelector->setMRMLScene(newScene);
    // Listen for any new new fiducial points
    //this->qvtkReconnect(oldScene, newScene, vtkMRMLScene::NodeAddedEvent,
    //this, SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)));
  }
  if (d->PhysicalPointsAnnotationNodeSelector)
  {
    d->PhysicalPointsAnnotationNodeSelector->setMRMLScene(newScene);
  }
  if (d->ImagePointsTableModel)
  {
    d->ImagePointsTableModel->setMRMLScene(newScene);
  }
  if (d->PhysicalPointsTableModel)
  {
    d->PhysicalPointsTableModel->setMRMLScene(newScene);
  }
  if (d->TrackerTransformNodeSelector)
  {
    d->TrackerTransformNodeSelector->setMRMLScene(newScene);
  }
  if(d->OutputTransformNodeSelector)
  {
    d->OutputTransformNodeSelector->setMRMLScene(newScene);
  }
  
}
*/

//-----------------------------------------------------------------------------
qSlicerPathPlannerPanelWidget
::~qSlicerPathPlannerPanelWidget()
{
}
