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

#ifndef __qSlicerPathPlannerPanelWidget_h
#define __qSlicerPathPlannerPanelWidget_h

// Qt includes
#include <QWidget>
#include <ctkVTKObject.h>

// PathPlannerPanel Widgets includes
#include "qSlicerPathPlannerModuleWidgetsExport.h"

// test code
//#include "vtkMRMLAnnotationNode"
#include "qtableview.h"

// test code
//#include "QVarient.h"

#define SELECTENTRYPOINTLIST 0
#define SELECTTARGETPOINTLIST 2
#define SELECTPATHLIST 3 // test

#define RESET -1

class qSlicerPathPlannerPanelWidgetPrivate;
class vtkObject;
class vtkMRMLScene;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_PathPlanner
class Q_SLICER_MODULE_PATHPLANNER_WIDGETS_EXPORT qSlicerPathPlannerPanelWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT  
public:
  typedef QWidget Superclass;
  qSlicerPathPlannerPanelWidget(QWidget *parent=0);
  virtual ~qSlicerPathPlannerPanelWidget();
  int toggleSwitchFlag;
  
  // test codes: pointers for item selection
  QItemSelectionModel *selectionEntryPointsTableModel;
  QItemSelectionModel *selectionTargetPointsTableModel;
  QItemSelectionModel *selectionPathsTableModel;
  
  int generatedPathColumnCounter;
  
  // test code
  //QVariant data(const QModelIndex &index, int role) const;
  
  // test code
  int selectedPathIndexOfRow;
  int selectedPathIndexofColumn;
  double differenceOfTip[3];
  double selectedEntryPoint[100][3];
  double selectedTargetPoint[100][3];
  

public slots:
  virtual void setMRMLScene(vtkMRMLScene *newScene);
  //void selectTargetPoint(const QItemSelection &selected,
  //                       const QItemSelection &deselected);
  void selectTargetPoint(const QItemSelection &selected,
                         const QItemSelection &deselected);
  void selectEntryPoint(const QItemSelection &selected,
                         const QItemSelection &deselected);
  void selectPathsTable(const QItemSelection &selected,
                        const QItemSelection &deselected);

  void selectTargetPointTable(int);  
  
  // test code
  void setTrackerTransform(vtkMRMLNode*);
  void onTrackerTransformModified();

  void setEntryPointsAnnotationNode(vtkMRMLNode*);  
  void setTargetPointsAnnotationNode(vtkMRMLNode*);
  // test code
  void setPathsAnnotationNode(vtkMRMLNode*);
  
  void enter();  
    
protected slots:
  void deleteEntryPoints();
  void deleteTargetPoints();
  void addTargetPoint();
  void entryPointToolBarClicked(QAction*);
  void addEntryPointButtonClicked();
  void switchCurrentAnotationNode(int);
  void addPathRow();
    
protected:
  QScopedPointer<qSlicerPathPlannerPanelWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerPathPlannerPanelWidget);
  Q_DISABLE_COPY(qSlicerPathPlannerPanelWidget);
};

#endif
