#include "PatchMatch.h"

// Submodules
#include "Mask/ITKHelpers/ITKHelpers.h"

// ITK
#include "itkImageRegionReverseIterator.h"

// STL
#include <ctime>

PatchMatch::PatchMatch()
{
  this->Output = PMImageType::New();
  this->Image = ImageType::New();
  this->SourceMask = Mask::New();
  this->TargetMask = Mask::New();
}

void PatchMatch::Compute(PMImageType* const initialization)
{
  srand(time(NULL));

  if(initialization)
  {
    ITKHelpers::DeepCopy(initialization, this->Output.GetPointer());
  }
  else
  {
    this->Output->SetRegions(this->Image->GetLargestPossibleRegion());
    this->Output->Allocate();
    RandomInit();
  }

  {
  VectorImageType::Pointer initialOutput = VectorImageType::New();
  GetPatchCentersImage(this->Output, initialOutput);
  ITKHelpers::WriteImage(initialOutput.GetPointer(), "initialization.mha");
  }

  unsigned int width = Image->GetLargestPossibleRegion().GetSize()[0];
  unsigned int height = Image->GetLargestPossibleRegion().GetSize()[1];

  // Convert patch diameter to patch radius
  int patchRadius = this->PatchDiameter / 2;

  bool forwardSearch = true;

  for(unsigned int iteration = 0; iteration < this->Iterations; ++iteration)
  {
    std::cout << "PatchMatch iteration " << iteration << std::endl;

    // PROPAGATION
    if (forwardSearch)
    {
      itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), patchRadius);
      // Iterate over patch centers
      itk::ImageRegionIteratorWithIndex<PMImageType> outputIterator(this->Output,
                                                                    internalRegion);

      // Forward propagation - compare left (-1, 0), center (0,0) and up (0, -1)

      while(!outputIterator.IsAtEnd())
      {
        // Only compute the NN-field where the target mask is valid
        if(!this->TargetMask->IsValid(outputIterator.GetIndex()))
        {
          ++outputIterator;
          continue;
        }

        // For inpainting, most of the NN-field will be an exact match. We don't have to search anymore
        // once the exact match is found.
        if(outputIterator.Get().Score == 0)
        {
          ++outputIterator;
          continue;
        }
        itk::Index<2> center = outputIterator.GetIndex();

        itk::Index<2> leftPixel = outputIterator.GetIndex();
        leftPixel[0] += -1;
        itk::Index<2> leftMatch = ITKHelpers::GetRegionCenter(this->Output->GetPixel(leftPixel).Region);
        leftMatch[0] += 1;

        itk::Index<2> upPixel = outputIterator.GetIndex();
        upPixel[1] += -1;
        itk::Index<2> upMatch = ITKHelpers::GetRegionCenter(this->Output->GetPixel(upPixel).Region);
        upMatch[1] += 1;

        Match currentMatch = outputIterator.Get();

        float distLeft = distance(ITKHelpers::GetRegionInRadiusAroundPixel(center, patchRadius),
                                  ITKHelpers::GetRegionInRadiusAroundPixel(leftMatch, patchRadius),
                                  currentMatch.Score);

        if (distLeft < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(leftMatch, patchRadius);
          currentMatch.Score = distLeft;
        }

        float distUp = distance(ITKHelpers::GetRegionInRadiusAroundPixel(center, patchRadius),
                                ITKHelpers::GetRegionInRadiusAroundPixel(upMatch, patchRadius),
                                currentMatch.Score);

        if (distUp < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(upMatch, patchRadius);
          currentMatch.Score = distUp;
        }

        outputIterator.Set(currentMatch);
        ++outputIterator;
      } // end forward loop

    } // end if (forwardSearch)
    else
    {
      // Iterate over patch centers in reverse
      itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), patchRadius);
      itk::ImageRegionReverseIterator<PMImageType> outputIterator(this->Output,
                                                                  internalRegion);

      // Backward propagation - compare right (1, 0) , center (0,0) and down (0, 1)

      while(!outputIterator.IsAtEnd())
      {
        // Only compute the NN-field where the target mask is valid
        if(!this->TargetMask->IsValid(outputIterator.GetIndex()))
        {
          ++outputIterator;
          continue;
        }

        // For inpainting, most of the NN-field will be an exact match. We don't have to search anymore
        // once the exact match is found.
        if(outputIterator.Get().Score == 0)
        {
          ++outputIterator;
          continue;
        }
        itk::Index<2> center = outputIterator.GetIndex();
        itk::ImageRegion<2> currentRegion = ITKHelpers::GetRegionInRadiusAroundPixel(center, patchRadius);

        itk::Index<2> rightPixel = outputIterator.GetIndex();
        rightPixel[0] += 1;
        itk::Index<2> rightMatch = ITKHelpers::GetRegionCenter(this->Output->GetPixel(rightPixel).Region);
        rightMatch[0] += -1;

        itk::Index<2> downPixel = outputIterator.GetIndex();
        downPixel[1] += 1;
        itk::Index<2> downMatch = ITKHelpers::GetRegionCenter(this->Output->GetPixel(downPixel).Region);
        downMatch[1] += -1;

        Match currentMatch = outputIterator.Get();

        float distRight = distance(currentRegion,
                                   ITKHelpers::GetRegionInRadiusAroundPixel(rightMatch, patchRadius),
                                   currentMatch.Score);

        if (distRight < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(rightMatch, patchRadius);
          currentMatch.Score = distRight;
        }

        float distDown = distance(currentRegion,
                                  ITKHelpers::GetRegionInRadiusAroundPixel(downMatch, patchRadius),
                                  currentMatch.Score);

        if (distDown < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(downMatch, patchRadius);
          currentMatch.Score = distDown;
        }

        outputIterator.Set(currentMatch);

        ++outputIterator;
      } // end backward loop
    } // end else ForwardSearch

    forwardSearch = !forwardSearch;

    // RANDOM SEARCH - try a random region in smaller windows around the current best patch.

    // Iterate over patch centers
    itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), patchRadius);
    itk::ImageRegionIteratorWithIndex<PMImageType> outputIterator(this->Output,
                                                                  internalRegion);
    while(!outputIterator.IsAtEnd())
    {
      // Only compute the NN-field where the target mask is valid
      if(!this->TargetMask->IsValid(outputIterator.GetIndex()))
      {
        ++outputIterator;
        continue;
      }

      // For inpainting, most of the NN-field will be an exact match. We don't have to search anymore
      // once the exact match is found.
      if(outputIterator.Get().Score == 0)
      {
        ++outputIterator;
        continue;
      }
      itk::Index<2> center = outputIterator.GetIndex();

      Match currentMatch = outputIterator.Get();

      int radius = std::max(width, height);

      // Search an exponentially smaller window each time through the loop
      itk::Index<2> searchRegionCenter = ITKHelpers::GetRegionCenter(outputIterator.Get().Region);

      while (radius > 8)
      {
        itk::ImageRegion<2> searchRegion = ITKHelpers::GetRegionInRadiusAroundPixel(searchRegionCenter, radius);
        searchRegion.Crop(this->Image->GetLargestPossibleRegion());

        int randX = Helpers::RandomInt(searchRegion.GetIndex()[0],
                                       searchRegion.GetIndex()[0] + searchRegion.GetSize()[0]);

        int randY = Helpers::RandomInt(searchRegion.GetIndex()[1],
                                       searchRegion.GetIndex()[1] + searchRegion.GetSize()[1]);

        itk::Index<2> randomIndex = {{randX, randY}};
        float dist = distance(ITKHelpers::GetRegionInRadiusAroundPixel(center, patchRadius),
                              ITKHelpers::GetRegionInRadiusAroundPixel(randomIndex, patchRadius),
                              currentMatch.Score);

        if (dist < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(randomIndex, patchRadius);
          currentMatch.Score = dist;
        }

        outputIterator.Set(currentMatch);

        radius /= 2;
      } // end while radius

      ++outputIterator;
    } // end random search loop

    std::stringstream ss;
    ss << "PatchMatch_" << Helpers::ZeroPad(iteration, 2) << ".mha";
    VectorImageType::Pointer temp = VectorImageType::New();
    GetPatchCentersImage(this->Output, temp);
    ITKHelpers::WriteImage(temp.GetPointer(), ss.str());

  } // end for (int i = 0; i < iterations; i++)

}

