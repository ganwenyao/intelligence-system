#include "group37.h"

void markerDetect_linux(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
  
    // Get code markerID 
    std::string code(*v8::String::Utf8Value(args[0]->ToString()));
    int markerID = args[1]->NumberValue();
    
    // Object 类型的声明
    Local<Object> objOut = v8::Object::New(isolate);

    // Video Size
    int rows = 480;
    int cols = 640;
    char key = '0';
    const int WAIT_TIME = 3000;
    const int MARKER_COUNTS = 30000;
    const float MAX_DIST = 2.5;
    cv::namedWindow("marker", cv::WINDOW_NORMAL);

    // Initialize Character image
    cv::Mat img_char = cv::Mat::zeros(rows, cols, CV_8UC3);
    std::string vocabulary = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    cv::Ptr<cv::text::OCRHMMDecoder::ClassifierCallback> ocr = cv::text::loadOCRHMMClassifierCNN("/home/ganwenyao/group37/project/model/OCRBeamSearch_CNN_model_data.xml.gz");
    std::vector<int> out_classes;
    std::vector<double> out_confidences;

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
    std::string user_code;

    // Initialize Camera Calibration Parameters
    double m[9] = {9.6635571716090658e+02, 0., 2.0679307818305685e+02, 0.,
                   9.6635571716090658e+02, 2.9370020600555273e+02, 0., 0., 1.};
    double d[5] = {-1.5007354215536557e-03, 9.8722389825801837e-01,
            1.7188452542408809e-02, -2.6805958820424611e-02,
            -2.3313928379240205e+00};
    cv::Mat cameraMatrix(3, 3, CV_64F, m), distCoeffs(5, 1, CV_64F, d);
    // cout << cameraMatrix;

    // Initialize Aruco
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);

    // Initialize Video
    int cnt = 0;
    cv::VideoCapture inputVideo;
    inputVideo.open(0);

    while (inputVideo.grab()) {
        ++cnt;
        if (key == 27) {
            cv::destroyAllWindows();
            break;
        }
        cv::Mat image, imageCopy;
        inputVideo.retrieve(image);
        image.copyTo(imageCopy);
        // cout << image.rows << " " << image.cols << endl;
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);
        // if at least one marker detected
        if (ids.size() > 0) {
            cv::aruco::drawDetectedMarkers(imageCopy, corners, ids);
            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(corners, 0.1, cameraMatrix, distCoeffs, rvecs, tvecs);
            // draw axis for each marker
            for(size_t i = 0; i < ids.size(); i++) {
                cv::aruco::drawAxis(imageCopy, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 0.1);

                cv::Point center;
                center.x = cv::saturate_cast<int>((corners[i][0].x + corners[i][1].x +
                        corners[i][2].x + corners[i][3].x) / 4);
                center.y = cv::saturate_cast<int>((corners[i][0].y + corners[i][1].y +
                        corners[i][2].y + corners[i][3].y) / 4);
                if (ids[0] == markerID && tvecs[i][2] < MAX_DIST) {
                    circle(img_char, center, 10, cv::Scalar(255, 255, 255, 255), -1);
                }
            }

        }
        cv::Mat image_remap, img_char_remap, image_result;
        cv::remap(imageCopy, image_remap, map_x, map_y, cv::INTER_LINEAR);
        cv::remap(img_char, img_char_remap, map_x, map_y, cv::INTER_LINEAR);
        cv::bitwise_or(image_remap, img_char_remap, image_result);
        cv::imshow("marker", image_result);
        cv::resizeWindow("marker", 2000, 1000);
        key = cv::waitKey(30);
        // cv::imshow("Intelligence Systems Desion: img_char Detection", img_char_remap);
        if (key == 't') {
            cv::imshow("Intelligence Systems Desion: img_char Detection", img_char_remap);
            cv::Mat dst;
            cv::resize(img_char_remap, dst, cv::Size(64,64), 0, 0, cv::INTER_LINEAR);
            cv::imshow("dst", dst);
            ocr->eval(dst, out_classes, out_confidences);
            img_char = cv::Mat::zeros(480, 640, CV_8UC3);
            std::cout << "OCR Output = " << vocabulary[out_classes[0]] << " with confidence "
                << out_confidences[0] << std::endl;
            char key_code = (char)cv::waitKey();
            std::cout << "Enter key 'y' to get the character" << std::endl;
            if (key_code == 'y'){
                user_code += vocabulary[out_classes[0]];
                std::cout << "output = " << user_code << std::endl;
            }
        }
        if (cnt == MARKER_COUNTS) {
            Local<Boolean> flag_local = Boolean::New(isolate, false);
            objOut->Set(String::NewFromUtf8(isolate, "flag"), flag_local);
            cv::putText(image_result, "Time Out!", cv::Point(100, 100),
                        cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 2);
            cv::imshow("marker", image_result);
            cv::resizeWindow("marker", 2000, 1000);
            cv::waitKey(WAIT_TIME);
            cv::destroyAllWindows();
            break;
        }
        if (!user_code.compare(code)) {
            Local<Boolean> flag_local = Boolean::New(isolate, true);
            objOut->Set(String::NewFromUtf8(isolate, "flag"), flag_local);
            cv::putText(image_result, "Certification Successful!", cv::Point(100, 100),
                        cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 0, 255), 2);
            cv::imshow("marker", image_result);
            cv::resizeWindow("marker", 2000, 1000);
            cv::waitKey(WAIT_TIME);
            cv::destroyAllWindows();
            break;
        }
    } 
  
    args.GetReturnValue().Set(objOut);
}
  
void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "markerDetect_linux", markerDetect_linux);
}
  
NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
  