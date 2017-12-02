#include<iostream>
using namespace std;

#if defined(__unix__) || defined(__APPLE__)

#ifndef fopen_s

#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),(mode)))==NULL

#endif //fopen_s

#endif //__unix

#include "seetaFace/face_identification.h"
#include "seetaFace/recognizer.h"
#include "seetaFace/face_detection.h"
#include "seetaFace/face_alignment.h"
#include "seetaFace/math_functions.h"

#include "opencv2/opencv.hpp"
#include "opencv2/text.hpp"
#include "opencv2/aruco.hpp"

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

#define TEST(major, minor) major##_##minor##_Tester()
#define EXPECT_NE(a, b) if ((a) == (b)) std::cout << "ERROR: "
#define EXPECT_EQ(a, b) if ((a) != (b)) std::cout << "ERROR: "
float ganwenyao[2048] = {0};
const int train_counts = 20;

int main(int argc, char* argv[]) {
    cout << "Enter key 't' to test character" << endl;
    // Video Size
    int rows = 480;
    int cols = 640;
    char key;
    int flag = 0;

    // Initialize face detection model
    seeta::FaceDetection detector("/home/ganwenyao/group37/project/model/seeta_fd_frontal_v1.0.bin");
    detector.SetMinFaceSize(40);
    detector.SetScoreThresh(2.f);
    detector.SetImagePyramidScaleFactor(0.8f);
    detector.SetWindowStep(4, 4);

    // Initialize face alignment model
    seeta::FaceAlignment point_detector("/home/ganwenyao/group37/project/model/seeta_fa_v1.1.bin");

    // Initialize face Identification model
    seeta::FaceIdentification face_recognizer("/home/ganwenyao/group37/project/model/seeta_fr_v1.0.bin");
    // cout << "Enter key 't' to test character" << endl;
    // Initialize Character image
    cv::Mat img_char = cv::Mat::zeros(rows, cols, CV_8UC3);
    //string vocabulary = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    string vocabulary = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    cv::Ptr<cv::text::OCRHMMDecoder::ClassifierCallback> ocr = cv::text::loadOCRHMMClassifierCNN("/home/ganwenyao/group37/project/model/OCRBeamSearch_CNN_model_data.xml.gz");
    vector<int> out_classes;
    vector<double> out_confidences;

    // Initialize Image Remap
    cv::Mat map_x( img_char.size(), CV_32FC1 );
    cv::Mat map_y( img_char.size(), CV_32FC1 );
    for( int j = 0; j < rows; j++ ){
        for( int i = 0; i < cols; i++ ){
            map_x.at<float>(j,i) = (float)(cols - i);
            map_y.at<float>(j,i) = (float)j;
        }
    }

    // Initialize User Code
    vector<string> user_code;
    user_code.push_back("");

    // Initialize Camera Calibration Parameters
    double m[9] = {9.6635571716090658e+02, 0., 2.0679307818305685e+02, 0.,
                   9.6635571716090658e+02, 2.9370020600555273e+02, 0., 0., 1.};
    double d[5] = {-1.5007354215536557e-03, 9.8722389825801837e-01,
            1.7188452542408809e-02, -2.6805958820424611e-02,
            -2.3313928379240205e+00};
    cv::Mat cameraMatrix(3, 3, CV_64F, m), distCoeffs(5, 1, CV_64F, d);
//    cout << cameraMatrix;

    // Initialize Aruco
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);

    // Initialize Video
    int cnt = 0;
    cv::VideoCapture inputVideo;
    inputVideo.open(0);

    while (inputVideo.grab()) {
        cv::Mat image, imageCopy;
        inputVideo.retrieve(image);
        image.copyTo(imageCopy);
        cout << image.rows << " " << image.cols << endl;
	if (!user_code[0].compare("GWY")) {
            cout << "Successful!";
        }
        if (flag == 0) {
            //load image
            cv::Mat gallery_img_gray;
            cv::cvtColor(imageCopy, gallery_img_gray, CV_BGR2GRAY);
            seeta::ImageData gallery_img_data_color(imageCopy.cols, imageCopy.rows, imageCopy.channels());
            gallery_img_data_color.data = imageCopy.data;
            seeta::ImageData gallery_img_data_gray(gallery_img_gray.cols, gallery_img_gray.rows, gallery_img_gray.channels());
            gallery_img_data_gray.data = gallery_img_gray.data;

            // Detect faces
            std::vector<seeta::FaceInfo> gallery_faces = detector.Detect(gallery_img_data_gray);
            int32_t gallery_face_num = static_cast<int32_t>(gallery_faces.size());
            if (gallery_face_num == 0) {
                continue;
            }

            cv::Rect face_rect;
            seeta::FacialLandmark gallery_points[5];
            for (int32_t i = 0; i < gallery_face_num; i++) {
                // Detect 5 facial landmarks
                point_detector.PointDetectLandmarks(gallery_img_data_gray, gallery_faces[i], gallery_points);
                for (int i = 0; i<5; i++) {
                  cv::circle(imageCopy, cv::Point(gallery_points[i].x, gallery_points[i].y), 2, cv::Scalar(0, 255, 0));
                }
                face_rect.x = gallery_faces[i].bbox.x;
                face_rect.y = gallery_faces[i].bbox.y;
                face_rect.width = gallery_faces[i].bbox.width;
                face_rect.height = gallery_faces[i].bbox.height;
                cv::rectangle(imageCopy, face_rect, cv::Scalar(0, 0, 255), 4, 8, 0);
            }

            // Extract face identity feature
            float gallery_fea[2048];
            face_recognizer.ExtractFeatureWithCrop(gallery_img_data_color, gallery_points, gallery_fea);

            for (int i =0; i < 2048; ++i) {
                ganwenyao[i] += gallery_fea[i];
            }
            ++cnt;
            if (cnt == train_counts) {
                flag = 1;
                cnt = 0;
            }
            cv::imshow("gallery_point_result", imageCopy);
        }
        else if (flag == 1) {
            cv::Mat probe_img_gray;
            cv::cvtColor(imageCopy, probe_img_gray, CV_BGR2GRAY);
            seeta::ImageData probe_img_data_color(imageCopy.cols, imageCopy.rows, imageCopy.channels());
            probe_img_data_color.data = imageCopy.data;

            seeta::ImageData probe_img_data_gray(probe_img_gray.cols, probe_img_gray.rows, probe_img_gray.channels());
            probe_img_data_gray.data = probe_img_gray.data;
            std::vector<seeta::FaceInfo> probe_faces = detector.Detect(probe_img_data_gray);
            int32_t probe_face_num = static_cast<int32_t>(probe_faces.size());

            if (probe_face_num == 0){
                continue;
            }

            cv::Rect face_rect;
            seeta::FacialLandmark probe_points[5];
            for (int32_t i = 0; i < probe_face_num; i++) {
                point_detector.PointDetectLandmarks(probe_img_data_gray, probe_faces[0], probe_points);

                for (int i = 0; i<5; i++) {
                    cv::circle(imageCopy, cv::Point(probe_points[i].x, probe_points[i].y), 2, cv::Scalar(0, 255, 0));
                }
                face_rect.x = probe_faces[i].bbox.x;
                face_rect.y = probe_faces[i].bbox.y;
                face_rect.width = probe_faces[i].bbox.width;
                face_rect.height = probe_faces[i].bbox.height;
                cv::rectangle(imageCopy, face_rect, cv::Scalar(0, 0, 255), 4, 8, 0);
            }
            
            float probe_fea[2048];
            face_recognizer.ExtractFeatureWithCrop(probe_img_data_color, probe_points, probe_fea);

            //Caculate similarity of two faces
            float sim = face_recognizer.CalcSimilarity(ganwenyao, probe_fea);
            //std::cout << sim <<endl;

            if (sim > 0.7) {
                ++cnt;
                cv::putText(imageCopy, "ganwenyao", cv::Point(face_rect.x, face_rect.y),
                            cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(255, 0, 0), 2);
            }
            if (cnt == train_counts) {
                flag = 2;
                cnt = 0;
            }
            cv::imshow("gallery_point_result", imageCopy);
        }
        else if (flag == 2) {
            std::vector<int> ids;
            std::vector<std::vector<cv::Point2f>> corners;
            cv::aruco::detectMarkers(image, dictionary, corners, ids);
            // if at least one marker detected
            if (ids.size() > 0) {
                cv::aruco::drawDetectedMarkers(imageCopy, corners, ids);
                std::vector<cv::Vec3d> rvecs, tvecs;
                cv::aruco::estimatePoseSingleMarkers(corners, 0.1, cameraMatrix, distCoeffs, rvecs, tvecs);
                // draw axis for each marker
                for(int i=0; i<ids.size(); i++) {
                    cv::aruco::drawAxis(imageCopy, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 0.1);

                    cv::Point center;
                    center.x = cv::saturate_cast<int>((corners[i][0].x + corners[i][1].x +
                            corners[i][2].x + corners[i][3].x) / 4);
                    center.y = cv::saturate_cast<int>((corners[i][0].y + corners[i][1].y +
                            corners[i][2].y + corners[i][3].y) / 4);
                    if (tvecs[i][2] < 2.5) {
                        circle(img_char, center, 10, cv::Scalar(255, 255, 255, 255), -1);
                    // cout << tvecs[i][2] << endl;
                    }
                }
            }
            cv::Mat image_remap, img_char_remap, image_result;
            cv::remap(imageCopy, image_remap, map_x, map_y, cv::INTER_LINEAR);
            cv::remap(img_char, img_char_remap, map_x, map_y, cv::INTER_LINEAR);
            cv::bitwise_or(image_remap, img_char_remap, image_result);
            cv::imshow("Intelligence Systems Desion: Marker Detection", image_result);
            // cv::imshow("Intelligence Systems Desion: img_char Detection", img_char_remap);
            if (key == 't') {
                cv::imshow("Intelligence Systems Desion: img_char Detection", img_char_remap);
                cv::Mat dst;
                cv::resize(img_char_remap, dst, cv::Size(64,64), 0, 0, cv::INTER_LINEAR);
                cv::imshow("dst", dst);
                ocr->eval(dst, out_classes, out_confidences);
                img_char = cv::Mat::zeros(480, 640, CV_8UC3);
                cout << "OCR Output = " << vocabulary[out_classes[0]] << " with confidence "
                    << out_confidences[0] << endl;
                char key_code = (char)cv::waitKey();
                cout << "Enter key 'y' to get the character" << endl;
                if (key_code == 'y'){
                    user_code[0] += vocabulary[out_classes[0]];
                    cout << "output = " << user_code[0] << endl;
                }
            }
        }
        
        key = (char) cv::waitKey(33);
        if (key == 27)
            break;

    }
    cv::waitKey();
    return 0;

}
