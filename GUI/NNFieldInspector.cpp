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
#include <QTextEdit> // for help

// VTK
#include <vtkCommand.h>
#include <vtkImageData.h>
#include <vtkImageSlice.h>
#include <vtkInteractorStyleImage.h>
#include <vtkPointPicker.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkImageSliceMapper.h>

// Submodules
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "ITKVTKCamera/ITKVTKCamera.h"
#include "VTKHelpers/VTKHelpers.h"

// Custom
#include "PointSelectionStyle2D.h"

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

  // Turn slices visibility off to prevent errors that there is not yet data.
  this->ImageLayer.ImageSlice->VisibilityOff();
  this->NNFieldLayer.ImageSlice->VisibilityOff();
  this->PickLayer.ImageSlice->VisibilityOff();

  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  // Add slices to renderer
  this->Renderer->AddViewProp(this->ImageLayer.ImageSlice);
  this->Renderer->AddViewProp(this->NNFieldLayer.ImageSlice);
  this->Renderer->AddViewProp(this->PickLayer.ImageSlice);

  this->NNField = NNFieldImageType::New();
  this->Image = ImageType::New();

  vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetPicker(pointPicker);

  this->SelectionStyle = PointSelectionStyle2D::New();
  this->SelectionStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->SelectionStyle);

  /** When the image is clicked, alert the GUI. */
  this->SelectionStyle->AddObserver(PointSelectionStyle2D::PixelClickedEvent, this,
                                    &NNFieldInspector::PixelClickedEventHandler);

  this->Camera.SetRenderer(this->Renderer);
  this->Camera.SetRenderWindow(this->qvtkWidget->GetRenderWindow());
  this->Camera.SetInteractorStyle(this->SelectionStyle);
  //this->Camera.Flip();
  this->Camera.SetCameraPosition1();
}

NNFieldInspector::NNFieldInspector(const std::string& imageFileName,
                                   const std::string& nnFieldFileName) : PatchRadius(7)
{
  SharedConstructor();

  LoadImage(imageFileName);
  LoadNNField(nnFieldFileName);
}

// Constructor
NNFieldInspector::NNFieldInspector() : PatchRadius(7)
{
  SharedConstructor();
};

void NNFieldInspector::LoadNNField(const std::string& fileName)
{
  typedef itk::ImageFileReader<NNFieldImageType> NNFieldReaderType;
  NNFieldReaderType::Pointer nnFieldReader = NNFieldReaderType::New();
  nnFieldReader->SetFileName(fileName);
  nnFieldReader->Update();

  ITKHelpers::DeepCopy(nnFieldReader->GetOutput(), this->NNField.GetPointer());

  // Extract the first two channels
  std::vector<unsigned int> channels;
  channels.push_back(0);
  channels.push_back(1);

  typedef itk::VectorImage<int, 2> VectorImageType;
  VectorImageType::Pointer vectorImage = VectorImageType::New();
  vectorImage->SetNumberOfComponentsPerPixel(2);
  vectorImage->SetRegions(this->NNField->GetLargestPossibleRegion());
  vectorImage->Allocate();

  ITKHelpers::ExtractChannels(this->NNField.GetPointer(), channels, vectorImage.GetPointer());
  ITKVTKHelpers::ITKImageToVTKMagnitudeImage(vectorImage.GetPointer(), this->NNFieldLayer.ImageData);

  UpdateDisplayedImages();

  this->Renderer->ResetCamera();

  this->qvtkWidget->GetRenderWindow()->Render();
}

void NNFieldInspector::LoadImage(const std::string& fileName)
{
  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName);
  reader->Update();

  ITKHelpers::DeepCopy(reader->GetOutput(), this->Image.GetPointer());

  ITKVTKHelpers::ITKImageToVTKRGBImage(this->Image.GetPointer(), this->ImageLayer.ImageData);

  UpdateDisplayedImages();

  this->Renderer->ResetCamera();

  this->qvtkWidget->GetRenderWindow()->Render();
}

void NNFieldInspector::on_actionFlipHorizontally_activated()
{
  this->Camera.FlipHorizontally();
}

void NNFieldInspector::on_actionFlipVertically_activated()
{
  this->Camera.FlipVertically();
}

