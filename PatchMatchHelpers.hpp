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

#ifndef PatchMatchHelpers_HPP
#define PatchMatchHelpers_HPP

// STL
#include <limits>

namespace PatchMatchHelpers
{

template <typename NNFieldType, typename CoordinateImageType>
void GetPatchCentersImage(const NNFieldType* const matchImage, CoordinateImageType* const output)
{
  output->SetRegions(matchImage->GetLargestPossibleRegion());
  output->Allocate();

  itk::ImageRegionConstIterator<NNFieldType> imageIterator(matchImage,
                                                           matchImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
  {
    typename CoordinateImageType::PixelType pixel;

    Match match = imageIterator.Get();

    itk::Index<2> center = ITKHelpers::GetRegionCenter(match.GetRegion());

    pixel[0] = center[0];
    pixel[1] = center[1];

    output->SetPixel(imageIterator.GetIndex(), pixel);
    ++imageIterator;
  }
}


/** Get an image where the channels are (x component, y component, score) from the nearest
  * neighbor field struct. */
template <typename NNFieldType>
void WriteNNField(const NNFieldType* const nnField, const std::string& fileName)
{
  PatchMatchHelpers::CoordinateImageType::Pointer coordinateImage = PatchMatchHelpers::CoordinateImageType::New();
  PatchMatchHelpers::GetPatchCentersImage(nnField, coordinateImage.GetPointer());
  ITKHelpers::WriteImage(coordinateImage.GetPointer(), fileName);
}

} // end PatchMatchHelpers namespace

#endif
