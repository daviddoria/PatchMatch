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

// Boost
#include <boost/signals2/signal.hpp>

// Submodules
#include <ITKHelpers/ITKHelpers.h>
#include <Mask/Mask.h>
#include <PatchComparison/PatchDistance.h>

// Custom
#include "Match.h"
#include "NNField.h"

/** This class computes a nearest neighbor field using the PatchMatch algorithm.
  * Note that this class does not actually need the image, as the acceptance test
  * and the patch distance functor already have the images that they need.*/
template <typename TImage, typename TPropagation, typename TRandomSearch>
class PatchMatch
{
public:

  /** Perform multiple iterations of propagation and random search.*/
  void Compute();

  /** Set the number of iterations to perform. */
  void SetIterations(const unsigned int iterations)
  {
    this->Iterations = iterations;
  }

  /** Set the patch radius. */
  void SetPatchRadius(const unsigned int patchRadius)
  {
    this->PatchRadius = patchRadius;
  }

  /** Set the propagation functor. */
  void SetPropagationFunctor(TPropagation* const propagationFunctor)
  {
      this->PropagationFunctor = propagationFunctor;
  }

  /** Get the propagation functor. */
  TPropagation* GetPropagationFunctor()
  {
      return this->PropagationFunctor;
  }

  /** Set the random search functor. */
  void SetRandomSearchFunctor(TRandomSearch* const randomSearchFunctor)
  {
      this->RandomSearchFunctor = randomSearchFunctor;
  }

  /** Get the random search functor. */
  TRandomSearch* GetRandomSearchFunctor()
  {
      return this->RandomSearchFunctor;
  }

  /** Set the image. */
  void SetImage(TImage* const image)
  {
      this->Image = image;
  }

  /** Set the image. */
  NNFieldType* GetNNField()
  {
      return this->NNField;
  }

  /** Get a random region in the image. */
  itk::ImageRegion<2> GetRandomRegion();

  boost::signals2::signal<void (NNFieldType*)> UpdatedSignal;

  void SetTargetPixels(const std::vector<itk::Index<2> > targetPixels)
  {
    this->TargetPixels = targetPixels;
  }

  void SetValidPatchCentersImage(itk::Image<bool, 2>* const validPatchCentersImage)
  {
    this->ValidPatchCentersImage = validPatchCentersImage;
    CorrectValidPatchCentersImage();
  }

protected:

  /** The number of iterations to perform. */
  unsigned int Iterations = 5;

  /** The nearest neighbor field. */
  NNFieldType::Pointer NNField = NNFieldType::New();

  /** Randomly initialize the NNField. */
  void RandomlyInitializeNNField();

  /** The radius of patches to compare. (Patch side length = 2*radius + 1)*/
  unsigned int PatchRadius = 5;

  /** Set the propagation functor. */
  TPropagation* PropagationFunctor = nullptr;

  /** Set the random search functor. */
  TRandomSearch* RandomSearchFunctor = nullptr;

  /** The image for which to compute the NNField. */
  TImage* Image = nullptr;

  /** The pixel indices at which to compute the NNField. */
  std::vector<itk::Index<2> > TargetPixels;

  /** An image where if a pixel is 'true', it is the center of a valid region. */
  typedef itk::Image<bool, 2> BoolImageType;
  BoolImageType* ValidPatchCentersImage = nullptr;

  /** Since the ValidPatchCentersImage can be constructed externally, this function ensures
    * that the pixels marked as valid are the centers of patches of radius PatchRadius that are fully inside the image. */
  void CorrectValidPatchCentersImage();

}; // end PatchMatch class

#include "PatchMatch.hpp"

#endif
