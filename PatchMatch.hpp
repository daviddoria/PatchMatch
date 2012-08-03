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
#include <Mask/ITKHelpers/ITKHelpers.h>
#include <Mask/MaskOperations.h>
#include <PatchComparison/SSD.h>
#include <Histogram/Histogram.h>
#include <ITKHelpers/ITKTypeTraits.h>

// ITK
#include "itkImageRegionReverseIterator.h"

// STL
#include <ctime>

template <typename TImage>
PatchMatch<TImage>::PatchMatch() : PatchRadius(0), PatchDistanceFunctor(NULL),
                                   InitializationStrategy(RANDOM), Random(true),
                                   AllowedPropagationMask(NULL),
                                   AddIfBetterStrategy(HISTOGRAM),
                                   HistogramAcceptanceThreshold(500.0f),
                                   PropagationStrategy(UNIFORM)
{
  this->Output = PMImageType::New();
  this->Image = TImage::New();
  this->SourceMask = Mask::New();
  this->TargetMask = Mask::New();
}

template <typename TImage>
void PatchMatch<TImage>::Initialize()
{
    if(this->InitializationStrategy == RANDOM)
    {
      RandomInit();
    }
    else if(this->InitializationStrategy == RANDOM_WITH_HISTOGRAM)
    {
      RandomInitWithHistogramTest();
    }
    else if(this->InitializationStrategy == RANDOM_WITH_HISTOGRAM_NEIGHBOR)
    {
      RandomInitWithHistogramNeighborTest();
    }
    else if(this->InitializationStrategy == BOUNDARY)
    {
      BoundaryInit();
    }
    else
    {
      throw std::runtime_error("PatchMatch::Initialize: An invalid initialization strategy was chosen!");
    }
}

template <typename TImage>
void PatchMatch<TImage>::AutomaticCompute(PMImageType* const initialization)
{
  // If an initialization is provided, use it. Otherwise, generate one.
  if(initialization)
  {
    ITKHelpers::DeepCopy(initialization, this->Output.GetPointer());
  }
  else
  {
    InitKnownRegion();

    Initialize();
  }

  Compute(initialization);
}

template <typename TImage>
void PatchMatch<TImage>::Compute(PMImageType* const initialization)
{
  if(this->Random)
  {
    srand(time(NULL));
  }
  else
  {
    srand(0);
  }

  this->DownsampledImage = TImage::New();

  ITKHelpers::Downsample(this->Image.GetPointer(), this->DownsampleFactor, this->DownsampledImage.GetPointer());

  { // Debug only
  CoordinateImageType::Pointer initialOutput = CoordinateImageType::New();
  GetPatchCentersImage(this->Output, initialOutput);
  ITKHelpers::WriteImage(initialOutput.GetPointer(), "initialization.mha");

  ITKHelpers::WriteImage(this->TargetMask.GetPointer(), "PatchMatch_TargetMask.png");
  ITKHelpers::WriteImage(this->SourceMask.GetPointer(), "PatchMatch_SourceMask.png");
  ITKHelpers::WriteImage(this->AllowedPropagationMask.GetPointer(), "PatchMatch_PropagationMask.png");
  }

  // Initialize this so that we propagate forward first (the propagation direction toggles at each iteration)
  bool forwardPropagation = true;

  // For the number of iterations specified, perform the appropriate propagation and then a random search
  for(unsigned int iteration = 0; iteration < this->Iterations; ++iteration)
  {
    std::cout << "PatchMatch iteration " << iteration << std::endl;

    if(this->PropagationStrategy == UNIFORM)
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
    CoordinateImageType::Pointer temp = CoordinateImageType::New();
    GetPatchCentersImage(this->Output, temp);
    ITKHelpers::WriteSequentialImage(temp.GetPointer(), "PatchMatch", iteration, 2, "mha");
    }
  }

  std::cout << "PatchMatch finished." << std::endl;
}

template <typename TImage>
void PatchMatch<TImage>::InitKnownRegion()
{
  // Create a zero region
  itk::Index<2> zeroIndex = {{0,0}};
  itk::Size<2> zeroSize = {{0,0}};
  itk::ImageRegion<2> zeroRegion(zeroIndex, zeroSize);

  // Create an invalid match
  Match invalidMatch;
  invalidMatch.Region = zeroRegion;
  //invalidMatch.Score = std::numeric_limits<float>::max();
  invalidMatch.Score = Match::InvalidScore;

  // Initialize the entire NNfield to be invalid matches
  ITKHelpers::SetImageToConstant(this->Output.GetPointer(), invalidMatch);

  // Get all of the regions that are entirely inside the image
  itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), this->PatchRadius);

  // Set all of the patches that are entirely inside the source region to exactly
  // themselves as their nearest neighbor
  itk::ImageRegionIteratorWithIndex<PMImageType> outputIterator(this->Output, internalRegion);

  while(!outputIterator.IsAtEnd())
  {
    // Construct the current region
    itk::Index<2> currentIndex = outputIterator.GetIndex();
    itk::ImageRegion<2> currentRegion =
         ITKHelpers::GetRegionInRadiusAroundPixel(currentIndex, this->PatchRadius);

    if(this->SourceMask->IsValid(currentRegion))
    {
      Match selfMatch;
      selfMatch.Region = currentRegion;
      selfMatch.Score = 0.0f;
      outputIterator.Set(selfMatch);
    }

    ++outputIterator;
  }

  { // Debug only
  CoordinateImageType::Pointer initialOutput = CoordinateImageType::New();
  GetPatchCentersImage(this->Output, initialOutput);
  ITKHelpers::WriteImage(initialOutput.GetPointer(), "InitKnownRegion_Output.mha");
  }
}

