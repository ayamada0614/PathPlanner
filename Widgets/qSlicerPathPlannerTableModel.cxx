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
#include "qSlicerPathPlannerTableModel.h"

#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationRulerNode.h"


#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLScene.h"

#include "vtkNew.h"
#include "vtkSmartPointer.h"
#include "vtkCollection.h"

#include <map>
#include <sstream>


class Q_SLICER_MODULE_PATHPLANNER_WIDGETS_EXPORT qSlicerPathPlannerTableModelPrivate
{
  Q_DECLARE_PUBLIC(qSlicerPathPlannerTableModel);
 protected:
  qSlicerPathPlannerTableModel * const q_ptr;

 public:
  qSlicerPathPlannerTableModelPrivate(
    qSlicerPathPlannerTableModel& object);
  virtual ~qSlicerPathPlannerTableModelPrivate();

  void init();
  void initForEntryList();
  void initForTargetList();
  void initForPathList();
  vtkMRMLAnnotationHierarchyNode* HierarchyNode;
  int PendingItemModified; // -1 means not updating
  vtkMRMLScene* Scene;
  int Counter;
  
};

qSlicerPathPlannerTableModelPrivate
::qSlicerPathPlannerTableModelPrivate(
  qSlicerPathPlannerTableModel& object)
  : q_ptr(&object)
{
  this->HierarchyNode = NULL;
  this->PendingItemModified = -1; // -1 means not updating
  this->Scene = NULL;
  this->Counter = 0;
}

qSlicerPathPlannerTableModelPrivate
::~qSlicerPathPlannerTableModelPrivate()
{
  //Q_D(qSlicerPathPlannerTableModel);
}


void qSlicerPathPlannerTableModelPrivate
::init()
{
  Q_Q(qSlicerPathPlannerTableModel);

  q->setColumnCount(6);
  q->setHorizontalHeaderLabels(QStringList()
                               << "Point Name"
                               << "R"
                               << "A"
                               << "S"
                               << "Time"
                               << "Memo");
  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onItemChanged(QStandardItem*)));

}

void qSlicerPathPlannerTableModelPrivate
::initForEntryList()
{
  Q_Q(qSlicerPathPlannerTableModel);
  
  q->setColumnCount(6);
  q->setHorizontalHeaderLabels(QStringList()
                               << "Entry Point"
                               << "R"
                               << "A"
                               << "S"
                               << "Time"
                               << "Memo");
  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onRulerItemChanged(QStandardItem*)));
  
}

void qSlicerPathPlannerTableModelPrivate
::initForTargetList()
{
  Q_Q(qSlicerPathPlannerTableModel);
  
  q->setColumnCount(6);
  q->setHorizontalHeaderLabels(QStringList()
                               << "Target Point"
                               << "R"
                               << "A"
                               << "S"
                               << "Time"
                               << "Memo");
  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onItemChanged(QStandardItem*)));
  
}

void qSlicerPathPlannerTableModelPrivate
::initForPathList()
{
  Q_Q(qSlicerPathPlannerTableModel);
  
  q->setColumnCount(6);
  q->setHorizontalHeaderLabels(QStringList()
                               << "Path"
                               << "Target"
                               << "Entry"
                               << "Length"
                               << "Time"
                               << "Memo");
  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onRulerItemChanged(QStandardItem*)));
  
}

qSlicerPathPlannerTableModel
::qSlicerPathPlannerTableModel(QObject *parent)
  : QStandardItemModel(parent)
  , d_ptr( new qSlicerPathPlannerTableModelPrivate(*this) )
{
  Q_D(qSlicerPathPlannerTableModel);
  d->init();
  
  // initialize
  this->addRowFlag = 0;
  this->nItemsPrevious = 1;
  
  this->selectedTargetPointItemRow = RESET;
  this->selectedTargetPointItemColumn = RESET;
  this->selectedEntryPointItemRow = RESET;
  this->selectedEntryPointItemColumn = RESET;
  this->selectedPathsTableRow = RESET;
  this->selectedPathsTableColumn = RESET;
  this->pathColumnCounter = 0;
  //this->pathDistance = 0.0;
}

qSlicerPathPlannerTableModel
::~qSlicerPathPlannerTableModel()
{
}

qSlicerPathPlannerTableModel
::qSlicerPathPlannerTableModel(qSlicerPathPlannerTableModelPrivate* pimpl, QObject *parent)
  : QStandardItemModel(parent)
  , d_ptr(pimpl)
{
  Q_D(qSlicerPathPlannerTableModel);
  d->init();
}

