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
#include "PatchMatchHelpers.h"

// Submodules
#include <Mask/Mask.h>

struct NeighborParent
{
  virtual std::vector<itk::Index<2> > GetNeighbors(const itk::Index<2>& queryIndex) const = 0;
};

struct ValidMaskVerifiedNeighbors : public NeighborParent
{
  ValidMaskVerifiedNeighbors(PatchMatchHelpers::NNFieldType* const nnField, Mask* const targetMask) :
  NNField(nnField), TargetMask(targetMask)
  {
  }

  std::vector<itk::Index<2> > GetNeighbors(const itk::Index<2>& queryIndex) const
  {
    assert(this->NNField);
    assert(this->TargetMask);

    std::vector<itk::Index<2> > potentialPropagationNeighbors =
        ITKHelpers::Get8NeighborsInRegion(this->TargetMask->GetLargestPossibleRegion(),
                                          queryIndex);

    std::vector<itk::Index<2> > allowedPropagationNeighbors;
    for(size_t i = 0; i < potentialPropagationNeighbors.size(); ++i)
    {
      if(this->NNField->GetPixel(potentialPropagationNeighbors[i]).HasVerifiedMatch() &&
         this->TargetMask->IsValid(potentialPropagationNeighbors[i]) )
      {
        allowedPropagationNeighbors.push_back(potentialPropagationNeighbors[i]);
      }
    }
    return allowedPropagationNeighbors;
  }

private:
  PatchMatchHelpers::NNFieldType* NNField;
  Mask* TargetMask;
};

struct ValidMaskValidScoreNeighbors : public NeighborParent
{
  ValidMaskValidScoreNeighbors(PatchMatchHelpers::NNFieldType* const nnField, Mask* const targetMask) :
  NNField(nnField), TargetMask(targetMask)
  {
  }

  std::vector<itk::Index<2> > GetNeighbors(const itk::Index<2>& queryIndex) const
  {
    assert(this->NNField);
    assert(this->TargetMask);

    std::vector<itk::Index<2> > potentialPropagationNeighbors =
        ITKHelpers::Get8NeighborsInRegion(this->TargetMask->GetLargestPossibleRegion(),
                                          queryIndex);

    std::vector<itk::Index<2> > allowedPropagationNeighbors;
    for(size_t i = 0; i < potentialPropagationNeighbors.size(); ++i)
    {
      if(this->NNField->GetPixel(potentialPropagationNeighbors[i]).GetMatch(0).IsValid() &&
         this->TargetMask->IsValid(potentialPropagationNeighbors[i]) )
      {
        allowedPropagationNeighbors.push_back(potentialPropagationNeighbors[i]);
      }
    }
    return allowedPropagationNeighbors;
  }

private:
  PatchMatchHelpers::NNFieldType* NNField;
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

struct VerifiedForwardPropagationNeighbors : public NeighborParent
{
  VerifiedForwardPropagationNeighbors(PatchMatchHelpers::NNFieldType* const nnField) : NNField(nnField)
  {
  }

  std::vector<itk::Index<2> > GetNeighbors(const itk::Index<2>& queryIndex) const
  {
    assert(this->NNField);

    std::vector<itk::Index<2> > allowedPropagationNeighbors;

    itk::Offset<2> leftPixelOffset = {{-1, 0}};
    if(this->NNField->GetPixel(queryIndex + leftPixelOffset).HasVerifiedMatch())
    {
      allowedPropagationNeighbors.push_back(queryIndex + leftPixelOffset);
    }

    itk::Offset<2> upPixelOffset = {{0, -1}};
    if(this->NNField->GetPixel(queryIndex + upPixelOffset).HasVerifiedMatch())
    {
      allowedPropagationNeighbors.push_back(queryIndex + upPixelOffset);
    }

    return allowedPropagationNeighbors;
  }

private:
  PatchMatchHelpers::NNFieldType* NNField;
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

struct VerifiedBackwardPropagationNeighbors : public NeighborParent
{
  VerifiedBackwardPropagationNeighbors(PatchMatchHelpers::NNFieldType* const nnField) : NNField(nnField)
  {
  }

  std::vector<itk::Index<2> > GetNeighbors(const itk::Index<2>& queryIndex) const
  {
    std::vector<itk::Index<2> > allowedPropagationNeighbors;

    itk::Offset<2> rightPixelOffset = {{1, 0}};
    if(this->NNField->GetPixel(queryIndex + rightPixelOffset).HasVerifiedMatch())
    {
      allowedPropagationNeighbors.push_back(queryIndex + rightPixelOffset);
    }

    itk::Offset<2> downPixelOffset = {{0, 1}};
    if(this->NNField->GetPixel(queryIndex + downPixelOffset).HasVerifiedMatch())
    {
      allowedPropagationNeighbors.push_back(queryIndex + downPixelOffset);
    }

    return allowedPropagationNeighbors;
  }

private:
  PatchMatchHelpers::NNFieldType* NNField;
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
