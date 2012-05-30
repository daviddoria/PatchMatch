#include "BDSInpainting.h"

// Submodules
#include "Mask/ITKHelpers/ITKHelpers.h"
#include "Mask/MaskOperations.h"

// ITK
#include "itkImageRegionReverseIterator.h"

// STL
#include <ctime>

// Custom
#include "PatchMatch.h"

BDSInpainting::BDSInpainting() : ResolutionLevels(3), Iterations(5), PatchRadius(7), PatchMatchIterations(3), DownsampleFactor(.5)
{
  this->Output = ImageType::New();
  this->Image = ImageType::New();
  this->MaskImage = Mask::New();
}

void BDSInpainting::Compute()
{
  Mask::Pointer level0mask = Mask::New();
  level0mask->DeepCopyFrom(this->MaskImage);

  ImageType::Pointer level0Image = ImageType::New();
  ITKHelpers::DeepCopy(this->Image.GetPointer(), level0Image.GetPointer());

  // Cannot do this! The same image is added as each element of the vector.
  // This is std::vector behavior, nothing to do with ITK.
  // std::vector<ImageType::Pointer> imageLevels(this->ResolutionLevels, ImageType::New());
  std::vector<ImageType::Pointer> imageLevels(this->ResolutionLevels);
  std::vector<Mask::Pointer> maskLevels(this->ResolutionLevels);

  imageLevels[0] = level0Image;
  maskLevels[0] = level0mask;

  for(unsigned int level = 1; level < this->ResolutionLevels; ++level)
  {
    itk::Size<2> destinationSize;
    destinationSize[0] = imageLevels[level-1]->GetLargestPossibleRegion().GetSize()[0] * this->DownsampleFactor;
    destinationSize[1] = imageLevels[level-1]->GetLargestPossibleRegion().GetSize()[1] * this->DownsampleFactor;

    // Downsample
    ImageType::Pointer downsampledImage = ImageType::New();
    ITKHelpers::ScaleImage(imageLevels[level - 1].GetPointer(), destinationSize, downsampledImage.GetPointer());
    imageLevels[level] = downsampledImage;

    // Downsample
    Mask::Pointer downsampledMask = Mask::New();
    ITKHelpers::ScaleImage(maskLevels[level - 1].GetPointer(), destinationSize, downsampledMask.GetPointer());
    downsampledMask->CopyInformationFrom(this->MaskImage);
    maskLevels[level] = downsampledMask;
  }

  for(unsigned int level = 0; level < this->ResolutionLevels; ++level)
  {
    std::cout << "Level " << level << " image resolution "
              << imageLevels[level]->GetLargestPossibleRegion().GetSize() << std::endl;

    std::cout << "Level " << level << " mask resolution "
              << maskLevels[level]->GetLargestPossibleRegion().GetSize() << std::endl;

    std::stringstream ssImage;
    ssImage << "Input_Level_" << Helpers::ZeroPad(level, 2) << ".png";
    ITKHelpers::WriteRGBImage(imageLevels[level].GetPointer(), ssImage.str());

    std::stringstream ssMask;
    ssMask << "Mask_Level_" << Helpers::ZeroPad(level, 2) << ".png";
    ITKHelpers::WriteImage(maskLevels[level].GetPointer(), ssMask.str());
  }

  for(unsigned int level = this->ResolutionLevels - 1; level >= 0; --level)
  {
    std::cout << "BDS level " << level << " (resolution "
              << imageLevels[level]->GetLargestPossibleRegion().GetSize() << ")" << std::endl;
    ImageType::Pointer output = ImageType::New();
    Compute(imageLevels[level].GetPointer(), maskLevels[level].GetPointer(), output);

    std::stringstream ss;
    ss << "Output_Level_" << Helpers::ZeroPad(level, 2) << ".png";
    ITKHelpers::WriteRGBImage(output.GetPointer(), ss.str());

    if(level == 0)
    {
      ITKHelpers::DeepCopy(output.GetPointer(), this->Output.GetPointer());
      break;
    }

    // Upsample result and copy it to the next level. A factor of 2 goes up one level.
    ImageType::Pointer upsampled = ImageType::New();
    ITKHelpers::ScaleImage(output.GetPointer(), imageLevels[level-1]->GetLargestPossibleRegion().GetSize(), upsampled.GetPointer());
//     std::cout << "Upsampled from " << output->GetLargestPossibleRegion().GetSize() << " to "
//               << upsampled->GetLargestPossibleRegion().GetSize() << std::endl;
//
//     std::cout << "Upsampled size: " << upsampled->GetLargestPossibleRegion().GetSize() << std::endl;
//     std::cout << "Next level size: " << imageLevels[level - 1]->GetLargestPossibleRegion().GetSize() << std::endl;

    // Only keep the computed pixels in the hole - the rest of the pixels are simply from one level up.
    MaskOperations::CopyInHoleRegion(upsampled.GetPointer(),
                                     imageLevels[level - 1].GetPointer(),
                                     maskLevels[level - 1].GetPointer());

    std::stringstream ssUpdated;
    ssUpdated << "UpdatedInput_Level_" << Helpers::ZeroPad(level - 1, 2) << ".png";
    ITKHelpers::WriteRGBImage(imageLevels[level - 1].GetPointer(), ssUpdated.str());
  }

}

