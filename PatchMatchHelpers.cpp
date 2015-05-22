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

#include "PatchMatchHelpers.h"


namespace PatchMatchHelpers
{

itk::ImageRegion<2> GetRandomRegionInRegion(const itk::ImageRegion<2>& region, const unsigned int patchRadius)
{
    itk::Index<2> randomPixel;
    randomPixel[0] = Helpers::RandomInt(region.GetIndex()[0], region.GetIndex()[0] + region.GetSize()[0]);
    randomPixel[1] = Helpers::RandomInt(region.GetIndex()[1], region.GetIndex()[1] + region.GetSize()[1]);

    itk::ImageRegion<2> randomRegion = ITKHelpers::GetRegionInRadiusAroundPixel(randomPixel, patchRadius);

    return randomRegion;
}


void ReadNNField(const std::string& fileName, const unsigned int patchRadius, NNFieldType* const nnField)
{
  typedef itk::ImageFileReader<CoordinateImageType> NNFieldReaderType;
  NNFieldReaderType::Pointer nnFieldReader = NNFieldReaderType::New();
  nnFieldReader->SetFileName(fileName);
  nnFieldReader->Update();

  itk::ImageRegionIterator<CoordinateImageType>
      imageIterator(nnFieldReader->GetOutput(),
                    nnFieldReader->GetOutput()->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
  {
    Match match;
    itk::Index<2> center = {{static_cast<unsigned int>(imageIterator.Get()[0]),
                             static_cast<unsigned int>(imageIterator.Get()[1])}};
    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(center, patchRadius);
    match.SetRegion(region);

    nnField->SetPixel(imageIterator.GetIndex(), match);

    ++imageIterator;
  }
}

itk::Index<2> GetRandomPixelInRegion(const itk::ImageRegion<2>& region)
{
    itk::Index<2> pixel;
    pixel[0] = region.GetIndex()[0] + Helpers::RandomInt(0, region.GetSize()[0] - 1);
    pixel[1] = region.GetIndex()[1] + Helpers::RandomInt(0, region.GetSize()[1] - 1);

    return pixel;
}

std::vector<itk::Index<2> > GetAllPixelIndices(const itk::ImageRegion<2>& region)
{
  std::vector<itk::Index<2> > pixelIndices;

  typedef itk::Image<int, 2> DummyImageType;
  DummyImageType::Pointer dummyImage = DummyImageType::New();
  dummyImage->SetRegions(region);
  dummyImage->Allocate();

  itk::ImageRegionIteratorWithIndex<DummyImageType> imageIterator(dummyImage, region);

  while(!imageIterator.IsAtEnd())
  {
    pixelIndices.push_back(imageIterator.GetIndex());
    ++imageIterator;
  }

  return pixelIndices;
}

} // namespace PatchMatchHelpers
