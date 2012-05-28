#include "PatchMatch.h"

// Submodules
#include "Mask/ITKHelpers/ITKHelpers.h"
#include "Mask/MaskOperations.h"

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

  bool forwardSearch = true;

  for(unsigned int iteration = 0; iteration < this->Iterations; ++iteration)
  {
    std::cout << "PatchMatch iteration " << iteration << std::endl;

    // PROPAGATION
    if (forwardSearch)
    {
      std::cout << "Forward propagation..." << std::endl;
      itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), this->PatchRadius);
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

        float distLeft = distance(ITKHelpers::GetRegionInRadiusAroundPixel(center, this->PatchRadius),
                                  ITKHelpers::GetRegionInRadiusAroundPixel(leftMatch, this->PatchRadius),
                                  currentMatch.Score);

        if (distLeft < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(leftMatch, this->PatchRadius);
          currentMatch.Score = distLeft;
        }

        float distUp = distance(ITKHelpers::GetRegionInRadiusAroundPixel(center, this->PatchRadius),
                                ITKHelpers::GetRegionInRadiusAroundPixel(upMatch, this->PatchRadius),
                                currentMatch.Score);

        if (distUp < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(upMatch, this->PatchRadius);
          currentMatch.Score = distUp;
        }

        outputIterator.Set(currentMatch);
        ++outputIterator;
      } // end forward loop

    } // end if (forwardSearch)
    else
    {
      std::cout << "Backward propagation..." << std::endl;
      // Iterate over patch centers in reverse
      itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), this->PatchRadius);
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
        itk::ImageRegion<2> currentRegion = ITKHelpers::GetRegionInRadiusAroundPixel(center, this->PatchRadius);

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
                                   ITKHelpers::GetRegionInRadiusAroundPixel(rightMatch, this->PatchRadius),
                                   currentMatch.Score);

        if (distRight < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(rightMatch, this->PatchRadius);
          currentMatch.Score = distRight;
        }

        float distDown = distance(currentRegion,
                                  ITKHelpers::GetRegionInRadiusAroundPixel(downMatch, this->PatchRadius),
                                  currentMatch.Score);

        if (distDown < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(downMatch, this->PatchRadius);
          currentMatch.Score = distDown;
        }

        outputIterator.Set(currentMatch);

        ++outputIterator;
      } // end backward loop
    } // end else ForwardSearch

    forwardSearch = !forwardSearch;

    // RANDOM SEARCH - try a random region in smaller windows around the current best patch.
    std::cout << "Random search..." << std::endl;
    // Iterate over patch centers
    itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), this->PatchRadius);
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

      unsigned int radius = std::max(width, height);

      // Search an exponentially smaller window each time through the loop
      itk::Index<2> searchRegionCenter = ITKHelpers::GetRegionCenter(outputIterator.Get().Region);

      while (radius > this->PatchRadius * 2) // the *2 is arbitrary, just want a small-ish region
      {
        itk::ImageRegion<2> searchRegion = ITKHelpers::GetRegionInRadiusAroundPixel(searchRegionCenter, radius);
        searchRegion.Crop(this->Image->GetLargestPossibleRegion());

        itk::ImageRegion<2> randomValidRegion =
                 MaskOperations::GetRandomValidPatchInRegion(this->SourceMask.GetPointer(),
                                                             searchRegion, this->PatchRadius);

        // If no suitable region is found, move on
        if(randomValidRegion.GetSize()[0] == 0)
        {
          radius /= 2;
          continue;
        }

        itk::Index<2> randomPatchCenter = ITKHelpers::GetRegionCenter(randomValidRegion);
        float dist = distance(ITKHelpers::GetRegionInRadiusAroundPixel(center, this->PatchRadius),
                              ITKHelpers::GetRegionInRadiusAroundPixel(randomPatchCenter, this->PatchRadius),
                              currentMatch.Score);

        if (dist < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(randomPatchCenter, this->PatchRadius);
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
  assert(this->SourceMask->IsValid(source));

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

  unsigned int numberOfPixelsCompared = 0;

  while(!sourceIterator.IsAtEnd())
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

    ++sourceIterator;
    ++targetIterator;
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
  std::vector<itk::ImageRegion<2> > ValidSourceRegions =
        MaskOperations::GetAllFullyValidRegions(this->SourceMask, this->PatchRadius);

//   unsigned int width = this->Output->GetLargestPossibleRegion().GetSize()[0];
//   unsigned int height = this->Output->GetLargestPossibleRegion().GetSize()[1];

  // Create a zero region
  itk::Index<2> zeroIndex = {{0,0}};
  itk::Size<2> zeroSize = {{0,0}};
  itk::ImageRegion<2> zeroRegion(zeroIndex, zeroSize);
  Match zeroMatch;
  zeroMatch.Region = zeroRegion;
  zeroMatch.Score = 0.0f;

  ITKHelpers::SetImageToConstant(this->Output.GetPointer(), zeroMatch);

  itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), this->PatchRadius);

  // std::cout << "Initializing region: " << internalRegion << std::endl;
  itk::ImageRegionIteratorWithIndex<PMImageType> outputIterator(this->Output, internalRegion);

  while(!outputIterator.IsAtEnd())
  {
    // Construct the current region
    itk::Index<2> currentIndex = outputIterator.GetIndex();
    itk::ImageRegion<2> currentRegion = ITKHelpers::GetRegionInRadiusAroundPixel(currentIndex, this->PatchRadius);

    itk::ImageRegion<2> randomRegion = ValidSourceRegions[Helpers::RandomInt(0, ValidSourceRegions.size() - 1)];

    Match randomMatch;
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

void PatchMatch::SetPatchRadius(const unsigned int patchRadius)
{
  this->PatchRadius = patchRadius;
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
