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
  unsigned int numberOfComponents = 3;
  output->SetNumberOfComponentsPerPixel(numberOfComponents);
  output->Allocate();

  itk::ImageRegionConstIterator<NNFieldType> imageIterator(matchImage,
                                                           matchImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
  {
    typename CoordinateImageType::PixelType pixel;
    pixel.SetSize(numberOfComponents);

    if(imageIterator.Get().GetNumberOfMatches() > 0)
    {
      MatchSet matchSet = imageIterator.Get();
      Match match = matchSet.GetMatch(0);
      itk::Index<2> center = ITKHelpers::GetRegionCenter(match.GetRegion());

      pixel[0] = center[0];
      pixel[1] = center[1];
      pixel[2] = matchSet.HasVerifiedMatch();

//       pixel[2] = match.GetSSDScore();
//       pixel[3] = match.GetVerificationScore();
//       pixel[4] = match.IsVerified();
//       pixel[5] = matchSet.HasVerifiedMatch();
    }
    else
    {
      pixel[0] = std::numeric_limits<float>::quiet_NaN();
      pixel[1] = std::numeric_limits<float>::quiet_NaN();
      pixel[2] = std::numeric_limits<float>::quiet_NaN();
    }

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

/** Count how many pixels in the 'nnField' which are Valid in the 'mask' pass (return true) the testFunctor. */
template <typename NNFieldType, typename TTestFunctor>
unsigned int CountTestedPixels(const NNFieldType* const nnField, const Mask* const mask,
                               TTestFunctor testFunctor)
{
  std::vector<itk::Index<2> > pixelsToTest = mask->GetValidPixels();

  unsigned int numberOfPassPixels = 0;
  for(size_t pixelId = 0; pixelId < pixelsToTest.size(); ++pixelId)
  {
    if(testFunctor(nnField->GetPixel(pixelsToTest[pixelId])))
    {
      numberOfPassPixels++;
    }
  }

  return numberOfPassPixels;
}

template <typename MatchImageType>
unsigned int CountInvalidPixels(const MatchImageType* const nnField, const Mask* const mask)
{
  std::vector<itk::Index<2> > regionPixels = mask->GetValidPixels();

  unsigned int numberOfInvalidPixels = 0;
  for(size_t pixelId = 0; pixelId < regionPixels.size(); ++pixelId)
  {
    if(!nnField->GetPixel(regionPixels[pixelId]).IsValid())
    {
      numberOfInvalidPixels++;
    }
  }

  return numberOfInvalidPixels;
}

template <typename TImage, typename TTestFunction>
void CopyPixelsIf(const TImage* const oldImage, const TImage* const possibleNewImage,
                  TTestFunction testFunction, TImage* const output)
{
  itk::ImageRegionConstIterator<TImage> oldImageIterator(oldImage, oldImage->GetLargestPossibleRegion());

  while(!oldImageIterator.IsAtEnd())
  {
    if(testFunction(oldImageIterator.Get(), possibleNewImage->GetPixel(oldImageIterator.GetIndex())))
    {
      output->SetPixel(oldImageIterator.GetIndex(), possibleNewImage->GetPixel(oldImageIterator.GetIndex()));
    }
    ++oldImageIterator;
  }
}

} // end PatchMatchHelpers namespace

#endif
