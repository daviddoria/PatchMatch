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

// STL
#include <cassert>

// Custom
#include "Match.h"
#include "PatchMatchHelpers.h"

// Submodules
#include <Mask/Mask.h>

/** This class decides which pixels are allowed to be propagated (spread their information). (This is the reverse of Process, which determines if pixels
  * can be propagated to (receive new information).)*/
struct PropagationAllowed
{
  virtual bool CanPropagate(const itk::Index<2>& queryIndex) const = 0;
};

struct PropagationAllowedComposite : public PropagationAllowed
{
  bool CanPropagate(const itk::Index<2>& queryIndex) const
  {
    for(size_t i = 0; i < this->Functors.size(); ++i)
    {
      if(!Functors->CanPropagate(queryIndex))
      {
        return false;
      }
    }
    return true;
  }

private:
  std::vector<PropagationAllowed*> Functors;
};

struct PropagationAllowedVerified : public PropagationAllowed
{

  PropagationAllowedVerified(PatchMatchHelpers::NNFieldType* const nnField)
  {
    this->NNField = nnField;
  }

  bool CanPropagate(const itk::Index<2>& queryIndex) const
  {
    assert(this->NNField);
    return this->NNField->IsVerified(queryIndex);
  }

private:
  PatchMatchHelpers::NNFieldType* NNField;
};

struct PropagationAllowedValidMask : public PropagationAllowed
{
  PropagationAllowedValidMask(Mask* const mask)
  {
    this->MaskImage = mask;
  }

  bool CanPropagate(const itk::Index<2>& queryIndex) const
  {
    assert(this->MaskImage);
    return this->MaskImage->IsValid(queryIndex);
  }

private:
  Mask* MaskImage;
};


struct PropagationAllowedAll : public PropagationAllowed
{
  bool CanPropagate(const itk::Index<2>& queryIndex) const
  {
    return true;
  }
};


#endif