template <typename TImage>
void PatchMatch<TImage>::BoundaryInit()
{
  itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), this->PatchRadius);

  // Expand the hole
  Mask::Pointer expandedSourceMask = Mask::New();
  expandedSourceMask->DeepCopyFrom(this->SourceMask);
  expandedSourceMask->ExpandHole(this->PatchRadius);

  // Get the expanded boundary
  Mask::BoundaryImageType::Pointer boundaryImage = Mask::BoundaryImageType::New();
  unsigned char outputBoundaryPixelValue = 255;
  expandedSourceMask->FindBoundary(boundaryImage.GetPointer(), Mask::VALID, outputBoundaryPixelValue);
  ITKHelpers::WriteImage(boundaryImage.GetPointer(), "ExpandedBoundary.png");

  // Get the boundary pixels
  std::vector<itk::Index<2> > boundaryIndices =
         ITKHelpers::GetPixelsWithValue(boundaryImage.GetPointer(), outputBoundaryPixelValue);

  std::vector<itk::Index<2> > targetPixels = this->TargetMask->GetValidPixels();

  for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
  {
    // Construct the current region
    itk::Index<2> targetPixelIndex = targetPixels[targetPixelId];

    // Only do this for pixels which have not already been initialized to themselves
    if(this->Output->GetPixel(targetPixelIndex).Score == 0)
    {
      continue;
    }

    itk::ImageRegion<2> currentRegion =
          ITKHelpers::GetRegionInRadiusAroundPixel(targetPixelIndex, this->PatchRadius);

    if(!this->Image->GetLargestPossibleRegion().IsInside(currentRegion))
    {
      continue;
    }
    // Find the nearest valid boundary patch
    unsigned int closestIndexId = ITKHelpers::ClosestIndexId(boundaryIndices, targetPixelIndex);
    itk::Index<2> closestBoundaryPatchCenter = boundaryIndices[closestIndexId];
    itk::ImageRegion<2> closestBoundaryPatchRegion =
          ITKHelpers::GetRegionInRadiusAroundPixel(closestBoundaryPatchCenter,
                                                    this->PatchRadius);

    if(!this->Image->GetLargestPossibleRegion().IsInside(closestBoundaryPatchRegion))
    {
      continue;
    }

    Match match;
    match.Region = closestBoundaryPatchRegion;
    match.Score = this->PatchDistanceFunctor->Distance(closestBoundaryPatchRegion, currentRegion);
    this->Output->SetPixel(targetPixelIndex, match);
  }

  std::cout << "Finished BoundaryInit()." << internalRegion << std::endl;
}

template <typename TImage>
void PatchMatch<TImage>::RandomInitWithHistogramTest()
{
  itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), this->PatchRadius);

  std::vector<itk::ImageRegion<2> > validSourceRegions =
        MaskOperations::GetAllFullyValidRegions(this->SourceMask, internalRegion, this->PatchRadius);

  if(validSourceRegions.size() == 0)
  {
    throw std::runtime_error("PatchMatch::RandomInitWithHistogramTest() No valid source regions!");
  }

  std::vector<itk::Index<2> > targetPixels = this->TargetMask->GetValidPixels();
  std::cout << "RandomInitWithHistogramTest: There are : " << targetPixels.size() << " target pixels." << std::endl;
  for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
  {
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixels[targetPixelId], this->PatchRadius);
    if(!this->Image->GetLargestPossibleRegion().IsInside(targetRegion))
    {
      continue;
    }

    itk::Index<2> targetPixel = targetPixels[targetPixelId];
    if(this->Output->GetPixel(targetPixel).IsValid())
    {
      continue;
    }

    unsigned int numberOfBinsPerDimension = 20;
    typename TypeTraits<typename HSVImageType::PixelType>::ComponentType rangeMin = 0;
    typename TypeTraits<typename HSVImageType::PixelType>::ComponentType rangeMax = 1;
