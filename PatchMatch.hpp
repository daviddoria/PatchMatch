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


template<typename TPropagation, typename TRandomSearch>
void PatchMatch::Compute(NNFieldType* nnField,
                         TPropagation propagationFunctor, TRandomSearch randomSearch)
{
  if(this->Random)
  {
    srand(time(NULL));
  }
  else
  {
    srand(0);
  }

  assert(nnField);
  assert(this->SourceMask);
  assert(this->TargetMask);

  // These simply test that all of the image data are the same size.
  assert(this->SourceMask->GetLargestPossibleRegion().GetSize() ==
         this->TargetMask->GetLargestPossibleRegion().GetSize());
  assert(nnField->GetLargestPossibleRegion().GetSize() ==
         this->TargetMask->GetLargestPossibleRegion().GetSize());

  itk::ImageRegion<2> sourceMaskBoundingBox;
  sourceMaskBoundingBox = MaskOperations::ComputeValidBoundingBox(this->SourceMask);

  itk::ImageRegion<2> targetMaskBoundingBox;
  targetMaskBoundingBox = MaskOperations::ComputeValidBoundingBox(this->TargetMask);

  { // Debug only
  ITKHelpers::WriteImage(this->TargetMask.GetPointer(), "PatchMatch_TargetMask.png");
  ITKHelpers::WriteImage(this->SourceMask.GetPointer(), "PatchMatch_SourceMask.png");
  }

  // For the number of iterations specified, perform the appropriate propagation and then a random search
  for(unsigned int iteration = 0; iteration < this->Iterations; ++iteration)
  {
    std::cout << "PatchMatch iteration " << iteration << std::endl;

    ProcessValidMaskPixels processValidMaskPixelsFunctor(this->TargetMask);
    std::vector<itk::Index<2> > pixelsToProcess =
      processValidMaskPixelsFunctor.GetPixelsToProcess();

    PatchMatchHelpers::WriteNNField(nnField, "AfterPropagation.mha");

    randomSearch.Search(nnField, pixelsToProcess);

    PatchMatchHelpers::WriteNNField(nnField, "AfterRandomSearch.mha");

    { // Debug only
    std::string sequentialFileName = Helpers::GetSequentialFileName("PatchMatch", iteration, "mha", 2);
    PatchMatchHelpers::WriteNNField(nnField, sequentialFileName);
    }
  } // end iteration loop

  std::cout << "PatchMatch finished." << std::endl;
}

#endif
