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
  AcceptanceTestNeighborHistogram() : NeighborHistogramMultiplier(2.0f)
                                      // PatchRadius(0) // can't do this here
  {
    this->PatchRadius = 0;
  }

  virtual bool IsBetter(const itk::ImageRegion<2>& queryRegion, const Match& currentMatch,
                        const Match& potentialBetterMatch)
  {
    if(potentialBetterMatch.Score < currentMatch.Score)
    {
      itk::Index<2> queryIndex = ITKHelpers::GetRegionCenter(queryRegion);

      const unsigned int numberOfBinsPerDimension = 20;

      typename TypeTraits<typename TImage::PixelType>::ComponentType rangeMin = 0;
      typename TypeTraits<typename TImage::PixelType>::ComponentType rangeMax = 1;

      typedef Histogram<int>::HistogramType HistogramType;
      HistogramType queryHistogram = Histogram<int>::ComputeImageHistogram1D(
                    this->Image, queryRegion, numberOfBinsPerDimension, rangeMin, rangeMax);

      HistogramType potentialMatchHistogram =
        Histogram<int>::ComputeImageHistogram1D(this->Image,
                                                potentialBetterMatch.Region, numberOfBinsPerDimension,
                                                rangeMin, rangeMax);

      itk::Offset<2> randomNeighborOffset = PatchMatchHelpers::RandomNeighborNonZeroOffset();

      itk::Index<2> neighbor = queryIndex + randomNeighborOffset;

      itk::ImageRegion<2> neighborRegion =
        ITKHelpers::GetRegionInRadiusAroundPixel(neighbor, this->PatchRadius);
      HistogramType neighborHistogram = Histogram<int>::ComputeImageHistogram1D(
                    this->Image, neighborRegion, numberOfBinsPerDimension, rangeMin, rangeMax);

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
    return false;
  }


private:
  float NeighborHistogramMultiplier;

};
#endif