float PatchMatch::distance(const itk::ImageRegion<2>& source,
                           const itk::ImageRegion<2>& target,
                           const float prevDist)
{
    // Do not use patches on boundaries
    if(!this->Output->GetLargestPossibleRegion().IsInside(source) ||
       !this->Output->GetLargestPossibleRegion().IsInside(target))
    {
      return std::numeric_limits<float>::max();
    }

    // Compute distance between patches
    // Average L2 distance in RGB space
    float distance = 0.0f;

    itk::ImageRegionIteratorWithIndex<ImageType> sourceIterator(this->Image, source);
    itk::ImageRegionIteratorWithIndex<ImageType> targetIterator(this->Image, target);
    itk::ImageRegionIteratorWithIndex<Mask> sourceMaskIterator(this->SourceMask, source);
    itk::ImageRegionIteratorWithIndex<Mask> targetMaskIterator(this->TargetMask, target);

    unsigned int numberOfPixelsCompared = 0;

    while(!sourceIterator.IsAtEnd())
    {
      if(this->SourceMask->IsValid(sourceMaskIterator.GetIndex()) &&
         this->TargetMask->IsValid(targetMaskIterator.GetIndex()))
      {
        numberOfPixelsCompared++;

        ImageType::PixelType sourcePixel = sourceIterator.Get();
        ImageType::PixelType targetPixel = targetIterator.Get();

        distance += sqrt( (sourcePixel[0] - targetPixel[0]) * (sourcePixel[0] - targetPixel[0]) +
                          (sourcePixel[1] - targetPixel[1]) * (sourcePixel[1] - targetPixel[1]) +
                          (sourcePixel[2] - targetPixel[2]) * (sourcePixel[2] - targetPixel[2]));

        // Early termination
        if(distance / static_cast<float>(numberOfPixelsCompared) > prevDist)
        {
          return std::numeric_limits<float>::max();
        }

      }
    ++sourceIterator;
    ++targetIterator;
    ++sourceMaskIterator;
    ++targetMaskIterator;
    }

  if(numberOfPixelsCompared == 0)
  {
    return std::numeric_limits<float>::max();
  }

  distance /= static_cast<float>(numberOfPixelsCompared);

  return distance;
}

