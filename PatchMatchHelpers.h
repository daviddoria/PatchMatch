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

#ifndef PatchMatchHelpers_H
#define PatchMatchHelpers_H

// ITK
#include "itkImageRegionConstIterator.h"
#include "itkImage.h"
#include "itkCovariantVector.h"

namespace PatchMatchHelpers
{

itk::Offset<2> RandomNeighborNonZeroOffset()
{
  int randomOffsetX = 0;
  int randomOffsetY = 0;
  while((randomOffsetX == 0) && (randomOffsetY == 0) ) // We don't want the random offset to be (0,0), because the histogram difference would be zero!
  {
    // Generate random numbers in the set (-1,0,1) by generating a number in (0,1,2) and then subtracting 1
    randomOffsetX = rand()%3 - 1;
    randomOffsetY = rand()%3 - 1;
  }

  itk::Offset<2> randomNeighborNonZeroOffset = {{randomOffsetX, randomOffsetY}};

  return randomNeighborNonZeroOffset;
}

typedef itk::Image<itk::CovariantVector<float, 3>, 2> CoordinateImageType;

template <typename MatchImageType, typename CoordinateImageType>
void GetPatchCentersImage(const MatchImageType* const matchImage, CoordinateImageType* const output)
{
  output->SetRegions(matchImage->GetLargestPossibleRegion());
  output->Allocate();

  itk::ImageRegionConstIterator<MatchImageType> imageIterator(matchImage,
                                                              matchImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    typename CoordinateImageType::PixelType pixel;

    Match match = imageIterator.Get();
    itk::Index<2> center = ITKHelpers::GetRegionCenter(match.Region);

    pixel[0] = center[0];
    pixel[1] = center[1];
    pixel[2] = match.Score;

    output->SetPixel(imageIterator.GetIndex(), pixel);

    ++imageIterator;
    }
}


/** Get an image where the channels are (x component, y component, score) from the nearest
  * neighbor field struct. */
template <typename MatchImageType>
void WriteNNField(const MatchImageType* const nnField, const std::string& fileName)
{
  PatchMatchHelpers::CoordinateImageType::Pointer coordinateImage = PatchMatchHelpers::CoordinateImageType::New();
  PatchMatchHelpers::GetPatchCentersImage(nnField, coordinateImage.GetPointer());
  ITKHelpers::WriteImage(coordinateImage.GetPointer(), fileName);
}

} // end PatchMatchHelpers namespace

#endif
