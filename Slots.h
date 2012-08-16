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

#ifndef Slots_H
#define Slots_H

// STL
#include <string>

// ITK
#include "itkImageRegion.h"

// Custom
#include "PatchMatchHelpers.h"

// Submodules
#include <ITKHelpers/ITKHelpers.h>

struct WriteSlot
{
  std::string Prefix;
  int Counter;

  WriteSlot(const std::string& prefix) : Prefix(prefix), Counter(0){}

  void Write(PatchMatchHelpers::NNFieldType* nnField)
  {
    PatchMatchHelpers::WriteNNField(nnField,
                Helpers::GetSequentialFileName(this->Prefix,
                                               this->Counter, "mha"));
    this->Counter++;
  }
};

template<typename TImage>
struct WritePatchPair
{
  TImage* Image;

  unsigned int PatchRadius;

  std::string Prefix;

  int Counter;

  WritePatchPair(TImage* const image, const unsigned int patchRadius, const std::string& prefix) :
  Image(image), PatchRadius(patchRadius), Prefix(prefix), Counter(0)
  {}

  void Write(const itk::Index<2>& queryCenter, const itk::Index<2>& matchCenter, const float score)
  {
    std::cout << "WritePatchPair writing queryCenter: " << queryCenter << " matchCenter: " << matchCenter << std::endl;

    std::ofstream fout("scores.txt", std::ios::app);
    fout << score << std::endl;
    fout.close();

    itk::ImageRegion<2> queryRegion = ITKHelpers::GetRegionInRadiusAroundPixel(queryCenter, this->PatchRadius);
    itk::ImageRegion<2> matchRegion = ITKHelpers::GetRegionInRadiusAroundPixel(matchCenter, this->PatchRadius);

    itk::Size<2> patchSize = matchRegion.GetSize();

    itk::Index<2> corner = {{0,0}};
    itk::Size<2> pairImageSize = {{queryRegion.GetSize()[0] * 2, queryRegion.GetSize()[1]}};
    itk::ImageRegion<2> region(corner, pairImageSize);

    itk::ImageRegion<2> pairQueryRegion(corner, patchSize);

    itk::Index<2> pairMatchCorner = {{patchSize[0], 0}};
    itk::ImageRegion<2> pairMatchRegion(pairMatchCorner, patchSize);

    typename TImage::Pointer patchPairImage = TImage::New();
    patchPairImage->SetRegions(region);
    patchPairImage->Allocate();

//     std::cout << "queryRegion: " << queryRegion << " matchRegion: " << matchRegion << std::endl;
// 
//     std::cout << "pairQueryRegion: " << pairQueryRegion << " pairMatchRegion: " << pairMatchRegion << std::endl;

    ITKHelpers::CopyRegion(this->Image, patchPairImage.GetPointer(), queryRegion,
                           pairQueryRegion);

    ITKHelpers::CopyRegion(this->Image, patchPairImage.GetPointer(), matchRegion,
                           pairMatchRegion);

    ITKHelpers::WriteImage(patchPairImage.GetPointer(),
                Helpers::GetSequentialFileName(this->Prefix, 
                                               this->Counter, "png"));
    this->Counter++;
  }
};

#endif
