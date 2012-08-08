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

#ifndef Initializer_H
#define Initializer_H

// ITK
#include "itkImage.h"

// Custom
#include "Match.h"

// Submodules
#include <Mask/Mask.h>
#include <PatchComparison/PatchDistance.h>

class Initializer
{
public:
  /** Create the 'initialization' image. */
  virtual void Initialize(itk::Image<Match, 2>* const initialization) = 0;
};

class InitializerPatch : public Initializer
{
  public:
  InitializerPatch() : PatchRadius(0), SourceMask(NULL),
                       TargetMask(NULL) {}

  InitializerPatch(const unsigned int patchRadius)
  {
    this->PatchRadius = patchRadius;
  }

  /** Create the 'initialization' image. */
  virtual void Initialize(itk::Image<Match, 2>* const initialization) = 0;

  void SetPatchRadius(const unsigned int patchRadius)
  {
    this->PatchRadius = patchRadius;
  }

  void SetSourceMask(Mask* const sourceMask)
  {
    this->SourceMask = sourceMask;
  }

  void SetTargetMask(Mask* const targetMask)
  {
    this->TargetMask = targetMask;
  }

protected:

  unsigned int PatchRadius;

  Mask* SourceMask;
  Mask* TargetMask;
};

// template <typename TImage, typename TPatchDistanceFunctor>
// class InitializerImage : public InitializerPatch
// {
//   public:
//   InitializerImage() : Image(NULL), PatchDistanceFunctor(NULL) {}
// 
//   InitializerImage(TImage* const image, const unsigned int patchRadius) : InitializerPatch(patchRadius)
//   {
//     this->Image = image;
//   }
// 
//   virtual void Initialize(itk::Image<Match, 2>* const initialization) = 0;
// 
//   void SetImage(TImage* const image)
//   {
//     this->Image = image;
//   }
// 
//   void SetPatchDistanceFunctor(TPatchDistanceFunctor* const patchDistanceFunctor)
//   {
//     this->PatchDistanceFunctor = patchDistanceFunctor;
//   }
// 
// protected:
//   TImage* Image;
// 
//   TPatchDistanceFunctor* PatchDistanceFunctor;
// };

#endif