//     std::cout << "Range min: " << rangeMin << std::endl;
//     std::cout << "Range max: " << rangeMax << std::endl;

    Histogram<int>::HistogramType queryHistogram = Histogram<int>::ComputeImageHistogram1D(this->HSVImage.GetPointer(),
                                                                                           targetRegion, numberOfBinsPerDimension, rangeMin, rangeMax);

    float histogramDifference;
    itk::ImageRegion<2> randomValidRegion;
    Histogram<int>::HistogramType randomPatchHistogram;
    unsigned int attempts = 0;
    bool acceptableMatchFound = true;
    do
    {
      unsigned int randomSourceRegionId = Helpers::RandomInt(0, validSourceRegions.size() - 1);
      randomValidRegion = validSourceRegions[randomSourceRegionId];
      randomPatchHistogram = Histogram<int>::ComputeImageHistogram1D(this->HSVImage.GetPointer(),
                                                                     randomValidRegion, numberOfBinsPerDimension, rangeMin, rangeMax);
      histogramDifference = Histogram<int>::HistogramDifference(queryHistogram, randomPatchHistogram);
      //std::cout << "histogramDifference: " << histogramDifference << std::endl;
      attempts++;
      if(attempts > 1000)
      {
//         std::stringstream ss;
//         ss << "Too many attempts to find a good random match for " << targetPixel;
//         throw std::runtime_error(ss.str());
        acceptableMatchFound = false;
        break;
      }
    }
    while(histogramDifference > this->HistogramAcceptanceThreshold);

    //std::cout << "Attempts: " << attempts << std::endl;
    Match randomMatch;
    randomMatch.Region = randomValidRegion;

    if(acceptableMatchFound)
    {
      randomMatch.Score = this->PatchDistanceFunctor->Distance(randomValidRegion, targetRegion);
    }
    else
    {
      //std::cout << "Random initialization failed for " << targetPixel << std::endl;
      randomMatch.Score = std::numeric_limits<float>::max();
    }

    this->Output->SetPixel(targetPixel, randomMatch);
  }


  { // Debug only
  CoordinateImageType::Pointer initialOutput = CoordinateImageType::New();
  GetPatchCentersImage(this->Output, initialOutput);
  ITKHelpers::WriteImage(initialOutput.GetPointer(), "RandomInit.mha");
  }
  //std::cout << "Finished RandomInit." << internalRegion << std::endl;
}


template <typename TImage>
void PatchMatch<TImage>::RandomInitWithHistogramNeighborTest()
{
  itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), this->PatchRadius);

  std::vector<itk::ImageRegion<2> > validSourceRegions =
        MaskOperations::GetAllFullyValidRegions(this->SourceMask, internalRegion, this->PatchRadius);

  if(validSourceRegions.size() == 0)
  {
    throw std::runtime_error("PatchMatch::RandomInitWithHistogramTest() No valid source regions!");
  }

  std::vector<itk::Index<2> > targetPixels = this->TargetMask->GetValidPixels();
  std::cout << "RandomInitWithHistogramTest: There are : " << targetPixels.size() << " target pixels." << std::endl;
  for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
  {
    if(targetPixelId % 10000 == 0)
    {
      std::cout << "RandomInitWithHistogramNeighborTest() processing " << targetPixelId << " of " << targetPixels.size() << std::endl;
    }
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixels[targetPixelId], this->PatchRadius);
    if(!this->Image->GetLargestPossibleRegion().IsInside(targetRegion))
    {
      continue;
    }

    itk::Index<2> targetPixel = targetPixels[targetPixelId];
    if(this->Output->GetPixel(targetPixel).IsValid())
    {
      continue;
    }

    unsigned int numberOfBinsPerDimension = 20;
    typename TypeTraits<typename HSVImageType::PixelType>::ComponentType rangeMin = 0;
    typename TypeTraits<typename HSVImageType::PixelType>::ComponentType rangeMax = 1;
