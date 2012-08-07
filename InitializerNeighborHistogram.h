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

#include "Initializer.h"

#include "PatchMatchHelpers.h"

template <typename TImage>
class InitializerNeighborHistogram : public InitializerImage<TImage>
{
public:
  InitializerNeighborHistogram() : NeighborHistogramMultiplier(2.0f)
  {
    this->RangeMin = itk::NumericTraits<typename TypeTraits<typename TImage::PixelType>::ComponentType>::min();
    this->RangeMax = itk::NumericTraits<typename TypeTraits<typename TImage::PixelType>::ComponentType>::max();
    std::cout << "InitializerNeighborHistogram: RangeMin = " << static_cast<float>(this->RangeMin) << std::endl;
    std::cout << "InitializerNeighborHistogram: RangeMax = " << static_cast<float>(this->RangeMax) << std::endl;
  }

  virtual void Initialize(itk::Image<Match, 2>* const initialization)
  {
    assert(initialization->GetLargestPossibleRegion().GetSize()[0] != 0);
    assert(this->Image->GetLargestPossibleRegion().GetSize()[0] == initialization->GetLargestPossibleRegion().GetSize()[0]);

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

    unsigned int failedMatches = 0;
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
      if(initialization->GetPixel(targetPixel).IsValid())
      {
        continue;
      }

      unsigned int numberOfBinsPerDimension = 20;

      typedef Histogram<int>::HistogramType HistogramType;
      HistogramType queryHistogram = Histogram<int>::ComputeImageHistogram1D(this->Image,
                                                                            targetRegion, numberOfBinsPerDimension, this->RangeMin, this->RangeMax);

      float randomHistogramDifference;
      float neighborHistogramDifference;
      itk::ImageRegion<2> randomValidRegion;
      HistogramType randomPatchHistogram;
      HistogramType neighborPatchHistogram;
      unsigned int attempts = 0;
      bool acceptableMatchFound = true;

      unsigned int maxAttempts = 10;
      do
      {
        unsigned int randomSourceRegionId = Helpers::RandomInt(0, validSourceRegions.size() - 1);
        randomValidRegion = validSourceRegions[randomSourceRegionId];
        randomPatchHistogram = Histogram<int>::ComputeImageHistogram1D(this->Image,
                                                                      randomValidRegion, numberOfBinsPerDimension, this->RangeMin, this->RangeMax);

        itk::Offset<2> randomNeighborOffset = PatchMatchHelpers::RandomNeighborNonZeroOffset();

        itk::Index<2> neighbor = targetPixel + randomNeighborOffset;

        itk::ImageRegion<2> neighborRegion = ITKHelpers::GetRegionInRadiusAroundPixel(neighbor, this->PatchRadius);
        neighborPatchHistogram = Histogram<int>::ComputeImageHistogram1D(this->Image,
                                                                        neighborRegion, numberOfBinsPerDimension, this->RangeMin, this->RangeMax);

        randomHistogramDifference = Histogram<int>::HistogramDifference(queryHistogram, randomPatchHistogram);

        neighborHistogramDifference = Histogram<int>::HistogramDifference(queryHistogram, neighborPatchHistogram);
        //std::cout << "histogramDifference: " << histogramDifference << std::endl;
        attempts++;
        if(attempts > maxAttempts)
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
        randomMatch.Region = randomValidRegion;
        randomMatch.Score = this->PatchDistanceFunctor->Distance(randomValidRegion, targetRegion);
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
    PatchMatch<TImage>::WriteNNField(initialization, "InitializerNeighborHistogram.mha");
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

private:
  float NeighborHistogramMultiplier;
  typename TypeTraits<typename TImage::PixelType>::ComponentType RangeMin;
  typename TypeTraits<typename TImage::PixelType>::ComponentType RangeMax;
};

#endif
