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
  this->MaskImage = Mask::New();
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

  for(unsigned int i = 0; i < this->Iterations; i++)
  {
    std::cout << "PatchMatch iteration " << i << std::endl;

    // PROPAGATION
    if (forwardSearch)
    {
      // Iterate over patch centers
      itk::ImageRegionIteratorWithIndex<PMImageType> outputIterator(this->Output,
                                                                   this->Output->GetLargestPossibleRegion());

      // Forward propagation - compare left (-1, 0), center (0,0) and up (0, -1)

      while(!outputIterator.IsAtEnd())
      {
        itk::Index<2> center = outputIterator.GetIndex();

        itk::Index<2> left = outputIterator.GetIndex();
        left[0] += -1;

        itk::Index<2> up = outputIterator.GetIndex();
        up[1] += -1;

        Match currentMatch = outputIterator.Get();

        float distLeft = distance(ITKHelpers::GetRegionInRadiusAroundPixel(center, patchRadius),
                                  ITKHelpers::GetRegionInRadiusAroundPixel(left, patchRadius),
                                  currentMatch.Score);

        if (distLeft < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(left, patchRadius);
          currentMatch.Score = distLeft;
        }

        float distUp = distance(ITKHelpers::GetRegionInRadiusAroundPixel(center, patchRadius),
                                ITKHelpers::GetRegionInRadiusAroundPixel(up, patchRadius),
                                currentMatch.Score);

        if (distUp < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(up, patchRadius);
          currentMatch.Score = distUp;
        }

        outputIterator.Set(currentMatch);
        ++outputIterator;
      } // end forward loop

    } // end if (forwardSearch)
    else
    {
      // Iterate over patch centers in reverse
      itk::ImageRegionReverseIterator<PMImageType> outputIterator(this->Output,
                                                                  this->Output->GetLargestPossibleRegion());

      // Backward propagation - compare right (1, 0) , center (0,0) and down (0, 1)

      while(!outputIterator.IsAtEnd())
      {
        itk::Index<2> center = outputIterator.GetIndex();

        itk::Index<2> right = outputIterator.GetIndex();
        right[0] += 1;

        itk::Index<2> down = outputIterator.GetIndex();
        down[1] += 1;

        Match currentMatch = outputIterator.Get();

        float distRight = distance(ITKHelpers::GetRegionInRadiusAroundPixel(center, patchRadius),
                                   ITKHelpers::GetRegionInRadiusAroundPixel(right, patchRadius),
                                   currentMatch.Score);

        if (distRight < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(right, patchRadius);
          currentMatch.Score = distRight;
        }

        float distDown = distance(ITKHelpers::GetRegionInRadiusAroundPixel(center, patchRadius),
                                  ITKHelpers::GetRegionInRadiusAroundPixel(down, patchRadius),
                                  currentMatch.Score);

        if (distDown < currentMatch.Score)
        {
          currentMatch.Region = ITKHelpers::GetRegionInRadiusAroundPixel(down, patchRadius);
          currentMatch.Score = distDown;
        }

        outputIterator.Set(currentMatch);

        ++outputIterator;
      } // end backward loop
    } // end else ForwardSearch

    forwardSearch = !forwardSearch;

    // RANDOM SEARCH - try a random region in smaller windows around the current best patch.

    // Iterate over patch centers
    itk::ImageRegionIteratorWithIndex<PMImageType> outputIterator(this->Output,
                                                                 this->Output->GetLargestPossibleRegion());
    while(!outputIterator.IsAtEnd())
    {
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
    ss << "PatchMatch_" << Helpers::ZeroPad(i, 2) << ".mha";
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
    itk::ImageRegionIteratorWithIndex<Mask> sourceMaskIterator(this->MaskImage, source);
    itk::ImageRegionIteratorWithIndex<Mask> targetMaskIterator(this->MaskImage, target);

    unsigned int numberOfPixelsCompared = 0;

    while(!sourceIterator.IsAtEnd())
    {
      if(this->MaskImage->IsValid(sourceMaskIterator.GetIndex()) &&
         this->MaskImage->IsValid(targetMaskIterator.GetIndex()))
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

  itk::Index<2> regionCorner = {{patchRadius, patchRadius}};
  itk::Size<2> regionSize = {{width - 2*patchRadius, height - 2*patchRadius}};
  itk::ImageRegion<2> region(regionCorner, regionSize);

  std::cout << "Initializing region: " << region << std::endl;
  itk::ImageRegionIteratorWithIndex<PMImageType> outputIterator(this->Output, region);

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

  std::cout << "Finished initializing." << region << std::endl;
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

void PatchMatch::SetMask(Mask* const mask)
{
  this->MaskImage->DeepCopyFrom(mask);
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
