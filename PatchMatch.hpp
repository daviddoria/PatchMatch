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

#ifndef PATCHMATCH_HPP
#define PATCHMATCH_HPP

#include "PatchMatch.h"

// Submodules
#include <ITKHelpers/ITKHelpers.h>
#include <ITKHelpers/ITKTypeTraits.h>

#include <Mask/MaskOperations.h>

#include <PatchComparison/SSD.h>

#include <Histogram/Histogram.h>

// ITK
#include "itkImageRegionReverseIterator.h"

// STL
#include <algorithm>
#include <ctime>

// Custom
#include "Neighbors.h"
#include "AcceptanceTestAcceptAll.h"
#include "PatchMatchHelpers.h"
#include "Process.h"

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
PatchMatch() : PatchRadius(0), PatchDistanceFunctor(NULL),
               Random(true), AllowedPropagationMask(NULL),
               AcceptanceTest(NULL),
               Propagator(NULL)
{
  this->Output = MatchImageType::New();

  this->SourceMask = Mask::New();
  this->TargetMask = Mask::New();
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::Compute()
{
  if(this->Random)
  {
    srand(time(NULL));
  }
  else
  {
    srand(0);
  }

  assert(this->SourceMask);
  assert(this->TargetMask);
  assert(this->SourceMask->GetLargestPossibleRegion().GetSize()[0] > 0);
  assert(this->TargetMask->GetLargestPossibleRegion().GetSize()[0] > 0);
  assert(this->SourceMask->GetLargestPossibleRegion().GetSize() ==
         this->TargetMask->GetLargestPossibleRegion().GetSize());

  { // Debug only
  ITKHelpers::WriteImage(this->TargetMask.GetPointer(), "PatchMatch_TargetMask.png");
  ITKHelpers::WriteImage(this->SourceMask.GetPointer(), "PatchMatch_SourceMask.png");
  ITKHelpers::WriteImage(this->AllowedPropagationMask.GetPointer(), "PatchMatch_PropagationMask.png");
  }

  this->Output->SetRegions(this->SourceMask->GetLargestPossibleRegion());
  this->Output->Allocate();

  // Initialize this so that we propagate forward first
  // (the propagation direction toggles at each iteration)
  bool forwardPropagation = true;

  // For the number of iterations specified, perform the appropriate propagation and then a random search
  for(unsigned int iteration = 0; iteration < this->Iterations; ++iteration)
  {
    std::cout << "PatchMatch iteration " << iteration << std::endl;

    ProcessTargetPixels processTargetPixelsFunctor;
    std::vector<itk::Index<2> > pixelsToProcess =
      processTargetPixelsFunctor.GetPixelsToProcess(this->TargetMask);

    this->Propagator->SetProcessFunctor(&processTargetPixelsFunctor);
    this->Propagator->SetAcceptanceTest(this->AcceptanceTest);

    this->Propagator->Propagate(this->Output);

    PatchMatchHelpers::WriteNNField(this->Output.GetPointer(), "AfterPropagation.mha");

    // Switch the propagation direction for the next iteration
    forwardPropagation = !forwardPropagation;

    RandomSearch->Search(this->Output, pixelsToProcess);

    PatchMatchHelpers::WriteNNField(this->Output.GetPointer(), "AfterRandomSearch.mha");

    { // Debug only
    std::string sequentialFileName = Helpers::GetSequentialFileName("PatchMatch", iteration, "mha", 2);
    PatchMatchHelpers::WriteNNField(this->Output.GetPointer(), sequentialFileName);
    }
  } // end iteration loop

  // As a final pass, propagate to all pixels which were not set to a valid nearest neighbor
//   std::cout << "Before ForcePropagation() there are "
//             << PatchMatchHelpers::CountInvalidPixels(this->Output.GetPointer(), this->TargetMask)
//             << " invalid pixels." << std::endl;
  //ForcePropagation();
//   std::cout << "After ForcePropagation() there are "
//             << PatchMatchHelpers::CountInvalidPixels(this->Output.GetPointer(), this->TargetMask)
//             << " invalid pixels." << std::endl;

  std::cout << "PatchMatch finished." << std::endl;
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
typename PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::MatchImageType*
PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::GetOutput()
{
  return this->Output;
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
SetIterations(const unsigned int iterations)
{
  this->Iterations = iterations;
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
SetPatchRadius(const unsigned int patchRadius)
{
  this->PatchRadius = patchRadius;
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
SetSourceMask(Mask* const mask)
{
  this->SourceMask->DeepCopyFrom(mask);
  this->SourceMaskBoundingBox = MaskOperations::ComputeValidBoundingBox(this->SourceMask);
  //std::cout << "SourceMaskBoundingBox: " << this->SourceMaskBoundingBox << std::endl;
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
SetTargetMask(Mask* const mask)
{
  this->TargetMask->DeepCopyFrom(mask);
  this->TargetMaskBoundingBox = MaskOperations::ComputeValidBoundingBox(this->TargetMask);
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
SetAllowedPropagationMask(Mask* const mask)
{
  if(!this->AllowedPropagationMask)
  {
    this->AllowedPropagationMask = Mask::New();
  }

  this->AllowedPropagationMask->DeepCopyFrom(mask);
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
TPatchDistance* PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
GetPatchDistanceFunctor()
{
  return this->PatchDistanceFunctor;
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest,
                TPropagation, TRandomSearch>::
SetRandomSearchFunctor(TRandomSearch* const randomSearchFunctor)
{

}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
SetPatchDistanceFunctor(TPatchDistance* const patchDistanceFunctor)
{
  this->PatchDistanceFunctor = patchDistanceFunctor;
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::ForcePropagation()
{
  std::cout << "ForcePropagation()" << std::endl;
  AcceptanceTestAcceptAll acceptanceTest;

  //AllNeighbors neighborFunctor;
  ValidTargetNeighbors neighborFunctor(this->Output, this->TargetMask);

  // Process the pixels that are invalid
  auto processInvalid = [this](const itk::Index<2>& queryIndex)
  {
    if(!this->Output->GetPixel(queryIndex).IsValid())
    {
      return true;
    }
    return false;
  };

  Propagation(neighborFunctor, processInvalid, &acceptanceTest);
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::InwardPropagation()
{
  AllowedPropagationNeighbors neighborFunctor(this->AllowedPropagationMask, this->TargetMask);

  auto processAll = [](const itk::Index<2>& queryIndex) {
      return true;
  };
  Propagation(neighborFunctor, processAll);
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::ForwardPropagation()
{
  ForwardPropagationNeighbors neighborFunctor;

  auto processAll = [](const itk::Index<2>& queryIndex) {
      return true;
  };
  Propagation(neighborFunctor, processAll);
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::BackwardPropagation()
{
  BackwardPropagationNeighbors neighborFunctor;

  auto processAll = [](const itk::Index<2>& queryIndex) {
      return true;
  };
  Propagation(neighborFunctor, processAll);
}


template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
SetInitialNNField(MatchImageType* const initialMatchImage)
{
  ITKHelpers::DeepCopy(initialMatchImage, this->Output.GetPointer());
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
SetAcceptanceTest(TAcceptanceTest* const acceptanceTest)
{
  this->AcceptanceTest = acceptanceTest;
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
SetRandom(const bool random)
{
  this->Random = random;
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
Mask* PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
GetAllowedPropagationMask()
{
  return this->AllowedPropagationMask;
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
WriteValidPixels(const std::string& fileName)
{
  typedef itk::Image<unsigned char> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(this->Output->GetLargestPossibleRegion());
  image->Allocate();
  image->FillBuffer(0);

  itk::ImageRegionConstIterator<MatchImageType> imageIterator(this->Output,
                                                              this->Output->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
  {
    if(imageIterator.Get().IsValid())
    {
      image->SetPixel(imageIterator.GetIndex(), 255);
    }

    ++imageIterator;
  }

  ITKHelpers::WriteImage(image.GetPointer(), fileName);
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
void PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::
SetPropagationFunctor(TPropagation* propagator)
{
  this->Propagator = propagator;
}

template<typename TPatchDistance, typename TAcceptanceTest,
         typename TPropagation, typename TRandomSearch>
TAcceptanceTest* PatchMatch<TPatchDistance, TAcceptanceTest, TPropagation, TRandomSearch>::GetAcceptanceTest()
{
  return this->AcceptanceTest;
}

#endif