//     std::cout << "Range min: " << rangeMin << std::endl;
//     std::cout << "Range max: " << rangeMax << std::endl;

    typedef Histogram<int>::HistogramType HistogramType;
    HistogramType queryHistogram = Histogram<int>::ComputeImageHistogram1D(this->HSVImage.GetPointer(),
                                                                           targetRegion, numberOfBinsPerDimension, rangeMin, rangeMax);

    float randomHistogramDifference;
    float neighborHistogramDifference;
    itk::ImageRegion<2> randomValidRegion;
    HistogramType randomPatchHistogram;
    HistogramType neighborPatchHistogram;
    unsigned int attempts = 0;
    bool acceptableMatchFound = true;
    float differenceMultiplier = 2.0f;
    do
    {
      unsigned int randomSourceRegionId = Helpers::RandomInt(0, validSourceRegions.size() - 1);
      randomValidRegion = validSourceRegions[randomSourceRegionId];
      randomPatchHistogram = Histogram<int>::ComputeImageHistogram1D(this->HSVImage.GetPointer(),
                                                                     randomValidRegion, numberOfBinsPerDimension, rangeMin, rangeMax);

      itk::Offset<2> randomNeighborOffset = RandomNeighborNonZeroOffset();

      itk::Index<2> neighbor = targetPixel + randomNeighborOffset;

      itk::ImageRegion<2> neighborRegion = ITKHelpers::GetRegionInRadiusAroundPixel(neighbor, this->PatchRadius);
      neighborPatchHistogram = Histogram<int>::ComputeImageHistogram1D(this->HSVImage.GetPointer(),
                                                                       neighborRegion, numberOfBinsPerDimension, rangeMin, rangeMax);
      
      randomHistogramDifference = Histogram<int>::HistogramDifference(queryHistogram, randomPatchHistogram);

      neighborHistogramDifference = Histogram<int>::HistogramDifference(queryHistogram, neighborPatchHistogram);
      //std::cout << "histogramDifference: " << histogramDifference << std::endl;
      attempts++;
      if(attempts > 1000)
      {
//         std::stringstream ss;
//         ss << "Too many attempts to find a good random match for " << targetPixel;
//         throw std::runtime_error(ss.str());
        acceptableMatchFound = false;
        break;
      }
    }
    while(randomHistogramDifference > (differenceMultiplier * neighborHistogramDifference));

    //std::cout << "Attempts: " << attempts << std::endl;
    Match randomMatch;
    randomMatch.Region = randomValidRegion;

    if(acceptableMatchFound)
    {
      randomMatch.Score = this->PatchDistanceFunctor->Distance(randomValidRegion, targetRegion);
    }
    else
    {
      //std::cout << "Random initialization failed for " << targetPixel << std::endl;
      randomMatch.Score = std::numeric_limits<float>::max();
    }

    this->Output->SetPixel(targetPixel, randomMatch);
  }


  { // Debug only
  CoordinateImageType::Pointer initialOutput = CoordinateImageType::New();
  GetPatchCentersImage(this->Output, initialOutput);
  ITKHelpers::WriteImage(initialOutput.GetPointer(), "RandomInit.mha");
  }
  //std::cout << "Finished RandomInit." << internalRegion << std::endl;
}

template <typename TImage>
void PatchMatch<TImage>::RandomInit()
{
  itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), this->PatchRadius);

  std::vector<itk::ImageRegion<2> > validSourceRegions =
        MaskOperations::GetAllFullyValidRegions(this->SourceMask, internalRegion, this->PatchRadius);

  if(validSourceRegions.size() == 0)
  {
    throw std::runtime_error("PatchMatch: No valid source regions!");
  }

  std::vector<itk::Index<2> > targetPixels = this->TargetMask->GetValidPixels();
  std::cout << "RandomInit: There are : " << targetPixels.size() << " target pixels." << std::endl;
  for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
  {
    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixels[targetPixelId], this->PatchRadius);
    if(!this->Image->GetLargestPossibleRegion().IsInside(targetRegion))
    {
      continue;
    }

    itk::Index<2> targetPixel = targetPixels[targetPixelId];
    if(this->Output->GetPixel(targetPixel).IsValid())
    {
      continue;
    }
    unsigned int randomSourceRegionId = Helpers::RandomInt(0, validSourceRegions.size() - 1);
    itk::ImageRegion<2> randomValidRegion = validSourceRegions[randomSourceRegionId];

    Match randomMatch;
    randomMatch.Region = randomValidRegion;
    randomMatch.Score = this->PatchDistanceFunctor->Distance(randomValidRegion, targetRegion);

    this->Output->SetPixel(targetPixel, randomMatch);
  }


  { // Debug only
  CoordinateImageType::Pointer initialOutput = CoordinateImageType::New();
  GetPatchCentersImage(this->Output, initialOutput);
  ITKHelpers::WriteImage(initialOutput.GetPointer(), "RandomInit.mha");
  }
  //std::cout << "Finished RandomInit." << internalRegion << std::endl;
}

template <typename TImage>
typename PatchMatch<TImage>::PMImageType* PatchMatch<TImage>::GetOutput()
{
  return this->Output;
}

template <typename TImage>
void PatchMatch<TImage>::SetIterations(const unsigned int iterations)
{
  this->Iterations = iterations;
}

template <typename TImage>
void PatchMatch<TImage>::SetPatchRadius(const unsigned int patchRadius)
{
  this->PatchRadius = patchRadius;
}

template <typename TImage>
void PatchMatch<TImage>::SetImage(TImage* const image)
{
  ITKHelpers::DeepCopy(image, this->Image.GetPointer());

  this->Output->SetRegions(this->Image->GetLargestPossibleRegion());
  this->Output->Allocate();

  this->HSVImage = HSVImageType::New();
  ITKHelpers::ITKImageToHSVImage(image, this->HSVImage.GetPointer());
  ITKHelpers::WriteImage(this->HSVImage.GetPointer(), "HSV.mha");
}

template <typename TImage>
void PatchMatch<TImage>::SetSourceMask(Mask* const mask)
{
  this->SourceMask->DeepCopyFrom(mask);
  this->SourceMaskBoundingBox = MaskOperations::ComputeValidBoundingBox(this->SourceMask);
  //std::cout << "SourceMaskBoundingBox: " << this->SourceMaskBoundingBox << std::endl;
}

