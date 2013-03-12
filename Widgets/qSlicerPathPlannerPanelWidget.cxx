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

#include "qSlicerPathPlannerTableModel.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerApplication.h"
#include "qSlicerCLIModule.h"

// SlicerLogic includes
#include <vtkSlicerApplicationLogic.h>
#include "qSlicerMouseModeToolBar.h"

#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include "vtkMatrix4x4.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLCommandLineModuleNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkSlicerAnnotationModuleLogic.h"
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
  vtkMRMLAnnotationHierarchyNode* createNewHierarchyNode(const char* basename);
  
  // Tables in "Entry Points" and "Target Points"
  qSlicerPathPlannerTableModel* EntryPointsTableModel;
  qSlicerPathPlannerTableModel* TargetPointsTableModel;
  qSlicerPathPlannerTableModel* PathsTableModel;
  
  // Pointer to Logic class of Annotations module to switch ActiveHierarchy node.
  vtkSlicerAnnotationModuleLogic* AnnotationsLogic;
  
  QString OriginalAnnotationID;  
};

// --------------------------------------------------------------------------
qSlicerPathPlannerPanelWidgetPrivate
::qSlicerPathPlannerPanelWidgetPrivate(
  qSlicerPathPlannerPanelWidget& object)
  : q_ptr(&object)
{
  this->EntryPointsTableModel = NULL;
  this->TargetPointsTableModel = NULL;
  this->PathsTableModel = NULL;
  //this->TrackerTransform = NULL;
  this->AnnotationsLogic = NULL;
  this->OriginalAnnotationID = "";  
}

// --------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidgetPrivate
::setupUi(qSlicerPathPlannerPanelWidget* widget)
{
  this->Ui_qSlicerPathPlannerPanelWidget::setupUi(widget);
}


