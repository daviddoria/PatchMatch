#ifndef BDSInpainting_H
#define BDSInpainting_H

#include "itkCovariantVector.h"
#include "itkImage.h"
#include "itkImageRegion.h"
#include "itkVectorImage.h"

#include "Mask/Mask.h"

class BDSInpainting
{
public:

  BDSInpainting();

  typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;

  void Compute();

  ImageType* GetOutput();

  void SetIterations(const unsigned int iterations);

  void SetPatchDiameter(const unsigned int patchDiameter);

  void SetImage(ImageType* const image);

  void SetMask(Mask* const mask);

private:

  unsigned int Iterations;
  unsigned int PatchDiameter;

  ImageType::Pointer Output;

  ImageType::Pointer Image;

  Mask::Pointer MaskImage;

};

#endif