template <typename TImage>
void PatchMatch<TImage>::SetTargetMask(Mask* const mask)
{
  this->TargetMask->DeepCopyFrom(mask);
  this->TargetMaskBoundingBox = MaskOperations::ComputeValidBoundingBox(this->TargetMask);
  //std::cout << "TargetMaskBoundingBox: " << this->TargetMaskBoundingBox << std::endl;

  // By default, we want to allow propagation from the source region
  if(!this->AllowedPropagationMask)
  {
    this->AllowedPropagationMask = Mask::New();
    this->AllowedPropagationMask->DeepCopyFrom(mask);
  }
}

template <typename TImage>
void PatchMatch<TImage>::SetAllowedPropagationMask(Mask* const mask)
{
  if(!this->AllowedPropagationMask)
  {
    this->AllowedPropagationMask = Mask::New();
  }

  this->AllowedPropagationMask->DeepCopyFrom(mask);
}

template <typename TImage>
void PatchMatch<TImage>::GetPatchCentersImage(PMImageType* const pmImage, CoordinateImageType* const output)
{
  output->SetRegions(pmImage->GetLargestPossibleRegion());
  output->Allocate();

  itk::ImageRegionIterator<PMImageType> imageIterator(pmImage, pmImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    CoordinateImageType::PixelType pixel;

    Match match = imageIterator.Get();
    itk::Index<2> center = ITKHelpers::GetRegionCenter(match.Region);

    pixel[0] = center[0];
    pixel[1] = center[1];
    pixel[2] = match.Score;

    output->SetPixel(imageIterator.GetIndex(), pixel);

    ++imageIterator;
    }
}

template <typename TImage>
void PatchMatch<TImage>::SetPatchDistanceFunctor(PatchDistance<TImage>* const patchDistanceFunctor)
{
  this->PatchDistanceFunctor = patchDistanceFunctor;
}

template <typename TImage>
void PatchMatch<TImage>::InwardPropagation()
{
  AllowedPropagationNeighbors neighborFunctor(this->AllowedPropagationMask, this->TargetMask);
  Propagation(neighborFunctor);
}

template <typename TImage>
template <typename TNeighborFunctor>
void PatchMatch<TImage>::Propagation(const TNeighborFunctor neighborFunctor)
{
  assert(this->AllowedPropagationMask);

  std::vector<itk::Index<2> > targetPixels = this->TargetMask->GetValidPixels();
  std::cout << "Propagation: There are " << targetPixels.size() << " target pixels." << std::endl;
  unsigned int skippedPixels = 0;
  for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
  {
//     if(targetPixelId % 10000 == 0)
//     {
//       std::cout << "Propagation() processing " << targetPixelId << " of " << targetPixels.size() << std::endl;
//     }

    itk::Index<2> targetRegionCenter = targetPixels[targetPixelId];
    // When using PatchMatch for inpainting, most of the NN-field will be an exact match.
    // We don't have to search anymore once the exact match is found.
    if((this->Output->GetPixel(targetRegionCenter).Score == 0) ||
       !this->Output->GetPixel(targetRegionCenter).IsValid() )
    {
      skippedPixels++;
      continue;
    }

    itk::ImageRegion<2> centerRegion =
          ITKHelpers::GetRegionInRadiusAroundPixel(targetRegionCenter, this->PatchRadius);

    if(!this->Image->GetLargestPossibleRegion().IsInside(centerRegion))
      {
        //std::cerr << "Pixel " << potentialPropagationPixel << " is outside of the image." << std::endl;
        continue;
      }

    std::vector<itk::Index<2> > potentialPropagationPixels = neighborFunctor(targetRegionCenter);

    for(size_t potentialPropagationPixelId = 0; potentialPropagationPixelId < potentialPropagationPixels.size();
        ++potentialPropagationPixelId)
    {
      itk::Index<2> potentialPropagationPixel = potentialPropagationPixels[potentialPropagationPixelId];

      //itk::Offset<2> potentialPropagationPixelOffset = targetRegionCenter - potentialPropagationPixel;
      itk::Offset<2> potentialPropagationPixelOffset = potentialPropagationPixel - targetRegionCenter;
      
      //assert(this->Image->GetLargestPossibleRegion().IsInside(potentialPropagationPixel));
      if(!this->Image->GetLargestPossibleRegion().IsInside(potentialPropagationPixel))
      {
        //std::cerr << "Pixel " << potentialPropagationPixel << " is outside of the image." << std::endl;
        continue;
      }

      if(!AllowPropagationFrom(potentialPropagationPixel))
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

      if(!this->SourceMask->GetLargestPossibleRegion().IsInside(potentialMatchRegion) ||
         !this->SourceMask->IsValid(potentialMatchRegion))
      {
        // do nothing - we don't want to propagate information that is not originally valid
      }
      else
      {
        float distance = this->PatchDistanceFunctor->Distance(potentialMatchRegion, centerRegion);

        Match potentialMatch;
        potentialMatch.Region = potentialMatchRegion;
        potentialMatch.Score = distance;

        //float oldScore = this->Output->GetPixel(targetRegionCenter).Score; // For debugging only
        //bool better = AddIfBetter(targetRegionCenter, potentialMatch);

        AddIfBetter(targetRegionCenter, potentialMatch);

//         if(better)
//         {
//           std::cout << "Propagation successful for " << center << " - lowered score from " << oldScore
//                     << " to " << potentialMatch.Score << std::endl;
//         }
//         else
//         {
//           std::cout << "Propagation not successful." << std::endl;
//         }

      }
    } // end loop over potentialPropagationPixels

//     { // Debug only
//     CoordinateImageType::Pointer temp = CoordinateImageType::New();
//     GetPatchCentersImage(this->Output, temp);
//     ITKHelpers::WriteSequentialImage(temp.GetPointer(), "PatchMatch_Propagation", targetPixelId, 6, "mha");
//     }
  } // end loop over target pixels

  std::cout << "Propagation skipped " << skippedPixels << " pixels (processed " << targetPixels.size() - skippedPixels << ")." << std::endl;
}

