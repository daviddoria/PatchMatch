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
#include <Mask/Mask.h>
#include <PatchComparison/PatchDistance.h>

/** A simple container to pair a region with its patch difference value/score. */
struct Match
{
  itk::ImageRegion<2> Region;
  float Score;

  static constexpr float InvalidScore = std::numeric_limits< float >::quiet_NaN();
  bool IsValid()
  {
    if(Helpers::IsNaN(this->Score))
    {
      return false;
    }

    if(this->Region.GetSize()[0] == 0 || this->Region.GetSize()[1] == 0)
    {
      return false;
    }

    return true;
  }
};

/** This class computes a nearest neighbor field using the PatchMatch algorithm. */
template<typename TImage>
class PatchMatch
{
public:

  /** Choices for initialization. */
  enum InitializationStrategyEnum {RANDOM, BOUNDARY};

  /** Choices for AddIfBetter. */
  enum AddIfBetterStrategyEnum {SSD, HISTOGRAM};

  static constexpr float DownsampleFactor = 0.5f;
  
  /** Constructor. */
  PatchMatch();

  /** Set the functor to use to compare patches. */
  void SetPatchDistanceFunctor(PatchDistance<TImage>* const patchDistanceFunctor);

  /** The type that is used to store the nearest neighbor field. */
  typedef itk::Image<Match, 2> PMImageType;

  /** The type that is used to output the (X,Y,Score) image for inspection. */
  typedef itk::Image<itk::CovariantVector<float, 3>, 2> CoordinateImageType;

  /** Initialize internally.*/
  virtual void AutomaticCompute(PMImageType* const initialization);
  
  /** Perform multiple iterations of propagation and random search (do the real work).
    * 'initialization' can come from a previous iteration of an algorithm like BDSInpainting. If
    * 'initialization' is null, this function computes an initialization using one of the algorithms
    * provided by this class (RandomInit() or BoundaryInit() ).*/
  virtual void Compute(PMImageType* const initialization);

  /** Propagate good matches from specified offsets. In the traditional algorithm,
    * ForwardPropagation() and BackwardPropagation() call this function with "above and left"
    * and "below and right" offsets, respectively. */
  void Propagation(const std::vector<itk::Offset<2> >& offsets);

  /** Propagate good matches from above and from the left of the current pixel. */
  void ForwardPropagation();

  /** Propagate good matches from below and from the right of the current pixel. */
  void BackwardPropagation();

  /** Search for a better match in several radii of the current pixel. */
  void RandomSearch();

  /** Get the Output. */
  PMImageType* GetOutput();

  /** Set the number of iterations to perform. */
  void SetIterations(const unsigned int iterations);

  /** Set the radius of the patches to use. */
  void SetPatchRadius(const unsigned int patchRadius);

  /** Set the image to operate on. */
  void SetImage(TImage* const image);

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

  /** Get an image where the channels are (x component, y component, score) from the nearest
    * neighbor field struct. */
  static void GetPatchCentersImage(PMImageType* const pmImage, CoordinateImageType* const output);

  /** Set the nearest neighbor of each patch entirely in the source region to itself . */
  void InitKnownRegion();

  /** Set the nearest neighbor field to exactly iself in the valid region, and random
    * values in the hole region. */
  void RandomInit();

  /** Set the NN to random pixels that have a reasonable histogram difference. */
  void RandomInitWithHistogramTest();

  /** Assume that hole pixels near the hole boundary will have best matching patches on
    * the other side of the hole
    * boundary in the valid region. */
  void BoundaryInit();

  /** Replace the best match if necessary. In this class, 'match' simply replaces
    * the current best match if it is better.
    * In subclasses (e.g. GeneralizedPatchMatch), this will
    * add the 'match' to the list of nearest matches if it is better than the worst match.
    * Returns true if the 'match' was added. */
  bool AddIfBetter(const itk::Index<2>& index, const Match& match);

  bool AddIfBetterSSD(const itk::Index<2>& index, const Match& match);
  bool AddIfBetterHistogram(const itk::Index<2>& index, const Match& match);

  /** Set the choice of initialization strategy. */
  void SetInitializationStrategy(const InitializationStrategyEnum initializationStrategy);

  void SetAddIfBetterStrategy(const AddIfBetterStrategyEnum addIfBetterStrategy);
  
  /** Set if the result should be randomized. This should only be false for testing purposes. */
  void SetRandom(const bool random);

  /** Write the valid pixels. */
  void WriteValidPixels(const std::string& fileName);

  void SetHistogramAcceptanceThreshold(const float histogramAcceptanceThreshold);
  
protected:

  /** Set the number of iterations to perform. */
  unsigned int Iterations;

  /** Set the radius of the patches to use. */
  unsigned int PatchRadius;

  /** The intermediate and final output. */
  PMImageType::Pointer Output;

  /** The image to operate on. */
  typename TImage::Pointer Image;

  /** The downsampled image to use for histogram computations. */
  typename TImage::Pointer DownsampledImage;
  
  /** This mask indicates where to take source patches from. Patches completely inside the valid
    * region of the source mask can be used as nearest neighbors. */
  Mask::Pointer SourceMask;

  /** This mask indicates where to compute the NN field. Only compute the NN where the
    * target mask is Valid. */
  Mask::Pointer TargetMask;

  /** The functor used to compare two patches. */
  PatchDistance<TImage>* PatchDistanceFunctor;

  /** The bounding box of the source mask. */
  itk::ImageRegion<2> SourceMaskBoundingBox;

  /** The bounding box of the target mask. */
  itk::ImageRegion<2> TargetMaskBoundingBox;

  /** The choice of initialization strategy. */
  InitializationStrategyEnum InitializationStrategy;

  /** Determine if information can be propagated from a specified pixel. */
  virtual bool AllowPropagationFrom(const itk::Index<2>& potentialPropagationPixel);

  /** Determine if the result should be randomized. This should only be false for testing purposes. */
  bool Random;

  /** Only valid pixels in this mask can be propagated. */
  Mask::Pointer AllowedPropagationMask;

  AddIfBetterStrategyEnum AddIfBetterStrategy;

  float HistogramAcceptanceThreshold;

  typedef itk::VectorImage<float, 2> HSVImageType;
  HSVImageType::Pointer HSVImage;

};

#include "PatchMatch.hpp"

#endif