void qSlicerPathPlannerTableModel
::initList(int i)
{
  Q_D(qSlicerPathPlannerTableModel);
  
  switch (i)
  {
    case LABEL_RAS_ENTRY:
    {
      d->initForEntryList();  
      break;
    }
    case LABEL_RAS_TARGET:
    {
      d->initForTargetList();  
      break;
    }
    case LABEL_RAS_PATH:
    {
      d->initForPathList();  
      break;
    }
    default:
    {
      d->init();  
      break;
    }
  }
  
}


void qSlicerPathPlannerTableModel
::setNode(vtkMRMLNode* node)
{
  Q_D(qSlicerPathPlannerTableModel);

  if (node == NULL)
    {
    d->HierarchyNode = NULL;
    }
  
  vtkMRMLAnnotationHierarchyNode* hnode;
  hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);
  if (hnode)
  {
    qvtkReconnect(d->HierarchyNode, node,
                  vtkMRMLHierarchyNode::ChildNodeAddedEvent,
                  this, SLOT(onMRMLChildNodeAdded(vtkObject*)));
    qvtkReconnect(d->HierarchyNode, node,
                  vtkMRMLHierarchyNode::ChildNodeRemovedEvent,
                  this, SLOT(onMRMLChildNodeRemoved(vtkObject*)));
    // NOTE (10/13/2012): ChildNodeRemovedEvent works when a child node is moved
    // to another annotation hierarchy, but doesn't work when a child
    // node is removed. For this reason,in addition to ChildNodeRemovedEvent,
    // onMRMLNodeRemovedEvent() is connected to NodeRemovedEvent invoked by the scene.

    // Disconnect slots from old child nodes
    if (d->HierarchyNode)
    {
      vtkNew<vtkCollection> collection;
      d->HierarchyNode->GetDirectChildren(collection.GetPointer());
      int nItems = collection->GetNumberOfItems();
      collection->InitTraversal();
      for (int i = 0; i < nItems; i ++)
      {
        vtkMRMLAnnotationFiducialNode* fnode;
        fnode = vtkMRMLAnnotationFiducialNode::SafeDownCast(collection->GetNextItemAsObject());
        if (fnode)
        {
          qvtkDisconnect(fnode, vtkMRMLAnnotationFiducialNode::ValueModifiedEvent,
                         this, SLOT(onMRMLChildNodeValueModified(vtkObject*)));
          fnode->SetAttribute("RFTEvent", NULL);
        }
      }
    
      // test code
      for (int i = 0; i < nItems; i ++)
      {
        vtkMRMLAnnotationRulerNode* rnode;
        rnode = vtkMRMLAnnotationRulerNode::SafeDownCast(collection->GetNextItemAsObject());
        if (rnode)
        {
          qvtkDisconnect(rnode, vtkMRMLAnnotationRulerNode::ValueModifiedEvent,
                         this, SLOT(onMRMLChildNodeValueModified(vtkObject*)));
          rnode->SetAttribute("RFTEvent", NULL);
        }
      }
      
    }

    vtkNew<vtkCollection> collection;
    // Connect slots to handle chlid node event
    hnode->GetDirectChildren(collection.GetPointer());
    int nItems = collection->GetNumberOfItems();
    collection->InitTraversal();
    for (int i = 0; i < nItems; i ++)
    {
      vtkMRMLAnnotationFiducialNode* fnode;
      fnode = vtkMRMLAnnotationFiducialNode::SafeDownCast(collection->GetNextItemAsObject());
      if (fnode)
      {
        // Connect the fiducial node to onMRMLChildNodeValueModified().
        // An attribute "RFTEvent" is set "Yes" to mark that the fiducial node is connected. 
        qvtkConnect(fnode, vtkMRMLAnnotationFiducialNode::ValueModifiedEvent,
                    this, SLOT(onMRMLChildNodeValueModified(vtkObject*)));
        fnode->SetAttribute("RFTEvent", "Yes");
      }
    }

    // test code
    for (int i = 0; i < nItems; i ++)
    {
      vtkMRMLAnnotationRulerNode* rnode;
      rnode = vtkMRMLAnnotationRulerNode::SafeDownCast(collection->GetNextItemAsObject());
      if (rnode)
      {
        // Connect the fiducial node to onMRMLChildNodeValueModified().
        // An attribute "RFTEvent" is set "Yes" to mark that the fiducial node is connected. 
        qvtkConnect(rnode, vtkMRMLAnnotationRulerNode::ValueModifiedEvent,
                    this, SLOT(onMRMLChildNodeValueModified(vtkObject*)));
        rnode->SetAttribute("RFTEvent", "Yes");
      }
    }
    
    
    d->HierarchyNode = hnode;
  }

  this->updateTable();

}


