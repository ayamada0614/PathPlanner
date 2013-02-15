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

#include <QDebug>
#include <QList>
#include <QTableWidgetSelectionRange>

#include "qSlicerAbstractCoreModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerApplication.h"
#include "qSlicerCLIModule.h"

#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLCommandLineModuleNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"

#include "vtkSlicerAnnotationModuleLogic.h"
#include "qSlicerCoreApplication.h"
#include "vtkSlicerCLIModuleLogic.h"

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
  
  // Tables in "Entry Points" and "Target Points"
  //qSlicerRegistrationFiducialsTableModel* EntryPointsTableModel;
  //qSlicerRegistrationFiducialsTableModel* TargetPointsTableModel;
  
  // Pointer to Logic class of Annotations module to switch ActiveHierarchy node.
  vtkSlicerAnnotationModuleLogic* AnnotationsLogic;
  
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
  
  qSlicerAbstractCoreModule* annotationsModule = 
    qSlicerCoreApplication::application()->moduleManager()->module("Annotations");
  if (annotationsModule)
  {
    d->AnnotationsLogic =
    vtkSlicerAnnotationModuleLogic::SafeDownCast(annotationsModule->logic());
  }
  
  
}

//-----------------------------------------------------------------------------

void qSlicerPathPlannerPanelWidget
::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerPathPlannerPanelWidget);
  
  if (d->EntryPointsAnnotationNodeSelector)
  {
    d->EntryPointsAnnotationNodeSelector->setMRMLScene(newScene);
    // Listen for any new new fiducial points
    //this->qvtkReconnect(oldScene, newScene, vtkMRMLScene::NodeAddedEvent,
    //this, SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)));
  }
  if (d->TargetPointsAnnotationNodeSelector)
  {
    d->TargetPointsAnnotationNodeSelector->setMRMLScene(newScene);
  }
  /*
  if (d->EntryPointsTableModel)
  {
    d->EntryPointsTableModel->setMRMLScene(newScene);
  }
  if (d->TargetPointsTableModel)
  {
    d->TargetPointsTableModel->setMRMLScene(newScene);
  }
   */
  /*
  if (d->TrackerTransformNodeSelector)
  {
    d->TrackerTransformNodeSelector->setMRMLScene(newScene);
  }
  if(d->OutputTransformNodeSelector)
  {
    d->OutputTransformNodeSelector->setMRMLScene(newScene);
  }
  */
}


//-----------------------------------------------------------------------------
qSlicerPathPlannerPanelWidget
::~qSlicerPathPlannerPanelWidget()
{
}