template <typename TImage>
void PatchMatch<TImage>::ForwardPropagation()
{
  ForwardPropagationNeighbors neighborFunctor;

  Propagation(neighborFunctor);
}

template <typename TImage>
void PatchMatch<TImage>::BackwardPropagation()
{
  BackwardPropagationNeighbors neighborFunctor;

  Propagation(neighborFunctor);
}

template <typename TImage>
void PatchMatch<TImage>::RandomSearch()
{
  std::vector<itk::Index<2> > targetPixels = this->TargetMask->GetValidPixels();
  std::cout << "RandomSearch: There are : " << targetPixels.size() << " target pixels." << std::endl;
  unsigned int skippedPixels = 0;
  for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
  {
//     if(targetPixelId % 10000 == 0)
//     {
//       std::cout << "RandomSearch() processing " << targetPixelId << " of " << targetPixels.size() << std::endl;
//     }

    itk::Index<2> targetRegionCenter = targetPixels[targetPixelId];

    // For inpainting, most of the NN-field will be an exact match. We don't have to search anymore
    // once the exact match is found.
    if((this->Output->GetPixel(targetRegionCenter).Score == 0) ||
       !this->Output->GetPixel(targetRegionCenter).IsValid() )
    {
      skippedPixels++;
      continue;
    }

    itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetRegionCenter, this->PatchRadius);

    if(!this->Image->GetLargestPossibleRegion().IsInside(targetRegion))
      {
        //std::cerr << "Pixel " << potentialPropagationPixel << " is outside of the image." << std::endl;
        continue;
      }

    unsigned int width = this->Image->GetLargestPossibleRegion().GetSize()[0];
    unsigned int height = this->Image->GetLargestPossibleRegion().GetSize()[1];

    // The maximum (first) search radius, as prescribed in PatchMatch paper section 3.2
    unsigned int radius = std::max(width, height);

    // The fraction by which to reduce the search radius at each iteration,
    // as prescribed in PatchMatch paper section 3.2
    float alpha = 1.0f/2.0f; 

    // Search an exponentially smaller window each time through the loop

    while (radius > this->PatchRadius) // while there is more than just the current patch to search
    {
      itk::ImageRegion<2> searchRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetRegionCenter, radius);
      searchRegion.Crop(this->Image->GetLargestPossibleRegion());

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
      
      AddIfBetter(targetRegionCenter, potentialMatch);
//       if(better)
//       {
//         std::cout << "Random search successful for " << center << " - lowered score from " << oldScore
//                   << " to " << potentialMatch.Score << std::endl;
//       }
//       else
//       {
//         std::cout << "Random search not successful." << std::endl;
//       }

      radius *= alpha;
    } // end decreasing radius loop

  } // end loop over target pixels

  std::cout << "RandomSearch skipped " << skippedPixels << " (processed " << targetPixels.size() - skippedPixels << ") pixels." << std::endl;
}