void NNFieldInspector::on_actionOpenImage_activated()
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

  LoadImage(fileName.toStdString());
}


void NNFieldInspector::on_actionOpenNNField_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".",
                                                  "Image Files (*.mha)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  LoadNNField(fileName.toStdString());
}

void NNFieldInspector::PixelClickedEventHandler(vtkObject* caller, long unsigned int eventId,
                                                void* callData)
{
  double* pixel = reinterpret_cast<double*>(callData);

  //std::cout << "Picked " << pixel[0] << " " << pixel[1] << std::endl;

  itk::Index<2> pickedIndex = {{static_cast<unsigned int>(pixel[0]), static_cast<unsigned int>(pixel[1])}};

  std::cout << "Picked index: " << pickedIndex << std::endl;

  itk::ImageRegion<2> pickedRegion = ITKHelpers::GetRegionInRadiusAroundPixel(pickedIndex, this->PatchRadius);

  if(!this->Image->GetLargestPossibleRegion().IsInside(pickedRegion))
  {
    std::cout << "Picked patch that is not entirely inside image!" << std::endl;
    return;
  }

  NNFieldImageType::PixelType nnFieldPixel = this->NNField->GetPixel(pickedIndex);

  // This assumes the NNField is an Offset field
  itk::Index<2> bestMatchCenter = {{static_cast<unsigned int>(nnFieldPixel[0]) + pickedIndex[0],
                                    static_cast<unsigned int>(nnFieldPixel[1]) + pickedIndex[1]}};
  // If we are using the Location field, do this:
//   itk::Index<2> bestMatchCenter = {{static_cast<unsigned int>(nnFieldPixel[0]),
//                                     static_cast<unsigned int>(nnFieldPixel[1])}};

  itk::ImageRegion<2> matchRegion = ITKHelpers::GetRegionInRadiusAroundPixel(bestMatchCenter, this->PatchRadius);
  std::cout << "Best match center: " << bestMatchCenter << std::endl;

  // Highlight patches
  ImageType::PixelType red;
  red[0] = 255; red[1] = 0; red[2] = 0;

  ImageType::PixelType green;
  green[0] = 0; green[1] = 255; green[2] = 0;

  ImageType::Pointer tempImage = ImageType::New();
  tempImage->SetRegions(this->Image->GetLargestPossibleRegion());
  tempImage->Allocate();

  ITKHelpers::OutlineRegion(tempImage.GetPointer(), pickedRegion, red);

  ITKHelpers::OutlineRegion(tempImage.GetPointer(), matchRegion, green);
  typedef itk::Image<float, 2> FloatImageType;
  FloatImageType::Pointer magnitudeImage = FloatImageType::New();
  ITKHelpers::MagnitudeImage(tempImage.GetPointer(), magnitudeImage.GetPointer());

  ITKVTKHelpers::InitializeVTKImage(this->Image->GetLargestPossibleRegion(), 4, this->PickLayer.ImageData); // 4 for RGBA
  VTKHelpers::MakeImageTransparent(this->PickLayer.ImageData);
  std::vector<itk::Index<2> > nonZeroPixels = ITKHelpers::GetNonZeroPixels(magnitudeImage.GetPointer());
  ITKVTKHelpers::SetPixelTransparency(this->PickLayer.ImageData, nonZeroPixels, VTKHelpers::OPAQUE_PIXEL);
  ITKVTKHelpers::ITKImageToVTKRGBImage(tempImage.GetPointer(), this->PickLayer.ImageData, true); // 'true' means 'already initialized'
  this->PickLayer.ImageSlice->VisibilityOn();
}

void NNFieldInspector::SetPatchRadius(const unsigned int patchRadius)
{
  this->PatchRadius = patchRadius;
}

void NNFieldInspector::on_radRGB_clicked()
{
  UpdateDisplayedImages();
}

void NNFieldInspector::on_radNNField_clicked()
{
  UpdateDisplayedImages();
}

void NNFieldInspector::UpdateDisplayedImages()
{
  this->NNFieldLayer.ImageSlice->SetVisibility(this->radNNField->isChecked());
  this->ImageLayer.ImageSlice->SetVisibility(this->radRGB->isChecked());
  this->qvtkWidget->GetRenderWindow()->Render();
}