void PatchMatch::RandomInit()
{
  unsigned int width = this->Output->GetLargestPossibleRegion().GetSize()[0];
  unsigned int height = this->Output->GetLargestPossibleRegion().GetSize()[1];

  // Create a zero region
  itk::Index<2> zeroIndex = {{0,0}};
  itk::Size<2> zeroSize = {{0,0}};
  itk::ImageRegion<2> zeroRegion(zeroIndex, zeroSize);
  Match zeroMatch;
  zeroMatch.Region = zeroRegion;
  zeroMatch.Score = 0.0f;

  ITKHelpers::SetImageToConstant(this->Output.GetPointer(), zeroMatch);

  unsigned int patchRadius = PatchDiameter/2;

  itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), patchRadius);

  // std::cout << "Initializing region: " << internalRegion << std::endl;
  itk::ImageRegionIteratorWithIndex<PMImageType> outputIterator(this->Output, internalRegion);

  while(!outputIterator.IsAtEnd())
  {
    // Construct the current region
    itk::Index<2> currentIndex = outputIterator.GetIndex();
    itk::ImageRegion<2> currentRegion = ITKHelpers::GetRegionInRadiusAroundPixel(currentIndex, patchRadius);

    // Construct a random region
    int randX = Helpers::RandomInt(patchRadius, width - patchRadius - 1);
    int randY = Helpers::RandomInt(patchRadius, height - patchRadius - 1);

    Match randomMatch;
    itk::Index<2> randomIndex = {{randX, randY}};
    itk::ImageRegion<2> randomRegion = ITKHelpers::GetRegionInRadiusAroundPixel(randomIndex, patchRadius);
    randomMatch.Region = randomRegion;
    randomMatch.Score = distance(currentRegion, randomRegion, std::numeric_limits<float>::max());
    outputIterator.Set(randomMatch);

    ++outputIterator;
  }

  //std::cout << "Finished initializing." << internalRegion << std::endl;
}

PatchMatch::PMImageType* PatchMatch::GetOutput()
{
  return Output;
}

void PatchMatch::SetIterations(const unsigned int iterations)
{
  this->Iterations = iterations;
}

void PatchMatch::SetPatchDiameter(const unsigned int patchDiameter)
{
  assert(patchDiameter >= 3 && (patchDiameter & 1)); // patchDiameter must be at least 3 and odd
  
  this->PatchDiameter = patchDiameter;
}

void PatchMatch::SetImage(ImageType* const image)
{
  ITKHelpers::DeepCopy(image, this->Image.GetPointer());
}

void PatchMatch::SetSourceMask(Mask* const mask)
{
  this->SourceMask->DeepCopyFrom(mask);
}

void PatchMatch::SetTargetMask(Mask* const mask)
{
  this->TargetMask->DeepCopyFrom(mask);
}

void PatchMatch::GetPatchCentersImage(PMImageType* const pmImage, itk::VectorImage<float, 2>* const output)
{
  output->SetRegions(pmImage->GetLargestPossibleRegion());
  output->SetNumberOfComponentsPerPixel(3);
  output->Allocate();

  itk::ImageRegionIterator<PMImageType> imageIterator(pmImage, pmImage->GetLargestPossibleRegion());

  typedef itk::VectorImage<float, 2> VectorImageType;
  
  while(!imageIterator.IsAtEnd())
    {
    VectorImageType::PixelType pixel;
    pixel.SetSize(3);

    itk::Index<2> center = ITKHelpers::GetRegionCenter(imageIterator.Get().Region);

    pixel[0] = center[0];
    pixel[1] = center[1];
    pixel[2] = imageIterator.Get().Score;

    output->SetPixel(imageIterator.GetIndex(), pixel);
    
    ++imageIterator;
    }
}
