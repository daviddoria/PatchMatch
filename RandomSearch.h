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

#ifndef RandomSearch_H
#define RandomSearch_H

// ITK
#include "itkImage.h"

// Custom
#include "Match.h"
#include "NNField.h"

// Submodules
#include <Mask/Mask.h>

template <typename TImage, typename TPatchDistanceFunctor>
struct RandomSearch
{
  /** Look for a better matching patch in a region of decreasing radius. */
  void Search(NNFieldType* const nnField);

  /** Set the patch radius. */
  void SetPatchRadius(const unsigned int patchRadius)
  {
    this->PatchRadius = patchRadius;
  }

  /** Set the image on which to operate. */
  void SetImage(TImage* const image)
  {
    this->Image = image;
  }

  /** Set the functor used to compare patches. */
  void SetPatchDistanceFunctor(TPatchDistanceFunctor* const patchDistanceFunctor)
  {
    this->PatchDistanceFunctor = patchDistanceFunctor;
  }

  /** Get the functor used to compare patches. */
  TPatchDistanceFunctor* GetPatchDistanceFunctor() const
  {
    return this->PatchDistanceFunctor;
  }

  /** A signal to indicate that we accepted a new patch. */
  boost::signals2::signal<void (const itk::Index<2>& queryCenter, const itk::Index<2>& matchCenter, const float)> AcceptedSignal;

  /** Set if the results are truly randomized. */
  void SetRandom(const bool random)
  {
    this->Random = random;
  }

  void SetPixelsToProcess(const std::vector<itk::Index<2> >& pixelsToProcess)
  {
      this->PixelsToProcess = pixelsToProcess;
  }

  void SetValidPatchCentersImage(itk::Image<bool, 2>* const validPatchCentersImage)
  {
    this->ValidPatchCentersImage = validPatchCentersImage;
  }

private:
  /** The image on which to operate. */
  TImage* Image = nullptr;

  /** The patch radius we are using to define regions to compare. */
  unsigned int PatchRadius = 0;

  /** The functor used to compare patches. */
  TPatchDistanceFunctor* PatchDistanceFunctor = nullptr;

  /** Determine if the result should be randomized. This should only be false for testing purposes. */
  bool Random = true;

  /** Seed the random number generator if we are supposed to. */
  void InitializeRandomGenerator();

  /** Get a random pixel in the specified region. */
  itk::Index<2> GetRandomPixelInRegion(const itk::ImageRegion<2>& region);

  /** The fraction by which to reduce the search radius at each iteration,
      given by 'alpha' in PatchMatch paper section 3.2 */
  float RegionReductionRatio = 0.5;

  /** The pixels for which we are trying to randomly find a better match. */
  std::vector<itk::Index<2> > PixelsToProcess;

  /** An image where if a pixel is 'true', it is the center of a valid region. */
  typedef itk::Image<bool, 2> BoolImageType;
  BoolImageType* ValidPatchCentersImage;

  bool GetRandomValidRegion(const itk::ImageRegion<2>& region, itk::ImageRegion<2>& randomValidRegion);

};

#include "RandomSearch.hpp"

#endif