/*
template <typename TImage>
bool PatchMatch<TImage>::AddIfBetterDownsampledHistogram(const itk::Index<2>& index, const Match& potentialMatch)
{
  Match currentMatch = this->Output->GetPixel(index);
  if(potentialMatch.Score < currentMatch.Score)
  {
    const unsigned int numberOfBinsPerDimension = 20;

    itk::Index<2> smallIndex = {{static_cast<int>(index[0] * this->DownsampleFactor),
                                  static_cast<int>(index[1] * this->DownsampleFactor)}};
    unsigned int smallPatchRadius = this->PatchRadius / 2;

    itk::Index<2> queryPatchCenter = ITKHelpers::GetRegionCenter(potentialMatch.Region);
    itk::Index<2> smallQueryPatchCenter = {{static_cast<int>(queryPatchCenter[0] * this->DownsampleFactor),
                                            static_cast<int>(queryPatchCenter[1] * this->DownsampleFactor)}};

    itk::ImageRegion<2> smallQueryRegion = ITKHelpers::GetRegionInRadiusAroundPixel(smallIndex, smallPatchRadius);

    itk::ImageRegion<2> smallPotentialMatchRegion = ITKHelpers::GetRegionInRadiusAroundPixel(smallQueryPatchCenter, smallPatchRadius);

    //typedef float BinValueType;
    typedef int BinValueType;
    typedef Histogram<BinValueType>::HistogramType HistogramType;

    HistogramType queryHistogram = Histogram<BinValueType>::Compute1DConcatenatedHistogramOfMultiChannelImage(
                  this->DownsampledImage.GetPointer(), smallQueryRegion, numberOfBinsPerDimension, 0, 255);

    HistogramType potentialMatchHistogram = Histogram<BinValueType>::Compute1DConcatenatedHistogramOfMultiChannelImage(
                  this->DownsampledImage.GetPointer(), smallPotentialMatchRegion, numberOfBinsPerDimension, 0, 255);

    float potentialMatchHistogramDifference = Histogram<BinValueType>::HistogramDifference(queryHistogram, potentialMatchHistogram);

    
    if(potentialMatchHistogramDifference < this->HistogramAcceptanceThreshold)
    {
//       std::cout << "Match accepted. SSD " << potentialMatch.Score << " (better than " << currentMatch.Score << ", "
//                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference << std::endl;
      this->Output->SetPixel(index, potentialMatch);
      return true;
    }
    else
    {
//       std::cout << "Rejected better SSD match:" << potentialMatch.Score << " (better than " << currentMatch.Score << std::endl
//                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference << std::endl;
      return false;
    }
    
  } // end if SSD is better

  return false;
}
*/


template <typename TImage>
bool PatchMatch<TImage>::AddIfBetterHistogram(const itk::Index<2>& index, const Match& potentialMatch)
{
  Match currentMatch = this->Output->GetPixel(index);
  if(potentialMatch.Score < currentMatch.Score)
  {
    const unsigned int numberOfBinsPerDimension = 20;

    itk::ImageRegion<2> queryRegion = ITKHelpers::GetRegionInRadiusAroundPixel(index, this->PatchRadius);

    //typedef float BinValueType;
    typedef int BinValueType;
    typedef Histogram<BinValueType>::HistogramType HistogramType;

    typename TypeTraits<typename HSVImageType::PixelType>::ComponentType rangeMin = 0;
    typename TypeTraits<typename HSVImageType::PixelType>::ComponentType rangeMax = 1;
    
    HistogramType queryHistogram = Histogram<BinValueType>::ComputeImageHistogram1D(
                  this->HSVImage.GetPointer(), queryRegion, numberOfBinsPerDimension, rangeMin, rangeMax);

    HistogramType potentialMatchHistogram = Histogram<BinValueType>::ComputeImageHistogram1D(
                  this->HSVImage.GetPointer(), potentialMatch.Region, numberOfBinsPerDimension, rangeMin, rangeMax);

    float potentialMatchHistogramDifference = Histogram<BinValueType>::HistogramDifference(queryHistogram, potentialMatchHistogram);

    float acceptanceThreshold = 500.0f;
    if(potentialMatchHistogramDifference < acceptanceThreshold)
    {
//       std::cout << "Match accepted. SSD " << potentialMatch.Score << " (better than " << currentMatch.Score << ", "
//                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference << std::endl;
      this->Output->SetPixel(index, potentialMatch);
      return true;
    }
    else
    {
//       std::cout << "Rejected better SSD match:" << potentialMatch.Score << " (better than " << currentMatch.Score << std::endl
//                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference << std::endl;
      return false;
    }

  }

  return false;
}

template <typename TImage>
bool PatchMatch<TImage>::AddIfBetterNeighborHistogram(const itk::Index<2>& queryIndex, const Match& potentialMatch)
{
  Match currentMatch = this->Output->GetPixel(queryIndex);
  if(potentialMatch.Score < currentMatch.Score)
  {
    const unsigned int numberOfBinsPerDimension = 20;

    typename TypeTraits<typename HSVImageType::PixelType>::ComponentType rangeMin = 0;
    typename TypeTraits<typename HSVImageType::PixelType>::ComponentType rangeMax = 1;

    itk::ImageRegion<2> queryRegion = ITKHelpers::GetRegionInRadiusAroundPixel(queryIndex, this->PatchRadius);

    typedef Histogram<int>::HistogramType HistogramType;
    HistogramType queryHistogram = Histogram<int>::ComputeImageHistogram1D(
                  this->HSVImage.GetPointer(), queryRegion, numberOfBinsPerDimension, rangeMin, rangeMax);

    HistogramType potentialMatchHistogram = Histogram<int>::ComputeImageHistogram1D(
                  this->HSVImage.GetPointer(), potentialMatch.Region, numberOfBinsPerDimension, rangeMin, rangeMax);

    itk::Offset<2> randomNeighborOffset = RandomNeighborNonZeroOffset();

    itk::Index<2> neighbor = queryIndex + randomNeighborOffset;

    itk::ImageRegion<2> neighborRegion = ITKHelpers::GetRegionInRadiusAroundPixel(neighbor, this->PatchRadius);
    HistogramType neighborHistogram = Histogram<int>::ComputeImageHistogram1D(
                  this->HSVImage.GetPointer(), neighborRegion, numberOfBinsPerDimension, rangeMin, rangeMax);

    float neighborHistogramDifference = Histogram<int>::HistogramDifference(neighborHistogram, queryHistogram);

    float potentialMatchHistogramDifference = Histogram<int>::HistogramDifference(queryHistogram, potentialMatchHistogram);

    float differenceMultiplier = 2.0f;
    
    if(potentialMatchHistogramDifference < differenceMultiplier * neighborHistogramDifference)
    {
//       std::cout << "Match accepted. SSD " << potentialMatch.Score << " (better than " << currentMatch.Score << ", "
//                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference
//                 << " vs neighbor histogram score " << neighborHistogramDifference << std::endl;
      this->Output->SetPixel(queryIndex, potentialMatch);
      return true;
    }
    else
    {
//       std::cout << "Rejected better SSD match:" << potentialMatch.Score << " (better than " << currentMatch.Score << std::endl
//                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference
//                 << " vs neighbor histogram score " << neighborHistogramDifference << std::endl;
      return false;
    }

  }
  return false;
}

