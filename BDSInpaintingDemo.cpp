#include <iostream>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCovariantVector.h"

#include "Mask/Mask.h"
#include "Mask/ITKHelpers/ITKHelpers.h"

#include "BDSInpainting.h"

typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;

int main(int argc, char*argv[])
{
  if(argc < 4)
  {
    std::cerr << "Required arguments: image mask output" << std::endl;
    return EXIT_FAILURE;
  }

  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];
  std::string outputFilename = argv[3];

  std::cout << "imageFilename: " << imageFilename << std::endl;
  std::cout << "maskFilename: " << maskFilename << std::endl;
  std::cout << "outputFilename: " << outputFilename << std::endl;

  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  Mask::Pointer mask = Mask::New();
  mask->SetHoleValue(0);
  mask->SetValidValue(255);
  mask->Read(maskFilename);

  BDSInpainting bdsInpainting;
  bdsInpainting.SetImage(imageReader->GetOutput());
  bdsInpainting.SetMask(mask);
  bdsInpainting.SetResolutionLevels(4);
  bdsInpainting.SetIterations(10);
  bdsInpainting.SetPatchRadius(7);
  bdsInpainting.SetPatchMatchIterations(4);
  bdsInpainting.Compute();

  ITKHelpers::WriteRGBImage(bdsInpainting.GetOutput(), outputFilename);

  return EXIT_SUCCESS;
}