//------------------------------------------------------------------------------
void qSlicerPathPlannerTableModel
::setCoordinateLabel(int m)
{
  Q_D(qSlicerPathPlannerTableModel);

  QStringList list;
  
  switch (m)
    {
    case LABEL_RAS:
      {
      list << "Point Name" << "R" << "A" << "S" << "Time" << "Memo";
      break;
      }
    case LABEL_RAS_ENTRY:
      {
      list << "Entry Point" << "R" << "A" << "S" << "Time" << "Memo";
      break;
      }
    case LABEL_RAS_TARGET:
      {
      list << "Target Point" << "R" << "A" << "S" << "Time" << "Memo";
      break;
      }
    case LABEL_RAS_PATH:
      {
        list << "Path" << "Target" << "Entry" << "Length" << "Time" << "Memo";
        break;
      }
    case LABEL_XYZ:
      {
      list << "Point Name" << "X" << "Y" << "Z" << "Time" << "Memo";
      break;
      }
    default:
      {
      return;
      break;
      }
    }
  this->setHorizontalHeaderLabels(list);
}


//------------------------------------------------------------------------------
void qSlicerPathPlannerTableModel
::updateTable()
{
  Q_D(qSlicerPathPlannerTableModel);

  
  // test code
  std::cout << "updatedTable" << std::endl;  

  if (d->HierarchyNode == 0)
    {
    this->setRowCount(0);

    return;
    }

  d->PendingItemModified = 0;

  //QObject::disconnect(this, SIGNAL(itemChanged(QStandardItem*)),
  //                    this, SLOT(onItemChanged(QStandardItem*)));

  // Count the number of child Fiducial nodes 
  vtkNew<vtkCollection> collection;
  d->HierarchyNode->GetDirectChildren(collection.GetPointer());
  int nItems = collection->GetNumberOfItems();
  int nFiducials = 0;
  
  // flag if time and memo should be refreshed in column 5 and 6
  if (nItems > this->nItemsPrevious)
  {
    this->addRowFlag = 1;
    this->nItemsPrevious = nItems;
  }
  else
  {
    this->addRowFlag = 0;    
    this->nItemsPrevious = nItems;
  }
  
  collection->InitTraversal();
  for (int i = 0; i < nItems; i ++)
    {
    vtkMRMLAnnotationFiducialNode* fnode;
    fnode = vtkMRMLAnnotationFiducialNode::SafeDownCast(collection->GetNextItemAsObject());
    if (fnode)
      {
      nFiducials ++;
      }
    }
  this->setRowCount(nFiducials);

  collection->InitTraversal();
  for (int i = 0; i < nItems; i ++)
    {
    vtkMRMLAnnotationFiducialNode* fnode;
    fnode = vtkMRMLAnnotationFiducialNode::SafeDownCast(collection->GetNextItemAsObject());
    if (fnode)
      {
      QStandardItem* item = this->invisibleRootItem()->child(i, 0);
      if (item == NULL)
        {
        item = new QStandardItem();
        this->invisibleRootItem()->setChild(i, 0, item);
        }
      item->setText(fnode->GetName());
      item->setData(fnode->GetID(),qSlicerPathPlannerTableModel::NodeIDRole);

      for (int j = 0; j < 5; j ++)
        {
        QStandardItem* item = this->invisibleRootItem()->child(i, j+1);
        if (item == NULL)
          {
          item = new QStandardItem();
          this->invisibleRootItem()->setChild(i, j+1, item);
          }
        QString str;
        str.setNum(fnode->GetFiducialCoordinates()[j]);
          if(j<3)
          {
            item->setText(str);
          }
          else if(j==3)
          {
            // time stamp for each row
            if(i == nItems-1 && this->addRowFlag == 1)
            {
              QString timeStamp = QTime::currentTime().toString();
              QByteArray byteArray(timeStamp.toAscii());
              const char *timeStampStr = byteArray.constData();
              item->setText(timeStampStr);                
            }
          }
          else
          {
            // memo column for each row
            if(i == nItems-1 && this->addRowFlag == 1)
            {
              item->setText("");                
            }            
          }
        }
      }
    }

  d->PendingItemModified = -1;

}