//-----------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode* qSlicerPathPlannerPanelWidgetPrivate
::createNewHierarchyNode(const char* basename)
{
  // NOTE: this method has to be called after setting AnnotationLogic;
  vtkMRMLScene * scene = qSlicerCoreApplication::application()->mrmlScene();

  if (!this->AnnotationsLogic || !scene)
  {
  return NULL;
  }

  QString parentNodeID = this->AnnotationsLogic->GetTopLevelHierarchyNodeID();
  vtkSmartPointer<vtkMRMLAnnotationHierarchyNode> newNode
    = vtkSmartPointer<vtkMRMLAnnotationHierarchyNode>::New();
  newNode->HideFromEditorsOff();
  newNode->SetName(scene->GetUniqueNameByString(basename));
  newNode->SetParentNodeID(parentNodeID.toLatin1());
  scene->AddNode(newNode.GetPointer());
  this->AnnotationsLogic->AddDisplayNodeForHierarchyNode(newNode.GetPointer());
  return newNode.GetPointer();
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
  
  this->toggleSwitchFlag = SELECTENTRYPOINTLIST; 
  
  qSlicerAbstractCoreModule* annotationsModule = 
    qSlicerCoreApplication::application()->moduleManager()->module("Annotations");
  if (annotationsModule)
  {
    d->AnnotationsLogic =
    vtkSlicerAnnotationModuleLogic::SafeDownCast(annotationsModule->logic());
  }
  vtkMRMLScene * scene = qSlicerCoreApplication::application()->mrmlScene();

  // set list models
  d->EntryPointsTableModel  = new qSlicerPathPlannerTableModel(this);
  d->EntryPointsTableModel->initList(qSlicerPathPlannerTableModel::LABEL_RAS_ENTRY);

  d->TargetPointsTableModel = new qSlicerPathPlannerTableModel(this);
  d->TargetPointsTableModel->initList(qSlicerPathPlannerTableModel::LABEL_RAS_TARGET);

  d->PathsTableModel = new qSlicerPathPlannerTableModel(this);
  d->PathsTableModel->initList(qSlicerPathPlannerTableModel::LABEL_RAS_PATH);
  
  d->EntryPointsTableModel->setCoordinateLabel(qSlicerPathPlannerTableModel::LABEL_RAS_ENTRY);
  d->TargetPointsTableModel->setCoordinateLabel(qSlicerPathPlannerTableModel::LABEL_RAS_TARGET);
  d->PathsTableModel->setCoordinateLabel(qSlicerPathPlannerTableModel::LABEL_RAS_PATH);
  
  d->EntryPointsTable->setModel(d->EntryPointsTableModel);
  d->TargetPointsTable->setModel(d->TargetPointsTableModel);
  d->PathsTable->setModel(d->PathsTableModel);
 
  if (d->EntryPointsAnnotationNodeSelector)
  {
    d->EntryPointsTableModel->setMRMLScene(d->EntryPointsAnnotationNodeSelector->mrmlScene());
    connect(d->EntryPointsAnnotationNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            d->EntryPointsTableModel, SLOT(setNode(vtkMRMLNode*)));
    connect(d->EntryPointsAnnotationNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            this, SLOT(setEntryPointsAnnotationNode(vtkMRMLNode*)));
    if (scene)
    {
      d->EntryPointsAnnotationNodeSelector->setMRMLScene(scene);
      // Create a new hierarchy node
      vtkMRMLAnnotationHierarchyNode* node = d->createNewHierarchyNode("EntryPoint");
      if (node)
      {
        d->EntryPointsAnnotationNodeSelector->setCurrentNode(node);
      }
    }
  }
  if (d->TargetPointsAnnotationNodeSelector)
  {
    d->TargetPointsTableModel->setMRMLScene(d->TargetPointsAnnotationNodeSelector->mrmlScene());
    connect(d->TargetPointsAnnotationNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            d->TargetPointsTableModel, SLOT(setNode(vtkMRMLNode*)));
    connect(d->EntryPointsAnnotationNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            this, SLOT(setTargetPointsAnnotationNode(vtkMRMLNode*)));
    if (scene)
    {
      d->TargetPointsAnnotationNodeSelector->setMRMLScene(scene);
      // Create a new hierarchy node
      vtkMRMLAnnotationHierarchyNode* node = d->createNewHierarchyNode("TargetPoint");
      if (node)
      {
        d->TargetPointsAnnotationNodeSelector->setCurrentNode(node);
      }
    }
  }
  
  if (d->ListClear)
  {
    connect(d->ListClear, SIGNAL(clicked()),
            this, SLOT(deleteEntryPoints()));
    connect(d->ListClear, SIGNAL(clicked()),
            this, SLOT(deleteTargetPoints()));
  }
  
  if (d->EntryPointListSelector && d->TargetPointListSelector)
  {
    connect(d->TargetPointListSelector, SIGNAL(released()),
            d->EntryPointListSelector, SLOT(toggle()));
    connect(d->EntryPointListSelector, SIGNAL(released()),
            d->TargetPointListSelector, SLOT(toggle()));
    connect(d->TargetPointListSelector, SIGNAL(stateChanged(int)),
            this, SLOT(toggleAction(int)));    
  }
  /*
  if (d->DeleteTargetPointsButton)
  {
    connect(d->DeleteTargetPointsButton, SIGNAL(clicked()),
            this, SLOT(deleteTargetPoints()));
  }
   */
  
/*
  if (d->AddTargetPointButton)
  {
    connect(d->AddTargetPointButton, SIGNAL(clicked()),
            this, SLOT(addTargetPoint()));
  }
*/ 
/*
  if (d->TrackerTransformNodeSelector)
  {
    connect(d->TrackerTransformNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            this, SLOT(setTrackerTransform(vtkMRMLNode*)));
  }
*/
/*
  if(d->AddEntryPointButton)
  {
    connect(d->AddEntryPointButton, SIGNAL(clicked()),
          this, SLOT(addEntryPointButtonClicked()));
    
    connect(d->AddEntryPointButton, SIGNAL(clicked()),
            d->AddEntryPointToolBar, SLOT(switchPlaceMode()));
    //connect(d->AddEntryPointButton, SIGNAL(clicked()),
    //        d->AddEntryPointToolBar, SLOT(switchToViewTransformMode()));
  }
*/   
  
  if (d->AddPointToolBar)
  {
    d->AddPointToolBar->setApplicationLogic(
                                             qSlicerApplication::application()->applicationLogic());
    d->AddPointToolBar->setMRMLScene(qSlicerApplication::application()->mrmlScene());
    QObject::connect(qSlicerApplication::application(),
                     SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                     d->AddPointToolBar,
                     SLOT(setMRMLScene(vtkMRMLScene*)));
    // change current annotationhierarchy node
    //QObject::connect(d->AddPointToolBar,SIGNAL(actionTriggered(QAction*)),
    //                 this,SLOT(entryPointToolBarClicked(QAction*)));
  }
  /*
  if (d->AddTargetPointToolBar)
  {
    d->AddTargetPointToolBar->setApplicationLogic(
                                                 qSlicerApplication::application()->applicationLogic());
    d->AddTargetPointToolBar->setMRMLScene(qSlicerApplication::application()->mrmlScene());
    QObject::connect(qSlicerApplication::application(),
                     SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                     d->AddTargetPointToolBar,
                     SLOT(setMRMLScene(vtkMRMLScene*)));
    // change current annotationhierarchy node
    //QObject::connect(d->PointsTabWidget,SIGNAL(currentChanged(int)),
    //                 this,SLOT(onTabSwitched(int)));
  }
  */
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
  
  if (d->EntryPointsTableModel)
  {
    d->EntryPointsTableModel->setMRMLScene(newScene);
  }
  if (d->TargetPointsTableModel)
  {
    d->TargetPointsTableModel->setMRMLScene(newScene);
  }
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
void qSlicerPathPlannerPanelWidget
::enter()
{
  Q_D(qSlicerPathPlannerPanelWidget);
  /*
  if (d->PointsTabWidget)
  {
    int i = d->PointsTabWidget->currentIndex();
    onTabSwitched(i);
  }
  */
}


//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::setTargetPointsAnnotationNode(vtkMRMLNode* node)
{
  Q_D(qSlicerPathPlannerPanelWidget);
  
  vtkMRMLAnnotationHierarchyNode* hnode;
  hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);
  if (hnode)
  {
    d->AnnotationsLogic->SetActiveHierarchyNodeID(hnode->GetID());
  }
}


