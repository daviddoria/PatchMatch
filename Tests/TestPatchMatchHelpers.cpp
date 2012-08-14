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

/** This program computes the NN field of an image. */

// STL
#include <iostream>

// ITK
#include "itkImage.h"

// Submodules
#include "Mask/Mask.h"
#include "Mask/ITKHelpers/ITKHelpers.h"

// Custom
#include "PatchMatchHelpers.h"

// Helpers
template <typename TImage>
void OutputPixelsLinear(const TImage* const image);

// Tests
static void TestCopyPixelsIf();

int main(int argc, char*argv[])
{
  TestCopyPixelsIf();

  return EXIT_SUCCESS;
}

static void TestCopyPixelsIf()
{
  typedef itk::Image<int, 2> ImageType;

  itk::Index<2> corner = {{0,0}};
  itk::Size<2> size = {{3,2}};
  itk::ImageRegion<2> region(corner,size);

  ImageType::Pointer oldImage = ImageType::New();
  oldImage->SetRegions(region);
  oldImage->Allocate();
  oldImage->FillBuffer(0);

  ImageType::Pointer possibleNewImage = ImageType::New();
  possibleNewImage->SetRegions(region);
  possibleNewImage->Allocate();
  possibleNewImage->FillBuffer(0);

  itk::Index<2> pixel1 = {{0,0}};
  possibleNewImage->SetPixel(pixel1, 3);

  itk::Index<2> pixel2 = {{1,1}};
  possibleNewImage->SetPixel(pixel2, 45);

  // Only copy pixels where the different in two corresponding pixels in the two images is greater than 10
  auto testFunction = [](const ImageType::PixelType& pixel1, const ImageType::PixelType& pixel2)
                        {
                          if(abs(pixel1 - pixel2) > 10)
                          {
                            return true;
                          }
                          return false;
                        };

  ImageType::Pointer output = ImageType::New();
  ITKHelpers::DeepCopy(oldImage.GetPointer(), output.GetPointer());

  // This should keep all input pixels, including the one where the possibleNewImage is 3 (because the difference (3-0) is only 3,
  // but it should change the 0 to a 45 at pixel2 because the difference is (45-0) = 45
  PatchMatchHelpers::CopyPixelsIf(oldImage.GetPointer(), possibleNewImage.GetPointer(),
                                  testFunction, output.GetPointer());

  OutputPixelsLinear(output.GetPointer());
}

template <typename TImage>
void OutputPixelsLinear(const TImage* const image)
{
  itk::ImageRegionConstIterator<TImage> imageIterator(image, image->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
  {
    std::cout << imageIterator.Get() << std::endl;
    ++imageIterator;
  }
}