//-----------------------------------------------------------------------------
void qSlicerPathPlannerTableModel
::addPoint(double x, double y, double z)
{
  Q_D(qSlicerPathPlannerTableModel);

  if (d->Scene && d->HierarchyNode)
    {
    // Generate fiducial point name
    vtkNew<vtkCollection> collection;
    d->HierarchyNode->GetDirectChildren(collection.GetPointer());
    int nItems = collection->GetNumberOfItems();

    std::stringstream ss;
    ss << "Path_" << (nItems+1);

    //vtkSmartPointer< vtkMRMLAnnotationFiducialNode > fid = vtkSmartPointer< vtkMRMLAnnotationFiducialNode >::New();
    vtkSmartPointer< vtkMRMLAnnotationFiducialNode > fid = vtkSmartPointer< vtkMRMLAnnotationFiducialNode >::New();
    fid->SetName(ss.str().c_str());
    double coord[3] = {x, y, z};
    fid->SetFiducialCoordinates(x, y, z);
    d->Scene->AddNode(fid);
    fid->CreateAnnotationTextDisplayNode();
    fid->CreateAnnotationPointDisplayNode();
    fid->GetAnnotationPointDisplayNode()->SetGlyphScale(5);
    fid->GetAnnotationPointDisplayNode()->SetGlyphType(vtkMRMLAnnotationPointDisplayNode::Sphere3D);
    //this->updateTable();
    
    // test code  
    this->updateRulerTable();
    
    }
}


//-----------------------------------------------------------------------------
void qSlicerPathPlannerTableModel
::addRuler(void)
{
  Q_D(qSlicerPathPlannerTableModel);
  
  if (d->Scene && d->HierarchyNode)
  {
        
    // test code
    std::cout << "this->pathColumnCounter = " << this->pathColumnCounter << std::endl;
    this->targetPointName[this->pathColumnCounter] = "Set Target Point";
    this->entryPointName[this->pathColumnCounter] = "Set Entry Point";
    this->pathDistance[this->pathColumnCounter] = 0.0;
    
    this->pathColumnCounter++;
    
    // Generate fiducial point name
    vtkNew<vtkCollection> collection;
    d->HierarchyNode->GetDirectChildren(collection.GetPointer());
    int nItems = collection->GetNumberOfItems();
    
    std::stringstream ss;
    ss << "P_" << (nItems+1);
    
    vtkSmartPointer< vtkMRMLAnnotationRulerNode > fid = vtkSmartPointer< vtkMRMLAnnotationRulerNode >::New();
    
    double tipPosition[2][3];
    
    tipPosition[0][0] = 0.0;
    tipPosition[0][1] = 0.0;
    tipPosition[0][2] = 0.0;

    tipPosition[1][0] = 0.0;
    tipPosition[1][1] = 0.0;
    tipPosition[1][2] = 0.0;
    
    fid->SetPosition1(tipPosition[0]);
    fid->SetPosition2(tipPosition[1]);
    
    double difference[3];

    // calculate distance
    difference[0] = tipPosition[0][0] - tipPosition[1][0];
    difference[1] = tipPosition[0][1] - tipPosition[1][1];
    difference[2] = tipPosition[0][2] - tipPosition[1][2];    
    fid->SetDistanceMeasurement(sqrt(difference[0]*difference[0]+difference[1]*difference[1]+difference[2]*difference[2]));
    
    // test code
    // the ruler is locked.
    fid->SetLocked(!fid->GetLocked());

    fid->SetName(ss.str().c_str());
    d->Scene->AddNode(fid);
    fid->CreateAnnotationTextDisplayNode();
    //fid->CreateAnnotationPointDisplayNode();
    //fid->GetAnnotationPointDisplayNode()->SetGlyphScale(5);
    //fid->GetAnnotationPointDisplayNode()->SetGlyphType(vtkMRMLAnnotationPointDisplayNode::Sphere3D);    
    
    std::cout << "AddRuler!! " << std::endl;  
    
    /*
    fid->SetName(ss.str().c_str());
    double coord[3] = {x, y, z};
    fid->SetFiducialCoordinates(x, y, z);
    d->Scene->AddNode(fid);
    fid->CreateAnnotationTextDisplayNode();
    fid->CreateAnnotationPointDisplayNode();
    fid->GetAnnotationPointDisplayNode()->SetGlyphScale(5);
    fid->GetAnnotationPointDisplayNode()->SetGlyphType(vtkMRMLAnnotationPointDisplayNode::Sphere3D);
    */
    //this->updateTable();
    this->updateRulerTable();
    
  }
}


