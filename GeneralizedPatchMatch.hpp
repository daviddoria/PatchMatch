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

#ifndef GeneralizedPatchMatch_HPP
#define GeneralizedPatchMatch_HPP

#include "GeneralizedPatchMatch.h"

template <typename TImage>
GeneralizedPatchMatch<TImage>::GeneralizedPatchMatch()
{
}


template <typename TImage>
void GeneralizedPatchMatch<TImage>::GetPatchCentersImage(GeneralizedPMImageType* const pmImage,
                                                         typename PatchMatch<TImage>::CoordinateImageType* const output)
{
  output->SetRegions(pmImage->GetLargestPossibleRegion());
  output->SetNumberOfComponentsPerPixel(3);
  output->Allocate();

  itk::ImageRegionIterator<GeneralizedPMImageType> imageIterator(pmImage, pmImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    typename PatchMatch<TImage>::CoordinateImageType::PixelType pixel;
    pixel.SetSize(3);

    Match match = imageIterator.Get()[0]; // This is the only difference from this function in PatchMatch - that we get the first element instead of the only element
    itk::Index<2> center = ITKHelpers::GetRegionCenter(match.Region);

    pixel[0] = center[0];
    pixel[1] = center[1];
    pixel[2] = match.Score;

    output->SetPixel(imageIterator.GetIndex(), pixel);

    ++imageIterator;
    }
}

template <typename TImage>
typename GeneralizedPatchMatch<TImage>::GeneralizedPMImageType* GeneralizedPatchMatch<TImage>::GetOutput()
{
  return Output;
}

#endif
