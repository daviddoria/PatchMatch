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

#ifndef PatchMatchRings_HPP
#define PatchMatchRings_HPP

#include "PatchMatchRings.h"

template <typename TImage>
PatchMatchRings<TImage>::PatchMatchRings() : PatchMatch<TImage>()
{
}

template <typename TImage>
bool PatchMatchRings<TImage>::AllowPropagationFrom(const itk::Index<2>& potentialPropagationPixel)
{
//   if(this->SourceMask->IsValid(potentialPropagationPixel) ||
//      this->TargetMask->IsValid(potentialPropagationPixel))
//   {
//     return true;
//   }
  if(this->AllowedPropagationMask->IsValid(potentialPropagationPixel))
  {
    return true;
  }

  return false;
}

template <typename TImage>
void PatchMatchRings<TImage>::Compute(typename PatchMatch<TImage>::PMImageType* const initialization)
{
  ITKHelpers::WriteImage(this->TargetMask.GetPointer(), "OriginalTargetMask.png");

  // Save the original mask, as we will be modifying the internal masks below
  Mask::Pointer currentTargetMask = Mask::New();
  currentTargetMask->DeepCopyFrom(this->TargetMask);

  // Initialize
  this->AllowedPropagationMask->DeepCopyFrom(this->SourceMask);

  unsigned int ringCounter = 0;
  while(currentTargetMask->HasValidPixels())
  {
//     {
//     std::stringstream ssCurrentTargetMask;
//     ssCurrentTargetMask << "CurrentTargetMask_" << Helpers::ZeroPad(ringCounter, 4) << ".png";
//     ITKHelpers::WriteImage(currentTargetMask.GetPointer(), ssCurrentTargetMask.str());
//     }

    // Get the outside boundary of the target region
    Mask::BoundaryImageType::Pointer boundaryImage = Mask::BoundaryImageType::New();
    Mask::BoundaryImageType::PixelType boundaryColor = 255;
    currentTargetMask->FindBoundary(boundaryImage, Mask::HOLE, boundaryColor);

//     {
//     std::stringstream ssBoundary;
//     ssBoundary << "Boundary_" << Helpers::ZeroPad(ringCounter, 4) << ".png";
//     ITKHelpers::WriteImage(boundaryImage.GetPointer(), ssBoundary.str());
//     }

    // Create a mask of just the boundary
    Mask::Pointer boundaryMask = Mask::New();
    Mask::BoundaryImageType::PixelType holeColor = boundaryColor;
    boundaryMask->CreateFromImage(boundaryImage.GetPointer(), holeColor);
    boundaryMask->InvertData(); // Make the thin boundary the only valid pixels

//     { // debug only
//     std::stringstream ssBoundaryMask;
//     ssBoundaryMask << "BoundaryMask_" << Helpers::ZeroPad(ringCounter, 4) << ".png";
//     ITKHelpers::WriteImage(boundaryImage.GetPointer(), ssBoundaryMask.str());
//     }

    // Set the mask to use in the PatchMatch algorithm
    this->TargetMask->DeepCopyFrom(boundaryMask);
    if(ringCounter == 0)
    {
      PatchMatch<TImage>::Compute(NULL);
    }
    else
    {
      PatchMatch<TImage>::Compute(this->GetOutput());
    }

    { // debug only
    typename PatchMatch<TImage>::CoordinateImageType::Pointer coordinateImage = PatchMatch<TImage>::CoordinateImageType::New();
    GetPatchCentersImage(this->GetOutput(), coordinateImage);
    std::stringstream ssOutput;
    ssOutput << "NNField_" << Helpers::ZeroPad(ringCounter, 3) << ".mha";
    ITKHelpers::WriteImage(coordinateImage.GetPointer(), ssOutput.str());
    }

    // Update to use the already computed field region - TODO: This is wrong, but works for simple cases (where the source region is the complement of the target region)
    // It should actually be an XOR of the currentTargetMask and the SourceMask or something like that.
    this->AllowedPropagationMask->DeepCopyFrom(currentTargetMask);
    
    // Reduce the size of the target region (we "enlarge the hole", because the "hole" is considered the valid part of the target mask)
    unsigned int kernelRadius = 1;
    currentTargetMask->ExpandHole(kernelRadius);

    ringCounter++;
  }
}

#endif