//------------------------------------------------------------------------------
void qSlicerPathPlannerTableModel
::updateRulerTable()
{
  
  Q_D(qSlicerPathPlannerTableModel);
  
  
  // test code
  std::cout << "updatedRulerTable" << std::endl;  

  
  if (d->HierarchyNode == 0)
  {
    this->setRowCount(0);
    // test code
    std::cout << "d->HierarchyNode == 0" << std::endl;
    
    return;
  }
  
  d->PendingItemModified = 0;
  
  //QObject::disconnect(this, SIGNAL(itemChanged(QStandardItem*)),
  //                    this, SLOT(onItemChanged(QStandardItem*)));
  
  // Count the number of child Fiducial nodes 
  vtkNew<vtkCollection> collection;
  d->HierarchyNode->GetDirectChildren(collection.GetPointer());
  int nItems = collection->GetNumberOfItems();
  int nFiducials = 0;
  
  // flag if time and memo should be refreshed in column 5 and 6
  if (nItems > this->nItemsPrevious)
  {
    this->addRowFlag = 1;
    this->nItemsPrevious = nItems;
  }
  else
  {
    this->addRowFlag = 0;    
    this->nItemsPrevious = nItems;
  }
  
  collection->InitTraversal();
  for (int i = 0; i < nItems; i ++)
  {
    //vtkMRMLAnnotationFiducialNode* fnode;
    //fnode = vtkMRMLAnnotationFiducialNode::SafeDownCast(collection->GetNextItemAsObject());
    vtkMRMLAnnotationRulerNode* fnode;
    fnode = vtkMRMLAnnotationRulerNode::SafeDownCast(collection->GetNextItemAsObject());
    if (fnode)
    {
      nFiducials ++;
    }
  }
  this->setRowCount(nFiducials);
  
  collection->InitTraversal();
  for (int i = 0; i < nItems; i ++)
  {
    //vtkMRMLAnnotationFiducialNode* fnode;
    //fnode = vtkMRMLAnnotationFiducialNode::SafeDownCast(collection->GetNextItemAsObject());
    vtkMRMLAnnotationRulerNode* fnode;
    fnode = vtkMRMLAnnotationRulerNode::SafeDownCast(collection->GetNextItemAsObject());
    if (fnode)
    {
      
      // test code
      //std::cout << "if(fnode)" << std::endl;
      
      
      QStandardItem* item = this->invisibleRootItem()->child(i, 0);
      if (item == NULL)
      {
        item = new QStandardItem();
        this->invisibleRootItem()->setChild(i, 0, item);
      }
      item->setText(fnode->GetName());
      item->setData(fnode->GetID(),qSlicerPathPlannerTableModel::NodeIDRole);
      
      // test code
      //item->getText(fnode->GetName());
      //std::cout << "fnode->GetName() = " << fnode->GetName() << std::endl;  
      //printf("fnode->GetName() = %s", fnode->GetName());  

      
      for (int j = 0; j < 5; j ++)
      {
        QStandardItem* item = this->invisibleRootItem()->child(i, j+1);
        if (item == NULL)
        {
          item = new QStandardItem();
          this->invisibleRootItem()->setChild(i, j+1, item);
        }
        QString str;
        /*
        str.setNum(fnode->GetFiducialCoordinates()[j]);
        if(j<3)
        {
          item->setText(str);
        }
        */
        
        // set tip position
        if(j==0)
        {
          QString string;
          string.append(this->targetPointName[i]);
          item->setText(string);
        }
        else if(j==1)
        {
          // test code
          QString string;
          string.append(this->entryPointName[i]);
          item->setText(string);          
        }
        else if(j==2) 
        { // get distance
          //str.setNum(fnode->GetDistanceMeasurement());          
          // test code
          std::cout << "SetDistanceMeasurement = " << this->pathDistance[i] << std::endl;
          str.setNum(this->pathDistance[i]);  
          fnode->SetDistanceMeasurement(this->pathDistance[i]);
          item->setText(str);
          
          // test code
          fnode->SetPosition1(this->selectedTargetPoint[i]);
          fnode->SetPosition2(this->selectedEntryPoint[i]);
          
        }
        else if(j==3)
        {
          // time stamp for each row
          if(i == nItems-1 && this->addRowFlag == 1)
          {
            QString timeStamp = QTime::currentTime().toString();
            QByteArray byteArray(timeStamp.toAscii());
            const char *timeStampStr = byteArray.constData();
            item->setText(timeStampStr);
          }
        }
        else
        {
          // memo column for each row
          if(i == nItems-1 && this->addRowFlag == 1)
          {
            item->setText("");                
          }            
        }
      }
    }
  }
  
  d->PendingItemModified = -1;
  
}


//-----------------------------------------------------------------------------
void qSlicerPathPlannerTableModel
::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerPathPlannerTableModel);

  qvtkReconnect(d->Scene, newScene,
                vtkMRMLScene::NodeRemovedEvent,
                this, SLOT(onMRMLNodeRemovedEvent(vtkObject*,vtkObject*)));
  d->Scene = newScene;
}



