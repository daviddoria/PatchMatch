/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

/** Demonstrate different initialization techniques. */

// STL
#include <iostream>

// ITK
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCovariantVector.h"

// Submodules
#include <Mask/Mask.h>
#include <Mask/ITKHelpers/ITKHelpers.h>
#include <PatchComparison/SSD.h>

// Custom
#include "PatchMatch.h"
#include "InitializerRandom.h"
#include "InitializerBoundary.h"

typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;

int main(int argc, char*argv[])
{
  if(argc < 3)
  {
    std::cerr << "Required arguments: image mask" << std::endl;
    return EXIT_FAILURE;
  }

  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  std::cout << "imageFilename: " << imageFilename << std::endl;
  std::cout << "maskFilename: " << maskFilename << std::endl;

  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  Mask::Pointer sourceMask = Mask::New();
  sourceMask->Read(maskFilename);

  Mask::Pointer targetMask = Mask::New();
  targetMask->SetRegions(sourceMask->GetLargestPossibleRegion());
  targetMask->Allocate();
  ITKHelpers::SetImageToConstant(targetMask.GetPointer(), targetMask->GetValidValue());

  unsigned int patchRadius = 3;

  SSD<ImageType>* patchDistanceFunctor = new SSD<ImageType>;
  patchDistanceFunctor->SetImage(imageReader->GetOutput());

  PatchMatch patchMatch;
//  patchMatch.SetImage(imageReader->GetOutput());
//  patchMatch.SetTargetMask(targetMask);
//  patchMatch.SetSourceMask(sourceMask);
//  patchMatch.SetIterations(10);
//  patchMatch.SetPatchRadius(patchRadius);
//  patchMatch.SetPatchDistanceFunctor(patchDistanceFunctor);

//  PatchMatch<ImageType>::CoordinateImageType::Pointer output =
//    PatchMatch<ImageType>::CoordinateImageType::New();

//  {
//  InitializerRandom<ImageType> initializer(imageReader->GetOutput(), patchRadius);
//  patchMatch.SetInitializer(&initializer);
//  std::cout << "Starting randomInit..." << std::endl;
//  patchMatch.Initialize();

//  PatchMatch<ImageType>::GetPatchCentersImage(patchMatch.GetOutput(), output);
//  ITKHelpers::WriteImage(output.GetPointer(), "randomInit.mha");
//  }

//   {
//   std::cout << "Starting boundaryInit..." << std::endl;
//   InitializerBoundary<ImageType> initializer(imageReader->GetOutput(), patchRadius);
//   patchMatch.SetInitializer(&initializer);
//   patchMatch.Initialize();
// 
//   PatchMatch<ImageType>::GetPatchCentersImage(patchMatch.GetOutput(), output);
//   ITKHelpers::WriteImage(output.GetPointer(), "boundaryInit.mha");
//   }

  return EXIT_SUCCESS;
}
