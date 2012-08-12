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

#ifndef AcceptanceTestHistogram_H
#define AcceptanceTestHistogram_H

// Custom
#include "AcceptanceTest.h"

template <typename TImage>
class AcceptanceTestHistogram : public AcceptanceTestImage<TImage>
{
public:
  AcceptanceTestHistogram() : HistogramAcceptanceThreshold(500.0f), PatchRadius(0)
  {
    this->RangeMin = itk::NumericTraits<typename TypeTraits<typename TImage::PixelType>::ComponentType>::min();
    this->RangeMax = itk::NumericTraits<typename TypeTraits<typename TImage::PixelType>::ComponentType>::max();
    std::cout << "AcceptanceTestHistogram: RangeMin = " << static_cast<float>(this->RangeMin) << std::endl;
    std::cout << "AcceptanceTestHistogram: RangeMax = " << static_cast<float>(this->RangeMax) << std::endl;
  }

  virtual bool IsBetter(const itk::ImageRegion<2>& queryRegion, const Match& oldMatch,
                        const Match& potentialBetterMatch)
  {
    Match currentMatch = this->Output->GetPixel(index);

    const unsigned int numberOfBinsPerDimension = 20;

    itk::ImageRegion<2> queryRegion = ITKHelpers::GetRegionInRadiusAroundPixel(index, this->PatchRadius);

    //typedef float BinValueType;
    typedef int BinValueType;
    typedef Histogram<BinValueType>::HistogramType HistogramType;

    HistogramType queryHistogram =
      Histogram<BinValueType>::ComputeImageHistogram1D(
                  this->HSVImage.GetPointer(), queryRegion, numberOfBinsPerDimension,
                  this->RangeMin, this->RangeMax);

    HistogramType potentialMatchHistogram =
      Histogram<BinValueType>::ComputeImageHistogram1D(
                  this->HSVImage.GetPointer(), potentialMatch.Region, numberOfBinsPerDimension,
                  this->RangeMin, this->RangeMax);

    float potentialMatchHistogramDifference = Histogram<BinValueType>::HistogramDifference(queryHistogram, potentialMatchHistogram);

    if(potentialMatchHistogramDifference < this->HistogramAcceptanceThreshold)
    {
//       std::cout << "Match accepted. SSD " << potentialMatch.Score
//                   << " (better than " << currentMatch.Score << ", "
//                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference << std::endl;
      return true;
    }
    else
    {
//       std::cout << "Rejected better SSD match: " << potentialMatch.Score
//                  << " (better than " << currentMatch.Score << std::endl
//                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference << std::endl;
      return false;
    }
  }

  void SetHistogramAcceptanceThreshold(const float histogramAcceptanceThreshold)
  {
    this->HistogramAcceptanceThreshold = histogramAcceptanceThreshold;
  }

  void SetRangeMin(const typename TypeTraits<typename TImage::PixelType>::ComponentType rangeMin)
  {
    this->RangeMin = rangeMin;
  }

  void SetRangeMax(const typename TypeTraits<typename TImage::PixelType>::ComponentType rangeMax)
  {
    this->RangeMax = rangeMax;
  }

private:
  float HistogramAcceptanceThreshold;
  typename TypeTraits<typename TImage::PixelType>::ComponentType RangeMin;
  typename TypeTraits<typename TImage::PixelType>::ComponentType RangeMax;
};
#endif
