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
class AcceptanceTestNeighborHistogramRatio : public AcceptanceTestImage<TImage>
{
public:
  AcceptanceTestNeighborHistogramRatio() : AcceptanceTestImage<TImage>(), MaxNeighborHistogramRatio(2.0f), NumberOfBinsPerDimension(20)
  {
    this->RangeMin = itk::NumericTraits<typename TypeTraits<typename TImage::PixelType>::ComponentType>::min();
    this->RangeMax = itk::NumericTraits<typename TypeTraits<typename TImage::PixelType>::ComponentType>::max();
  }

  virtual bool IsBetterWithScore(const itk::ImageRegion<2>& queryRegion, const Match& currentMatch,
                        const Match& potentialBetterMatch, float& score)
  {
//     std::cout << "AcceptanceTestNeighborHistogram: "
//                  << " RangeMin = " << static_cast<float>(this->RangeMin) << std::endl;
//     std::cout << "AcceptanceTestNeighborHistogram: "
//                  << RangeMax = " << static_cast<float>(this->RangeMax) << std::endl;
    assert(this->PatchRadius > 0);
    assert(this->Image);
    assert(this->RangeMax > this->RangeMin);

    itk::Index<2> queryIndex = ITKHelpers::GetRegionCenter(queryRegion);

    typedef Histogram<int>::HistogramType HistogramType;
    HistogramType queryHistogram = Histogram<int>::ComputeImageHistogram1D(
                  this->Image, queryRegion, this->NumberOfBinsPerDimension, this->RangeMin, this->RangeMax);

    HistogramType potentialMatchHistogram =
      Histogram<int>::ComputeImageHistogram1D(this->Image,
                                              potentialBetterMatch.GetRegion(), this->NumberOfBinsPerDimension,
                                              this->RangeMin, this->RangeMax);

    itk::Offset<2> randomNeighborOffset = PatchMatchHelpers::RandomNeighborNonZeroOffset();

    itk::Index<2> neighbor = queryIndex + randomNeighborOffset;

    itk::ImageRegion<2> neighborRegion =
      ITKHelpers::GetRegionInRadiusAroundPixel(neighbor, this->PatchRadius);
    HistogramType neighborHistogram = Histogram<int>::ComputeImageHistogram1D(
                  this->Image, neighborRegion, this->NumberOfBinsPerDimension, this->RangeMin, this->RangeMax);

    float neighborHistogramDifference =
      Histogram<int>::HistogramDifference(neighborHistogram, queryHistogram);

    if(neighborHistogramDifference == 0)
    {
      throw std::runtime_error("neighborHistogramDifference is 0!");
    }
    float potentialMatchHistogramDifference =
      Histogram<int>::HistogramDifference(queryHistogram, potentialMatchHistogram);

    float neighborHistogramRatio = potentialMatchHistogramDifference / neighborHistogramDifference;

    if(this->IncludeInScore)
    {
      score += fabs(potentialMatchHistogramDifference - neighborHistogramDifference);
    }

    if(neighborHistogramRatio < this->MaxNeighborHistogramRatio)
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

  void SetMaxNeighborHistogramRatio(const float maxNeighborHistogramRatio)
  {
    this->MaxNeighborHistogramRatio = maxNeighborHistogramRatio;
  }

  void SetNumberOfBinsPerDimension(const unsigned int numberOfBinsPerDimension)
  {
    this->NumberOfBinsPerDimension = numberOfBinsPerDimension;
  }

private:
  /** The largest the the (queryHistogramDifference/neighborHistogramDifference) is allowed to be to accepte the new match. */
  float MaxNeighborHistogramRatio;

  /** Minimum value of the lowest histogram bin. */
  typename TypeTraits<typename TImage::PixelType>::ComponentType RangeMin;

  /** Maximum value of the highest histogram bin. */
  typename TypeTraits<typename TImage::PixelType>::ComponentType RangeMax;

  /** The number of bins to use per image channel/dimension. */
  unsigned int NumberOfBinsPerDimension;
};

#endif
