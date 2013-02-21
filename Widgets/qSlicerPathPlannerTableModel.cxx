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

  q->setColumnCount(4);
  q->setHorizontalHeaderLabels(QStringList()
                               << "Point Name"
                               << "R"
                               << "A"
                               << "S"
                               << "Time");
  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onItemChanged(QStandardItem*)));

}


qSlicerPathPlannerTableModel
::qSlicerPathPlannerTableModel(QObject *parent)
  : QStandardItemModel(parent)
  , d_ptr( new qSlicerPathPlannerTableModelPrivate(*this) )
{
  Q_D(qSlicerPathPlannerTableModel);
  d->init();
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
      list << "Point Name" << "R" << "A" << "S" << "Time";
      break;
      }
    case LABEL_XYZ:
      {
      list << "Point Name" << "X" << "Y" << "Z" << "Time";
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
      //item->setData(QVariant(),Qt::SizeHintRole);
      item->setData(fnode->GetID(),qSlicerPathPlannerTableModel::NodeIDRole);

      for (int j = 0; j < 3; j ++)
        {
        QStandardItem* item = this->invisibleRootItem()->child(i, j+1);
        if (item == NULL)
          {
          item = new QStandardItem();
          this->invisibleRootItem()->setChild(i, j+1, item);
          }
        QString str;
        str.setNum(fnode->GetFiducialCoordinates()[j]);
        item->setText(str);
        //item->setData(QVariant(),Qt::SizeHintRole);
        }
      }
    }

  //QObject::connect(this, SIGNAL(itemChanged(QStandardItem*)),
  //this, SLOT(onItemChanged(QStandardItem*)));
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
    ss << "Physical_" << (nItems+1);

    vtkSmartPointer< vtkMRMLAnnotationFiducialNode > fid = vtkSmartPointer< vtkMRMLAnnotationFiducialNode >::New();
    fid->SetName(ss.str().c_str());
    double coord[3] = {x, y, z};
    fid->SetFiducialCoordinates(x, y, z);
    d->Scene->AddNode(fid);
    fid->CreateAnnotationTextDisplayNode();
    fid->CreateAnnotationPointDisplayNode();
    fid->GetAnnotationPointDisplayNode()->SetGlyphScale(5);
    fid->GetAnnotationPointDisplayNode()->SetGlyphType(vtkMRMLAnnotationPointDisplayNode::Sphere3D);
    this->updateTable();

    }
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
          double coord[3];
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
  this->updateTable();
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

  this->updateTable();

}