template <typename TImage>
bool PatchMatch<TImage>::AddIfBetterSSD(const itk::Index<2>& index, const Match& potentialMatch)
{
  Match currentMatch = this->Output->GetPixel(index);
  if(potentialMatch.Score < currentMatch.Score)
  {
    this->Output->SetPixel(index, potentialMatch);
    return true;
  }
  return false;
}

template <typename TImage>
bool PatchMatch<TImage>::AddIfBetter(const itk::Index<2>& index, const Match& potentialMatch)
{
  if(this->AddIfBetterStrategy == HISTOGRAM)
  {
    return AddIfBetterHistogram(index, potentialMatch);
  }
  if(this->AddIfBetterStrategy == NEIGHBOR_HISTOGRAM)
  {
    return AddIfBetterNeighborHistogram(index, potentialMatch);
  }
  else if(this->AddIfBetterStrategy == SSD)
  {
    return AddIfBetterSSD(index, potentialMatch);
  }
  else
  {
    throw std::runtime_error("Invalid AddIfBetterStrategy specified!");
  }
}

template <typename TImage>
void PatchMatch<TImage>::SetAddIfBetterStrategy(const AddIfBetterStrategyEnum addIfBetterStrategy)
{
  this->AddIfBetterStrategy = addIfBetterStrategy;
}

template <typename TImage>
void PatchMatch<TImage>::SetInitializationStrategy(const InitializationStrategyEnum initializationStrategy)
{
  this->InitializationStrategy = initializationStrategy;
}

template <typename TImage>
bool PatchMatch<TImage>::AllowPropagationFrom(const itk::Index<2>& potentialPropagationPixel)
{
  if(!this->Output->GetPixel(potentialPropagationPixel).IsValid())
  {
    return false;
  }

  if(this->AllowedPropagationMask->IsValid(potentialPropagationPixel) ||
     this->TargetMask->IsValid(potentialPropagationPixel))
  {
    return true;
  }

  return false;
}

template <typename TImage>
void PatchMatch<TImage>::SetRandom(const bool random)
{
  this->Random = random;
}

template <typename TImage>
Mask* PatchMatch<TImage>::GetAllowedPropagationMask()
{
  return this->AllowedPropagationMask;
}

template <typename TImage>
void PatchMatch<TImage>::WriteValidPixels(const std::string& fileName)
{
  typedef itk::Image<unsigned char> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(this->Output->GetLargestPossibleRegion());
  image->Allocate();
  image->FillBuffer(0);

  itk::ImageRegionConstIterator<PMImageType> imageIterator(this->Output, this->Output->GetLargestPossibleRegion());

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

template <typename TImage>
void PatchMatch<TImage>::SetHistogramAcceptanceThreshold(const float histogramAcceptanceThreshold)
{
  this->HistogramAcceptanceThreshold = histogramAcceptanceThreshold;
}

template <typename TImage>
void PatchMatch<TImage>::SetPropagationStrategy(const PropagationStrategyEnum propagationStrategy)
{
  this->PropagationStrategy = propagationStrategy;
}

template <typename TImage>
itk::Offset<2> PatchMatch<TImage>::RandomNeighborNonZeroOffset()
{
  int randomOffsetX = 0;
  int randomOffsetY = 0;
  while(!( (randomOffsetX == 0) && (randomOffsetX == randomOffsetY) )) // We don't want the random offset to be (0,0), because the histogram difference would be zero!
  {
    // Generate random numbers in the set (-1,0,1)
    randomOffsetX = rand()%3 + 1;
    randomOffsetY = rand()%3 + 1;
  }

  itk::Offset<2> randomNeighborNonZeroOffset = {{randomOffsetX, randomOffsetY}};

  return randomNeighborNonZeroOffset;
}

#endif
