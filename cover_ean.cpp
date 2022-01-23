// based on https://github.com/rportugal/opencv-zbar
//#define DEBUG
#define CONTOUR_MIN 128

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#if (!defined(CV_VERSION_EPOCH) && CV_VERSION_MAJOR >= 4)
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#endif

#include <zbar.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace cv;
using namespace zbar;

struct FoundBarcode
{
    std::string ean;
    int x1, y1, x2, y2;
};

void detect_barcodes(ImageScanner &scanner, vector<FoundBarcode> &found_barcodes, cv::Mat subframe, cv::Rect bounding)
{
    int width = subframe.cols;
    int height = subframe.rows;
    // Obtain image data
    uchar *raw = (uchar *)(subframe.data);

    // Wrap image data
    Image image(width, height, "Y800", raw, width * height);

    // Scan the image for barcodes
    int n = scanner.scan(image);

    // Extract results
    for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
    {
        int x1 = width, y1 = height, x2 = 0, y2 = 0;
        FoundBarcode found_barcode;

        for (int i = 0; i < symbol->get_location_size(); i++)
        {
            int curx, cury;
            curx = symbol->get_location_x(i);
            cury = symbol->get_location_y(i);
            if (curx < x1)
                x1 = curx;
            if (cury < y1)
                y1 = cury;
            if (curx > x2)
                x2 = curx;
            if (cury > y2)
                y2 = cury;
        }

        found_barcode.ean = symbol->get_data();
        found_barcode.x1 = x1 + bounding.x;
        found_barcode.y1 = y1 + bounding.y;
        found_barcode.x2 = x2 + bounding.x;
        found_barcode.y2 = y2 + bounding.y;

        found_barcodes.push_back(found_barcode);
    }

    // clean up
    image.set_data(NULL, 0);
}

int main(int argc, char **argv)
{
    vector<FoundBarcode> found_barcodes;
    cv::Mat frame;

    if (argc < 2)
    {
        cerr << "Usage: cover_ean file" << endl;
        exit(1);
    }

    frame = imread(argv[1], CV_LOAD_IMAGE_COLOR);
    if (frame.data == NULL)
    {
        cerr << "Invalid file " << argv[1] << endl;
        exit(1);
    }

    // Create a zbar reader
    ImageScanner scanner;

    // Configure the reader
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 0);
    scanner.set_config(ZBAR_EAN13, ZBAR_CFG_ENABLE, 1);
    scanner.set_config(ZBAR_ISBN13, ZBAR_CFG_ENABLE, 1);
    scanner.set_config(ZBAR_ISBN10, ZBAR_CFG_ENABLE, 1);

    cv::Mat frame_grayscale;

    // Convert to grayscale
    cvtColor(frame, frame_grayscale, CV_BGR2GRAY);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    // on commence par isoler les différents rectangles pouvant contenir le code barre
    cv::Mat frame_cont;
    threshold(frame_grayscale, frame_cont, 100, 255, THRESH_BINARY_INV);
    findContours(frame_cont, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

#ifdef DEBUG
    imwrite("out.png", frame_cont);
#endif

#ifdef DEBUG
    cerr << "DBG: found " << contours.size() << " contours" << endl;
#endif

    int n_rect = 0;
    for (size_t i = 0; i < contours.size(); i++)
    {
        Mat subframe;
        Rect bounding = boundingRect(contours.at(i));
        // on limite aux rectangle assez grand mais plus petit que l'image entière
        if (bounding.width > CONTOUR_MIN && bounding.height > CONTOUR_MIN &&
            bounding.width < frame_grayscale.rows && bounding.height < frame_grayscale.cols)
        {
            // cout << bounding << endl;
            frame_grayscale(bounding).copyTo(subframe);

#ifdef DEBUG
            imwrite("out" + std::to_string(n_rect) + ".png", subframe);
#endif

            detect_barcodes(scanner, found_barcodes, subframe, bounding);

            subframe.release();
            n_rect++;
        }
    }

    // on essaye sur l'image entière si on a rien trouvé
    if (found_barcodes.size() == 0)
    {
        detect_barcodes(scanner, found_barcodes, frame_grayscale, Rect(0, 0, frame_grayscale.cols, frame_grayscale.rows));
    }

#ifdef DEBUG
    cerr << "DBG: found " << n_rect << " possible rectangles" << endl;
    cerr << "DBG: found " << found << " barcodes" << endl;
#endif

    cout << "<cover_ean file=\"" << argv[1] << "\" width=\"" << frame_grayscale.cols << "\" height=\"" << frame_grayscale.rows << "\" found=\"" << found_barcodes.size() << "\">" << endl;
    for (FoundBarcode found_barcode : found_barcodes)
    {
        cout << " <result ean=\"" << found_barcode.ean << "\" x1=\"" << found_barcode.x1 << "\" y1=\"" << found_barcode.y1 << "\" x2=\"" << found_barcode.x2 << "\" y2=\"" << found_barcode.y2 << "\"/>" << endl;
    }
    cout << "</cover_ean>" << endl;

    frame_cont.release();
    frame_grayscale.release();
    frame.release();

    return 0;
}
