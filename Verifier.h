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

#ifndef Verifier_H
#define Verifier_H

// STL
#include <iostream>

// ITK
#include "itkImage.h"

// Custom
#include "Match.h"

// Submodules
#include <Mask/Mask.h>

template <typename TVerifyFunctor>
class Verifier
{
  typedef itk::Image<Match, 2> MatchImageType;
public:

  Verifier() : ProcessMask(NULL), VerifyFunctor(NULL)
  {
  }

  void SetMask(Mask* const mask)
  {
    this->ProcessMask = mask;
  }

  void SetVerifyFunctor(TVerifyFunctor* const verifyFunctor)
  {
    this->VerifyFunctor = verifyFunctor;
  }

  // Test unverified pixels to see if they can be verified
  void Verify(MatchImageType* const nnField)
  {
    assert(this->ProcessMask);
    assert(this->VerifyFunctor);

    std::vector<itk::Index<2> > processPixels = this->ProcessMask->GetValidPixels();

    unsigned int numberVerified = 0;
    for(size_t pixelId = 0; pixelId < processPixels.size(); ++pixelId)
    {
      if(!nnField->GetPixel(processPixels[pixelId]).Verified)
      {
        if(this->VerifyFunctor->Verify(processPixels[pixelId]))
        {
          //nnField->GetPixel(processPixels[pixelId]).Verified = true;
          Match match = nnField->GetPixel(processPixels[pixelId]);
          match.Verified = true;
          nnField->SetPixel(processPixels[pixelId], match);
          numberVerified++;
        }
      }
    }

    std::cout << "Verified " << numberVerified << " new pixels." << std::endl;
  }

protected:

  Mask* ProcessMask;

  TVerifyFunctor* VerifyFunctor;

};


template <typename TImage>
class VerifierNeighborHistogram
{

protected:
  TImage* Image;

  typedef itk::Image<Match, 2> MatchImageType;
  MatchImageType* MatchImage;

  unsigned int PatchRadius;

  float NeighborHistogramMultiplier;

  typename TypeTraits<typename TImage::PixelType>::ComponentType RangeMin;
  typename TypeTraits<typename TImage::PixelType>::ComponentType RangeMax;

public:
  VerifierNeighborHistogram() :Image(NULL), MatchImage(NULL), PatchRadius(0), NeighborHistogramMultiplier(2.0f)
  {
    this->RangeMin = itk::NumericTraits<typename TypeTraits<typename TImage::PixelType>::ComponentType>::min();
    this->RangeMax = itk::NumericTraits<typename TypeTraits<typename TImage::PixelType>::ComponentType>::max();
  }

  void SetPatchRadius(const unsigned int patchRadius)
  {
    this->PatchRadius = patchRadius;
  }

  void SetImage(TImage* const image)
  {
    this->Image = image;
  }

  void SetMatchImage(MatchImageType* const matchImage)
  {
    this->MatchImage = matchImage;
  }

  bool Verify(const itk::Index<2>& queryCenter)
  {
//     std::cout << "AcceptanceTestNeighborHistogram: RangeMin = " << static_cast<float>(this->RangeMin) << std::endl;
//     std::cout << "AcceptanceTestNeighborHistogram: RangeMax = " << static_cast<float>(this->RangeMax) << std::endl;
    assert(this->PatchRadius > 0);
    assert(this->Image);
    assert(this->MatchImage);
    assert(this->RangeMax > this->RangeMin);
    assert(this->NeighborHistogramMultiplier > 1.0f);

    itk::ImageRegion<2> queryRegion = ITKHelpers::GetRegionInRadiusAroundPixel(queryCenter, this->PatchRadius);
    itk::ImageRegion<2> sourceRegion = this->MatchImage->GetPixel(queryCenter).Region;

    const unsigned int numberOfBinsPerDimension = 20;

    typedef Histogram<int>::HistogramType HistogramType;
    HistogramType queryHistogram = Histogram<int>::ComputeImageHistogram1D(
                  this->Image, queryRegion, numberOfBinsPerDimension, this->RangeMin, this->RangeMax);

    HistogramType sourceHistogram =
        Histogram<int>::ComputeImageHistogram1D(this->Image,
                                              sourceRegion, numberOfBinsPerDimension,
                                              this->RangeMin, this->RangeMax);

    itk::Offset<2> randomNeighborOffset = PatchMatchHelpers::RandomNeighborNonZeroOffset();

    itk::Index<2> neighborCenter = queryCenter + randomNeighborOffset;

    itk::ImageRegion<2> neighborRegion =
      ITKHelpers::GetRegionInRadiusAroundPixel(neighborCenter, this->PatchRadius);
    HistogramType neighborHistogram = Histogram<int>::ComputeImageHistogram1D(
                  this->Image, neighborRegion, numberOfBinsPerDimension, this->RangeMin, this->RangeMax);

    float neighborHistogramDifference =
      Histogram<int>::HistogramDifference(neighborHistogram, queryHistogram);

    float matchHistogramDifference =
      Histogram<int>::HistogramDifference(queryHistogram, sourceHistogram);

    if(matchHistogramDifference < (this->NeighborHistogramMultiplier * neighborHistogramDifference))
    {
//       std::cout << "VerifierNeighborHistogram::Verify()"
//                 << " Match Histogram score: " << matchHistogramDifference
//                 << " vs neighbor histogram score " << neighborHistogramDifference << std::endl;
      return true; // Verified
    }

    return false; // Not verified
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

};

#endif
