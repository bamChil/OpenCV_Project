#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// 使用HSV提取红色轮廓
Mat extractRedHSV(const Mat& img) {
    Mat hsv;
    cvtColor(img, hsv, COLOR_BGR2HSV);

    // 定义HSV中的红色范围
    Scalar lowerRed1(0, 43, 46), upperRed1(10, 255, 255);
    Scalar lowerRed2(156, 43, 46), upperRed2(180, 255, 255);

    // 提取红色mask
    Mat mask1, mask2;
    inRange(hsv, lowerRed1, upperRed1, mask1);
    inRange(hsv, lowerRed2, upperRed2, mask2);

    // 结合mask
    Mat redMask = mask1 | mask2;
    return redMask;
}

int main() {
    // 加载图片
    string imagePath = "../resources/test_image.png";
    Mat img = imread(imagePath);
    if (img.empty()) {
        cout << "Error: Could not load image." << endl;
        return -1;
    }

    // 创建结果文件夹
    string resultDir = "../results/";
    system(("mkdir -p " + resultDir).c_str());

    // 1.1 转为灰度图
    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);
    imwrite(resultDir + "gray_image.png", gray);

    // 1.2 转为HSV
    Mat hsv;
    cvtColor(img, hsv, COLOR_BGR2HSV);
    imwrite(resultDir + "hsv_image.png", hsv);

    // 2.1 使用均值滤波
    Mat meanBlur;
    blur(img, meanBlur, Size(10, 10));
    imwrite(resultDir + "mean_blur.png", meanBlur);

    // 2.2 使用高斯滤波
    Mat gaussBlur;
    GaussianBlur(img, gaussBlur, Size(9, 9), 0, 0);
    imwrite(resultDir + "gauss_blur.png", gaussBlur);

    // 3.1 使用HSV提取红色区域并输出红色区域mask
    Mat redMask = extractRedHSV(img);
    imwrite(resultDir + "red_mask.png", redMask);

    // 3.2 寻找红色的外轮廓并在原图上绘制
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(redMask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    Mat contourImg = img.clone();
    for (size_t i = 0; i < contours.size(); i++) {
        drawContours(contourImg, contours, (int)i, Scalar(0, 255, 0), 2);
    }
    imwrite(resultDir + "red_contours.png", contourImg);

    // 3.3 计算每个轮廓的面积并输出
    for (size_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        cout << "Contour " << i + 1 << " area: " << area << endl;
    }

    // 3.4 寻找红色的bbox并绘制
    Mat boundingBoxImg = img.clone();
    for (size_t i = 0; i < contours.size(); i++) {
        Rect boundingBox = boundingRect(contours[i]);
        rectangle(boundingBoxImg, boundingBox, Scalar(0, 255, 0), 2);
    }
    imwrite(resultDir + "bounding_boxes.png", boundingBoxImg);

    // 3.5 提取高亮区域并进行图形学处理
    Mat HL_region;
    // 只保留红色区域的图像
    img.copyTo(HL_region, redMask);
    imwrite(resultDir + "HL.png", HL_region);

    Mat grayBright, binaryBright, dilated, eroded, flooded;
    cvtColor(HL_region, grayBright, COLOR_BGR2GRAY);
    imwrite(resultDir + "HL_gray.png", grayBright);
    threshold(grayBright, binaryBright, 20, 255, THRESH_BINARY);
    imwrite(resultDir + "HL_binary.png", binaryBright);

    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
    dilate(binaryBright, dilated, element, Point(-1, -1), 2);
    imwrite(resultDir + "HL_dilated.png", dilated);

    erode(dilated, eroded, element, Point(-1, -1), 2);
    imwrite(resultDir + "HL_eroded.png", eroded);

    // 漫水处理
    flooded = eroded.clone();
	Rect ccomp;
    floodFill(flooded, Point(730, 350), Scalar(255), &ccomp, Scalar(0, 0, 0), Scalar(0, 0, 0));
    imwrite(resultDir + "HL_flood_filled.png", flooded);

    // 4.1 绘制圆形、方形和文字
    Mat shapeImg = img.clone();
    circle(shapeImg, Point(100, 100), 80, Scalar(0, 0, 0), 3);
    rectangle(shapeImg, Point(500, 500), Point(800, 800), Scalar(255, 0, 0), 3);
    putText(shapeImg, "OpenCV Project", Point(400, 1200), FONT_HERSHEY_SIMPLEX, 4, Scalar(0, 255, 255), 8);
    imwrite(resultDir + "shapes_text.png", shapeImg);

    // 5.1 将图像旋转35度
    Point2f center((img.cols - 1) / 2.0, (img.rows - 1) / 2.0);
    Mat rotMat = getRotationMatrix2D(center, 35, 1.0);
    Mat rotated;
    warpAffine(img, rotated, rotMat, img.size());
    imwrite(resultDir + "rotated.png", rotated);

    // 5.2 图像裁减为左上角1/4
    Rect cropRegion(0, 0, img.cols / 2, img.rows / 2);
    Mat cropped = img(cropRegion);
    imwrite(resultDir + "cropped.png", cropped);

    return 0;
}