void qSlicerPathPlannerTableModel
::onItemChanged(QStandardItem * item)
{
  Q_D(qSlicerPathPlannerTableModel);

  // test code
  std::cout << "onItemChanged()" << std::endl;
  
  if (item == this->invisibleRootItem())
    {
    return;
    }
  if (d->PendingItemModified >= 0)
    {
    return;
    }

  // TODO:  item->parent()-> does not work here...
  QStandardItem* nameItem = this->invisibleRootItem()->child(item->row(), 0);
  if (nameItem)
    {
    QString id = nameItem->data(qSlicerPathPlannerTableModel::NodeIDRole).toString();

    // Find fiducial node from item
    vtkNew<vtkCollection> collection;
    d->HierarchyNode->GetDirectChildren(collection.GetPointer());
    int nItems = collection->GetNumberOfItems();
    int nFiducials = 0;
    collection->InitTraversal();

    for (int i = 0; i < nItems; i ++)
      {
      vtkMRMLAnnotationFiducialNode* fnode;
      fnode = vtkMRMLAnnotationFiducialNode::SafeDownCast(collection->GetNextItemAsObject());
      if (fnode)
        {
        if (id == fnode->GetID())
          {
          QString qstr = item->text();
          double coord[4];
          switch (item->column())
            {
            case 0:
              {
              const char* str = qstr.toAscii();
              fnode->SetName(str);
              break;
              }
            case 1:
              {
              fnode->GetFiducialCoordinates(coord);
              coord[0] = qstr.toDouble();
              fnode->SetFiducialCoordinates(coord);
              break;
              }              
            case 2:
              {
              fnode->GetFiducialCoordinates(coord);
              coord[1] = qstr.toDouble();
              fnode->SetFiducialCoordinates(coord);
              break;
              }              
            case 3:
              {
              fnode->GetFiducialCoordinates(coord);
              coord[2] = qstr.toDouble();
              fnode->SetFiducialCoordinates(coord);
              break;
              }
            }
          fnode->Modified();
          this->updateTable();
          }
        }
      }
    
    }

  /*
  // test code
  if(this->selectedPathsTableRow != RESET)
  {
    this->updateRulerTable();
  } 
  */
  // test
  //std::cout << "finish onItemChanged()" << std::endl;
  
}


// test code
void qSlicerPathPlannerTableModel
::onRulerItemChanged(QStandardItem * item)
{
  Q_D(qSlicerPathPlannerTableModel);
  
  // test code
  std::cout << "onRulerItemChanged()" << std::endl;  
  
  if (item == this->invisibleRootItem())
  {
    return;
  }
  if (d->PendingItemModified >= 0)
  {
    return;
  }
  
  // TODO:  item->parent()-> does not work here...
  QStandardItem* nameItem = this->invisibleRootItem()->child(item->row(), 0);
  if (nameItem)
  {
    QString id = nameItem->data(qSlicerPathPlannerTableModel::NodeIDRole).toString();
    
    // Find ruler node from item
    vtkNew<vtkCollection> collection;
    d->HierarchyNode->GetDirectChildren(collection.GetPointer());
    int nItems = collection->GetNumberOfItems();
    int nFiducials = 0;
    collection->InitTraversal();
    
    for (int i = 0; i < nItems; i ++)
    {
      vtkMRMLAnnotationRulerNode* rnode;
      rnode = vtkMRMLAnnotationRulerNode::SafeDownCast(collection->GetNextItemAsObject());
      if (rnode)
      {
        if (id == rnode->GetID())
        {
          QString qstr = item->text();
          double value;
          switch (item->column())
          {
            // path name
            case 0:
            {
              const char* str = qstr.toAscii();
              rnode->SetName(str);
              break;
            }
              
            // target point name
            case 1:
            {
              this->targetPointName[i] = qstr.toAscii();
              std::cout << "this->targetPointName[i] = "<< this->targetPointName[i] << std::endl;            
              break;
            }

            // entry point name
            case 2:
            {
              this->entryPointName[i] = qstr.toAscii();
              std::cout << "this->entryPointName[i] = "<< this->entryPointName[i] << std::endl;            
              break;
            }
            
            // distance  
            case 3:
            {
              //value = qstr.toDouble();
              this->pathDistance[i] = qstr.toDouble(); 
              std::cout << "this->pathDistance[i] = "<< this->pathDistance[i] << std::endl;            
              //rnode->SetDistanceMeasurement(value);
              rnode->SetDistanceMeasurement(this->pathDistance[i]);
              
              break;
            }
              /*
               case 2:
               {
               fnode->GetFiducialCoordinates(coord);
               coord[1] = qstr.toDouble();
               fnode->SetFiducialCoordinates(coord);
               break;
               }              
               case 3:
               {
               fnode->GetFiducialCoordinates(coord);
               coord[2] = qstr.toDouble();
               fnode->SetFiducialCoordinates(coord);
               break;
               }
               */
          }
          rnode->Modified();
          //this->updateTable();
          // test code
          if(this->selectedPathsTableRow != RESET)
          {
            this->updateRulerTable();
            std::cout << "this->selectedPathsTableRow != RESET" << std::endl;            
          }else{
            std::cout << "this->selectedPathsTableRow == RESET" << std::endl;            
          }
        }
      }
    }
    
    
  }
  
  /*
   // test code
   if(this->selectedPathsTableRow != RESET)
   {
   this->updateRulerTable();
   } 
   */
  // test
  //std::cout << "finish onItemChanged()" << std::endl;
  
}




