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

void BDSInpainting::Compute()
{
  // Smoothly fill the hole
//   Image image(imageIn.width, imageIn.height, 1, imageIn.channels);
//   image.CopyData(Inpaint::apply(imageIn, mask));


  unsigned int width = Image->GetLargestPossibleRegion().GetSize()[0];
  unsigned int height = Image->GetLargestPossibleRegion().GetSize()[1];

  // Convert patch diameter to patch radius
  int patchRadius = this->PatchDiameter / 2;

  // Initialize the output with the input
  ITKHelpers::DeepCopy(this->Image.GetPointer(), this->Output.GetPointer());

  ImageType::PixelType zeroPixel;
  zeroPixel.Fill(0.0f);

  for(unsigned int i = 0; i < this->Iterations; ++i)
  {
    std::cout << "BDSInpainting Iteration " << i << std::endl;

    PatchMatch patchMatch;
    patchMatch.SetImage(this->Image);
    patchMatch.SetMask(this->MaskImage);
    patchMatch.SetIterations(5);
    patchMatch.SetPatchDiameter(this->PatchDiameter);

    patchMatch.Compute(NULL);

    // The contribution of each pixel q to the error term (d_cohere) = 1/N_T \sum_{i=1}^m (S(p_i) - T(q))^2
    // To find the best color T(q) (iterative update rule), differentiate with respect to T(q),
    // set to 0, and solve for T(q):
    // T(q) = \frac{1}{m} \sum_{i=1}^m S(p_i)

    ImageType::Pointer UpdateImage = ImageType::New(); // We don't want to change pixels directly on the
    // output image during the iteration, but rather compute them all and then update them all simultaneously.
    ITKHelpers::DeepCopy(this->Image.GetPointer(), UpdateImage.GetPointer());

    ImageType::Pointer CurrentImage = ImageType::New();
    ITKHelpers::DeepCopy(this->Image.GetPointer(), CurrentImage.GetPointer());

    // Loop over the whole image (patch centers)
    unsigned int numberOfPixelsFilled = 0;
    itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), patchRadius);

    itk::ImageRegionIteratorWithIndex<ImageType> imageIterator(UpdateImage,
                                                               internalRegion);

    while(!imageIterator.IsAtEnd())
    {
      itk::Index<2> currentPixel = imageIterator.GetIndex();
      if(this->MaskImage->IsHole(currentPixel)) // We have come across a pixel to be filled
      {
        // Zero the pixel - it will be additively updated
        UpdateImage->SetPixel(currentPixel, zeroPixel);

        numberOfPixelsFilled++;

        std::vector<itk::ImageRegion<2> > patchesContainingPixel =
              ITKHelpers::GetAllPatchesContainingPixel(currentPixel,
                                                       patchRadius,
                                                       this->Image->GetLargestPossibleRegion());

        unsigned int numberOfContributingPatches = patchesContainingPixel.size();
        // (matchX, matchY) is the center of the best matching patch to the patch centered at (x+dx, y+dy)
//         int matchX = static_cast<int>(patchMatch(x+dx,y+dy)[0]);
//         int matchY = static_cast<int>(patchMatch(x+dx,y+dy)[1]);


      } // end if (!targetMask || targMaskPtr[0] > 0)

      ++imageIterator;
    } // end iterator loop

    MaskOperations::CopyInHoleRegion(UpdateImage.GetPointer(), CurrentImage.GetPointer(), this->MaskImage);

    std::stringstream ssMeta;
    ssMeta << "Iteration_" << i << ".mha";

    std::stringstream ssPNG;
    ssPNG << "Iteration_" << Helpers::ZeroPad(i, 2) << ".png";

    std::cout << "numberOfPixelsFilled: " << numberOfPixelsFilled << std::endl;
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

void BDSInpainting::SetPatchDiameter(const unsigned int patchDiameter)
{
  assert(patchDiameter >= 3 && (patchDiameter & 1)); // patchDiameter must be at least 3 and odd

  this->PatchDiameter = patchDiameter;
}

void BDSInpainting::SetImage(ImageType* const image)
{
  ITKHelpers::DeepCopy(image, this->Image.GetPointer());
}

void BDSInpainting::SetMask(Mask* const mask)
{
  this->MaskImage->DeepCopyFrom(mask);
}
