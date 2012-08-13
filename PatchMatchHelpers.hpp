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

//typedef itk::Image<itk::CovariantVector<float, 3>, 2> CoordinateImageType;
typedef itk::VectorImage<float, 2> CoordinateImageType;

template <typename MatchImageType, typename CoordinateImageType>
void GetPatchCentersImage(const MatchImageType* const matchImage, CoordinateImageType* const output)
{
  output->SetRegions(matchImage->GetLargestPossibleRegion());
  unsigned int numberOfComponents = 5;
  output->SetNumberOfComponentsPerPixel(numberOfComponents); // Currently we write (X,Y,Score,Verified)
  output->Allocate();

  itk::ImageRegionConstIterator<MatchImageType> imageIterator(matchImage,
                                                              matchImage->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    typename CoordinateImageType::PixelType pixel;
    pixel.SetSize(numberOfComponents);

    Match match = imageIterator.Get().GetMatch(0);
    itk::Index<2> center = ITKHelpers::GetRegionCenter(match.GetRegion());

    pixel[0] = center[0];
    pixel[1] = center[1];
    pixel[2] = match.GetSSDScore();
    pixel[3] = match.GetVerificationScore();
    pixel[4] = match.IsVerified();

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

void WriteValidPixels(const NNFieldType* const nnField, const std::string& fileName)
{
  // This function writes the validity of the first Match in the MatchSet at every pixel
  // to an image.

  typedef itk::Image<unsigned char> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(nnField->GetLargestPossibleRegion());
  image->Allocate();
  image->FillBuffer(0);

  itk::ImageRegionConstIterator<NNFieldType> imageIterator(nnField,
                                                           nnField->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
  {
    if(imageIterator.Get().GetMatch(0).IsValid())
    {
      image->SetPixel(imageIterator.GetIndex(), 255);
    }

    ++imageIterator;
  }

  ITKHelpers::WriteImage(image.GetPointer(), fileName);
}

} // end PatchMatchHelpers namespace

#endif
