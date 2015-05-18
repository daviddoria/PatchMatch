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

/** This program computes the NN field of an image. */

// STL
#include <iostream>

// ITK
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCovariantVector.h"

// Submodules
#include "Mask/Mask.h"
#include "ITKHelpers/ITKHelpers.h"
#include "PatchComparison/SSD.h"

// Custom
#include "PatchMatch.h"
#include "Propagator.h"
#include "RandomSearch.h"

typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;

int main(int argc, char*argv[])
{
  if(argc < 4)
  {
    std::cerr << "Required arguments: image mask output" << std::endl;
    return EXIT_FAILURE;
  }

  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::string outputFilename = argv[3];

  std::cout << "imageFilename: " << imageFilename << std::endl;
  std::cout << "maskFilename: " << maskFilename << std::endl;
  std::cout << "outputFilename: " << outputFilename << std::endl;

  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  Mask::Pointer sourceMask = Mask::New();
  sourceMask->Read(maskFilename);

  Mask::Pointer targetMask = Mask::New();
  targetMask->SetRegions(sourceMask->GetLargestPossibleRegion());
  targetMask->Allocate();
  ITKHelpers::SetImageToConstant(targetMask.GetPointer(), HoleMaskPixelTypeEnum::VALID);

  typedef SSD<ImageType> DistanceFunctorType;
  DistanceFunctorType* patchDistanceFunctor = new DistanceFunctorType;
  patchDistanceFunctor->SetImage(imageReader->GetOutput());

  typedef Propagator<DistanceFunctorType> PropagatorType;
  PropagatorType* propagationFunctor = new PropagatorType;

  typedef RandomSearch<ImageType, DistanceFunctorType> RandomSearchType;
  RandomSearchType* randomSearchFunctor = new RandomSearchType;

  typedef PatchMatch<ImageType, PropagatorType, RandomSearchType> PatchMatchType;
  PatchMatchType patchMatch;
  patchMatch.SetImage(imageReader->GetOutput());
  patchMatch.SetPatchRadius(3);
  
  patchMatch.SetPropagationFunctor(propagationFunctor);
  patchMatch.SetRandomSearchFunctor(randomSearchFunctor);

  patchMatch.Compute();

  NNFieldType::Pointer output = patchMatch.GetNNField();
  PatchMatchHelpers::WriteNNField(output.GetPointer(), "nnfield.mha");

  return EXIT_SUCCESS;
}
