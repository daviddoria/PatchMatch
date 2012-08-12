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

#ifndef PATCHMATCH_H
#define PATCHMATCH_H

// ITK
#include "itkImage.h"
#include "itkImageRegion.h"
#include "itkVectorImage.h"

// Submodules
#include <ITKHelpers/ITKHelpers.h>
#include <Mask/Mask.h>
#include <PatchComparison/PatchDistance.h>

// Custom
#include "Match.h"
#include "AcceptanceTest.h"

/** This class computes a nearest neighbor field using the PatchMatch algorithm.
  * Note that this class does not actually need the image, as the acceptance test
  * and the patch distance functor already have the images that they need.*/
class PatchMatch
{
public:

  /** The type that is used to store the nearest neighbor field. */
  typedef itk::Image<Match, 2> NNFieldType;

  /** Perform multiple iterations of propagation and random search.*/
  template<typename TPropagation, typename TRandomSearch, typename TProcessFunctor>
  void Compute(NNFieldType* nnField, TPropagation* const propagation,
               TRandomSearch* const randomSearch, TProcessFunctor* const processFunctor);

  /** Set the number of iterations to perform. */
  void SetIterations(const unsigned int iterations)
  {
    this->Iterations = iterations;
  }

protected:

  /** Set the number of iterations to perform. */
  unsigned int Iterations;

  /** Determine if the result should be randomized. This should only be false for testing purposes. */
  bool Random;

  /** Seed the random number generator if we are supposed to. */
  void InitRandom();

}; // end PatchMatch class

#include "PatchMatch.hpp"

#endif
