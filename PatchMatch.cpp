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

#include "PatchMatch.h"

PatchMatch::PatchMatch() : PatchRadius(0), Random(true)
{
  this->Output = MatchImageType::New();

  this->SourceMask = Mask::New();
  this->TargetMask = Mask::New();
}


typename PatchMatch::MatchImageType*
PatchMatch::GetOutput()
{
  return this->Output;
}

void PatchMatch::SetIterations(const unsigned int iterations)
{
  this->Iterations = iterations;
}

void PatchMatch::SetPatchRadius(const unsigned int patchRadius)
{
  this->PatchRadius = patchRadius;
}

void PatchMatch::SetSourceMask(Mask* const mask)
{
  this->SourceMask->DeepCopyFrom(mask);
}

void PatchMatch::SetTargetMask(Mask* const mask)
{
  this->TargetMask->DeepCopyFrom(mask);
}

