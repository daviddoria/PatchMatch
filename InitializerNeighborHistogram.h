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

#ifndef InitializerNeighborHistogram_H
#define InitializerNeighborHistogram_H

// Custom
#include "Initializer.h"
#include "PatchMatchHelpers.h"

// ITK
#include "itkNumericTraits.h"

// STL
#include <algorithm> // for remove_if

/** Assign random nearest neighbors to invalid pixels in the target region, as long as the difference between the histogram of the neighbor and the histogram of the query
  * is less than a multiplier times the difference between the histogram of the query and the histogram of the random patch.*/
template <typename TImage, typename TPatchDistanceFunctor>
class InitializerNeighborHistogram : public InitializerPatch
{
public:
  InitializerNeighborHistogram() : Image(NULL), NeighborHistogramMultiplier(2.0f), MaxAttempts(10), PatchDistanceFunctor(NULL), NumberOfBinsPerDimension(20)
  {
    this->RangeMin = itk::NumericTraits<typename TypeTraits<typename TImage::PixelType>::ComponentType>::min();
    this->RangeMax = itk::NumericTraits<typename TypeTraits<typename TImage::PixelType>::ComponentType>::max();
  }

  virtual void Initialize(itk::Image<Match, 2>* const initialization)
  {
    assert(initialization);
    assert(initialization->GetLargestPossibleRegion().GetSize()[0] != 0);
    assert(this->Image);
    assert(this->Image->GetLargestPossibleRegion().GetSize()[0] == initialization->GetLargestPossibleRegion().GetSize()[0]);

//     std::cout << "InitializerNeighborHistogram: RangeMin = " << static_cast<float>(this->RangeMin) << std::endl;
//     std::cout << "InitializerNeighborHistogram: RangeMax = " << static_cast<float>(this->RangeMax) << std::endl;

    itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), this->PatchRadius);

    std::vector<itk::ImageRegion<2> > validSourceRegions =
          MaskOperations::GetAllFullyValidRegions(this->SourceMask, internalRegion, this->PatchRadius);

    if(validSourceRegions.size() == 0)
    {
      throw std::runtime_error("InitializerNeighborHistogram::Initialize() No valid source regions!");
    }

    std::vector<itk::Index<2> > targetPixels = this->TargetMask->GetValidPixels();

    // Remove the valid pixels from this list, because we only want to initialize invalid pixels
    targetPixels.erase(std::remove_if(targetPixels.begin(), targetPixels.end(),
                      [initialization](const itk::Index<2>& queryPixel)
                      {
                        return initialization->GetPixel(queryPixel).IsValid();
                      }),
                      targetPixels.end());
    std::cout << "InitializerNeighborHistogram: There are : " << targetPixels.size() << " pixels to initialize." << std::endl;

    unsigned int failedMatches = 0;
    for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
    {
      itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(targetPixels[targetPixelId], this->PatchRadius);
      if(!this->Image->GetLargestPossibleRegion().IsInside(targetRegion))
      {
        continue;
      }

      itk::Index<2> targetPixel = targetPixels[targetPixelId];

      typedef Histogram<int>::HistogramType HistogramType;
      HistogramType queryHistogram = Histogram<int>::ComputeImageHistogram1D(this->Image,
                                                                            targetRegion, this->NumberOfBinsPerDimension, this->RangeMin, this->RangeMax);

      float randomHistogramDifference;
      float neighborHistogramDifference;
      itk::ImageRegion<2> randomValidRegion;
      HistogramType randomPatchHistogram;
      HistogramType neighborPatchHistogram;
      unsigned int attempts = 0;
      bool acceptableMatchFound = true;

      do
      {
        unsigned int randomSourceRegionId = Helpers::RandomInt(0, validSourceRegions.size() - 1);
        randomValidRegion = validSourceRegions[randomSourceRegionId];
        randomPatchHistogram = Histogram<int>::ComputeImageHistogram1D(this->Image,
                                                                      randomValidRegion, this->NumberOfBinsPerDimension, this->RangeMin, this->RangeMax);

        itk::Offset<2> randomNeighborOffset = PatchMatchHelpers::RandomNeighborNonZeroOffset();

        itk::Index<2> neighbor = targetPixel + randomNeighborOffset;

        itk::ImageRegion<2> neighborRegion = ITKHelpers::GetRegionInRadiusAroundPixel(neighbor, this->PatchRadius);
        neighborPatchHistogram = Histogram<int>::ComputeImageHistogram1D(this->Image,
                                                                        neighborRegion, this->NumberOfBinsPerDimension, this->RangeMin, this->RangeMax);

        randomHistogramDifference = Histogram<int>::HistogramDifference(queryHistogram, randomPatchHistogram);

        neighborHistogramDifference = Histogram<int>::HistogramDifference(queryHistogram, neighborPatchHistogram);

        if(neighborHistogramDifference == 0)
        {
          throw std::runtime_error("neighborHistogramDifference is 0!");
        }
        //std::cout << "histogramDifference: " << histogramDifference << std::endl;
        attempts++;
        if(attempts > this->MaxAttempts)
        {
  //         std::stringstream ss;
  //         ss << "Too many attempts to find a good random match for " << targetPixel;
  //         throw std::runtime_error(ss.str());
          acceptableMatchFound = false;
          break;
        }
      }
      while(randomHistogramDifference > (this->NeighborHistogramMultiplier * neighborHistogramDifference));

      //std::cout << "Attempts: " << attempts << std::endl;
      Match randomMatch;

      if(acceptableMatchFound)
      {
        randomMatch.SetRegion(randomValidRegion);
        randomMatch.SetSSDScore(this->PatchDistanceFunctor->Distance(randomValidRegion, targetRegion));
      }
      else
      {
        randomMatch.MakeInvalid();
        failedMatches++;
      }

      initialization->SetPixel(targetPixel, randomMatch);
    } // end loop over target pixels

    std::cout << failedMatches << " matches failed (out of " << targetPixels.size() << ")" << std::endl;

    { // Debug only
    PatchMatchHelpers::WriteNNField(initialization, "InitializerNeighborHistogram.mha");
    }
    //std::cout << "Finished InitializerNeighborHistogram." << internalRegion << std::endl;
  }

  void SetRangeMin(const typename TypeTraits<typename TImage::PixelType>::ComponentType rangeMin)
  {
    this->RangeMin = rangeMin;
  }

  void SetRangeMax(const typename TypeTraits<typename TImage::PixelType>::ComponentType rangeMax)
  {
    this->RangeMax = rangeMax;
  }

  void SetNeighborHistogramMultiplier(const float neighborHistogramMultiplier)
  {
    this->NeighborHistogramMultiplier = neighborHistogramMultiplier;
  }

  void SetMaxAttempts(const unsigned int maxAttempts)
  {
    this->MaxAttempts = maxAttempts;
  }

  void SetPatchDistanceFunctor(TPatchDistanceFunctor* const patchDistanceFunctor)
  {
    this->PatchDistanceFunctor = patchDistanceFunctor;
  }

  void SetImage(TImage* const image)
  {
    this->Image = image;
  }


private:
  TImage* Image;

  float NeighborHistogramMultiplier;
  typename TypeTraits<typename TImage::PixelType>::ComponentType RangeMin;
  typename TypeTraits<typename TImage::PixelType>::ComponentType RangeMax;

  unsigned int MaxAttempts;
  TPatchDistanceFunctor* PatchDistanceFunctor;

  unsigned int NumberOfBinsPerDimension;
};

#endif
