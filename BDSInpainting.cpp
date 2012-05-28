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

BDSInpainting::BDSInpainting()
{
  this->Output = ImageType::New();
  this->Image = ImageType::New();
  this->MaskImage = Mask::New();
}


void BDSInpainting::Compute(ImageType* const image, Mask* const mask, const unsigned int patchRadius)
{

}

void BDSInpainting::Compute()
{
  // Smoothly fill the hole
//   Image image(imageIn.width, imageIn.height, 1, imageIn.channels);
//   image.CopyData(Inpaint::apply(imageIn, mask));

  Mask::Pointer targetMask = Mask::New();
  targetMask->SetRegions(this->MaskImage->GetLargestPossibleRegion());
  targetMask->Allocate();
  ITKHelpers::SetImageToConstant(targetMask.GetPointer(), targetMask->GetValidValue());

  // Initialize the output with the input
  ITKHelpers::DeepCopy(this->Image.GetPointer(), this->Output.GetPointer());

  // Initialize the image to operate on
  ImageType::Pointer currentImage = ImageType::New();
  ITKHelpers::DeepCopy(this->Image.GetPointer(), currentImage.GetPointer());

  ImageType::PixelType zeroPixel;
  zeroPixel.Fill(0.0f);

  itk::ImageRegion<2> fullRegion = this->Image->GetLargestPossibleRegion();
  
  for(unsigned int iteration = 0; iteration < this->Iterations; ++iteration)
  {
    std::cout << "BDSInpainting Iteration " << iteration << std::endl;

    PatchMatch patchMatch;
    patchMatch.SetImage(currentImage);
    patchMatch.SetSourceMask(this->MaskImage);
    patchMatch.SetTargetMask(targetMask);
    patchMatch.SetIterations(5);
    patchMatch.SetPatchRadius(this->PatchRadius);
    if(iteration == 0)
    {
      patchMatch.Compute(NULL);
    }
    else
    {
      patchMatch.Compute(NULL); // For now don't initialize with the previous NN field - though this might work and be a huge speed up.
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
      if(this->MaskImage->IsHole(currentPixel)) // We have come across a pixel to be filled
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

    MaskOperations::CopyInHoleRegion(updateImage.GetPointer(), currentImage.GetPointer(), this->MaskImage);

//     std::stringstream ssMeta;
//     ssMeta << "Iteration_" << iteration << ".mha";

    std::stringstream ssPNG;
    ssPNG << "Iteration_" << Helpers::ZeroPad(iteration, 2) << ".png";
    ITKHelpers::WriteRGBImage(currentImage.GetPointer(), ssPNG.str());
  } // end iterations loop

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
