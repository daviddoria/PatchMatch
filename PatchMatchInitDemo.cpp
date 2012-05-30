#include <iostream>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCovariantVector.h"

#include "Mask/Mask.h"
#include "Mask/ITKHelpers/ITKHelpers.h"

#include "PatchMatch.h"

typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;

int main(int argc, char*argv[])
{
  if(argc < 3)
  {
    std::cerr << "Required arguments: image mask" << std::endl;
    return EXIT_FAILURE;
  }

  std::string imageFilename = argv[1];
  std::string maskFilename = argv[2];

  std::cout << "imageFilename: " << imageFilename << std::endl;
  std::cout << "maskFilename: " << maskFilename << std::endl;

  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFilename);
  imageReader->Update();

  Mask::Pointer sourceMask = Mask::New();
  sourceMask->Read(maskFilename);

  Mask::Pointer targetMask = Mask::New();
  targetMask->SetRegions(sourceMask->GetLargestPossibleRegion());
  targetMask->Allocate();
  ITKHelpers::SetImageToConstant(targetMask.GetPointer(), targetMask->GetValidValue());

  PatchMatch patchMatch;
  patchMatch.SetImage(imageReader->GetOutput());
  patchMatch.SetTargetMask(targetMask);
  patchMatch.SetSourceMask(sourceMask);
  patchMatch.SetIterations(10);
  patchMatch.SetPatchRadius(3);

  PatchMatch::VectorImageType::Pointer output = PatchMatch::VectorImageType::New();

  {
  std::cout << "Starting randomInit..." << std::endl;
  patchMatch.RandomInit();

  PatchMatch::GetPatchCentersImage(patchMatch.GetOutput(), output);
  ITKHelpers::WriteImage(output.GetPointer(), "randomInit.mha");
  }

  {
  std::cout << "Starting boundaryInit..." << std::endl;
  patchMatch.BoundaryInit();
  PatchMatch::GetPatchCentersImage(patchMatch.GetOutput(), output);
  ITKHelpers::WriteImage(output.GetPointer(), "boundaryInit.mha");
  }

  return EXIT_SUCCESS;
}
