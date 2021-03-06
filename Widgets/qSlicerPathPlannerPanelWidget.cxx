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
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLCommandLineModuleNode.h"

#include "vtkSlicerAnnotationModuleLogic.h"
#include "vtkSlicerCLIModuleLogic.h"

#include "qtableview.h"



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
  
  // test code
  // Linear transform node to import tacking data
  vtkMRMLLinearTransformNode* TrackerTransform;
  
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
  this->AnnotationsLogic = NULL;
  this->OriginalAnnotationID = "";  

  // test code
  this->TrackerTransform = NULL;

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
  
  // set model
  d->EntryPointsTable->setModel(d->EntryPointsTableModel);
  d->TargetPointsTable->setModel(d->TargetPointsTableModel);
  d->PathsTable->setModel(d->PathsTableModel);
  
  // test codes
  // set item selectors
  this->selectionEntryPointsTableModel = d->EntryPointsTable->selectionModel();
  this->selectionTargetPointsTableModel = d->TargetPointsTable->selectionModel();
  this->selectionPathsTableModel = d->PathsTable->selectionModel();
  
  // test code
  this->selectedPathIndexOfRow = 0;
  this->selectedPathIndexofColumn = 0;
  
  this->generatedPathColumnCounter = 0;
  
/*  
  // test codes
  // ------------------------------
  QModelIndex topLeft;
  QModelIndex bottomRight;
  
  topLeft = d->TargetPointsTableModel->index(0,0,QModelIndex());
  bottomRight = d->TargetPointsTableModel->index(0,5,QModelIndex());
  
  QItemSelection selection(topLeft, bottomRight);
  this->selectionTargetPointsTableModel->select(selection, QItemSelectionModel::Select);
  // ------------------------------
*/  
 
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
        node->Delete();
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
        node->Delete();
      }
    }
  }

  // test code
  // it works
  //if (d->testNodeSelector)
  //{
    //d->EntryPointsTableModel->setMRMLScene(d->testNodeSelector->mrmlScene());
    /*
    connect(d->testNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            d->EntryPointsTableModel, SLOT(setNode(vtkMRMLNode*)));
    connect(d->testNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            this, SLOT(setEntryPointsAnnotationNode(vtkMRMLNode*)));
     */
    /*
    if (scene)
    {
      d->testNodeSelector->setMRMLScene(scene);
      
      // Create a new hierarchy node
      vtkMRMLAnnotationHierarchyNode* node = d->createNewHierarchyNode("test");
      if (node)
      {
        d->testNodeSelector->setCurrentNode(node);
        node->Delete();
      }
      
    }
     */
  //}
  
  
  // test code for path selector
  if (d->PathsAnnotationNodeSelector)
  {
    d->PathsTableModel->setMRMLScene(d->PathsAnnotationNodeSelector->mrmlScene());
    connect(d->PathsAnnotationNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            d->PathsTableModel, SLOT(setNode(vtkMRMLNode*)));
    //connect(d->ImagePointsAnnotationNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    //        this, SLOT(setPhysicalPointsAnnotationNode(vtkMRMLNode*)));

    // test code
    connect(d->EntryPointsAnnotationNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            this, SLOT(setPathsAnnotationNode(vtkMRMLNode*)));
    connect(d->PathsAnnotationNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            this, SLOT(setPathsAnnotationNode(vtkMRMLNode*)));
    
    if (scene)
    {
      d->PathsAnnotationNodeSelector->setMRMLScene(scene);
      // Create a new hierarchy node
      vtkMRMLAnnotationHierarchyNode* node = d->createNewHierarchyNode("Path");
      if (node)
      {
        d->PathsAnnotationNodeSelector->setCurrentNode(node);
        node->Delete();
      }
    }
  }
  
  if (d->ListClear)
  {
    connect(d->ListClear, SIGNAL(clicked()),
            this, SLOT(deleteEntryPoints()));
    connect(d->ListClear, SIGNAL(clicked()),
            this, SLOT(deleteTargetPoints()));
    //connect(d->ListClear, SIGNAL(clicked()),
    //        this, SLOT(deletePaths()));
    
    //test
    //connect(d->ListClear, SIGNAL(clicked()),
    //        this, SLOT(addPaths()));
    
  }
  
  if (d->EntryPointListSelector && d->TargetPointListSelector)
  {
    connect(d->TargetPointListSelector, SIGNAL(released()),
            d->EntryPointListSelector, SLOT(toggle()));
    connect(d->EntryPointListSelector, SIGNAL(released()),
            d->TargetPointListSelector, SLOT(toggle()));
    connect(d->TargetPointListSelector, SIGNAL(stateChanged(int)),
            this, SLOT(switchCurrentAnotationNode(int)));    
  }

  
  // test code
  if (d->TargetPointsTable)
  {
    //connect(d->TargetPointsTable, SLOT(selectRow(int row)),
    //        this, SLOT(selectTargetPoint(int row)));    
    //connect(d->TargetPointsTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)),
    //        this, SLOT(selectTargetPoint(const QItemSelection &selected, const QItemSelection &deselected)));    
    connect(d->TargetPointsTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)),
            this, SLOT(selectTargetPoint(const QItemSelection, const QItemSelection)));    
    connect(d->TargetPointsTable->horizontalHeader(), SIGNAL(sectionClicked(int)),
            this, SLOT(selectTargetPointTable(int)));    
    //connect(d->TargetPointsTable, SIGNAL(selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)),
    //        this, SLOT(selectTargetPoint(const QItemSelection &selected, const QItemSelection &deselected)));    
  }


  // test code
  if (d->EntryPointsTable)
  {
    connect(d->EntryPointsTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)),
            this, SLOT(selectEntryPoint(const QItemSelection, const QItemSelection)));    
    connect(d->EntryPointsTable->horizontalHeader(), SIGNAL(sectionClicked(int)),
            this, SLOT(selectEntryPointTable(int)));    
  }
  
  
  // test code
  if (d->PathsTable)
  {
    connect(d->PathsTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)),
            this, SLOT(selectPathsTable(const QItemSelection, const QItemSelection)));    
    connect(d->PathsTable->horizontalHeader(), SIGNAL(sectionClicked(int)),
            this, SLOT(selectPathsTable(int)));    
  }

  
  // test code
  if (d->addPathButton)
  {
    connect(d->addPathButton, SIGNAL(clicked()),
            this, SLOT(addPathRow()));
    //connect(d->addPathButton, SIGNAL(clicked()),
    //        this, SLOT(addPathRow()));
    
    //connect(d->TargetPointListSelector, SIGNAL(stateChanged(int)),
    //        this, SLOT(toggleAction(int)));    
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

  // test code
  if (d->TrackerTransformNodeSelector)
  {
    connect(d->TrackerTransformNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
            this, SLOT(setTrackerTransform(vtkMRMLNode*)));
  }

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
  
  // initialization for toggleSwitch
  this->switchCurrentAnotationNode(2);
  
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
  
  if (d->PathsAnnotationNodeSelector)
  {
    d->PathsAnnotationNodeSelector->setMRMLScene(newScene);
  }  
 
  if (d->EntryPointsTableModel)
  {
    d->EntryPointsTableModel->setMRMLScene(newScene);
  }
  if (d->TargetPointsTableModel)
  {
    d->TargetPointsTableModel->setMRMLScene(newScene);
  }
  if (d->PathsTableModel)
  {
    d->PathsTableModel->setMRMLScene(newScene);
  }  

  
  // test code
  if (d->TrackerTransformNodeSelector)
  {
    d->TrackerTransformNodeSelector->setMRMLScene(newScene);
  }
  
  /*
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


// test code
//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::setPathsAnnotationNode(vtkMRMLNode* node)
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

/*
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
*/

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
  if (/*test code*/ /*d->TrackerTransform &&*/ d->TargetPointsTableModel && d->AnnotationsLogic)
  {
    // Check the current active AnnotationHierarchy node.
    // If it is different from the one specified in "Fiducials" menu,
    // we switch the active node before adding a fiducial node. Once the node
    // is added, we switch back to the original active node.
    
    //vtkMRMLAnnotationHierarchyNode* original = d->AnnotationsLogic->GetActiveHierarchyNodeID();
    //vtkMRMLAnnotationHierarchyNode* current  = NULL;
    std::string original = d->AnnotationsLogic->GetActiveHierarchyNodeID();
    std::string current = "";
    
    // test code
    std::cout << "GetActiveHierarchyNodeID = " << original << std::endl;
    
    if (original.compare("") != 0)
    {
      current = original;
      vtkMRMLAnnotationHierarchyNode* hnode;
      hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast
      (d->PathsAnnotationNodeSelector->currentNode());
      if (hnode && original.compare(hnode->GetID()) != 0)
      {
        // test code
        std::cout << "current = hnode->GetID()" << std::endl;
        current = hnode->GetID();
      }
      // test code
      std::cout << "original.compare("") != 0" << std::endl;
      
    }
    else
    {
      vtkMRMLAnnotationHierarchyNode* hnode;
      hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast
      (d->PathsAnnotationNodeSelector->currentNode());
      if (hnode)
      {
        current = hnode->GetID();
      }
      // test code
      std::cout << "else of original.compare("") != 0" << std::endl;
      
    }
    if (current.compare("") != 0)
    {
      // test code
      std::cout << "current.compare("") != 0" << std::endl;

      // Switch the active hierarchy node
      d->AnnotationsLogic->SetActiveHierarchyNodeID(current.c_str());
      
      // Add a new fiducial node to the active hierarchy
      
      // test code
      vtkSmartPointer< vtkMatrix4x4 > matrix = vtkSmartPointer< vtkMatrix4x4 >::New();
      //d->TrackerTransform->GetMatrixTransformToWorld(matrix);
      //d->TargetPointsTableModel->addPoint(matrix->Element[0][3],
      //                                      matrix->Element[1][3],
      //                                      matrix->Element[2][3]);
      //d->PathsTableModel->addPoint(1.0,2.0,3.0);
      
      // test code 3/27/2013
      //d->PathsTableModel->addRuler();
      
      // Switch the active hierarchy node to the original
      d->AnnotationsLogic->SetActiveHierarchyNodeID(original.c_str());
    }
  }
  
  // test code
  std::cout << "addTargetPoint() is finished. " << std::endl;
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
::switchCurrentAnotationNode(int index)
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


// test code
//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::addPathRow()
{
  Q_D(qSlicerPathPlannerPanelWidget);

  std::cout << "clicked addPathRowButton" << std::endl;
  
  //d->PathsTableModel->targetPointName[d->PathsTableModel->pathColumnCounter] = (char*)malloc(sizeof(char) * 50);
  //d->PathsTableModel->targetPointName[d->PathsTableModel->pathColumnCounter] = "Set Target Point";
  //d->PathsTableModel->pathColumnCounter++;

  // initialize variables
  for(int i = 0; i < 4; i++)
  {
    //d->EntryPointsTableModel->selectedEntryPoint[this->generatedPathColumnCounter][i] = 0.0;
    //d->TargetPointsTableModel->selectedTargetPoint[this->generatedPathColumnCounter][i] = 0.0;
    this->selectedEntryPoint[this->generatedPathColumnCounter][i] = 0.0;
    this->selectedTargetPoint[this->generatedPathColumnCounter][i] = 0.0;
  }
  
  this->generatedPathColumnCounter++;

  //d->qSlicerPathPlannerTableModel->updateTable();
  //d->PathsTableModel->onMRMLChildNodeAdded();
  
  // make pathsAnnotationNode currentNode
  
  vtkMRMLAnnotationHierarchyNode* hnode;
  hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(d->PathsAnnotationNodeSelector->currentNode());
  if (hnode)
  {
    d->AnnotationsLogic->SetActiveHierarchyNodeID(hnode->GetID());
  }
  
  //
  //d->PathsTableModel->pathTableExistance = 1;
  //d->TargetPointsTableModel->pathTableExistance = 1;
  //d->EntryPointsTableModel->pathTableExistance = 1;
  
  // test 
  //d->PathsTableModel->addPoint(1.5,1.5,1.5);
  //d->PathsTableModel->updateTable();

  // test code 3/27/2013
  //this->addTargetPoint();
  d->PathsTableModel->addRuler();
  
  //d->PathsTableModel->updateTable();
  
}


// test code
//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::setTrackerTransform(vtkMRMLNode* o)
{
  Q_D(qSlicerPathPlannerPanelWidget);
  
  vtkMRMLLinearTransformNode* trans = vtkMRMLLinearTransformNode::SafeDownCast(o);
  if (trans)
  {
    qvtkReconnect(d->TrackerTransform, trans,
                  vtkMRMLTransformableNode::TransformModifiedEvent,
                  this, SLOT(onTrackerTransformModified()));
    d->TrackerTransform = trans;
  }
}

// test code
//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::onTrackerTransformModified()
{
  Q_D(qSlicerPathPlannerPanelWidget);
  
  vtkMatrix4x4* matrix = d->TrackerTransform->GetMatrixTransformToParent();
  QString buf;
  //d->PositionXEdit->setText(buf.setNum(matrix->Element[0][3]));
  //d->PositionYEdit->setText(buf.setNum(matrix->Element[1][3]));
  //d->PositionZEdit->setText(buf.setNum(matrix->Element[2][3]));
  std::cout << "matrix->Element[0][3] = " << matrix->Element[0][3] << std::endl;
  std::cout << "matrix->Element[1][3] = " << matrix->Element[1][3] << std::endl;
  std::cout << "matrix->Element[2][3] = " << matrix->Element[2][3] << std::endl;
  
}


// test code
//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::selectTargetPoint(const QItemSelection &selected, const QItemSelection &deselected)
{
  Q_D(qSlicerPathPlannerPanelWidget);
      
  QModelIndexList indexes = this->selectionTargetPointsTableModel->selectedIndexes();
  QModelIndex index;
  
  QStringList stringList;
  QString string;
    
  // selected row and column identification
  if(d->PathsTableModel->selectedPathsTableColumn != RESET)
  {
    foreach(index, indexes)
    {
      // test code: update the target point name
      std::cout << "Picked up target!!" << std::endl;

      d->PathsTableModel->selectedTargetPointItemRow = index.row();
      d->PathsTableModel->selectedTargetPointItemColumn = index.column();
      std::cout << "index.row() for target point = " << index.row() << std::endl;      
      std::cout << "index.column() for target point = " << index.column() << std::endl;      
            
      d->TargetPointsTableModel->identifyTipOfPath(index.row(), index.column());
      std::cout << "selected target name = " << d->TargetPointsTableModel->selectedName << std::endl;            
      
      // test code for renewing path
      this->targetRow[this->selectedPathIndexOfRow] = index.row();
      this->targetColumn[this->selectedPathIndexOfRow] = index.column();
      
      // update target point name
      d->PathsTableModel->targetPointName[this->selectedPathIndexOfRow] = d->TargetPointsTableModel->selectedName; 
      
      // update target point coordinate;
      //d->TargetPointsTableModel->selectedTargetPoint[this->selectedPathIndexOfRow][0] = d->TargetPointsTableModel->selectedCoordinate[index.row()][0];
      //d->TargetPointsTableModel->selectedTargetPoint[this->selectedPathIndexOfRow][1] = d->TargetPointsTableModel->selectedCoordinate[index.row()][1];
      //d->TargetPointsTableModel->selectedTargetPoint[this->selectedPathIndexOfRow][2] = d->TargetPointsTableModel->selectedCoordinate[index.row()][2];
      this->selectedTargetPoint[this->selectedPathIndexOfRow][0] = d->TargetPointsTableModel->selectedCoordinate[index.row()][0];
      this->selectedTargetPoint[this->selectedPathIndexOfRow][1] = d->TargetPointsTableModel->selectedCoordinate[index.row()][1];
      this->selectedTargetPoint[this->selectedPathIndexOfRow][2] = d->TargetPointsTableModel->selectedCoordinate[index.row()][2];
      
      // calculate distance
      d->PathsTableModel->calculatePath(this->selectedEntryPoint[this->selectedPathIndexOfRow], this->selectedTargetPoint[this->selectedPathIndexOfRow],this->selectedPathIndexOfRow);

      /*
      // update distance
      double difference[3];
      difference[0] = this->selectedTargetPoint[this->selectedPathIndexOfRow][0] - this->selectedEntryPoint[this->selectedPathIndexOfRow][0];
      difference[0] = this->selectedTargetPoint[this->selectedPathIndexOfRow][1] - this->selectedEntryPoint[this->selectedPathIndexOfRow][1];
      difference[0] = this->selectedTargetPoint[this->selectedPathIndexOfRow][2] - this->selectedEntryPoint[this->selectedPathIndexOfRow][2];
      
      fid->SetDistanceMeasurement(sqrt(difference[0]*difference[0]+difference[1]*difference[1]+difference[2]*difference[2]));
      */
      
      
      
      //d->PathsTableModel->updateTable();
      d->PathsTableModel->updateRulerTable();
    
    }
  }
  
   /*
   // reset focus
   this->selectionTargetPointsTableModel
   ->select(selected, QItemSelectionModel::Deselect | QItemSelectionModel::Columns);
   
   // reset entry point
   QModelIndex topLeft;
   QModelIndex bottomRight;
   
   topLeft = d->EntryPointsTableModel->index(0,0,QModelIndex());
   bottomRight = d->EntryPointsTableModel->index(0,5,QModelIndex());
   
   QItemSelection selection(topLeft, bottomRight);
   this->selectionEntryPointsTableModel->select(selection, QItemSelectionModel::Deselect | QItemSelectionModel::Columns);
   */
  
  
  
}


// test code
//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::selectEntryPoint(const QItemSelection &selected, const QItemSelection &deselected)
{
  Q_D(qSlicerPathPlannerPanelWidget);
  
  QModelIndexList indexes = this->selectionEntryPointsTableModel->selectedIndexes();
  QModelIndex index;
  
  // selected row and column identification
  if(d->PathsTableModel->selectedPathsTableColumn != RESET)
  {
    foreach(index, indexes)
    {
      // test code: update the entry point name
      std::cout << "Picked up entry!!" << std::endl;      
      
      d->PathsTableModel->selectedEntryPointItemRow = index.row();
      d->PathsTableModel->selectedEntryPointItemColumn = index.column();
      std::cout << "index.row() for entry point = " << index.row() << std::endl;      
      std::cout << "index.column() for entry point = " << index.column() << std::endl;      
      
      // identify the selected item name
      d->EntryPointsTableModel->identifyTipOfPath(index.row(), index.column());
      std::cout << "selected entry name = " << d->EntryPointsTableModel->selectedName << std::endl;  
      
      // test code for renewing path
      this->entryRow[this->selectedPathIndexOfRow] = index.row();
      this->entryColumn[this->selectedPathIndexOfRow] = index.column();
      
      d->PathsTableModel->entryPointName[this->selectedPathIndexOfRow] = d->EntryPointsTableModel->selectedName; 

      // update entry point coordinate;
      //d->EntryPointsTableModel->selectedEntryPoint[this->selectedPathIndexOfRow][0] = d->EntryPointsTableModel->selectedCoordinate[index.row()][0];
      //d->EntryPointsTableModel->selectedEntryPoint[this->selectedPathIndexOfRow][1] = d->EntryPointsTableModel->selectedCoordinate[index.row()][1];
      //d->EntryPointsTableModel->selectedEntryPoint[this->selectedPathIndexOfRow][2] = d->EntryPointsTableModel->selectedCoordinate[index.row()][2];
      this->selectedEntryPoint[this->selectedPathIndexOfRow][0] = d->EntryPointsTableModel->selectedCoordinate[index.row()][0];
      this->selectedEntryPoint[this->selectedPathIndexOfRow][1] = d->EntryPointsTableModel->selectedCoordinate[index.row()][1];
      this->selectedEntryPoint[this->selectedPathIndexOfRow][2] = d->EntryPointsTableModel->selectedCoordinate[index.row()][2];
      
      // calculate distance
      d->PathsTableModel->calculatePath(this->selectedEntryPoint[this->selectedPathIndexOfRow], this->selectedTargetPoint[this->selectedPathIndexOfRow],this->selectedPathIndexOfRow);
      
      //d->PathsTableModel->updateTable();
      d->PathsTableModel->updateRulerTable();
      
    }
  }
}


// test code
//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::selectPathsTable(const QItemSelection &selected, const QItemSelection &deselected)
{
  Q_D(qSlicerPathPlannerPanelWidget);
  
  QModelIndexList indexes = this->selectionPathsTableModel->selectedIndexes();
  QModelIndex index;
  
  // selected row and column identification
  foreach(index, indexes)
  {
    
    d->PathsTableModel->selectedPathsTableRow = index.row();
    d->PathsTableModel->selectedPathsTableColumn = index.column();
    // if you execute the under line, the path table will be disappeared.
    //d->PathsTableModel->updateTable();
    
    // test code: selected path table
    this->selectedPathIndexOfRow = index.row();
    this->selectedPathIndexofColumn = index.column();
  }
  
  d->PathsTableModel->selectedTargetPointItemRow = RESET;
  d->PathsTableModel->selectedTargetPointItemColumn = RESET;
  d->PathsTableModel->selectedEntryPointItemRow = RESET;
  d->PathsTableModel->selectedEntryPointItemColumn = RESET;

  // calculate distance
  //d->TargetPointsTableModel->identifyTipOfPath(this->targetRow[this->selectedPathIndexOfRow], this->targetColumn[this->selectedPathIndexOfRow]);
  //d->EntryPointsTableModel->identifyTipOfPath(this->entryRow[this->selectedPathIndexOfRow], this->entryColumn[this->selectedPathIndexOfRow]);
  // calculate distance
  //d->PathsTableModel->calculatePath(d->EntryPointsTableModel->selectedCoordinate[this->entryRow[this->selectedPathIndexOfRow]], d->TargetPointsTableModel->selectedCoordinate[this->targetRow[this->selectedPathIndexOfRow]],this->selectedPathIndexOfRow);


  // reset entry point
  QModelIndex topLeft;
  QModelIndex bottomRight;
  
  topLeft = d->EntryPointsTableModel->index(0,0,QModelIndex());
  bottomRight = d->EntryPointsTableModel->index(0,5,QModelIndex());
  
  QItemSelection entrySelectionReset(topLeft, bottomRight);
  this->selectionEntryPointsTableModel->select(entrySelectionReset, QItemSelectionModel::Deselect | QItemSelectionModel::Columns);
  
  topLeft = d->TargetPointsTableModel->index(0,0,QModelIndex());
  bottomRight = d->TargetPointsTableModel->index(0,5,QModelIndex());
  
  QItemSelection targetSelectionReset(topLeft, bottomRight);
  this->selectionTargetPointsTableModel->select(targetSelectionReset, QItemSelectionModel::Deselect | QItemSelectionModel::Columns);
  
}


//-----------------------------------------------------------------------------
void qSlicerPathPlannerPanelWidget
::selectTargetPointTable(int i)
{
  Q_D(qSlicerPathPlannerPanelWidget);
  
  std::cout << "selectTargetPointTableRow =  " << i << std::endl;
  
}


//-----------------------------------------------------------------------------
qSlicerPathPlannerPanelWidget
::~qSlicerPathPlannerPanelWidget()
{
}
