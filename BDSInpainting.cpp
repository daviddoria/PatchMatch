#include "BDSInpainting.h"

// Submodules
#include "Mask/ITKHelpers/ITKHelpers.h"

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

  // Initialize the output with the input
  ITKHelpers::DeepCopy(this->Image.GetPointer(), this->Output.GetPointer());

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

    // Loop over the whole image (patch centers)
    unsigned int numberOfPixelsFilled = 0;
    for(int y = 0; y < image.height; ++y)
    {
      for(int x = 0; x < image.width; ++x)
      {
        if(mask(x,y)[0] == 0) // We have come across a pixel to be filled
        {
          out.SetAllComponents(x, y, 0.0f);

          numberOfPixelsFilled++;

          unsigned int numberOfContributors = 0;

          // Iterate over the patch centered on this pixel
          for (int dy = -patchDiameter/2; dy <= patchDiameter/2; ++dy)
          {
            if (y+dy < 0)// skip this row
            {
              continue;
            }
            if (y+dy >= out.height) // quit when we get past the last row
            {
              break;
            }

            for(int dx = -patchDiameter/2; dx <= patchDiameter/2; ++dx)
            {
              if (x+dx < 0) // skip this pixel
              {
                continue;
              }
              else if(x+dx >= out.width) // quit when we get to the end of the row
              {
                break;
              }
              else // Use a pixel from this patch in the update
              {
                // (matchX, matchY) is the center of the best matching patch to the patch centered at (x+dx, y+dy)
                int matchX = (int)patchMatch(x+dx,y+dy)[0];
                int matchY = (int)patchMatch(x+dx,y+dy)[1];

                numberOfContributors++;
                for(int c = 0; c < image.channels; ++c)
                {
                  out(x, y)[c] += image(matchX-dx, matchY-dy)[c];
                }
              }
            } // end loop over row
          } // end loop over patch
          //std::cout << "numberOfContributors: " << numberOfContributors << std::endl;
          //float weight = 1.0f/static_cast<float>(numberOfContributors);
          //std::cout << "weight: " << weight << std::endl;
          for(int c = 0; c < image.channels; ++c)
          {
            out(x,y)[c] /= static_cast<float>(numberOfContributors);
          }
        } // end if (!targetMask || targMaskPtr[0] > 0)
      }
    } // end loop over image

    std::stringstream ssMeta;
    ssMeta << "Iteration_" << i << ".mha";
    out.WriteMeta(ssMeta.str());

    std::stringstream ssPNG;
    ssPNG << "Iteration_" << Helpers::ZeroPad(i, 2) << ".png";
    out.WritePNG(ssPNG.str());

    // reset for the next iteration
    //image = out;
    //image = out.copy();
    image.CopyData(out);

    std::cout << "numberOfPixelsFilled: " << numberOfPixelsFilled << std::endl;
  } // end for(int i = 0; i < numIter; i++)
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
