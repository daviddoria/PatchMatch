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
template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
class PatchMatch
{
public:

  /** Choices for propagation. RASTER means that the top/left and bottom/right pixels will be
    * used in successive passes. INWARD means that all available ValidPropagation pixels will be used.
    * FORCE means that no acceptance test will be used and the region will be forcefully propagated. */
  enum PropagationStrategyEnum {RASTER, INWARD, FORCE};

  /** Constructor. */
  PatchMatch();

  /** Set the functor to use to compare patches. */
  void SetPatchDistanceFunctor(TPatchDistanceFunctor* const patchDistanceFunctor);

  /** Get the functor to use to compare patches. */
  TPatchDistanceFunctor* GetPatchDistanceFunctor();

  /** The type that is used to store the nearest neighbor field. */
  typedef itk::Image<Match, 2> MatchImageType;

  /** Perform multiple iterations of propagation and random search.*/
  virtual void Compute();

  /** Propagate good matches from specified offsets. In the traditional algorithm,
    * ForwardPropagation() and BackwardPropagation() call this function with "above and left"
    * and "below and right" offsets, respectively. */
  template <typename TNeighborFunctor, typename TProcessFunctor>
  void Propagation(const TNeighborFunctor neighborFunctor, TProcessFunctor processFunctor,
                   AcceptanceTest* acceptanceTest = NULL);

  /** Propagate good matches from above and from the left of the current pixel. */
  void ForwardPropagation();

  /** Propagate good matches from below and from the right of the current pixel. */
  void BackwardPropagation();

  /** Propagate best valid neighbor match to every invalid pixel. */
  void ForcePropagation();

  /** Propagate good matches from outside in. */
  void InwardPropagation();
  
  /** Search for a better match in several radii of the current pixel. */
  void RandomSearch();

  /** Get the Output. */
  MatchImageType* GetOutput();

  /** Set the number of iterations to perform. */
  void SetIterations(const unsigned int iterations);

  /** Set the radius of the patches to use. */
  void SetPatchRadius(const unsigned int patchRadius);

  /** Set the acceptance test to use. */
  void SetAcceptanceTest(TAcceptanceTest* const acceptanceTest);

  /** Get the acceptance test to use. */
  TAcceptanceTest* GetAcceptanceTest();

  /** Set the mask indicating where to take source patches from. Patches completely inside the valid
    * region of the source mask can be used as nearest neighbors. */
  void SetSourceMask(Mask* const mask);

  /** Set the mask indicating where to compute the NNField. Only compute the NN where
    * the target mask is Valid. */
  void SetTargetMask(Mask* const mask);

  /** Set the mask indicating which pixels (only valid pixels) can be propagated. */
  void SetAllowedPropagationMask(Mask* const mask);

  /** Get the mask indicating which pixels (only valid pixels) can be propagated. */
  Mask* GetAllowedPropagationMask();

  /** Set the choice of propagation strategy. */
  void SetPropagationStrategy(const PropagationStrategyEnum propagationStrategy);

  /** Set if the result should be randomized. This should only be false for testing purposes. */
  void SetRandom(const bool random);

  /** Write the valid pixels. */
  void WriteValidPixels(const std::string& fileName);

  /** Initialize the NNField. */
  void SetInitialNNField(MatchImageType* const initialization);

protected:

  /** Set the number of iterations to perform. */
  unsigned int Iterations;

  /** Set the radius of the patches to use. */
  unsigned int PatchRadius;

  /** The intermediate and final output. */
  MatchImageType::Pointer Output;

  /** This mask indicates where to take source patches from. Patches completely inside the valid
    * region of the source mask can be used as nearest neighbors. */
  Mask::Pointer SourceMask;

  /** This mask indicates where to compute the NN field. Only compute the NN where the
    * target mask is Valid. */
  Mask::Pointer TargetMask;

  /** The functor used to compare two patches. */
  TPatchDistanceFunctor* PatchDistanceFunctor;

  /** The bounding box of the source mask. */
  itk::ImageRegion<2> SourceMaskBoundingBox;

  /** The bounding box of the target mask. */
  itk::ImageRegion<2> TargetMaskBoundingBox;

  /** Determine if the result should be randomized. This should only be false for testing purposes. */
  bool Random;

  /** Only valid pixels in this mask can be propagated. */
  Mask::Pointer AllowedPropagationMask;

  /** The choice of propagation strategy. */
  PropagationStrategyEnum PropagationStrategy;

  TAcceptanceTest* AcceptanceTestFunctor;

}; // end PatchMatch class

#include "PatchMatch.hpp"

#endif