void qSlicerPathPlannerTableModel
::onMRMLChildNodeAdded(vtkObject* o)
{
  Q_D(qSlicerPathPlannerTableModel);

  // Find the newly added node
  vtkNew<vtkCollection> collection;
  d->HierarchyNode->GetDirectChildren(collection.GetPointer());
  int nItems = collection->GetNumberOfItems();
  int nFiducials = 0;
  collection->InitTraversal();

  for (int i = 0; i < nItems; i ++)
    {
    vtkMRMLAnnotationFiducialNode* fnode;
    fnode = vtkMRMLAnnotationFiducialNode::SafeDownCast(collection->GetNextItemAsObject());
    if (fnode)
      {
      if (!fnode->GetAttribute("RFTEvent"))
        {
        qvtkConnect(fnode, vtkMRMLAnnotationFiducialNode::ValueModifiedEvent,
                    this, SLOT(onMRMLChildNodeValueModified(vtkObject*)));
        fnode->SetAttribute("RFTEvent", "Yes");
        }
      }
    }

  
  // test code
  for (int i = 0; i < nItems; i ++)
  {
    vtkMRMLAnnotationRulerNode* rnode;
    rnode = vtkMRMLAnnotationRulerNode::SafeDownCast(collection->GetNextItemAsObject());
    if (rnode)
    {
      if (!rnode->GetAttribute("RFTEvent"))
      {
        qvtkConnect(rnode, vtkMRMLAnnotationRulerNode::ValueModifiedEvent,
                    this, SLOT(onMRMLChildNodeValueModified(vtkObject*)));
        rnode->SetAttribute("RFTEvent", "Yes");
      }
    }
  }
  
  
  // test
  //this->updateTable();
}

void qSlicerPathPlannerTableModel
::onMRMLChildNodeRemoved(vtkObject* o)
{  
  vtkMRMLNode* n = vtkMRMLNode::SafeDownCast(o);
  vtkMRMLAnnotationFiducialNode* fnode = vtkMRMLAnnotationFiducialNode::SafeDownCast(n);
  if (fnode && fnode->GetAttribute("RFTEvent"))
    {
    if (strcmp("Yes", fnode->GetAttribute("RFTEvent")) == 0)
      {
      qvtkDisconnect(fnode, vtkMRMLAnnotationFiducialNode::ValueModifiedEvent,
                     this, SLOT(onMRMLChildNodeValueModified(vtkObject*)));
      fnode->SetAttribute("RFTEvent", NULL);
      this->updateTable();
      }
    }

  
  // test code
  vtkMRMLAnnotationRulerNode* rnode = vtkMRMLAnnotationRulerNode::SafeDownCast(n);
  if (rnode && rnode->GetAttribute("RFTEvent"))
  {
    if (strcmp("Yes", rnode->GetAttribute("RFTEvent")) == 0)
    {
      qvtkDisconnect(rnode, vtkMRMLAnnotationRulerNode::ValueModifiedEvent,
                     this, SLOT(onMRMLChildNodeValueModified(vtkObject*)));
      rnode->SetAttribute("RFTEvent", NULL);
      this->updateTable();
    }
  }

}

void qSlicerPathPlannerTableModel
::onMRMLNodeRemovedEvent(vtkObject* caller, vtkObject* callData)
{
  Q_D(qSlicerPathPlannerTableModel);

  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(caller);
  if (scene && d->Scene && scene == d->Scene)
    {
    onMRMLChildNodeRemoved(callData);
    }
}

void qSlicerPathPlannerTableModel
::onMRMLChildNodeValueModified(vtkObject* obj)
{
  Q_D(qSlicerPathPlannerTableModel);

  vtkMRMLAnnotationFiducialNode* fnode;
  fnode = vtkMRMLAnnotationFiducialNode::SafeDownCast(obj);
  
  // test
  std::cout << "onMRMLChildNodeValueModified" << std::endl;
  //std::cout << "it's enough to add only UpdateTable function here" << std::endl;
  
  //this->addPoint(1,1,1);
  this->updateTable();
  
  /*
  // test code
  vtkMRMLAnnotationRulerNode* rnode;
  rnode = vtkMRMLAnnotationRulerNode::SafeDownCast(obj);
  */
  // test code
  if(this->selectedPathsTableRow != RESET)
  {
    this->updateRulerTable();
  }
  
}


