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

#ifndef PatchMatch_H
#define PatchMatch_H

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"
#include "itkVectorImage.h"

// Boost
#include <boost/signals2/signal.hpp>

// Submodules
#include <ITKHelpers/ITKHelpers.h>
#include <Mask/Mask.h>
#include <PatchComparison/PatchDistance.h>

// Custom
#include "AcceptanceTest.h"
#include "MatchSet.h"
#include "PatchMatchHelpers.h"
#include "Process.h"

/** This class computes a nearest neighbor field using the PatchMatch algorithm.
  * Note that this class does not actually need the image, as the acceptance test
  * and the patch distance functor already have the images that they need.*/
class PatchMatch
{
public:
  typedef itk::Image<MatchSet, 2> NNFieldType;

  /** Perform multiple iterations of propagation and random search.*/
  template<typename TPropagation, typename TRandomSearch>
  void Compute(TPropagation* const propagation,
               TRandomSearch* const randomSearch, Process* const processFunctor);

  /** Set the number of iterations to perform. */
  void SetIterations(const unsigned int iterations)
  {
    this->Iterations = iterations;
  }

  boost::signals2::signal<void (PatchMatchHelpers::NNFieldType*)> UpdatedSignal;

protected:

  /** The number of iterations to perform. */
  unsigned int Iterations;

  /** The number of iterations to perform. */
  PatchMatchHelpers::NNFieldType* nnField;

}; // end PatchMatch class

#include "PatchMatch.hpp"

#endif