void BDSInpainting::Compute(ImageType* const image, Mask* const mask, ImageType* const output)
{
  ITKHelpers::WriteRGBImage(image, "ComputeInput.png");
  // Smoothly fill the hole
//   Image image(imageIn.width, imageIn.height, 1, imageIn.channels);
//   image.CopyData(Inpaint::apply(imageIn, mask));

  // For inpainting, the target mask for PatchMatch is the entire image
  Mask::Pointer targetMask = Mask::New();
  targetMask->SetRegions(mask->GetLargestPossibleRegion());
  targetMask->Allocate();
  ITKHelpers::SetImageToConstant(targetMask.GetPointer(), targetMask->GetValidValue());

  // Initialize the output with the input
  ITKHelpers::DeepCopy(image, output);

  // Initialize the image to operate on
  ImageType::Pointer currentImage = ImageType::New();
  ITKHelpers::DeepCopy(image, currentImage.GetPointer());

  ImageType::PixelType zeroPixel;
  zeroPixel.Fill(0.0f);

  itk::ImageRegion<2> fullRegion = image->GetLargestPossibleRegion();

  std::cout << "Computing BDS on resolution " << fullRegion.GetSize() << std::endl;

  for(unsigned int iteration = 0; iteration < this->Iterations; ++iteration)
  {
    std::cout << "BDSInpainting Iteration " << iteration << std::endl;

    PatchMatch patchMatch;
    patchMatch.SetImage(currentImage);
    patchMatch.SetSourceMask(mask);
    patchMatch.SetTargetMask(targetMask);
    patchMatch.SetIterations(this->PatchMatchIterations);
    patchMatch.SetPatchRadius(this->PatchRadius);
    try
    {
      if(iteration == 0)
      {
        patchMatch.Compute(NULL);
      }
      else
      {
        // For now don't initialize with the previous NN field - though this might work and be a huge speed up.
        patchMatch.Compute(NULL);
        //patchMatch.Compute(init);
      }
    }
    catch (std::runtime_error &e)
    {
      if(e.what() == std::string("PatchMatch: No valid source regions!"))
      {
        //std::cout << "Caught exception." << std::endl;
        return;
      }
    }

    PatchMatch::PMImageType* nnField = patchMatch.GetOutput();

    // The contribution of each pixel q to the error term (d_cohere) = 1/N_T \sum_{i=1}^m (S(p_i) - T(q))^2
    // To find the best color T(q) (iterative update rule), differentiate with respect to T(q),
    // set to 0, and solve for T(q):
    // T(q) = \frac{1}{m} \sum_{i=1}^m S(p_i)

    ImageType::Pointer updateImage = ImageType::New(); // We don't want to change pixels directly on the
    // output image during the iteration, but rather compute them all and then update them all simultaneously.
    ITKHelpers::DeepCopy(currentImage.GetPointer(), updateImage.GetPointer());

    // Loop over the whole image (patch centers)
    itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(fullRegion, this->PatchRadius);

    itk::ImageRegionIteratorWithIndex<ImageType> imageIterator(updateImage,
                                                               internalRegion);

    while(!imageIterator.IsAtEnd())
    {
      itk::Index<2> currentPixel = imageIterator.GetIndex();
      if(mask->IsHole(currentPixel)) // We have come across a pixel to be filled
      {
        // Zero the pixel - it will be additively updated
        updateImage->SetPixel(currentPixel, zeroPixel);

        itk::ImageRegion<2> currentRegion =
             ITKHelpers::GetRegionInRadiusAroundPixel(currentPixel, this->PatchRadius);

        std::vector<itk::ImageRegion<2> > patchesContainingPixel =
              ITKHelpers::GetAllPatchesContainingPixel(currentPixel,
                                                       this->PatchRadius,
                                                       fullRegion);

        std::vector<ImageType::PixelType> contributingPixels(patchesContainingPixel.size());
        std::vector<float> contributingScores(patchesContainingPixel.size());

        for(unsigned int containingPatchId = 0;
            containingPatchId < patchesContainingPixel.size(); ++containingPatchId)
        {
          itk::Index<2> containingRegionCenter =
                      ITKHelpers::GetRegionCenter(patchesContainingPixel[containingPatchId]);
          Match bestMatch = nnField->GetPixel(containingRegionCenter);
          itk::ImageRegion<2> bestMatchRegion = bestMatch.Region;
          itk::Index<2> bestMatchRegionCenter = ITKHelpers::GetRegionCenter(bestMatchRegion);

//           std::cout << "containingRegionCenter: " << containingRegionCenter << std::endl;
//           std::cout << "bestMatchRegionCenter: " << bestMatchRegionCenter << std::endl;

          // Compute the offset of the pixel in question relative to the center of the current patch that contains the pixel
          itk::Offset<2> offset = currentPixel - containingRegionCenter;

          // Compute the location of the pixel in the best matching patch that is the
          // same position of the pixel in question relative to the containing patch
          itk::Index<2> correspondingPixel = bestMatchRegionCenter + offset;

          contributingPixels[containingPatchId] = currentImage->GetPixel(correspondingPixel);
          contributingScores[containingPatchId] = bestMatch.Score;

        } // end loop over containing patches

        // Compute new pixel value

        // Select a method to construct new pixel
        // ImageType::PixelType newValue = ITKStatistics::Average(contributingPixels);

        // ImageType::PixelType newValue = WeightedSum(contributingPixels, contributingScores);

        // Take the pixel from the best matching patch
        unsigned int patchId = Helpers::argmin(contributingScores);
        ImageType::PixelType newValue = contributingPixels[patchId];

        // Use the pixel closest to the average pixel
//         ImageType::PixelType averagePixel = ITKStatistics::Average(contributingPixels);
        //unsigned int patchId = ITKHelpers::ClosestPoint(contributingPixels, averagePixel);
//         ImageType::PixelType newValue = contributingPixels[patchId];

        updateImage->SetPixel(currentPixel, newValue);

//         std::cout << "Pixel was " << currentImage->GetPixel(currentPixel)
//                   << " and is now " << updateImage->GetPixel(currentPixel) << std::endl;
      } // end if is hole

      ++imageIterator;
    } // end loop over image

    MaskOperations::CopyInHoleRegion(updateImage.GetPointer(), currentImage.GetPointer(), mask);

    std::stringstream ssPNG;
    ssPNG << "BDS_Iteration_" << Helpers::ZeroPad(iteration, 2) << ".png";
    ITKHelpers::WriteRGBImage(currentImage.GetPointer(), ssPNG.str());
  } // end iterations loop

  ITKHelpers::DeepCopy(currentImage.GetPointer(), output);

  ITKHelpers::WriteRGBImage(output, "ComputeOutput.png");
}


