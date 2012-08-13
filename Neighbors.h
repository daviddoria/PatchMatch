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

#ifndef Neighbors_H
#define Neighbors_H

// Custom
#include "Match.h"

// Submodules
#include <Mask/Mask.h>

struct NeighborParent
{
  virtual std::vector<itk::Index<2> > GetNeighbors(const itk::Index<2>& queryIndex) const = 0;
};

struct ValidMaskValidScoreNeighbors : public NeighborParent
{
  typedef itk::Image<Match, 2> MatchImageType;

  ValidMaskValidScoreNeighbors(MatchImageType* const matchImage, Mask* const targetMask)
  {
    this->MatchImage = matchImage;
    this->TargetMask = targetMask;
  }

  std::vector<itk::Index<2> > GetNeighbors(const itk::Index<2>& queryIndex) const
  {
    std::vector<itk::Index<2> > potentialPropagationNeighbors =
        ITKHelpers::Get8NeighborsInRegion(this->TargetMask->GetLargestPossibleRegion(),
                                          queryIndex);

    std::vector<itk::Index<2> > allowedPropagationNeighbors;
    for(size_t i = 0; i < potentialPropagationNeighbors.size(); ++i)
    {
      if(this->MatchImage->GetPixel(potentialPropagationNeighbors[i]).IsValid() &&
         this->TargetMask->IsValid(potentialPropagationNeighbors[i]) )
      {
        allowedPropagationNeighbors.push_back(potentialPropagationNeighbors[i]);
      }
    }
    return allowedPropagationNeighbors;
  }

private:
  MatchImageType* MatchImage;
  Mask* TargetMask;
};

struct ValidMaskNeighbors : public NeighborParent
{
  ValidMaskNeighbors(Mask* const mask)
  {
    this->MaskImage = mask;
  }

  std::vector<itk::Index<2> > GetNeighbors(const itk::Index<2>& queryIndex) const
  {
    assert(this->MaskImage);

    std::vector<itk::Index<2> > potentialPropagationNeighbors =
      ITKHelpers::Get8NeighborsInRegion(this->MaskImage->GetLargestPossibleRegion(),
                                        queryIndex);

    std::vector<itk::Index<2> > allowedPropagationNeighbors;
    for(size_t i = 0; i < potentialPropagationNeighbors.size(); ++i)
    {
      if(this->MaskImage->IsValid(potentialPropagationNeighbors[i]) )
      {
        allowedPropagationNeighbors.push_back(potentialPropagationNeighbors[i]);
      }
    }
    return allowedPropagationNeighbors;
  }

private:
  Mask* MaskImage;
};

struct ForwardPropagationNeighbors : public NeighborParent
{
  std::vector<itk::Index<2> > GetNeighbors(const itk::Index<2>& queryIndex) const
  {
    std::vector<itk::Index<2> > allowedPropagationNeighbors;

    itk::Offset<2> leftPixelOffset = {{-1, 0}};
    allowedPropagationNeighbors.push_back(queryIndex + leftPixelOffset);

    itk::Offset<2> upPixelOffset = {{0, -1}};
    allowedPropagationNeighbors.push_back(queryIndex + upPixelOffset);

    return allowedPropagationNeighbors;
  }
};

struct BackwardPropagationNeighbors : public NeighborParent
{
  std::vector<itk::Index<2> > GetNeighbors(const itk::Index<2>& queryIndex) const
  {
    std::vector<itk::Index<2> > allowedPropagationNeighbors;

    itk::Offset<2> rightPixelOffset = {{1, 0}};
    allowedPropagationNeighbors.push_back(queryIndex + rightPixelOffset);

    itk::Offset<2> downPixelOffset = {{0, 1}};
    allowedPropagationNeighbors.push_back(queryIndex + downPixelOffset);

    return allowedPropagationNeighbors;
  }
};

struct AllNeighbors : public NeighborParent
{
  std::vector<itk::Index<2> > GetNeighbors(const itk::Index<2>& queryIndex) const
  {
    std::vector<itk::Index<2> > neighbors = ITKHelpers::Get8Neighbors(queryIndex);

    return neighbors;
  }
};


#endif
