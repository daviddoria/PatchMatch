#ifndef PATCHMATCH_H
#define PATCHMATCH_H

#include "itkCovariantVector.h"
#include "itkImage.h"
#include "itkImageRegion.h"
#include "itkVectorImage.h"

#include "Mask/Mask.h"

struct Match
{
  itk::ImageRegion<2> Region;
  float Score;
};

class PatchMatch
{
public:

  PatchMatch();
  
  typedef itk::Image<Match, 2> PMImageType;

  typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;
  typedef itk::VectorImage<float, 2> VectorImageType;

  void Compute(PMImageType* const initialization);

  PMImageType* GetOutput();

  void SetIterations(const unsigned int iterations);

  void SetPatchDiameter(const unsigned int patchDiameter);

  void SetImage(ImageType* const image);

  void SetMask(Mask* const mask);

  void GetPatchCentersImage(PMImageType* const pmImage, itk::VectorImage<float, 2>* const output);
  
private:

  static float distance(const itk::ImageRegion<2>& source,
                        const itk::ImageRegion<2>& target,
                        const float prevDist);

  void Random();

  unsigned int Iterations;
  unsigned int PatchDiameter;

  PMImageType::Pointer Output;

  ImageType::Pointer Image;

  Mask::Pointer MaskImage;

};

#endif
