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

// STL
#include <functional>

// Eigen
#include <Eigen/Dense>

// ITK
#include "itkCovariantVector.h"
#include "itkImage.h"
#include "itkImageRegion.h"
#include "itkVectorImage.h"

// Submodules
#include "Mask/Mask.h"

#include "PatchComparison/PatchDistance.h"

struct Match
{
  itk::ImageRegion<2> Region;
  float Score;
};

template<typename TImage>
class PatchMatch
{
public:

  /** Constructor. */
  PatchMatch();

  /** Set the functor to use to compare patches. */
  void SetPatchDistanceFunctor(PatchDistance* const patchDistanceFunctor);

  /** The type that is used to store the nearest neighbor field. */
  typedef itk::Image<Match, 2> PMImageType;

  /** The type that is used to output the (X,Y,Score) image for inspection. */
  typedef itk::VectorImage<float, 2> CoordinateImageType;

  /** Do the real work. */
  void Compute(PMImageType* const initialization);

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

  /** Set the mask indicating where to ignore patches for comparison. */
  void SetSourceMask(Mask* const mask);

  /** Set the mask indicating where to compute the NNField. */
  void SetTargetMask(Mask* const mask);

  /** Get an image where the channels are (x component, y component, score) from the nearest neighbor field struct. */
  static void GetPatchCentersImage(PMImageType* const pmImage, CoordinateImageType* const output);

  /** Set the nearest neighbor field to exactly iself in the valid region, and random values in the hole region. */
  void RandomInit();

  /** Assume that hole pixels near the hole boundary will have best matching patches on the other side of the hole
   *  boundary in the valid region. */
  void BoundaryInit();

private:

  /** Set the nearest neighbor field to exactly iself in the valid region. */
  void InitKnownRegion();

  /** Set the number of iterations to perform. */
  unsigned int Iterations;

  /** Set the radius of the patches to use. */
  unsigned int PatchRadius;

  /** The intermediate and final output. */
  PMImageType::Pointer Output;

  /** The image to operate on. */
  typename TImage::Pointer Image;

  /** This mask indicates where to take source patches from. */
  Mask::Pointer SourceMask;

  /** This mask indicates where to compute the NN field. */
  Mask::Pointer TargetMask;

  /** The functor used to compare two patches. */
  PatchDistance* PatchDistanceFunctor;

  /** The bounding box of the source mask. */
  itk::ImageRegion<2> SourceMaskBoundingBox;

  /** The bounding box of the target mask. */
  itk::ImageRegion<2> TargetMaskBoundingBox;
};

#include "PatchMatch.hpp"

#endif