//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::setEntryPointsAnnotationNode(vtkMRMLNode* node)
{
  Q_D(qSlicerPathPlannerPanelWidget);
  
  vtkMRMLAnnotationHierarchyNode* hnode;
  hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);
  if (hnode)
  {
    d->AnnotationsLogic->SetActiveHierarchyNodeID(hnode->GetID());
  }
}


//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::deleteEntryPoints()
{
  Q_D(qSlicerPathPlannerPanelWidget);

  if(this->toggleSwitchFlag == SELECTENTRYPOINTLIST)
  {
    if (d->EntryPointsAnnotationNodeSelector)
    {
      vtkMRMLAnnotationHierarchyNode* hnode;
      hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(d->EntryPointsAnnotationNodeSelector->currentNode());
      if (hnode)
      {
        hnode->RemoveChildrenNodes();
        hnode->InvokeEvent(vtkMRMLAnnotationHierarchyNode::HierarchyModifiedEvent);
      }
    }
  }
}


//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::deleteTargetPoints()
{
  Q_D(qSlicerPathPlannerPanelWidget);
 
  if(this->toggleSwitchFlag == SELECTTARGETPOINTLIST)
  {
    if (d->TargetPointsAnnotationNodeSelector)
    {
      vtkMRMLAnnotationHierarchyNode* hnode;
      hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(d->TargetPointsAnnotationNodeSelector->currentNode());
      if (hnode)
      { 
        hnode->RemoveChildrenNodes();
        hnode->InvokeEvent(vtkMRMLAnnotationHierarchyNode::HierarchyModifiedEvent);
      }
    }
  }
}


