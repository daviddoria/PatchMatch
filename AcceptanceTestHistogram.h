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
  virtual bool IsBetter(const itk::ImageRegion<2>& queryRegion, const Match& oldMatch, const Match& potentialBetterMatch)
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

      if(potentialMatchHistogramDifference < this->HistogramAcceptanceThreshold)
      {
  //       std::cout << "Match accepted. SSD " << potentialMatch.Score << " (better than " << currentMatch.Score << ", "
  //                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference << std::endl;
        this->Output->SetPixel(index, potentialMatch);
        return true;
      }
      else
      {
  //       std::cout << "Rejected better SSD match: " << potentialMatch.Score << " (better than " << currentMatch.Score << std::endl
  //                 << " Potential Match Histogram score: " << potentialMatchHistogramDifference << std::endl;
        return false;
      }

    }

    return false;
  }

  void SetHistogramAcceptanceThreshold(const float histogramAcceptanceThreshold)
  {
    this->HistogramAcceptanceThreshold = histogramAcceptanceThreshold;
  }

private:
  float HistogramAcceptanceThreshold;

};
#endif
