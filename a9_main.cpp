#include <iostream>

#include "a9.h"

using namespace std;

void testFFDistances_clock() {
  // load images
  vector<Image> seq;
  int n_images = 37;
  for (int i = 1; i <= n_images; ++i) {
      ostringstream fname;
      fname << "./Input/vtclk1a_frames/";
      fname << i;
      fname << ".png";
      seq.push_back(Image(fname.str()));
  }

  // calculate frame-to-frame distances
  Matrix D = FFDistances(seq);

  // calculate frame-to-frame probabilities
  Matrix P = probabilityFromDistance(D);

  visualizeMatrix(D).write("./Output/clock_ffdistances.png");
  visualizeMatrix(P, true).write("./Output/clock_ffprobabilities.png");
}

void testBinomialWeights() {
  vector<float> weights = binomialWeights();
  for (float w : weights) {
  	cout << w << ", ";
  }
  cout << endl;
}

void testFFDistances_preserveDynamics_clock() {
  // load images
  vector<Image> seq;
  int n_images = 37;
  for (int i = 1; i <= n_images; ++i) {
      ostringstream fname;

      fname << "./Input/vtclk1a_frames/";
      fname << i;
      fname << ".png";
      seq.push_back(Image(fname.str()));
  }

  // calculate frame-to-frame distances
  Matrix Dp = FFDistances_preserveDynamics(seq);

  // calculate frame-to-frame probabilities
  Matrix Pp = probabilityFromDistance(Dp);

  visualizeMatrix(Dp).write("./Output/clock_PD_ffdistances.png");
  visualizeMatrix(Pp, true).write("./Output/clock_PD_ffprobabilities.png");
}

void testQLEARN_clock() {
  // load images
  vector<Image> seq;
  int n_images = 67;
  for (int i = 1; i <= n_images; ++i) {
      ostringstream fname;
      fname << "./Input/vtclk2a_frames/";
      fname << i;
      fname << ".png";
      seq.push_back(Image(fname.str()));
  }

  // calculate frame-to-frame distances
  Matrix Dp = FFDistances_preserveDynamics(seq);
  Matrix Dpp = QLEARN(Dp, 50);

  // calculate frame-to-frame probabilities
  Matrix Pp = probabilityFromDistance(Dp);
  Matrix Ppp = probabilityFromDistance(Dpp);

  visualizeMatrix(Pp, true).write("./Output/clock2_PD_ffprobabilities.png");
  visualizeMatrix(Ppp, true).write("./Output/clock2_QLEARN_ffprobabilities.png");
}

void testPruneTransitions_clock() {
  // load images
  vector<Image> seq;
  int n_images = 67;
  for (int i = 1; i <= n_images; ++i) {
      ostringstream fname;
      fname << "./Input/vtclk2a_frames/";
      fname << i;
      fname << ".png";
      seq.push_back(Image(fname.str()));
  }

  // calculate frame-to-frame distances
  Matrix Dp = FFDistances_preserveDynamics(seq);
  Matrix Dpp = QLEARN(Dp, 50);

  // calculate frame-to-frame probabilities
  Matrix Ppp = probabilityFromDistance(Dpp);
  Matrix P = pruneTransitions_Matrix(Ppp);

  visualizeMatrix(P, true).write("./Output/clock2_PT_ffprobabilities.png");
}

void testRandomPlay_clock() {
  // load images
  vector<Image> seq;
  int n_images = 67;
  for (int i = 1; i <= n_images; ++i) {
      ostringstream fname;
      fname << "./Input/vtclk2a_frames/";
      fname << i;
      fname << ".png";
      seq.push_back(Image(fname.str()));
  }

  Texture clock(seq);
  Matrix Ppp = clock.futureCostDiscountedProbabilities();
  Matrix P = pruneTransitions_Matrix(Ppp);
  visualizeMatrix(P, true).write("./Output/clock2_PT_ffprobabilities.png");
  vector<Image> out = clock.randomPlay(600);

  // This is how you can write an image sequence
  for (int i = 0; i < (int)out.size(); ++i) {
    ostringstream fname;
    fname << "./Output/randomPlay_clock_";
    fname << setfill('0') << setw(3);
    fname << i;
    fname << ".png";
    out[i].write(fname.str());	
  }
}

void testRandomPlay_plant() {
  // load images
  vector<Image> seq;
  int n_images = 148;
  for (int i = 1; i <= n_images; ++i) {
      ostringstream fname;
      fname << "./Input/plant360_frames/";
      fname << i;
      fname << ".png";
      seq.push_back(Image(fname.str()));
  }

  Texture plant(seq);

  Matrix Ppp = plant.futureCostDiscountedProbabilities();
  Matrix P = pruneTransitions_Matrix(Ppp);
  visualizeMatrix(P, true).write("./Output/plant_PT_ffprobabilities.png");
  vector<Image> out = plant.randomPlay(600);

  // This is how you can write an image sequence
  for (int i = 0; i < (int)out.size(); ++i) {
    ostringstream fname;
    fname << "./Output/randomPlay_plant_";
    fname << setfill('0') << setw(3);
    fname << i;
    fname << ".png";
    out[i].write(fname.str());	
  }
}

int main()
{
  // this sets the random number generator seed, without varying this value, all
  // runs should produce the same results (e.g. same sequence of "random"
  // number)
  // when you are ready for multiple runs try srand(time(0));
  // this will set it to the current system time
  srand(0); // Fixed seed for deterministic results

  // Test your intermediate functions
  testFFDistances_clock();
  testBinomialWeights();
  testFFDistances_preserveDynamics_clock();
  testQLEARN_clock();
  testPruneTransitions_clock();

  // results
  testRandomPlay_clock();
  testRandomPlay_plant();
  return EXIT_SUCCESS;
}
