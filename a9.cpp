#include <iostream>

#include "a9.h"

using namespace std;

// Write your implementations here, or extend the Makefile if you add source
// files

Matrix FFDistances(const vector<Image> frames) {
  // Return the matrix of frame-to-frame distances using L2 norm.
  // D[in, out] -> frame in, out (in should exclude first frame, out should exclude last frame)
  int numFrames = frames.size();
  Matrix D(numFrames, numFrames-1);
  for (int i=0; i<numFrames; i++) {
    if (i < numFrames-1) {
      D(i, i) = 0;
    }
    for (int j=i+1; j<numFrames; j++) {
      float dist = (frames[i] - frames[j]).var();
      if (j < numFrames-1) {
        D(i, j) = dist;
      }
      if (i < numFrames-1) {
        D(j, i) = dist;
      }
    }
  }
  return D;
}

Matrix probabilityFromDistance(const Matrix &D, float sigma) {
  // Ensure rows sum to one (i.e. valid probability matrix)
  Matrix P(D.rows()-1, D.cols());
  for (int i=0; i<P.rows(); i++) {
    float sum = 0.0f;
    for (int j=0; j<P.cols(); j++) {
      P(i, j) = exp(-1*D(i+1, j)/D.mean()/sigma);
      sum += P(i, j);
    }
    P.row(i) /= sum;
  }
  return P;
}

vector<float> binomialWeights(int filterSize) {
  vector <float> fData (filterSize, 0.0f);

  // compute the un-normalized value of the gaussian
  // float normalizer = 0.0f;
  for( int i = 0; i < filterSize; i++){
      fData[i] = nChoosek(filterSize - 1, i);
      // normalizer += fData[i];
  }

  // // normalize
  // for( int i=0; i < filterSize; i++){
  //     fData[i] /= normalizer;
  // }
  return fData;
}

Matrix FFDistances_preserveDynamics(const vector<Image> frames, int m) {
  // Return the matrix of frame-to-frame distances using L2 norm and 2m-tap filter.
  int numFrames = frames.size();
  Matrix D = FFDistances(frames);
  vector<float> weights = binomialWeights(2*m);

  Matrix Dp(D.rows() - 2*m, D.cols() - 2*m);
  for (int i=m; i<numFrames-m; i++) {
    if (i-m < Dp.cols()) {
      Dp(i-m, i-m) = 0.0f;
    }
    for (int j=i+1; j<numFrames-m; j++) {
      float dist = 0.0f;
      for (int k=-1*m; k <= m-1; k++) {
        if (j+k < D.cols()) {
          dist += weights[k + m]*D(i+k, j+k);
        }
      }
      if (j-m < Dp.cols()) {
        Dp(i-m, j-m) = dist;
      }
      if (i-m < Dp.cols()) {
        Dp(j-m, i-m) = dist;
      }
    }
  }
  return Dp;
}

Matrix QLEARN(const Matrix &Dp, int Niter, float p, float alpha) {
  // Put together the modified Q-Learning algorithm.

  Matrix Dpp(Dp.rows(), Dp.cols());

  for (int i=0; i<Dpp.rows(); i++) {
    for (int j=0; j<Dpp.cols(); j++) {
      Dpp(i, j) = pow(Dp(i, j), p);
    }
  }

  // int bestCount = 0;
  for (int iter=0; iter<Niter; iter++) {
    int i, j;
    for (int i=Dpp.rows()-1; i>=0; i--) {
      for (int j=0; j<Dpp.cols(); j++) {
        float minCost = Dpp.row(j+1).minCoeff();
        Dpp(i, j) = pow(Dp(i, j), p) + alpha*minCost;
      }
    }
  }
  return Dpp;
}

