#ifndef PATCHMATCH_H
#define PATCHMATCH_H

// STL
#include <functional>

// Eigen
#include <Eigen/Dense>

// ITK
#include "itkCovariantVector.h"
#include "itkImage.h"
#include "itkImageRegion.h"
#include "itkVectorImage.h"

// Submodules
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

  enum ENUM_DISTANCE_TYPE { PIXELWISE, PCA };

  void SetDistanceType(const ENUM_DISTANCE_TYPE);
  
  typedef itk::Image<Match, 2> PMImageType;

  typedef itk::Image<itk::CovariantVector<float, 3>, 2> ImageType;
  typedef itk::VectorImage<float, 2> VectorImageType;

  /** Do the real work. */
  void Compute(PMImageType* const initialization);

  /** Get the Output. */
  PMImageType* GetOutput();

  /** Set the number of iterations to perform. */
  void SetIterations(const unsigned int iterations);

  /** Set the radius of the patches to use. */
  void SetPatchRadius(const unsigned int patchRadius);

  /** Set the image to operate on. */
  void SetImage(ImageType* const image);

  /** Set the mask indicating where to ignore patches for comparison. */
  void SetSourceMask(Mask* const mask);

  /** Set the mask indicating where to compute the NNField. */
  void SetTargetMask(Mask* const mask);

  /** Get an image where the channels are (x component, y component, score) from the nearest neighbor field struct. */
  static void GetPatchCentersImage(PMImageType* const pmImage, itk::VectorImage<float, 2>* const output);

  /** Set the nearest neighbor field to exactly iself in the valid region, and random values in the hole region. */
  void RandomInit();

  /** Assume that hole pixels near the hole boundary will have best matching patches on the other side of the hole
   *  boundary in the valid region. */
  void BoundaryInit();

private:

  std::function<float(const itk::ImageRegion<2>&,const itk::ImageRegion<2>&,const float)> Distance;
  
  float PixelWiseDistance(const itk::ImageRegion<2>& source,
                 const itk::ImageRegion<2>& target,
                 const float prevDist = std::numeric_limits<float>::max());

  float PCADistance(const itk::ImageRegion<2>& source,
                 const itk::ImageRegion<2>& target,
                 const float prevDist = std::numeric_limits<float>::max());

  /** Set the nearest neighbor field to exactly iself in the valid region. */
  void InitKnownRegion();

  /** Set the number of iterations to perform. */
  unsigned int Iterations;

  /** Set the radius of the patches to use. */
  unsigned int PatchRadius;

  /** The intermediate and final output. */
  PMImageType::Pointer Output;

  /** The image to operate on. */
  ImageType::Pointer Image;

  /** This mask indicates where to take source patches from. */
  Mask::Pointer SourceMask;

  /** This mask indicates where to compute the NN field. */
  Mask::Pointer TargetMask;

  /** The projection matrix to project patches to a lower dimensional space. */
  typedef Eigen::MatrixXf MatrixType;
  typedef Eigen::VectorXf VectorType;
  MatrixType ProjectionMatrix;
};

#endif
