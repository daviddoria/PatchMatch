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

BDSInpainting::BDSInpainting() : ResolutionLevels(3), Iterations(5), PatchRadius(7), PatchMatchIterations(3)
{
  this->Output = ImageType::New();
  this->Image = ImageType::New();
  this->MaskImage = Mask::New();
}

void BDSInpainting::Compute()
{
  //itk::Size<2> currentSize = this->Image->GetLargestPossibleRegion().GetSize();

  //unsigned int patchDiameter = this->PatchRadius * 2 + 1;

  Mask::Pointer currentMask = Mask::New();
  ITKHelpers::DeepCopy(this->MaskImage.GetPointer(), currentMask.GetPointer());

  ImageType::Pointer currentImage = ImageType::New();
  ITKHelpers::DeepCopy(this->Image.GetPointer(), currentImage.GetPointer());

  std::vector<ImageType::Pointer> imageLevels(this->ResolutionLevels, ImageType::New());
  std::vector<Mask::Pointer> maskLevels(this->ResolutionLevels, Mask::New());

  ITKHelpers::DeepCopy(this->Image.GetPointer(), imageLevels[0].GetPointer());
  ITKHelpers::DeepCopy(this->MaskImage.GetPointer(), maskLevels[0].GetPointer());
  
  for(unsigned int level = 1; level < this->ResolutionLevels; ++level)
  {
    ImageType::Pointer currentImageLevel = ImageType::New();
    imageLevels.push_back(currentImageLevel);
    ITKHelpers::Downsample(this->Image.GetPointer(), pow(level, 2), currentImageLevel.GetPointer());
    ITKHelpers::DeepCopy(currentImageLevel.GetPointer(), imageLevels[level].GetPointer());

    std::cout << "Level " << level << " resolution "
              << imageLevels[level]->GetLargestPossibleRegion().GetSize() << std::endl;
    
    std::stringstream ss;
    ss << "Input_Level_" << level << ".png";
    ITKHelpers::WriteRGBImage(imageLevels[level].GetPointer(), ss.str());
    
    Mask::Pointer currentMaskLevel = Mask::New();
    maskLevels.push_back(currentMaskLevel);
    ITKHelpers::Downsample(this->MaskImage.GetPointer(), pow(level, 2), currentMaskLevel.GetPointer());
    ITKHelpers::DeepCopy(currentMaskLevel.GetPointer(), maskLevels[level].GetPointer());
  }
  
  //while(currentSize[0] > patchDiameter && currentSize[1] > patchDiameter)

  for(unsigned int level = this->ResolutionLevels - 1; level >= 0; --level)
  {
    std::cout << "BDS level " << level << " (resolution "
              << imageLevels[level]->GetLargestPossibleRegion().GetSize() << std::endl;
    ImageType::Pointer output = ImageType::New();
    Compute(imageLevels[level].GetPointer(), maskLevels[level].GetPointer(), output);

    std::stringstream ss;
    ss << "Output_Level_" << level << ".png";
    ITKHelpers::WriteRGBImage(output.GetPointer(), ss.str());
    
    if(level == 0)
    {
      ITKHelpers::DeepCopy(output.GetPointer(), this->Output.GetPointer());
      break;
    }

    // Upsample result and copy it to the next level. A factor of 2 goes up one level.
    ImageType::Pointer upsampled = ImageType::New();
    ITKHelpers::Upsample(output.GetPointer(), 2, upsampled.GetPointer());

    // Only keep the computed pixels in the hole - the rest of the pixels are simply from one level up.
    MaskOperations::CopyInHoleRegion(upsampled.GetPointer(),
                                     imageLevels[level - 1].GetPointer(),
                                     maskLevels[level-1].GetPointer());

    std::stringstream ssUpdated;
    ssUpdated << "UpdatedInput_Level_" << level << ".png";
    ITKHelpers::WriteRGBImage(imageLevels[level - 1].GetPointer(), ssUpdated.str());
  }

}

void BDSInpainting::Compute(ImageType* const image, Mask* const mask, ImageType* const output)
{
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

  for(unsigned int iteration = 0; iteration < this->Iterations; ++iteration)
  {
    std::cout << "BDSInpainting Iteration " << iteration << std::endl;

    PatchMatch patchMatch;
    patchMatch.SetImage(currentImage);
    patchMatch.SetSourceMask(mask);
    patchMatch.SetTargetMask(targetMask);
    patchMatch.SetIterations(5);
    patchMatch.SetPatchRadius(this->PatchRadius);
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

        for(unsigned int containingPatchId = 0;
            containingPatchId < patchesContainingPixel.size(); ++containingPatchId)
        {
          itk::Index<2> containingRegionCenter =
                      ITKHelpers::GetRegionCenter(patchesContainingPixel[containingPatchId]);
          itk::ImageRegion<2> bestMatchRegion = nnField->GetPixel(containingRegionCenter).Region;
          itk::Index<2> bestMatchRegionCenter = ITKHelpers::GetRegionCenter(bestMatchRegion);

          itk::Offset<2> offset = currentPixel - containingRegionCenter;

          itk::Index<2> correspondingPixel = bestMatchRegionCenter + offset;

          ImageType::PixelType normalizedContribution =
              currentImage->GetPixel(correspondingPixel) / static_cast<float>(patchesContainingPixel.size());
          ImageType::PixelType newValue = updateImage->GetPixel(currentPixel) + normalizedContribution;
          updateImage->SetPixel(currentPixel, newValue);
        }

      } // end if is hole

      ++imageIterator;
    } // end loop over image

    MaskOperations::CopyInHoleRegion(updateImage.GetPointer(), currentImage.GetPointer(), mask);

//     std::stringstream ssMeta;
//     ssMeta << "Iteration_" << iteration << ".mha";

    std::stringstream ssPNG;
    ssPNG << "Iteration_" << Helpers::ZeroPad(iteration, 2) << ".png";
    ITKHelpers::WriteRGBImage(currentImage.GetPointer(), ssPNG.str());
  } // end iterations loop

  ITKHelpers::DeepCopy(currentImage.GetPointer(), output);
  std::cout << std::endl;
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
  this->ResolutionLevels = resolutionLevels;
}

void BDSInpainting::SetPatchMatchIterations(const unsigned int patchMatchIterations)
{
  this->PatchMatchIterations = patchMatchIterations;
}
