#include <opencv2/opencv.hpp>
#include <fstream>

using namespace cv;
using namespace std;

int main() {
    // Input image
    Mat img = imread("road.jpg");
    if (img.empty()) {
        printf("Error: Image not found!\n");
        return -1;
    }

    // Convert to grayscale
    Mat gray, blurImg, thresh, morph;
    cvtColor(img, gray, COLOR_BGR2GRAY);

    // Reduce noise
    GaussianBlur(gray, blurImg, Size(5, 5), 0);

    // Adaptive Thresholding to detect dark potholes
    adaptiveThreshold(blurImg, thresh, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 11, 5);

    // Morphological closing to remove small holes
    Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
    morphologyEx(thresh, morph, MORPH_CLOSE, kernel);

    // Find contours (pothole regions)
    vector<vector<Point>> contours;
    findContours(morph, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // Create GeoJSON file
    ofstream geo("potholes.geojson");
    geo << "{\n  \"type\": \"FeatureCollection\",\n  \"features\": [\n";

    bool first = true;
    for (auto cnt : contours) {
        double area = contourArea(cnt);
        // Skip very small areas
        if (area < 300) continue;

        // Calculate centroid
        Moments m = moments(cnt);
        int cx = int(m.m10 / m.m00);
        int cy = int(m.m01 / m.m00);

        if (!first) geo << ",\n";
        first = false;

        // Write to GeoJSON
        geo << "    {\n";
        geo << "      \"type\": \"Feature\",\n";
        geo << "      \"geometry\": {\n";
        geo << "        \"type\": \"Point\",\n";
        geo << "        \"coordinates\": [" << cx << ", " << cy << "]\n";
        geo << "      },\n";
        geo << "      \"properties\": {\n";
        geo << "        \"area\": " << area << "\n";
        geo << "      }\n";
        geo << "    }\n";
    }

    geo << "  ]\n}\n";
    geo.close();

    // Save the mask for output view
    imwrite("mask_output.png", morph);
    printf("Pothole detection completed. GeoJSON saved as potholes.geojson\n");

    return 0;
}
