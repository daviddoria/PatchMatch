/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "NNFieldInspector.h"

// STL
#include <stdexcept>

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkVector.h"

// Qt
#include <QFileDialog>
#include <QIcon>
#include <QProgressDialog>
#include <QTextEdit>
#include <QtConcurrentRun>

// VTK
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkFloatArray.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageSlice.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPointPicker.h>
#include <vtkProperty2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkImageSliceMapper.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkXMLPolyDataReader.h>

// Submodules
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "ITKVTKCamera/ITKVTKCamera.h"

// Custom
#include "PointSelectionStyle2D.h"
#include "Pane2D.h"

void NNFieldInspector::on_actionHelp_activated()
{
  QTextEdit* help=new QTextEdit();
  
  help->setReadOnly(true);
  help->append("<h1>Nearest Neighbor Field Inspector</h1>\
  Click on a pixel. The surrounding region will be outlined, and the best matching region will be outlined.<br/>"
  );
  help->show();
}

void NNFieldInspector::on_actionQuit_activated()
{
  exit(0);
}

void NNFieldInspector::SharedConstructor()
{
  this->setupUi(this);

  this->LeftPane = new Pane2D(this->qvtkWidgetLeft);
  this->RightPane = NULL;

  this->NNField = NNFieldImageType::New();
  this->Image = ImageType::New();
}

NNFieldInspector::NNFieldInspector(const std::string& imageFileName, const std::string& nnFieldFileName)
{
  SharedConstructor();

  LoadImage(LeftPane, imageFileName);

  this->Camera.SetRenderer(this->LeftPane->Renderer);
  //this->Camera.SetRenderWindow(this->LeftPane->RenderW);
  
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFileName);
  imageReader->Update();

  ITKHelpers::DeepCopy(imageReader->GetOutput(), this->Image.GetPointer());
  
  typedef itk::ImageFileReader<NNFieldImageType> NNFieldReaderType;
  NNFieldReaderType::Pointer nnFieldReader = NNFieldReaderType::New();
  nnFieldReader->SetFileName(nnFieldFileName);
  nnFieldReader->Update();

  ITKHelpers::DeepCopy(nnFieldReader->GetOutput(), this->NNField.GetPointer());
}

// Constructor
NNFieldInspector::NNFieldInspector()
{
  SharedConstructor();

};

void NNFieldInspector::LoadImage(Pane2D* const inputPane, const std::string& fileName)
{

/*
  QFileInfo fileInfo(fileName.toStdString().c_str());
  std::string extension = fileInfo.suffix().toStdString();
  std::cout << "extension: " << extension << std::endl;*/
  
  Pane2D* pane = static_cast<Pane2D*>(inputPane);

  if(!pane)
  {
    throw std::runtime_error("LoadImage: inputPane is NULL!");
  }
  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName);
  reader->Update();

  pane->Image = reader->GetOutput();

  ITKVTKHelpers::ITKImageToVTKRGBImage(pane->Image.GetPointer(), pane->ImageData);

  pane->ImageSliceMapper->SetInputData(pane->ImageData);
  pane->ImageSlice->SetMapper(pane->ImageSliceMapper);
  
  // Add Actor to renderer

  pane->Renderer->AddActor(pane->ImageSlice);
  pane->Renderer->ResetCamera();

  vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();
  pane->qvtkWidget->GetRenderWindow()->GetInteractor()->SetPicker(pointPicker);
  pane->SelectionStyle = PointSelectionStyle2D::New();
  pane->SelectionStyle->SetCurrentRenderer(pane->Renderer);
  //pane->SelectionStyle->Initialize();
  pane->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(pane->SelectionStyle);

  pane->Renderer->ResetCamera();

  pane->qvtkWidget->GetRenderWindow()->Render();


  /** When the image is clicked, alert the GUI. */
  this->LeftPane->SelectionStyle->AddObserver(PointSelectionStyle2D::PixelClickedEvent, this,
                                              &NNFieldInspector::PixelClickedEventHandler);
}

void NNFieldInspector::on_actionFlipLeftHorizontally_activated()
{
  if(dynamic_cast<Pane2D*>(this->LeftPane))
    {
    static_cast<Pane2D*>(this->LeftPane)->FlipHorizontally();
    }
  else
    {
    std::cerr << "Cannot flip a point cloud!" << std::endl;
    }
}

void NNFieldInspector::on_actionFlipLeftVertically_activated()
{
  if(dynamic_cast<Pane2D*>(this->LeftPane))
    {
    static_cast<Pane2D*>(this->LeftPane)->FlipVertically();
    }
  else
    {
    std::cerr << "Cannot flip a point cloud!" << std::endl;
    }
}

void NNFieldInspector::on_actionFlipRightHorizontally_activated()
{
  if(dynamic_cast<Pane2D*>(this->RightPane))
    {
    static_cast<Pane2D*>(this->RightPane)->FlipHorizontally();
    }
  else
    {
    std::cerr << "Cannot flip a point cloud!" << std::endl;
    }
}

void NNFieldInspector::on_actionFlipRightVertically_activated()
{
  this->RightPane->FlipVertically();
}

void NNFieldInspector::on_actionOpenImageLeft_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".",
                                                  "Image Files (*.jpg *.jpeg *.bmp *.png *.mha)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  if(this->LeftPane)
    {
    delete this->LeftPane;
    }
  this->LeftPane = new Pane2D(this->qvtkWidgetLeft);
  LoadImage(this->LeftPane, fileName.toStdString());
}

void NNFieldInspector::on_actionOpenImageRight_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".",
                                                  "Image Files (*.jpg *.jpeg *.bmp *.png *.mha)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }
    
  if(this->RightPane)
    {
    delete this->RightPane;
    }
  this->RightPane = new Pane2D(this->qvtkWidgetRight);
  LoadImage(this->RightPane, fileName.toStdString());
}

void NNFieldInspector::PixelClickedEventHandler(vtkObject* caller, long unsigned int eventId,
                                void* callData)
{
  double* pixel = reinterpret_cast<double*>(callData);

  //std::cout << "Picked " << pixel[0] << " " << pixel[1] << std::endl;

  itk::Index<2> pickedIndex = {{static_cast<unsigned int>(pixel[0]), static_cast<unsigned int>(pixel[1])}};

  std::cout << "Picked index: " << pickedIndex << std::endl;
  
  NNFieldImageType::PixelType bestMatch = this->NNField->GetPixel(pickedIndex);
  itk::Index<2> bestMatchIndex = {{static_cast<unsigned int>(bestMatch[0]),
                                   static_cast<unsigned int>(bestMatch[1])}};

  std::cout << "Best match: " << bestMatchIndex << std::endl;
}
