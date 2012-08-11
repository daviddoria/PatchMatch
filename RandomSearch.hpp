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

#ifndef RandomSearch_HPP
#define RandomSearch_HPP

template <typename TImage>
RandomSearch<TImage>::RandomSearch() : Image(NULL)
{

}

template <typename TImage>
void RandomSearch<TImage>::
Search(MatchImageType* const nnField, const std::vector<itk::Index<2> >& pixelsToProcess)
{
  assert(nnField);
  assert(this->Image);
  assert(nnField->GetLargestPossibleRegion().GetSize()[0] > 0);
  assert(this->Image->GetLargestPossibleRegion().GetSize()[0] > 0);
  assert(nnField->GetLargestPossibleRegion().GetSize() ==
         this->Image->GetLargestPossibleRegion().GetSize());

  // The full region - so we can refer to this without specifying an image/mask that it is associated with
  itk::ImageRegion<2> region = nnField->GetLargestPossibleRegion();

  unsigned int exactMatchPixels = 0;
  for(size_t pixelId = 0; pixelId < pixelsToProcess.size(); ++pixelId)
  {
    itk::Index<2> queryPixel = pixelsToProcess[pixelId];

    // For inpainting, most of the NN-field will be an exact match. We don't have to search anymore
    // once the exact match is found.
    if((nnField->GetPixel(queryPixel).Score == 0))
    {
      exactMatchPixels++;
      continue;
    }

    itk::ImageRegion<2> queryRegion =
      ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);

    if(!region.IsInside(queryRegion))
      {
        //std::cerr << "Pixel " << potentialPropagationPixel << " is outside of the image." << std::endl;
        continue;
      }

    unsigned int width = region.GetSize()[0];
    unsigned int height = region.GetSize()[1];

    // The maximum (first) search radius, as prescribed in PatchMatch paper section 3.2
    unsigned int radius = std::max(width, height);

    // The fraction by which to reduce the search radius at each iteration,
    // as prescribed in PatchMatch paper section 3.2
    float alpha = 1.0f/2.0f;

    // Search an exponentially smaller window each time through the loop

    while (radius > this->PatchRadius) // while there is more than just the current patch to search
    {
      itk::ImageRegion<2> searchRegion = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, radius);
      searchRegion.Crop(region);

      unsigned int maxNumberOfAttempts = 5; // How many random patches to test for validity before giving up

      itk::ImageRegion<2> randomValidRegion;
      try
      {
      // This function throws an exception if no valid patch was found
      randomValidRegion =
                MaskOperations::GetRandomValidPatchInRegion(this->SourceMask.GetPointer(),
                                                            searchRegion, this->PatchRadius,
                                                            maxNumberOfAttempts);
      }
      catch (...) // If no suitable region is found, move on
      {
        //std::cout << "No suitable region found." << std::endl;
        radius *= alpha;
        continue;
      }

      // Compute the patch difference
      float dist = this->PatchDistanceFunctor->Distance(randomValidRegion, queryRegion);

      // Construct a match object
      Match potentialMatch;
      potentialMatch.Region = randomValidRegion;
      potentialMatch.Score = dist;

      // Store this match as the best match if it meets the criteria.
      // In this class, the criteria is simply that it is
      // better than the current best patch. In subclasses (i.e. GeneralizedPatchMatch),
      // it must be better than the worst patch currently stored.
      Match currentMatch = nnField->GetPixel(queryPixel);
      if(this->AcceptanceTestFunctor->IsBetter(queryRegion,
                                               nnField->GetPixel(queryPixel), potentialMatch))
      {
        nnField->SetPixel(queryPixel, potentialMatch);
      }

      radius *= alpha;
    } // end decreasing radius loop

  } // end loop over target pixels

  //std::cout << "RandomSearch: already exact match " << exactMatchPixels << std::endl;
}

#endif
