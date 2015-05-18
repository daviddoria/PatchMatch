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
#include <Mask/Mask.h>
#include <Mask/ITKHelpers/ITKHelpers.h>
#include <PatchComparison/SSD.h>

// Custom
#include "PatchMatch.h"
#include "Propagator.h"
#include "RandomSearch.h"

int main(int argc, char*argv[])
{
  // Verify arguments
  if(argc < 5)
  {
    std::cerr << "Required arguments: image mask patchRadius output" << std::endl;
    return EXIT_FAILURE;
  }

  // Parse arguments
  std::stringstream ss;
  for(int i = 1; i < argc; ++i)
  {
    ss << argv[i] << " ";
  }
  std::string imageFilename;
  std::string maskFilename;
  unsigned int patchRadius;
  std::string outputFilename;

  ss >> imageFilename >> maskFilename >> patchRadius >> outputFilename;

  // Output arguments
  std::cout << "imageFilename: " << imageFilename << std::endl;
  std::cout << "maskFilename: " << maskFilename << std::endl;
  std::cout << "patchRadius: " << patchRadius << std::endl;
  std::cout << "outputFilename: " << outputFilename << std::endl;

  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;

  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  ImageType* image = imageReader->GetOutput();

  // Read the source mask
  Mask::Pointer sourceMask = Mask::New();
  sourceMask->Read(maskFilename);

  // Read or create the target mask
  Mask::Pointer targetMask = Mask::New();

  // Compute the entire NN-field
//   targetMask->SetRegions(sourceMask->GetLargestPossibleRegion());
//   targetMask->Allocate();
//   ITKHelpers::SetImageToConstant(targetMask.GetPointer(), targetMask->GetValidValue());

  // Only compute the NN-field in the hole
  targetMask->Read(maskFilename);
  targetMask->InvertData();

  typedef SSD<ImageType> PatchDistanceFunctorType;
  PatchDistanceFunctorType* patchDistanceFunctor = new PatchDistanceFunctorType;
  patchDistanceFunctor->SetImage(image);

  typedef Propagator<PatchDistanceFunctorType> PropagatorType;
  PropagatorType* propagator = new PropagatorType;

  typedef RandomSearch<ImageType, PatchDistanceFunctorType> RandomSearchType;
  RandomSearchType* randomSearchFunctor = new RandomSearchType;
  randomSearchFunctor->SetPatchDistanceFunctor(patchDistanceFunctor);

  typedef PatchMatch<ImageType,
                     PropagatorType, RandomSearchType> PatchMatchType;
  PatchMatchType patchMatch;
  patchMatch.SetPatchRadius(patchRadius);
  patchMatch.SetPropagationFunctor(propagator);
  patchMatch.SetRandomSearchFunctor(randomSearchFunctor);

  patchMatch.Compute();

  PatchMatchHelpers::WriteNNField(patchMatch.GetNNField(), outputFilename);

  return EXIT_SUCCESS;
}
