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

// test codes
#include "qSlicerApplication.h"
#include "vtkMRMLScene.h"

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
qSlicerPathPlannerModuleWidgetPrivate::qSlicerPathPlannerModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPathPlannerModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerPathPlannerModuleWidget::qSlicerPathPlannerModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerPathPlannerModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerPathPlannerModuleWidget::~qSlicerPathPlannerModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPathPlannerModuleWidget::setup()
{
  Q_D(qSlicerPathPlannerModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}


//-----------------------------------------------------------------------------
void qSlicerPathPlannerModuleWidget::enter()
{
  Q_D(qSlicerPathPlannerModuleWidget);
  
  this->Superclass::enter();
  if (d->PathPlannerPanel)
  {
    d->PathPlannerPanel->enter();
  }
}

//-----------------------------------------------------------------------------
void qSlicerPathPlannerModuleWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerPathPlannerModuleWidget);
  
  vtkMRMLScene* oldScene = this->mrmlScene();
  
  this->Superclass::setMRMLScene(newScene);
  
  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
  {
    return;
  }
  
  if (oldScene != newScene)
  {
    if (d->PathPlannerPanel)
    {
      d->PathPlannerPanel->setMRMLScene(newScene);
    }
  }
  
  newScene->InitTraversal();
  
}
