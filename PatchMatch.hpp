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
#include "PatchMatchHelpers.h"
#include "RandomSearch.h"

//template<typename TImage, typename TPropagation, typename TRandomSearch>
//PatchMatch<TImage, TPropagation, TRandomSearch>::PatchMatch()
//{
//    this->NNField = NNFieldType::New();
//}

template<typename TImage, typename TPropagation, typename TRandomSearch>
void PatchMatch<TImage, TPropagation, TRandomSearch>::Compute()
{
  assert(this->PropagationFunctor);
  assert(this->RandomSearchFunctor);

  RandomlyInitializeNNField();

  // For the number of iterations specified, perform the appropriate propagation and then a random search
  for(unsigned int iteration = 0; iteration < this->Iterations; ++iteration)
  {
    std::cout << "PatchMatch iteration " << iteration << std::endl;

    // We can propagate before random search because we are hoping the the random initialization gave us something good enough to propagate
    this->PropagationFunctor->Propagate(this->NNField);

    UpdatedSignal(this->NNField);

    PatchMatchHelpers::WriteNNField(this->NNField.GetPointer(),
                                    Helpers::GetSequentialFileName("AfterPropagation", iteration, "mha"));

    this->RandomSearchFunctor->Search(this->NNField);

    UpdatedSignal(this->NNField);

    PatchMatchHelpers::WriteNNField(this->NNField.GetPointer(),
                                    Helpers::GetSequentialFileName("AfterRandomSearch", iteration, "mha"));

    { // Debug only
    std::string sequentialFileName = Helpers::GetSequentialFileName("PatchMatch", iteration, "mha", 2);
    PatchMatchHelpers::WriteNNField(this->NNField.GetPointer(), sequentialFileName);
    }
  } // end iteration loop

  std::cout << "PatchMatch finished." << std::endl;
}

template<typename TImage, typename TPropagation, typename TRandomSearch>
void PatchMatch<TImage, TPropagation, TRandomSearch>::RandomlyInitializeNNField()
{
    itk::ImageRegion<2> internalRegion = ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(),
                                              this->PatchRadius);

    this->NNField->SetRegions(this->Image->GetLargestPossibleRegion());
    this->NNField->Allocate();

    itk::ImageRegionIteratorWithIndex<NNFieldType> nnFieldIterator(this->NNField, internalRegion);

    while(!nnFieldIterator.IsAtEnd())
    {
      itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(nnFieldIterator.GetIndex(), this->PatchRadius);
      itk::ImageRegion<2> randomRegion = GetRandomRegion();
      Match randomMatch;
      randomMatch.SetRegion(randomRegion);
      randomMatch.SetScore(this->RandomSearchFunctor->GetPatchDistanceFunctor()->Distance(randomRegion, targetRegion));

      nnFieldIterator.Set(randomMatch);
      ++nnFieldIterator;
    }
}

template<typename TImage, typename TPropagation, typename TRandomSearch>
itk::ImageRegion<2> PatchMatch<TImage, TPropagation, TRandomSearch>::GetRandomRegion()
{
    itk::ImageRegion<2> internalRegion = ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(),
                                              this->PatchRadius);

    itk::Index<2> randomPixel;
    randomPixel[0] = Helpers::RandomInt(internalRegion.GetIndex()[0], internalRegion.GetIndex()[0] + internalRegion.GetSize()[0]);
    randomPixel[1] = Helpers::RandomInt(internalRegion.GetIndex()[1], internalRegion.GetIndex()[1] + internalRegion.GetSize()[1]);

    itk::ImageRegion<2> randomRegion = ITKHelpers::GetRegionInRadiusAroundPixel(randomPixel, this->PatchRadius);

    return randomRegion;
}

#endif
