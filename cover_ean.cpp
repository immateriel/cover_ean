// based on https://github.com/rportugal/opencv-zbar

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <zbar.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace cv;
using namespace zbar;

int main(int argc, char **argv) {
    cv::Mat frame;

    if(argc < 2) {
        cerr << "Usage: cover_ean file"<<endl;
        exit(1);
    }

    frame = imread(argv[1], CV_LOAD_IMAGE_COLOR);
    if(frame.data == NULL) {
        cerr << "Invalid file "<<argv[1]<<endl;
        exit(1);        
    }

    // Create a zbar reader
    ImageScanner scanner;

    // Configure the reader
    scanner.set_config(ZBAR_ISBN13, ZBAR_CFG_ENABLE, 1);

    cv::Mat frame_grayscale;

    // Convert to grayscale
    cvtColor(frame, frame_grayscale, CV_BGR2GRAY);

    // Obtain image data
    int width = frame_grayscale.cols;
    int height = frame_grayscale.rows;
    uchar *raw = (uchar *)(frame_grayscale.data);

    // Wrap image data
    Image image(width, height, "Y800", raw, width * height);

    // Scan the image for barcodes
    int n = scanner.scan(image);

    cout << "<cover_ean file=\""<< argv[1] <<"\" width=\""<<width << "\" height=\""<<height << "\" found=\""<< n << "\">" << endl;

    // Extract results
    int counter = 0;
    for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
        int x1 = width,y1 = height,x2 = 0,y2 = 0;
        time_t now;
        tm *current;
        now = time(0);
        current = localtime(&now);

        for(int i = 0; i < symbol->get_location_size(); i++) {
            int curx, cury;
            curx = symbol->get_location_x(i);
            cury = symbol->get_location_y(i);
            if(curx < x1)
                x1 = curx;
            if(cury < y1)
                y1 = cury;
            if(curx > x2)
                x2 = curx;
            if(cury > y2)
                y2 = cury;
        }

        cout << " <result ean=\""<<symbol->get_data()<<"\" x1=\""<<x1<<"\" y1=\""<<y1<<"\" x2=\""<<x2<<"\" y2=\""<<y2<<"\"/>" << endl;

        counter++;
    }

    // clean up
    image.set_data(NULL, 0);

    cout << "</cover_ean>" << endl;

    return 0;
}
