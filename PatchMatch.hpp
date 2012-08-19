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
#include "RandomSearch.h"

template<typename TPropagation, typename TRandomSearch>
void PatchMatch::Compute(PatchMatchHelpers::NNFieldType* nnField,
                         TPropagation* const propagationFunctor,
                         TRandomSearch* const randomSearch,
                         Process* const processFunctor)
{
  assert(nnField);
  assert(propagationFunctor);
  assert(randomSearch);
  assert(processFunctor);

  // For the number of iterations specified, perform the appropriate propagation and then a random search
  for(unsigned int iteration = 0; iteration < this->Iterations; ++iteration)
  {
    //std::cout << "PatchMatch iteration " << iteration << std::endl;

    // We can propagate before random search because we are hoping the the random initialization gave us something good enough to propagate
    propagationFunctor->SetProcessFunctor(processFunctor);
    propagationFunctor->Propagate(nnField);

    UpdatedSignal(nnField);

    PatchMatchHelpers::WriteNNField(nnField,
                                    Helpers::GetSequentialFileName("AfterPropagation", iteration, "mha"));

    randomSearch->SetProcessFunctor(processFunctor);
    randomSearch->Search(nnField);

    UpdatedSignal(nnField);

    PatchMatchHelpers::WriteNNField(nnField,
                                    Helpers::GetSequentialFileName("AfterRandomSearch", iteration, "mha"));

    { // Debug only
    std::string sequentialFileName = Helpers::GetSequentialFileName("PatchMatch", iteration, "mha", 2);
    PatchMatchHelpers::WriteNNField(nnField, sequentialFileName);
    }
  } // end iteration loop

  std::cout << "PatchMatch finished." << std::endl;
}


#endif
