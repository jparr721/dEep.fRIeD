#include <armadillo>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <sys/stat.h>
#include <opencv2/opencv.hpp>

#define yeet throw;

bool valid_path(const std::string& path) {
  struct stat buf;
  return (stat(path.c_str(), &buf) == 0);
}

struct Image {
  Image(const std::string& img_path, const std::string& output_path="") : img_path(img_path), output_path(output_path) {
    if (!valid_path(img_path)) {
      yeet std::invalid_argument("wrong path bud");
      exit(1);
    }
    cv::Mat temp_image = cv::imread(img_path);
    cv::Mat temp_image_2;

    // Convert to floating point image for SVD
    temp_image.convertTo(temp_image_2, CV_64F, 1.0/255);

    arma::mat image(reinterpret_cast<double*>(temp_image_2.data), temp_image_2.rows, temp_image_2.cols);
    this->image = image;
  }

  arma::mat image;

  private:
    const std::string img_path;
    const std::string output_path;
};

arma::mat approximation(arma::mat image) {
  // Anything below our threshold we do not care about
  const int relevancy_threshold{-10};

  // Our U matrix
  arma::mat U;
  // Our sigma vector of singular values
  arma::vec S;
  // Our non-transposed V matrix
  arma::mat V;

  // Calculate our singular value decomposition
  arma::svd(U, S, V, image);

  int k = std::accumulate(S.begin(), S.end(), 0,
      [](int count, const int& v) { return v > relevancy_threshold ? ++count : count; });

  arma::mat Sigma(arma::size(image), arma::fill::zeros);

  for (int i = 0; i < k; ++i) {
    Sigma.at(i, i) = S(i);
  }

  auto sv = Sigma * V;
  return U * sv;
}

void draw_that_boy(arma::mat image) {
  const auto rows = image.n_rows;
  const auto cols = image.n_cols;

  // Convert now to opencv to display
  cv::Mat cv_img(rows, cols, CV_64F, image.memptr());
  cv::imshow("FRESH OUTTA THE FRIER", cv_img);
  cv::waitKey();
}

void help() {
  std::cout << "dEeP.fRIeD\t\t Usage" << std::endl;
  std::cout << "cookout <img_path>\t\t\t\t - Compress the shit out of an image in place" << std::endl;
  std::cout << "coolout <img_path> <output_path>\t\t - Compress the shit out of an image and save to another one" << std::endl;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    help();
  }

  switch(argc) {
    case 2:
      Image img(argv[1]);
      draw_that_boy(approximation(img.image));
  }
}