BDSInpainting::ImageType* BDSInpainting::GetOutput()
{
  return Output;
}

void BDSInpainting::SetIterations(const unsigned int iterations)
{
  this->Iterations = iterations;
}

void BDSInpainting::SetPatchRadius(const unsigned int patchRadius)
{
  this->PatchRadius = patchRadius;
}

void BDSInpainting::SetImage(ImageType* const image)
{
  ITKHelpers::DeepCopy(image, this->Image.GetPointer());
}

void BDSInpainting::SetMask(Mask* const mask)
{
  this->MaskImage->DeepCopyFrom(mask);
}

void BDSInpainting::SetResolutionLevels(const unsigned int resolutionLevels)
{
  if(resolutionLevels < 1)
  {
    std::cerr << "ResolutionLevels: " << resolutionLevels << std::endl;
    throw std::runtime_error("ResolutionLevels must be >= 1!");
  }
  this->ResolutionLevels = resolutionLevels;
}

void BDSInpainting::SetPatchMatchIterations(const unsigned int patchMatchIterations)
{
  this->PatchMatchIterations = patchMatchIterations;
}

void BDSInpainting::SetDownsampleFactor(const float downsampleFactor)
{
  this->DownsampleFactor = downsampleFactor;
}

BDSInpainting::ImageType::PixelType BDSInpainting::WeightedSum(const std::vector<ImageType::PixelType>& contributingPixels,
                                   const std::vector<float>& contributingScores)
{
  // Average weighted by patch scores
  ImageType::PixelType newValue;
  newValue.Fill(0);

  //float scoreSum = Helpers::Sum(contributingScores.begin(), contributingScores.end());
  float maxScore = *(std::min_element(contributingScores.begin(), contributingScores.end()));
  float weightSum = 0.0f;
  for(unsigned int i = 0; i < contributingScores.size(); ++i)
  {
    //float weight = (scoreSum - contributingScores[i]);
    float weight = (maxScore - contributingScores[i]);
    weightSum += weight;
    newValue += weight * contributingPixels[i];
  }

  newValue /= weightSum;

  return newValue;
}