//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::addTargetPoint()
{
  Q_D(qSlicerPathPlannerPanelWidget);
  if (/*d->TrackerTransform && */d->TargetPointsTableModel && d->AnnotationsLogic)
  {
    // Check the current active AnnotationHierarchy node.
    // If it is different from the one specified in "Fiducials" menu,
    // we switch the active node before adding a fiducial node. Once the node
    // is added, we switch back to the original active node.
    
    //vtkMRMLAnnotationHierarchyNode* original = d->AnnotationsLogic->GetActiveHierarchyNodeID();
    //vtkMRMLAnnotationHierarchyNode* current  = NULL;
    std::string original = d->AnnotationsLogic->GetActiveHierarchyNodeID();
    std::string current = "";
    
    if (original.compare("") != 0)
    {
      current = original;
      vtkMRMLAnnotationHierarchyNode* hnode;
      hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast
      (d->TargetPointsAnnotationNodeSelector->currentNode());
      if (hnode && original.compare(hnode->GetID()) != 0)
      {
        current = hnode->GetID();
      }
    }
    else
    {
      vtkMRMLAnnotationHierarchyNode* hnode;
      hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast
      (d->TargetPointsAnnotationNodeSelector->currentNode());
      if (hnode)
      {
        current = hnode->GetID();
      }
    }
    if (current.compare("") != 0)
    {
      // Switch the active hierarchy node
      d->AnnotationsLogic->SetActiveHierarchyNodeID(current.c_str());
      
      // Add a new fiducial node to the active hierarchy
      vtkSmartPointer< vtkMatrix4x4 > matrix = vtkSmartPointer< vtkMatrix4x4 >::New();
      //d->TrackerTransform->GetMatrixTransformToWorld(matrix);
      d->TargetPointsTableModel->addPoint(matrix->Element[0][3],
                                            matrix->Element[1][3],
                                            matrix->Element[2][3]);
      
      // Switch the active hierarchy node to the original
      d->AnnotationsLogic->SetActiveHierarchyNodeID(original.c_str());
    }
  }
}


//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::entryPointToolBarClicked(QAction*)
{
  //std::cout << "entryPointToolBarClicked()" << std::endl;
}



//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::addEntryPointButtonClicked()
{
  Q_D(qSlicerPathPlannerPanelWidget);  
  
  vtkMRMLAnnotationHierarchyNode* hnode = NULL;
  hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast
  (d->EntryPointsAnnotationNodeSelector->currentNode());
  if (hnode)
  {
    d->AnnotationsLogic->SetActiveHierarchyNodeID(hnode->GetID());
  }  
}



//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::toggleAction(int index)
{
  Q_D(qSlicerPathPlannerPanelWidget);
  
  vtkMRMLAnnotationHierarchyNode* hnode = NULL;
  
  switch(index)
  {
    case 0: // Target point node
    {
      this->toggleSwitchFlag = SELECTTARGETPOINTLIST;
      
      hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast
      (d->TargetPointsAnnotationNodeSelector->currentNode());
      break;
    }
    case 2: // Entry point node
    {
      this->toggleSwitchFlag = SELECTENTRYPOINTLIST;
      
      hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast
      (d->EntryPointsAnnotationNodeSelector->currentNode());
      break;
    }
    default:
      break;
  }
  if (hnode)
  {
    d->AnnotationsLogic->SetActiveHierarchyNodeID(hnode->GetID());
  }
}

//-----------------------------------------------------------------------------
qSlicerPathPlannerPanelWidget
::~qSlicerPathPlannerPanelWidget()
{
}
