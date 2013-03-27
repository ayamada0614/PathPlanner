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

#ifndef __qSlicerPathPlannerTableModel_h
#define __qSlicerPathPlannerTableModel_h

#define RESET -1

//#include <QAbstractTableModel>
#include <QStandardItemModel>

#include <ctkPimpl.h>
#include <ctkVTKObject.h>
#include <qdatetime.h>

#include "qSlicerPathPlannerModuleWidgetsExport.h"

class vtkObject;
class vtkMRMLNode;
class vtkMRMLScene;
class qSlicerPathPlannerTableModelPrivate;

class Q_SLICER_MODULE_PATHPLANNER_WIDGETS_EXPORT qSlicerPathPlannerTableModel
//  : public QAbstractTableModel
  : public QStandardItemModel
{
  Q_OBJECT
  QVTK_OBJECT
    //Q_PROPERTY(bool noneEnabled READ noneEnabled WRITE setNoneEnabled)

public:
  typedef QAbstractTableModel Superclass;
  qSlicerPathPlannerTableModel(QObject *parent=0);
  virtual ~qSlicerPathPlannerTableModel();
  qSlicerPathPlannerTableModel(QList< QPair<QString, QString> > listofPairs, QObject *parent=0);

  enum ItemDataRole {
    NodeIDRole = Qt::UserRole,
  };
  enum CoordinateLabel {
    LABEL_RAS = 1,
    LABEL_XYZ = 2,
    LABEL_RAS_ENTRY = 3,
    LABEL_RAS_TARGET = 4,
    LABEL_RAS_PATH = 5,
  };
  int addRowFlag;
  int nItemsPrevious;
  
  // test code
  int selectedTargetPointItemRow;
  int selectedTargetPointItemColumn;
  int selectedEntryPointItemRow;
  int selectedEntryPointItemColumn;
  int selectedPathsTableRow;
  int selectedPathsTableColumn;

protected:
  qSlicerPathPlannerTableModel(qSlicerPathPlannerTableModelPrivate* pimpl, QObject *parent=0);

public:  
  void setCoordinateLabel(int m); // LABEL_RAS or LABEL_XYZ
  void updateTable();
  void updateRulerTable();
  
  // test code
  void identifyTipOfPath(int row, int column);
  void calculatePath(double entryPoint[3], double targetPoint[3], int row);
  const char* selectedName;
  double selectedCoordinate[100][3];
  double selectedTargetPoint[100][3];
  double selectedEntryPoint[100][3];
  
  const char* selectedTime;
  //char selectedTargetName;
  //char selectedEntryName;
  
  void addPoint(double x, double y, double z);
  void addRuler(void);
  void initList(int);
  const char* targetPointName[100];
  const char* entryPointName[100];
  double pathDistance[100];
  int** columItemFlag;
  int pathColumnCounter;
  int pathTableExistance;
  //double pathDistance;
  
public slots:
  void setMRMLScene(vtkMRMLScene *newScene);

protected slots:
  void setNode(vtkMRMLNode* node);
  void onItemChanged(QStandardItem * item);
  // test code
  void onRulerItemChanged(QStandardItem * item);
  void onMRMLChildNodeAdded(vtkObject*);
  void onMRMLChildNodeRemoved(vtkObject*);
  void onMRMLChildNodeValueModified(vtkObject*);
  void onMRMLNodeRemovedEvent(vtkObject*,vtkObject*);
  
protected:
  QScopedPointer<qSlicerPathPlannerTableModelPrivate> d_ptr;
  
private:
  Q_DECLARE_PRIVATE(qSlicerPathPlannerTableModel);
  Q_DISABLE_COPY(qSlicerPathPlannerTableModel);

};

#endif // __qSlicerPathPlannerTableModel_h
