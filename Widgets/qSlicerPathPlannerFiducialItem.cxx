/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qSlicerPathPlannerFiducialItem.h"

// --------------------------------------------------------------------------
qSlicerPathPlannerFiducialItem
::qSlicerPathPlannerFiducialItem() : QTableWidgetItem()
{
  this->FiducialNode = NULL;
}

// --------------------------------------------------------------------------
qSlicerPathPlannerFiducialItem::
~qSlicerPathPlannerFiducialItem()
{
}

// --------------------------------------------------------------------------
void qSlicerPathPlannerFiducialItem::
setFiducialNode(vtkMRMLAnnotationFiducialNode* fiducialNode)
{
  if (fiducialNode)
    {
    qvtkReconnect(this->FiducialNode, fiducialNode, vtkCommand::ModifiedEvent,
		  this, SLOT(updateItem()));
    this->FiducialNode = fiducialNode;
    this->updateItem();
    }
}

// --------------------------------------------------------------------------
vtkMRMLAnnotationFiducialNode* qSlicerPathPlannerFiducialItem::
getFiducialNode()
{
  return this->FiducialNode;
}

// --------------------------------------------------------------------------
void qSlicerPathPlannerFiducialItem::
updateItem()
{
  if (!this->FiducialNode)
    {
    return;
    }

  double* targetPosition;
  targetPosition = this->FiducialNode->GetFiducialCoordinates();

  double itemRow = this->row();
  QTableWidget* tableWidget = this->tableWidget();

  if (!tableWidget)
    {
    return;
    }

  // Update text
  std::stringstream fiduPos;
  fiduPos.precision(2);
  fiduPos.setf(std::ios::fixed);

  // Name
  QString pointName = tableWidget->item(itemRow,0)->text();
  QString fiducialName(this->FiducialNode->GetName());
  
  if (pointName.isEmpty())
    {
    // No name set. Use fiducial name.
    tableWidget->item(itemRow,0)->setText(fiducialName.toStdString().c_str());
    }
  else
    {
    if (pointName.compare(fiducialName) != 0)
      {
      // Item name is different from fiducial name
      // Change fiducial name
      this->FiducialNode->SetName(tableWidget->item(itemRow,0)->text().toStdString().c_str());
      }
    }

  //   R
  fiduPos << targetPosition[0];  
  tableWidget->item(itemRow, 1)->setText(fiduPos.str().c_str());
  fiduPos.str(std::string());
  
  //   A
  fiduPos << targetPosition[1];  
  tableWidget->item(itemRow, 2)->setText(fiduPos.str().c_str());
  fiduPos.str(std::string());
  
  //   S
  fiduPos << targetPosition[2];  
  tableWidget->item(itemRow, 3)->setText(fiduPos.str().c_str());
  fiduPos.str(std::string());

  // Time
  tableWidget->item(itemRow,4)->setText(QTime::currentTime().toString());
}

