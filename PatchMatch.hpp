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
#include <ctime>

// Custom
#include "Neighbors.h"
#include "AcceptanceTestAcceptAll.h"
#include "PatchMatchHelpers.h"

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::PatchMatch() : PatchRadius(0), PatchDistanceFunctor(NULL),
                                   Random(true),
                                   AllowedPropagationMask(NULL),
                                   PropagationStrategy(RASTER),
                                   AcceptanceTestFunctor(NULL)
{
  this->Output = MatchImageType::New();

  this->SourceMask = Mask::New();
  this->TargetMask = Mask::New();
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::Compute()
{
  if(this->Random)
  {
    srand(time(NULL));
  }
  else
  {
    srand(0);
  }

  assert(this->SourceMask);
  assert(this->TargetMask);
  assert(this->SourceMask->GetLargestPossibleRegion().GetSize()[0] > 0);
  assert(this->TargetMask->GetLargestPossibleRegion().GetSize()[0] > 0);
  assert(this->SourceMask->GetLargestPossibleRegion().GetSize() ==
         this->TargetMask->GetLargestPossibleRegion().GetSize());

  { // Debug only
  ITKHelpers::WriteImage(this->TargetMask.GetPointer(), "PatchMatch_TargetMask.png");
  ITKHelpers::WriteImage(this->SourceMask.GetPointer(), "PatchMatch_SourceMask.png");
  ITKHelpers::WriteImage(this->AllowedPropagationMask.GetPointer(), "PatchMatch_PropagationMask.png");
  }

  this->Output->SetRegions(this->SourceMask->GetLargestPossibleRegion());
  this->Output->Allocate();

  // Initialize this so that we propagate forward first
  // (the propagation direction toggles at each iteration)
  bool forwardPropagation = true;

  // For the number of iterations specified, perform the appropriate propagation and then a random search
  for(unsigned int iteration = 0; iteration < this->Iterations; ++iteration)
  {
    std::cout << "PatchMatch iteration " << iteration << std::endl;

    if(this->PropagationStrategy == RASTER)
    {
      if(forwardPropagation)
      {
        ForwardPropagation();
      }
      else
      {
        BackwardPropagation();
      }
    }
    else if(this->PropagationStrategy == INWARD)
    {
      InwardPropagation();
    }
    else
    {
      throw std::runtime_error("Invalid propagation strategy specified!");
    }

    // Switch the propagation direction for the next iteration
    forwardPropagation = !forwardPropagation;

    RandomSearch();

    { // Debug only
    PatchMatchHelpers::CoordinateImageType::Pointer temp = PatchMatchHelpers::CoordinateImageType::New();
    PatchMatchHelpers::GetPatchCentersImage(this->Output.GetPointer(), temp.GetPointer());
    ITKHelpers::WriteSequentialImage(temp.GetPointer(), "PatchMatch", iteration, 2, "mha");
    }
  } // end iteration loop

  // As a final pass, propagate to all pixels which were not set to a valid nearest neighbor
  std::cout << "Before ForcePropagation() there are " << CountInvalidPixels() << " invalid pixels." << std::endl;
  ForcePropagation();
  std::cout << "After ForcePropagation() there are " << CountInvalidPixels() << " invalid pixels." << std::endl;

  std::cout << "PatchMatch finished." << std::endl;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
typename PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::MatchImageType* PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::GetOutput()
{
  return this->Output;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::SetIterations(const unsigned int iterations)
{
  this->Iterations = iterations;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::SetPatchRadius(const unsigned int patchRadius)
{
  this->PatchRadius = patchRadius;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::SetSourceMask(Mask* const mask)
{
  this->SourceMask->DeepCopyFrom(mask);
  this->SourceMaskBoundingBox = MaskOperations::ComputeValidBoundingBox(this->SourceMask);
  //std::cout << "SourceMaskBoundingBox: " << this->SourceMaskBoundingBox << std::endl;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::SetTargetMask(Mask* const mask)
{
  this->TargetMask->DeepCopyFrom(mask);
  this->TargetMaskBoundingBox = MaskOperations::ComputeValidBoundingBox(this->TargetMask);
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::SetAllowedPropagationMask(Mask* const mask)
{
  if(!this->AllowedPropagationMask)
  {
    this->AllowedPropagationMask = Mask::New();
  }

  this->AllowedPropagationMask->DeepCopyFrom(mask);
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
TPatchDistanceFunctor* PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::GetPatchDistanceFunctor()
{
  return this->PatchDistanceFunctor;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::SetPatchDistanceFunctor(TPatchDistanceFunctor* const patchDistanceFunctor)
{
  this->PatchDistanceFunctor = patchDistanceFunctor;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::ForcePropagation()
{
  std::cout << "ForcePropagation()" << std::endl;
  AcceptanceTestAcceptAll acceptanceTest;
  AllNeighbors neighborFunctor;

  // Process the pixels that are invalid
  auto processInvalid = [this](const itk::Index<2>& queryIndex)
  {
    if(!this->Output->GetPixel(queryIndex).IsValid())
    {
      return true;
    }
    return false;
  };

  Propagation(neighborFunctor, processInvalid, &acceptanceTest);
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::InwardPropagation()
{
  AllowedPropagationNeighbors neighborFunctor(this->AllowedPropagationMask, this->TargetMask);

  auto processAll = [](const itk::Index<2>& queryIndex) {
      return true;
  };
  Propagation(neighborFunctor, processAll);
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
template <typename TNeighborFunctor, typename TProcessFunctor>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::Propagation(const TNeighborFunctor neighborFunctor, TProcessFunctor processFunctor,
                                     AcceptanceTest* acceptanceTest)
{
  assert(this->AllowedPropagationMask);

  itk::ImageRegion<2> region = this->Output->GetLargestPossibleRegion();

  // Use the acceptance test that is passed in unless it is null, in which case use the internal acceptance test
  if(!acceptanceTest)
  {
    std::cout << "Using internal acceptance test functor." << std::endl;
    acceptanceTest = this->AcceptanceTestFunctor;
  }
  assert(acceptanceTest);

  assert(this->Output->GetLargestPossibleRegion().GetSize()[0] > 0); // An initialization must be provided

  std::vector<itk::Index<2> > targetPixels = this->TargetMask->GetValidPixels();
  std::cout << "Propagation(): There are " << targetPixels.size() << " target pixels." << std::endl;
  unsigned int skippedPixels = 0;
  unsigned int alreadyExactMatch = 0;
  unsigned int propagatedPixels = 0;
  for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
  {
//     if(targetPixelId % 10000 == 0)
//     {
//       std::cout << "Propagation() processing " << targetPixelId << " of " << targetPixels.size() << std::endl;
//     }

    itk::Index<2> targetPixel = targetPixels[targetPixelId];

    // If we don't want to process this pixel, skip it
    if(!processFunctor(targetPixel))
    {
      skippedPixels++;
      continue;
    }

    // When using PatchMatch for inpainting, most of the NN-field will be an exact match.
    // We don't have to search anymore once the exact match is found.
    if((this->Output->GetPixel(targetPixel).Score == 0))
    {
      alreadyExactMatch++;
      continue;
    }

    itk::ImageRegion<2> targetRegion =
          ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->PatchRadius);

    if(!region.IsInside(targetRegion))
      {
        std::cerr << "targetRegion " << targetRegion << " is outside of the image." << std::endl;
        continue;
      }

    std::vector<itk::Index<2> > potentialPropagationPixels = neighborFunctor(targetPixel);

    bool propagated = false;
    for(size_t potentialPropagationPixelId = 0; potentialPropagationPixelId < potentialPropagationPixels.size();
        ++potentialPropagationPixelId)
    {
      itk::Index<2> potentialPropagationPixel = potentialPropagationPixels[potentialPropagationPixelId];

      itk::Offset<2> potentialPropagationPixelOffset = potentialPropagationPixel - targetPixel;

      //assert(this->Image->GetLargestPossibleRegion().IsInside(potentialPropagationPixel));
      if(!region.IsInside(potentialPropagationPixel))
      {
        //std::cerr << "Pixel " << potentialPropagationPixel << " is outside of the image." << std::endl;
        continue;
      }

      if(!this->AllowedPropagationMask->GetPixel(potentialPropagationPixel))
      {
        continue;
      }

      if(!this->Output->GetPixel(potentialPropagationPixel).IsValid())
      {
        continue;
      }

      // The potential match is the opposite (hence the " - offset" in the following line)
      // of the offset of the neighbor. Consider the following case:
      // - We are at (4,4) and potentially propagating from (3,4)
      // - The best match to (3,4) is (10,10)
      // - potentialMatch should be (11,10), because since the current pixel is 1 to the right
      // of the neighbor, we need to consider the patch one to the right of the neighbors best match
      itk::Index<2> potentialMatchPixel =
            ITKHelpers::GetRegionCenter(this->Output->GetPixel(potentialPropagationPixel).Region) -
                                        potentialPropagationPixelOffset;

      itk::ImageRegion<2> potentialMatchRegion =
            ITKHelpers::GetRegionInRadiusAroundPixel(potentialMatchPixel, this->PatchRadius);

      if(!region.IsInside(potentialMatchRegion) ||
         !this->SourceMask->IsValid(potentialMatchRegion))
      {
        // do nothing - we don't want to propagate information that is not originally valid
        //std::cerr << "Cannot propagate from this source region!" << std::endl;
      }
      else
      {
        float distance = this->PatchDistanceFunctor->Distance(potentialMatchRegion, targetRegion);

        Match potentialMatch;
        potentialMatch.Region = potentialMatchRegion;
        potentialMatch.Score = distance;

        //float oldScore = this->Output->GetPixel(targetRegionCenter).Score; // For debugging only
        //bool better = AddIfBetter(targetRegionCenter, potentialMatch);

        if(acceptanceTest->IsBetter(targetRegion, this->Output->GetPixel(targetPixel), potentialMatch))
        {
          this->Output->SetPixel(targetPixel, potentialMatch);
          propagated = true;
        }
        else
        {
          //std::cerr << "Acceptance test failed!" << std::endl;
        }

      } // end else source region valid
    } // end loop over potentialPropagationPixels

    if(propagated)
    {
      propagatedPixels++;
    }
    {
      //std::cerr << "Failed to propagate to " << targetPixel << std::endl;
    }
//     { // Debug only
//     CoordinateImageType::Pointer temp = CoordinateImageType::New();
//     GetPatchCentersImage(this->Output, temp);
//     ITKHelpers::WriteSequentialImage(temp.GetPointer(), "PatchMatch_Propagation", targetPixelId, 6, "mha");
//     }
  } // end loop over target pixels

  std::cout << "Propagation() skipped " << skippedPixels << " pixels." << std::endl;
  std::cout << "Propagation() propagated " << propagatedPixels << " pixels." << std::endl;
  std::cout << "Propagation() already exact match " << alreadyExactMatch << " pixels." << std::endl;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::ForwardPropagation()
{
  ForwardPropagationNeighbors neighborFunctor;

  auto processAll = [](const itk::Index<2>& queryIndex) {
      return true;
  };
  Propagation(neighborFunctor, processAll);
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::BackwardPropagation()
{
  BackwardPropagationNeighbors neighborFunctor;

  auto processAll = [](const itk::Index<2>& queryIndex) {
      return true;
  };
  Propagation(neighborFunctor, processAll);
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::RandomSearch()
{
  assert(this->Output->GetLargestPossibleRegion().GetSize()[0] > 0);

  itk::ImageRegion<2> region = this->Output->GetLargestPossibleRegion();

  std::vector<itk::Index<2> > targetPixels = this->TargetMask->GetValidPixels();
  std::cout << "RandomSearch: There are : " << targetPixels.size() << " target pixels." << std::endl;
  unsigned int exactMatchPixels = 0;
  for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
  {
//     if(targetPixelId % 10000 == 0)
//     {
//       std::cout << "RandomSearch() processing " << targetPixelId << " of " << targetPixels.size() << std::endl;
//     }

    itk::Index<2> targetPixel = targetPixels[targetPixelId];

    // For inpainting, most of the NN-field will be an exact match. We don't have to search anymore
    // once the exact match is found.
    if((this->Output->GetPixel(targetPixel).Score == 0))
    {
      exactMatchPixels++;
      continue;
    }

    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->PatchRadius);

    if(!region.IsInside(targetRegion))
      {
        //std::cerr << "Pixel " << potentialPropagationPixel << " is outside of the image." << std::endl;
        continue;
      }

    unsigned int width = region.GetSize()[0];
    unsigned int height = region.GetSize()[1];

    // The maximum (first) search radius, as prescribed in PatchMatch paper section 3.2
    unsigned int radius = std::max(width, height);

    // The fraction by which to reduce the search radius at each iteration,
    // as prescribed in PatchMatch paper section 3.2
    float alpha = 1.0f/2.0f; 

    // Search an exponentially smaller window each time through the loop

    while (radius > this->PatchRadius) // while there is more than just the current patch to search
    {
      itk::ImageRegion<2> searchRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, radius);
      searchRegion.Crop(region);

      unsigned int maxNumberOfAttempts = 5; // How many random patches to test for validity before giving up

      itk::ImageRegion<2> randomValidRegion;
      try
      {
      // This function throws an exception if no valid patch was found
      randomValidRegion =
                MaskOperations::GetRandomValidPatchInRegion(this->SourceMask.GetPointer(),
                                                            searchRegion, this->PatchRadius,
                                                            maxNumberOfAttempts);
      }
      catch (...) // If no suitable region is found, move on
      {
        //std::cout << "No suitable region found." << std::endl;
        radius *= alpha;
        continue;
      }

      // Compute the patch difference
      float dist = this->PatchDistanceFunctor->Distance(randomValidRegion, targetRegion);

      // Construct a match object
      Match potentialMatch;
      potentialMatch.Region = randomValidRegion;
      potentialMatch.Score = dist;

      // Store this match as the best match if it meets the criteria.
      // In this class, the criteria is simply that it is
      // better than the current best patch. In subclasses (i.e. GeneralizedPatchMatch),
      // it must be better than the worst patch currently stored.
//       float oldScore = this->Output->GetPixel(targetRegionCenter).Score; // For debugging only
//       bool better = AddIfBetter(targetRegionCenter, potentialMatch);

      if(this->AcceptanceTestFunctor->IsBetter(targetRegion, this->Output->GetPixel(targetPixel), potentialMatch))
      {
        this->Output->SetPixel(targetPixel, potentialMatch);
      }

      radius *= alpha;
    } // end decreasing radius loop

  } // end loop over target pixels

  std::cout << "RandomSearch: already exact match " << exactMatchPixels << std::endl;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::SetInitialNNField(MatchImageType* const initialMatchImage)
{
  ITKHelpers::DeepCopy(initialMatchImage, this->Output.GetPointer());
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::SetAcceptanceTest(TAcceptanceTest* const acceptanceTest)
{
  this->AcceptanceTestFunctor = acceptanceTest;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::SetRandom(const bool random)
{
  this->Random = random;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
Mask* PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::GetAllowedPropagationMask()
{
  return this->AllowedPropagationMask;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::WriteValidPixels(const std::string& fileName)
{
  typedef itk::Image<unsigned char> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(this->Output->GetLargestPossibleRegion());
  image->Allocate();
  image->FillBuffer(0);

  itk::ImageRegionConstIterator<MatchImageType> imageIterator(this->Output,
                                                              this->Output->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
  {
    if(imageIterator.Get().IsValid())
    {
      image->SetPixel(imageIterator.GetIndex(), 255);
    }

    ++imageIterator;
  }

  ITKHelpers::WriteImage(image.GetPointer(), fileName);
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
void PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::SetPropagationStrategy(const PropagationStrategyEnum propagationStrategy)
{
  this->PropagationStrategy = propagationStrategy;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
TAcceptanceTest* PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::GetAcceptanceTest()
{
  return this->AcceptanceTestFunctor;
}

template<typename TPatchDistanceFunctor, typename TAcceptanceTest>
unsigned int PatchMatch<TPatchDistanceFunctor, TAcceptanceTest>::CountInvalidPixels()
{
  std::vector<itk::Index<2> > targetPixels = this->TargetMask->GetValidPixels();

  unsigned int invalidPixels = 0;
  for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
  {
    if(!this->Output->GetPixel(targetPixels[targetPixelId]).IsValid())
    {
      invalidPixels++;
    }
  }

  return invalidPixels;
}

#endif
