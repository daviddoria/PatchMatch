#include <iostream>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCovariantVector.h"

#include "Mask/Mask.h"

#include "PatchMatch.h"

typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;

int main(int argc, char*argv[])
{
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

  typedef itk::ImageFileReader<Mask> MaskReaderType;
  MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName(maskFilename);
  maskReader->Update();

  PatchMatch patchMatch;
  patchMatch.SetIterations(20);
  patchMatch.SetPatchDiameter(15);

  typedef itk::VectorImage<float, 2> VectorImageType;
  VectorImageType::Pointer output = VectorImageType::New();
  
  patchMatch.GetPatchCentersImage(patchMatch.GetOutput(), output.GetPointer());
  
  typedef  itk::ImageFileWriter<VectorImageType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(outputFilename);
  writer->SetInput(output);
  writer->Update();

  return EXIT_SUCCESS;
}
