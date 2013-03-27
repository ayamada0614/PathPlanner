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

#ifndef __qSlicerPathPlannerModuleWidget_h
#define __qSlicerPathPlannerModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerPathPlannerModuleExport.h"

#include <ctkVTKObject.h>

#include <QTableWidget>

class qSlicerPathPlannerModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLAnnotationFiducialNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_PATHPLANNER_EXPORT qSlicerPathPlannerModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerPathPlannerModuleWidget(QWidget *parent=0);
  virtual ~qSlicerPathPlannerModuleWidget();

public slots:
  void onEntryListNodeChanged(vtkMRMLNode* newList);
  void onTargetListNodeChanged(vtkMRMLNode* newList);
  void onItemChanged(QTableWidgetItem *item);
  void refreshEntryView();
  void refreshTargetView();

protected:
  QScopedPointer<qSlicerPathPlannerModuleWidgetPrivate> d_ptr;
  
  virtual void setup();
  void addNewItem(QTableWidget* tableWidget, vtkMRMLAnnotationFiducialNode* fiducialNode);

private:
  Q_DECLARE_PRIVATE(qSlicerPathPlannerModuleWidget);
  Q_DISABLE_COPY(qSlicerPathPlannerModuleWidget);
};

#endif
