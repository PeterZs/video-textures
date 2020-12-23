#ifndef A9_H_PHUDVTKB
#define A9_H_PHUDVTKB


#include "Image.h"
#include "basicImageManipulation.h"
#include "filtering.h"
#include "matrix.h"
#include <algorithm> // std::max, random_shuffle
#include <cmath>
#include <iostream>
#include <iomanip>

// Write your declarations here, or extend the Makefile if you add source
// files
Matrix FFDistances(const vector<Image> video);
Matrix probabilityFromDistance(const Matrix &D, float sigma = 0.2);

vector<float> binomialWeights(int filterSize = 4);
Matrix FFDistances_preserveDynamics(const vector<Image> frames, int m = 2);
Matrix QLEARN(const Matrix &D, int Niter = 50, float p = 1.0f, float alpha = 0.99f);

vector<vector<pair<float, int>>> pruneTransitions(const Matrix &Ppp, float threshold = 0.3333f);
Matrix pruneTransitions_Matrix(const Matrix &Ppp, float threshold = 0.3333f);

class Texture {
public:
  // Constructor to create a video texture from a video.
  Texture(const vector<Image> frames,
        const std::string &name = "");

  // Texture Class Destructor. Because there is no explicit memory management
  // here, this doesn't do anything
  ~Texture();

  // Returns the texture's name, should you specify one
  const std::string &name() const { return texture_name; }

  Matrix dynamicsPreservingDistances() const { return dynamics_preserving_distances; }
  Matrix futureCostDiscountedDistances() const { return future_cost_discounted_distances; }
  Matrix futureCostDiscountedProbabilities() const { return future_cost_discounted_probabilities; }

  // Returns the next frame to render, according to probability matrix
  int nextFrame(int currentFrame);

  // Generates video from texture using simple random play
  vector<Image> randomPlay(int numFrames);

  // The "private" section contains functions and variables that cannot be
  // accessed from outside the class.
private:
  std::vector<Image> frame_data;
  Matrix dynamics_preserving_distances;
  Matrix future_cost_discounted_distances;
  Matrix future_cost_discounted_probabilities;
  std::vector<std::vector<std::pair<float, int>>> list_of_transitions;
  std::string texture_name;
};

/***********************************************************************
 * Helper Functions *
 ***********************************************************************/

Image visualizeMatrix(const Matrix &D, bool probabilities = false);
unsigned nChoosek( unsigned n, unsigned k );

#endif /* end of include guard: A9_H_PHUDVTKB */

