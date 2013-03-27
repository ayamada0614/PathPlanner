/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerPathPlannerModuleWidget.h"
#include "ui_qSlicerPathPlannerModuleWidget.h"

#include "qSlicerPathPlannerFiducialItem.h"

#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPathPlannerModuleWidgetPrivate: public Ui_qSlicerPathPlannerModuleWidget
{
public:
  qSlicerPathPlannerModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPathPlannerModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerPathPlannerModuleWidgetPrivate::
qSlicerPathPlannerModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPathPlannerModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerPathPlannerModuleWidget::
qSlicerPathPlannerModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerPathPlannerModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerPathPlannerModuleWidget::
~qSlicerPathPlannerModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPathPlannerModuleWidget::
setup()
{
  Q_D(qSlicerPathPlannerModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect(d->EntryPointListNodeSelector, SIGNAL(nodeActivated(vtkMRMLNode*)),
	  this, SLOT(onEntryListNodeChanged(vtkMRMLNode*)));
  connect(d->TargetPointListNodeSelector, SIGNAL(nodeActivated(vtkMRMLNode*)),
	  this, SLOT(onTargetListNodeChanged(vtkMRMLNode*)));

}

//-----------------------------------------------------------------------------
void qSlicerPathPlannerModuleWidget::
onEntryListNodeChanged(vtkMRMLNode* newList)
{
  Q_D(qSlicerPathPlannerModuleWidget);

  if (!newList || !d->EntryPointWidget)
    {
    return;
    }

  vtkMRMLAnnotationHierarchyNode* entryList = 
    vtkMRMLAnnotationHierarchyNode::SafeDownCast(newList);

  if (!entryList)
    {
    return;
    }

  // Clear entry table widget
  d->EntryPointWidget->getTableWidget()->clearContents();

  // Populate table with new items in the new list
  for(int i = 0; i < entryList->GetNumberOfChildrenNodes(); i++)
    {
    vtkMRMLAnnotationFiducialNode* fiducialPoint =
      vtkMRMLAnnotationFiducialNode::SafeDownCast(entryList->GetNthChildNode(i)->GetAssociatedNode());
    if (fiducialPoint)
      {
      this->addNewItem(d->EntryPointWidget->getTableWidget(), fiducialPoint);
      }
    }

  // Update widget
  d->EntryPointWidget->setSelectedHierarchyNode(entryList);
}

//-----------------------------------------------------------------------------
void qSlicerPathPlannerModuleWidget::
onTargetListNodeChanged(vtkMRMLNode* newList)
{
  Q_D(qSlicerPathPlannerModuleWidget);

  if (!newList || !d->TargetPointWidget)
    {
    return;
    }

  vtkMRMLAnnotationHierarchyNode* targetList = 
    vtkMRMLAnnotationHierarchyNode::SafeDownCast(newList);

  if (!targetList)
    {
    return;
    }

  // Clear entry table widget
  d->TargetPointWidget->getTableWidget()->clear();
  d->TargetPointWidget->getTableWidget()->setRowCount(0);
  

  // Populate table with new items in the new list
  for(int i = 0; i < targetList->GetNumberOfChildrenNodes(); i++)
    {
    vtkMRMLAnnotationFiducialNode* fiducialPoint =
      vtkMRMLAnnotationFiducialNode::SafeDownCast(targetList->GetNthChildNode(i)->GetAssociatedNode());
    if (fiducialPoint)
      {
      this->addNewItem(d->TargetPointWidget->getTableWidget(), fiducialPoint);
      }
    }

  // Update widget
  d->TargetPointWidget->setSelectedHierarchyNode(targetList);
}

//-----------------------------------------------------------------------------
void qSlicerPathPlannerModuleWidget::
addNewItem(QTableWidget* tableWidget, vtkMRMLAnnotationFiducialNode* fiducialNode)
{
  if (!tableWidget || !fiducialNode)
    {
    return;
    }
  
  int numberOfItems = tableWidget->rowCount(); 
  tableWidget->insertRow(numberOfItems);

  // Add row
  qSlicerPathPlannerFiducialItem* newItem =
    new qSlicerPathPlannerFiducialItem();
  tableWidget->setItem(numberOfItems, 0, newItem);
  tableWidget->setItem(numberOfItems, 1, new QTableWidgetItem());
  tableWidget->setItem(numberOfItems, 2, new QTableWidgetItem());
  tableWidget->setItem(numberOfItems, 3, new QTableWidgetItem());
  tableWidget->setItem(numberOfItems, 4, new QTableWidgetItem());
  newItem->setFiducialNode(fiducialNode);

  // Set only name editable
  tableWidget->item(numberOfItems,1)->setFlags(tableWidget->item(numberOfItems,1)->flags() & ~Qt::ItemIsEditable);
  tableWidget->item(numberOfItems,2)->setFlags(tableWidget->item(numberOfItems,2)->flags() & ~Qt::ItemIsEditable);
  tableWidget->item(numberOfItems,3)->setFlags(tableWidget->item(numberOfItems,3)->flags() & ~Qt::ItemIsEditable);
  tableWidget->item(numberOfItems,4)->setFlags(tableWidget->item(numberOfItems,4)->flags() & ~Qt::ItemIsEditable);

  // Update item if changed  
  connect(tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
          this, SLOT(onItemChanged(QTableWidgetItem*)));
}

//-----------------------------------------------------------------------------
void qSlicerPathPlannerModuleWidget::
onItemChanged(QTableWidgetItem *item)
{
  if (!item)
    {
    return;
    }

  qSlicerPathPlannerFiducialItem *itemChanged
    = dynamic_cast<qSlicerPathPlannerFiducialItem*>(item);
  
  if (!itemChanged)
    {
    return;
    }

  itemChanged->updateItem();
}
