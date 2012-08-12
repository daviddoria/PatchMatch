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

#ifndef AcceptanceTestNeighborHistogram_H
#define AcceptanceTestNeighborHistogram_H

// Custom
#include "AcceptanceTest.h"
#include "PatchMatchHelpers.h"

// Submodules
#include <ITKHelpers/ITKHelpers.h>
#include <ITKHelpers/ITKTypeTraits.h>

template <typename TImage>
class AcceptanceTestNeighborHistogram : public AcceptanceTestImage<TImage>
{
public:
  AcceptanceTestNeighborHistogram() : AcceptanceTestImage<TImage>(), NeighborHistogramMultiplier(2.0f)
  {
    this->RangeMin = itk::NumericTraits<typename TypeTraits<typename TImage::PixelType>::ComponentType>::min();
    this->RangeMax = itk::NumericTraits<typename TypeTraits<typename TImage::PixelType>::ComponentType>::max();
  }

  virtual bool IsBetter(const itk::ImageRegion<2>& queryRegion, const Match& currentMatch,
                        const Match& potentialBetterMatch)
  {
//     std::cout << "AcceptanceTestNeighborHistogram: "
//                  << " RangeMin = " << static_cast<float>(this->RangeMin) << std::endl;
//     std::cout << "AcceptanceTestNeighborHistogram: "
//                  << RangeMax = " << static_cast<float>(this->RangeMax) << std::endl;
    assert(this->PatchRadius > 0);
    assert(this->Image);
    assert(this->RangeMax > this->RangeMin);

    itk::Index<2> queryIndex = ITKHelpers::GetRegionCenter(queryRegion);

    const unsigned int numberOfBinsPerDimension = 20;

    typedef Histogram<int>::HistogramType HistogramType;
    HistogramType queryHistogram = Histogram<int>::ComputeImageHistogram1D(
                  this->Image, queryRegion, numberOfBinsPerDimension, this->RangeMin, this->RangeMax);

    HistogramType potentialMatchHistogram =
      Histogram<int>::ComputeImageHistogram1D(this->Image,
                                              potentialBetterMatch.Region, numberOfBinsPerDimension,
                                              this->RangeMin, this->RangeMax);

    itk::Offset<2> randomNeighborOffset = PatchMatchHelpers::RandomNeighborNonZeroOffset();

    itk::Index<2> neighbor = queryIndex + randomNeighborOffset;

    itk::ImageRegion<2> neighborRegion =
      ITKHelpers::GetRegionInRadiusAroundPixel(neighbor, this->PatchRadius);
    HistogramType neighborHistogram = Histogram<int>::ComputeImageHistogram1D(
                  this->Image, neighborRegion, numberOfBinsPerDimension, this->RangeMin, this->RangeMax);

    float neighborHistogramDifference =
      Histogram<int>::HistogramDifference(neighborHistogram, queryHistogram);

    float potentialMatchHistogramDifference =
      Histogram<int>::HistogramDifference(queryHistogram, potentialMatchHistogram);

    if(potentialMatchHistogramDifference < (this->NeighborHistogramMultiplier * neighborHistogramDifference))
    {
//       std::cout << "AddIfBetterNeighborHistogram: Match accepted. SSD " << potentialMatch.Score
//                 << " (better than " << currentMatch.Score << ", "
//                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference
//                 << " vs neighbor histogram score " << neighborHistogramDifference << std::endl;
      return true;
    }
    else
    {
//       std::cout << "AddIfBetterNeighborHistogram: Rejected better SSD match: "
//                 << potentialMatch.Score << " (better than " << currentMatch.Score << std::endl
//                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference
//                 << " vs neighbor histogram score " << neighborHistogramDifference << std::endl;
      return false;
    }
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