//------------------------------------------------------------------------------
void qSlicerPathPlannerTableModel
::identifyTipOfPath(int row, int column)
{
  Q_D(qSlicerPathPlannerTableModel);
  
  //const char* selectedName;
  
  // test code
  std::cout << "identifyName" << std::endl;  
  
  if (d->HierarchyNode == 0)
  {
    this->setRowCount(0);
    
    return;
  }
  
  d->PendingItemModified = 0;
  
  // Count the number of child Fiducial nodes 
  vtkNew<vtkCollection> collection;
  d->HierarchyNode->GetDirectChildren(collection.GetPointer());
  int nItems = collection->GetNumberOfItems();
  int nFiducials = 0;
  
  // flag if time and memo should be refreshed in column 5 and 6
  if (nItems > this->nItemsPrevious)
  {
    this->addRowFlag = 1;
    this->nItemsPrevious = nItems;
  }
  else
  {
    this->addRowFlag = 0;    
    this->nItemsPrevious = nItems;
  }
  
  collection->InitTraversal();
  
  /*
  for (int i = 0; i < nItems; i ++)
  {
    vtkMRMLAnnotationFiducialNode* fnode;
    fnode = vtkMRMLAnnotationFiducialNode::SafeDownCast(collection->GetNextItemAsObject());
    if (fnode)
    {
      nFiducials ++;
    }
  }
  this->setRowCount(nFiducials);
  */
  
  collection->InitTraversal();
  for (int i = 0; i < nItems; i ++)
  {
    vtkMRMLAnnotationFiducialNode* fnode;
    fnode = vtkMRMLAnnotationFiducialNode::SafeDownCast(collection->GetNextItemAsObject());
    if (fnode)
    {
      QStandardItem* item = this->invisibleRootItem()->child(i, 0);
      if (item == NULL)
      {
        item = new QStandardItem();
        this->invisibleRootItem()->setChild(i, 0, item);
      }

      if(i == row)
      {
        this->selectedName = fnode->GetName(); 
        std::cout << "selectedName = " << this->selectedName << std::endl;  
      }else
      {
        item->setText(fnode->GetName());        
        std::cout << "item->setText(fnode->GetName()) = " << fnode->GetName() << std::endl;  
      }

      // ----------
      for (int j = 0; j < 5; j ++)
      {
        QStandardItem* item = this->invisibleRootItem()->child(i, j+1);
        if (item == NULL)
        {
          item = new QStandardItem();
          this->invisibleRootItem()->setChild(i, j+1, item);
        }
        QString str;
        str.setNum(fnode->GetFiducialCoordinates()[j]);
        if(j<3)
        {
          // test code
          //QByteArray byteArray(str.toAscii());
          //this->selectedCoordinate[j] = byteArray.constData();
          this->selectedCoordinate[i][j] = fnode->GetFiducialCoordinates()[j];
          //item->setText(str);
        }
        else if(j==3)
        {
          // time stamp for each row
          if(i == nItems-1 && this->addRowFlag == 1)
          {
            QString timeStamp = QTime::currentTime().toString();
            QByteArray byteArray(timeStamp.toAscii());
            const char *timeStampStr = byteArray.constData();
            
            // test code
            this->selectedTime = timeStampStr;
            //item->setText(timeStampStr);                
          }
        }
        else
        {
          // memo column for each row
          if(i == nItems-1 && this->addRowFlag == 1)
          {
            item->setText("");                
          }else
          {
             // write memo column reading part
          }
            
        }
      }
      // ----------
      

    }
  }
  
}


//------------------------------------------------------------------------------
void qSlicerPathPlannerTableModel
::calculatePath(double entryPoint[3], double targetPoint[3], int row)
{
  Q_D(qSlicerPathPlannerTableModel);

  double difference[3];

  // set the tip positions
  this->selectedTargetPoint[row][0] = targetPoint[0];
  this->selectedTargetPoint[row][1] = targetPoint[1];
  this->selectedTargetPoint[row][2] = targetPoint[2];
  
  this->selectedEntryPoint[row][0] = entryPoint[0];
  this->selectedEntryPoint[row][1] = entryPoint[1];
  this->selectedEntryPoint[row][2] = entryPoint[2];
    
  // test code
  std::cout << "pathDistance row = " << row << std::endl;
  
  // calculate distance
  difference[0] = entryPoint[0] - targetPoint[0];
  difference[1] = entryPoint[1] - targetPoint[1];
  difference[2] = entryPoint[2] - targetPoint[2];    
  
  this->pathDistance[row] = sqrt(difference[0]*difference[0]+difference[1]*difference[1]+difference[2]*difference[2]);
    
}
