/**
 * @Author: PALARD Nicolas <nclsp>
 * @Date:   2019-04-11T10:48:56+02:00
 * @Email:  palard@rea.lity.tech
 * @Project: Natar.io
 * @Last modified by:   nclsp
 * @Last modified time: 2019-04-11T10:49:29+02:00
 * @Copyright: RealityTech 2018-2019
 */

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <RedisImageHelper.hpp>

int main(int argc, char** argv) {
    std::string host = "127.0.0.1";
    int port = 6379;

    if (argc != 2) {
        std::cerr << "Missing commandline argument: ./natar-blur image-key";
        return EXIT_FAILURE;
    }
    std::string imageKey = argv[1];

    RedisImageHelperSync clientSync(host, port, "");
    if (!clientSync.connect()) {
        std::cerr << "Cannot connect to redis server. Please ensure that a redis server is up and running." << std::endl;
        return EXIT_FAILURE;
    }

    // Getting image data from Natar (every image has its associated width, height, channels keys)
    int width = clientSync.getInt(imageKey + ":width");
    int height = clientSync.getInt(imageKey + ":height");
    int channels = clientSync.getInt(imageKey + ":channels");

    // Getting image from Natar
    Image* image = clientSync.getImage(width, height, channels, imageKey);
    if (image == NULL) { std::cerr << "Error: Could not retrieve image from data\nExiting..." << std::endl; return EXIT_FAILURE; }

    // Converting Natar image to OpenCV
    cv::Mat rgbImage = cv::Mat(image->height(), image->width(), CV_8UC3, (void*)image->data());

    // Converting image from RGB to BGR (OpenCV format)
    cv::Mat bgrImage;
    cv::cvtColor(rgbImage, bgrImage, cv::COLOR_RGB2BGR);

    // Blurring the image
    cv::Mat blurImage;
    cv::GaussianBlur(bgrImage, blurImage, cv::Size(11, 11), 11);

    // Converting image from BGR to RGB (Natar format)
    cv::Mat rgbOutputImage;
    cv::cvtColor(blurImage, rgbOutputImage, cv::COLOR_BGR2RGB);

    // Converting OpenCV image to Natar
    Image* outputImage = new Image(rgbOutputImage.cols, rgbOutputImage.rows, rgbOutputImage.channels(), rgbOutputImage.data);
    // Setting image into Natar
    clientSync.setImage(outputImage, imageKey + ":blurred");

    delete image, outputImage;
}