vector<vector<pair<float, int>>> pruneTransitions(const Matrix &Ppp, float threshold) {
  // Keep only local maxima above a threshold
  // Ensure rows sum to one (i.e. valid probability matrix)

  vector<vector<pair<float, int>>> listOfTransitions;

  for (int i=0; i<Ppp.rows(); i++) {
    vector<pair<float, int>> transitions;
    float sum = 0.0f;
    cout << "i=" << i << " -> ";
    for (int j=0; j<Ppp.cols(); j++) {
      bool maxima = true;
      if (j < Ppp.cols()-1 && Ppp(i, j+1) > Ppp(i, j)) {
        maxima = false;
      }
      if (j > 0 && Ppp(i, j-1) > Ppp(i, j)) {
        maxima = false;
      }
      if (maxima) {
        transitions.push_back(make_pair(Ppp(i, j), j));
        sum += Ppp(i, j);
      }
    }

    // ensure row sums to one
    for (int j=0; j<transitions.size(); j++) {
      pair<float, int> frame = transitions[j];
      transitions[j] = make_pair(frame.first/sum, frame.second);
    }

    // filter by threshold
    sum = 0.0f;
    float bestValue = 0.0f;
    int bestIndex;
    for (int j=0; j<transitions.size(); j++) {
      if (transitions[j].first > bestValue) {
        bestValue = transitions[j].first;
        bestIndex = j;
      }
    }
    vector<pair<float, int>> bestTransitions;
    for (int j=0; j<transitions.size(); j++) {
      if (transitions[j].first > threshold || j == bestIndex) {
        bestTransitions.push_back(transitions[j]);
        sum += transitions[j].first;
      }
    }
    transitions = bestTransitions;

    // ensure row sums to one
    for (int j=0; j<transitions.size(); j++) {
      pair<float, int> frame = transitions[j];
      cout << frame.second << ": " << frame.first/sum << " // ";
      transitions[j] = make_pair(frame.first/sum, frame.second);
    }
    listOfTransitions.push_back(transitions);
    cout << endl;
  }
  return listOfTransitions;
}

Matrix pruneTransitions_Matrix(const Matrix &Ppp, float threshold) {
  Matrix P(Ppp.rows(), Ppp.cols());
  for (int i=0; i<Ppp.rows(); i++) {
    for (int j=0; j<Ppp.cols(); j++) {
        P(i, j) = 0.0f;
    }
  }

  vector<vector<pair<float, int>>> listOfTransitions = pruneTransitions(Ppp, threshold);
  for (int i=0; i<Ppp.rows(); i++) {
    for (pair<float, int> frame : listOfTransitions[i]) {
      P(i, frame.second) = frame.first;
    }
  }
  return P;
}

int Texture::nextFrame(int currentFrame) {
  // Returns the next frame to render, according to probability matrix
  float r = (float) rand()/RAND_MAX;
  float total = 0.0f;
  for (pair<float, int> frame : list_of_transitions[currentFrame]) {
    total += frame.first;
    if (r < total) {
      return frame.second;
    }
  }
  return currentFrame;
}

vector<Image> Texture::randomPlay(int numFrames) {
  // Generates video from texture using simple random play
  vector<Image> seq;
  int currentFrame = 0;
  for (int i = 0; i <= numFrames; ++i) {
    seq.push_back(frame_data[currentFrame]);
    int newFrameNumber = nextFrame(currentFrame);
    cout << currentFrame << " -> " << newFrameNumber << endl;
    currentFrame = newFrameNumber;
  }
  return seq;
}

Texture::Texture(const vector<Image> frames, const std::string &name_) {
  frame_data = frames;

  Matrix Dp = FFDistances_preserveDynamics(frames);
  Matrix Dpp = QLEARN(Dp, 50);
  Matrix Ppp = probabilityFromDistance(Dpp);

  dynamics_preserving_distances = Dp;
  future_cost_discounted_distances = Dpp;
  future_cost_discounted_probabilities = Ppp;
  list_of_transitions = pruneTransitions(Ppp);

  texture_name = name_;
}

Texture::~Texture() {} // Nothing to clean up

/***********************************************************************
 * Helper Functions *
 ***********************************************************************/

// Matrix visualization, normalize (divide by max)
Image visualizeMatrix(const Matrix &D, bool probabilities) {
  Image im(D.cols(), D.rows(), 1);
  for (int y=0; y<im.height(); y++) {
    for (int x=0; x<im.width(); x++) {
      if (probabilities) {
        im(x,y,0) = max(log10(D(y, x)), -1.4f) + 1.9f;
      } else {
        im(x,y,0) = D(y, x);
      }
    }
  }
  if (!probabilities)
    im = im / im.max();
  return im;
}

// https://stackoverflow.com/questions/9330915/number-of-combinations-n-choose-r-in-c
unsigned nChoosek( unsigned n, unsigned k )
{
    if (k > n) return 0;
    if (k * 2 > n) k = n-k;
    if (k == 0) return 1;

    int result = n;
    for( int i = 2; i <= k; ++i ) {
        result *= (n-i+1);
        result /= i;
    }
    return result;
}