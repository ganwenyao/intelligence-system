#include "group37.h"

void userIn(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    // Object 类型的声明
    Local<Object> objOut = v8::Object::New(isolate);
    Local<Boolean> flag_local = Boolean::New(isolate, false);
    objOut->Set(String::NewFromUtf8(isolate, "flag"), flag_local);
    
    // Light Sensor 500 - 28000
    mraa::I2c i2c(0);
    i2c.address(0x49); // A4-A7
    i2c.writeWordReg(1, 0x83C1); // A0,A4
    int16_t raw = i2c.readWordReg(0);
    int16_t value = ((raw & 0xff00) >> 8) + ((raw & 0x00ff) << 8);

    // Led
    mraa::Pwm pwm(32); 
    pwm.enable(true);

    // Touch Sensor 
    mraa::Gpio touchGpio(25);
    touchGpio.dir(mraa::DIR_IN);
    int16_t touchResponse = touchGpio.read();

    // PIR Sensor
    mraa::Gpio PIRgpio(27);
    PIRgpio.dir(mraa::DIR_IN);
    int16_t PIRResponse = PIRgpio.read();

    // QR code 
    cv::Mat QRcode = cv::imread("/home/group37/project/binding/pic/scanQRcodeText.png");
    cv::Mat text_initial = cv::Mat::zeros(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);

    int cnt = 0;

    cv::namedWindow("userIn", cv::WINDOW_NORMAL);
    cv::resizeWindow("userIn", WINDOW_WIDTH, WINDOW_HEIGHT);    

    while (true) {
        ++cnt;
        if (cnt % 10 == 0) {
            i2c.writeWordReg(1, 0x83C1); // A0,A4
            raw = i2c.readWordReg(0);
            value = ((raw & 0xff00) >> 8) + ((raw & 0x00ff) << 8);
            // printf("value : %d\n", value);
            float pwm_value = 1 - float(value) / 28000;
            pwm_value = pwm_value > 0.0f ? pwm_value : 0.0f;
            pwm_value = pwm_value < 1.0f ? pwm_value : 0.1f;
            pwm.write(pwm_value);
        }
        if (cnt % 200 == 0) {
            text_initial = cv::Mat::zeros(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);
        }
        touchResponse = touchGpio.read();
        if (touchResponse) {
            cnt = 0;
            cv::imshow("userIn", QRcode);
            cv::waitKey(10000);
            text_initial = cv::Mat::zeros(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);
        }
        PIRResponse = PIRgpio.read();
        if (PIRResponse) {
            cv::putText(text_initial, "Touch The Sensor to Get QR code!", cv::Point(100, 500),
            cv::FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(255, 255, 255), 2);
        }
        cv::imshow("userIn", text_initial);
        cv::waitKey(30);
    }
    args.GetReturnValue().Set(objOut);
}

void userFaceIn(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    // Get Name
    std::string name(*v8::String::Utf8Value(args[0]->ToString()));
    
    // Object 类型的声明
    Local<Object> obj = v8::Object::New(isolate);
    
    char key = '0';
    float faceID[2048] = {0};
    int cnt = 0;
    int findTimes = 0;
    cv::namedWindow("faceID", cv::WINDOW_NORMAL);
    cv::resizeWindow("faceID", WINDOW_WIDTH, WINDOW_HEIGHT);    

    // Digital Buzzer
    mraa::Gpio gpio(23);
    gpio.dir(mraa::DIR_OUT);

    // Initialize face detection model
    seeta::FaceDetection detector("/home/group37/project/model/seeta_fd_frontal_v1.0.bin");
    detector.SetMinFaceSize(40);
    detector.SetScoreThresh(2.f);
    detector.SetImagePyramidScaleFactor(0.8f);
    detector.SetWindowStep(4, 4);

    // Initialize face alignment model
    seeta::FaceAlignment point_detector("/home/group37/project/model/seeta_fa_v1.1.bin");

    // Initialize face Identification model
    seeta::FaceIdentification face_recognizer("/home/group37/project/model/seeta_fr_v1.0.bin");

    // Initialize Video
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
        if (cnt == TRAIN_COUNTS) {
            // Array 类型的声明
            Local<Array> arr_false = Array::New(isolate);
            // Boolean 类型的声明
            Local<Boolean> flag_false = Boolean::New(isolate, false);
            // Array Bool 赋值
            for (size_t i = 0; i < 2048; ++i) {
                arr_false->Set(i, Number::New(isolate, faceID[i]));
            }
            obj->Set(String::NewFromUtf8(isolate, "faceID"), arr_false);
            obj->Set(String::NewFromUtf8(isolate, "flag"), flag_false);
            cv::imwrite("/home/group37/project/server/error.jpg", imageCopy);
            cv::putText(imageCopy, "Time Out!", cv::Point(100, 100),
                        cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 2);
            cv::imshow("faceID", imageCopy);
            gpio.write(1);
            cv::waitKey(WAIT_TIME);
            gpio.write(0);
            cv::destroyAllWindows();
            break;
        }
        if (cnt % 10 == 0) {
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
                for (size_t i = 0; i<5; i++) {
                  cv::circle(imageCopy, cv::Point(gallery_points[i].x, gallery_points[i].y), 2, cv::Scalar(0, 255, 0));
                }
                face_rect.x = gallery_faces[i].bbox.x;
                face_rect.y = gallery_faces[i].bbox.y;
                face_rect.width = gallery_faces[i].bbox.width;
                face_rect.height = gallery_faces[i].bbox.height;
                cv::rectangle(imageCopy, face_rect, cv::Scalar(0, 0, 255), 4, 8, 0);
            }
            ++findTimes;
            if (findTimes == FIND_COUNTS) {
                // Array 类型的声明
                Local<Array> arr_true = Array::New(isolate);
                // Boolean 类型的声明
                Local<Boolean> flag_true = Boolean::New(isolate, true);
                // Array Bool 赋值
                for (size_t i = 0; i < 2048; ++i) {
                    arr_true->Set(i, Number::New(isolate, faceID[i]));
                }
                obj->Set(String::NewFromUtf8(isolate, "faceID"), arr_true);
                obj->Set(String::NewFromUtf8(isolate, "flag"), flag_true);
                cv::putText(imageCopy, "Registration Successful!", cv::Point(80, 50),
                            cv::FONT_HERSHEY_SIMPLEX, INFO_FONT, cv::Scalar(0, 0, 255), 2);
                cv::putText(imageCopy, name, cv::Point(face_rect.x, face_rect.y - 20),
                            cv::FONT_HERSHEY_SIMPLEX, NAME_FONT, cv::Scalar(255, 0, 0), 2);
                cv::imshow("faceID", imageCopy);
                cv::waitKey(WAIT_TIME);
                cv::destroyAllWindows();
                break;
            }
            cv::putText(imageCopy, std::to_string(findTimes), cv::Point(face_rect.x + face_rect.width / 2 - 20, face_rect.y - 10),
                        cv::FONT_HERSHEY_SIMPLEX, TIME_FONT, cv::Scalar(255, 255, 255), 2);
            // Extract face identity feature
            float gallery_fea[2048];
            face_recognizer.ExtractFeatureWithCrop(gallery_img_data_color, gallery_points, gallery_fea);
            for (int i =0; i < 2048; ++i) {
                faceID[i] += gallery_fea[i];
            }
            cv::imshow("faceID", imageCopy);
            key = (char)cv::waitKey(DETECT_TIME);
        }
        else {
            cv::imshow("faceID", imageCopy);
            key = (char)cv::waitKey(FRAME_TIME);
        }
    }
    args.GetReturnValue().Set(obj);
}

void userFaceCheck(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    // Get Name
    std::string name(*v8::String::Utf8Value(args[0]->ToString()));

    // Get FaceID
    float faceID[2048] = {0};
    Local<Object> obj = args[1]->ToObject();
    Local<Array> props = obj->GetPropertyNames();
    for (unsigned int i = 0; i < props->Length(); i++) {
        faceID[i] = obj->Get(i)->NumberValue();
    }
    // Object 类型的声明
    Local<Object> objOut = v8::Object::New(isolate);

    char key = '0';
    int cnt = 0;
    bool flag = false;
    int checkTimes = 0;
    int fialTimes = 0;
    cv::namedWindow("faceID", cv::WINDOW_NORMAL);
    cv::resizeWindow("faceID", WINDOW_WIDTH, WINDOW_HEIGHT);    

    // Digital Buzzer
    mraa::Gpio gpio(23);
    gpio.dir(mraa::DIR_OUT);

    // Initialize face detection model
    seeta::FaceDetection detector("/home/group37/project/model/seeta_fd_frontal_v1.0.bin");
    detector.SetMinFaceSize(40);
    detector.SetScoreThresh(2.f);
    detector.SetImagePyramidScaleFactor(0.8f);
    detector.SetWindowStep(4, 4);

    // Initialize face alignment model
    seeta::FaceAlignment point_detector("/home/group37/project/model/seeta_fa_v1.1.bin");

    // Initialize face Identification model
    seeta::FaceIdentification face_recognizer("/home/group37/project/model/seeta_fr_v1.0.bin");

    // Initialize Video
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
//        std::cout << cnt << std::endl;
        if (cnt == VERTIFY_COUNTS || fialTimes == CHECK_COUNTS) {
            // Boolean 类型的声明
            Local<Boolean> flag_local = Boolean::New(isolate, false);
            objOut->Set(String::NewFromUtf8(isolate, "flag"), flag_local);
            cv::imwrite("/home/group37/project/server/error.jpg", imageCopy);
            if (cnt == VERTIFY_COUNTS) {
                cv::putText(imageCopy, "Time Out!", cv::Point(100, 100),
                            cv::FONT_HERSHEY_SIMPLEX, TIME_FONT, cv::Scalar(0, 0, 255), 2);
            } else {
                cv::putText(imageCopy, "Wrong Face!", cv::Point(100, 100),
                            cv::FONT_HERSHEY_SIMPLEX, TIME_FONT, cv::Scalar(0, 0, 255), 2);
            }
            cv::imshow("faceID", imageCopy);
            gpio.write(1);
            cv::waitKey(WAIT_TIME);
            gpio.write(0);
            cv::destroyAllWindows();
            break;
        }
        if (cnt % 10 == 0) {
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

                for (size_t i = 0; i < 5; i++) {
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
            float sim = face_recognizer.CalcSimilarity(faceID, probe_fea);
            std::cout << "confidence: " << sim << std::endl;
            flag = (sim > CONFIDENCE) ? true : false;
            std::cout << "flag: " << flag << std::endl;
            if (flag) {
                if (checkTimes == CHECK_COUNTS) {
                    // Boolean 类型的声明
                    Local<Boolean> flag_local = Boolean::New(isolate, true);
                    objOut->Set(String::NewFromUtf8(isolate, "flag"), flag_local);
                    cv::putText(imageCopy, "Certification Successful!", cv::Point(80, 50),
                                cv::FONT_HERSHEY_SIMPLEX, INFO_FONT, cv::Scalar(0, 0, 255), 2);
                    cv::putText(imageCopy, name, cv::Point(face_rect.x, face_rect.y - 10),
                                cv::FONT_HERSHEY_SIMPLEX, NAME_FONT, cv::Scalar(255, 0, 0), 2);
                    cv::imshow("faceID", imageCopy);
                    cv::waitKey(WAIT_TIME);
                    cv::destroyAllWindows();
                    break;
                }
                flag = false;
                ++checkTimes;
                fialTimes = 0;
            } else {
                checkTimes = 0;
                ++fialTimes;
            }
            std::cout << "checkTimes: " << checkTimes << std::endl;
            cv::imshow("faceID", imageCopy);
            key = (char)cv::waitKey(DETECT_TIME);
        }
        else {
            cv::imshow("faceID", imageCopy);
            key = (char)cv::waitKey(FRAME_TIME);
        }
    }
    args.GetReturnValue().Set(objOut);
}

void markerDetect_linux(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    // Button
    mraa::I2c i2c(0);
    i2c.address(0x48);
  
    // Get code markerID 
    std::string code(*v8::String::Utf8Value(args[0]->ToString()));
    int markerID = args[1]->NumberValue();
    
    // Object 类型的声明
    Local<Object> objOut = v8::Object::New(isolate);

    char key = '0';
    cv::namedWindow("marker", cv::WINDOW_NORMAL);
    cv::resizeWindow("marker", WINDOW_WIDTH, WINDOW_HEIGHT);

    // Digital Buzzer
    mraa::Gpio gpio(23);
    gpio.dir(mraa::DIR_OUT);

    // Initialize Character image
    cv::Mat img_char = cv::Mat::zeros(ROWS, COLS, CV_8UC3);
    std::string vocabulary = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    cv::Ptr<cv::text::OCRHMMDecoder::ClassifierCallback> ocr = cv::text::loadOCRHMMClassifierCNN("/home/group37/project/model/OCRBeamSearch_CNN_model_data.xml.gz");
    std::vector<int> out_classes;
    std::vector<double> out_confidences;

    // Initialize Image Remap
    cv::Mat map_x( img_char.size(), CV_32FC1 );
    cv::Mat map_y( img_char.size(), CV_32FC1 );
    for( int j = 0; j < ROWS; j++ ){
        for( int i = 0; i < COLS; i++ ){
            map_x.at<float>(j,i) = (float)(COLS - i);
            map_y.at<float>(j,i) = (float)j;
        }
    }

    // Initialize User Code
    std::string user_code = "";
    std::string user_charactor = "";

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
            Local<Boolean> flag_local = Boolean::New(isolate, false);
            objOut->Set(String::NewFromUtf8(isolate, "flag"), flag_local);
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
        cv::rectangle(image_result, cv::Rect(180, 100, 340, 340), cv::Scalar(255, 0, 0), 4, 8, 0);
        // cv::putText(image_result, "Press p to confirm the code!", cv::Point(100, 100),
        //             cv::FONT_HERSHEY_SIMPLEX, MARKER_FONT, cv::Scalar(0, 0, 255), 2); 
        // cv::putText(image_result, "Press t to get the charactor!", cv::Point(100, 200),
        //             cv::FONT_HERSHEY_SIMPLEX, MARKER_FONT, cv::Scalar(0, 0, 255), 2); 
        cv::putText(image_result, "You Have Written: " + user_code, cv::Point(80, 50),
                    cv::FONT_HERSHEY_SIMPLEX, MARKER_FONT, cv::Scalar(0, 0, 255), 2);  
        cv::imshow("marker", image_result);
        key = cv::waitKey(30);
        // cv::imshow("Intelligence Systems Desion: img_char Detection", img_char_remap);
        // if (key == 't') {
        i2c.writeWordReg(1, 0x83C1);
        int16_t raw = i2c.readWordReg(0);
        int16_t key_value = ((raw & 0xff00) >> 8) + ((raw & 0x00ff) << 8);
        if (17240 < key_value && key_value < 19530) { // blue
            // cv::imshow("Intelligence Systems Desion: img_char Detection", img_char_remap);
            cv::Mat dst = img_char_remap(cv::Rect(180, 100, 340, 340));
            // cv::resize(img_char_remap, dst, cv::Size(64,64), 0, 0, cv::INTER_LINEAR);
            // cv::imshow("dst", dst);
            ocr->eval(dst, out_classes, out_confidences);
            img_char = cv::Mat::zeros(ROWS, COLS, CV_8UC3);
            std::string firstletter(1, vocabulary[out_classes[0]]);
            user_charactor = firstletter;
            cv::putText(image_result, "The charactor is: " + user_charactor, cv::Point(80, 80),
                        cv::FONT_HERSHEY_SIMPLEX, MARKER_FONT, cv::Scalar(0, 0, 255), 2); 
            std::cout << "OCR Output = " << vocabulary[out_classes[0]] << " with confidence "
                      << out_confidences[0] << std::endl;
            
            while (true) {
                cv::imshow("marker", image_result);
                i2c.writeWordReg(1, 0x83C1);
                raw = i2c.readWordReg(0);
                key_value = ((raw & 0xff00) >> 8) + ((raw & 0x00ff) << 8);

                // cv::putText(image_result, "Output: " + firstletter, cv::Point(100, 100),
                //             cv::FONT_HERSHEY_SIMPLEX, INFO_FONT, cv::Scalar(0, 0, 255), 2);

                // char key_code = (char)cv::waitKey();          
                // std::cout << "Enter key 'y' to get the character" << std::endl;
                // if (key_code == 'y'){
                if (12000 < key_value && key_value < 15200) { // yellow
                    user_code += vocabulary[out_classes[0]];
                    std::cout << "output = " << user_code << std::endl;
                    break;
                }
                if (19530 < key_value && key_value < 22600) { // red
                    break;
                }
                cv::waitKey(300);
            }
        }
        if (cnt == MARKER_COUNTS) {
            Local<Boolean> flag_local = Boolean::New(isolate, false);
            objOut->Set(String::NewFromUtf8(isolate, "flag"), flag_local);
            cv::imwrite("/home/group37/project/server/error.jpg", imageCopy);
            cv::putText(image_result, "Time Out!", cv::Point(100, 100),
                        cv::FONT_HERSHEY_SIMPLEX, TIME_FONT, cv::Scalar(0, 0, 255), 2);
            cv::imshow("marker", image_result);
            gpio.write(1);
            cv::waitKey(WAIT_TIME);
            gpio.write(0);
            cv::destroyAllWindows();
            break;
        }
        // if (key == 'p') {
        if (cnt > 30 && 22600 < key_value && key_value < 25600) { // white
            if (!user_code.compare(code)) {
                Local<Boolean> flag_local = Boolean::New(isolate, true);
                objOut->Set(String::NewFromUtf8(isolate, "flag"), flag_local);
                cv::putText(image_result, "Certification Successful!", cv::Point(80, 200),
                            cv::FONT_HERSHEY_SIMPLEX, INFO_FONT, cv::Scalar(0, 0, 255), 2);
                cv::imshow("marker", image_result);
                cv::waitKey(WAIT_TIME);
                cv::destroyAllWindows();
                break;
            } else {
                Local<Boolean> flag_local = Boolean::New(isolate, false);
                objOut->Set(String::NewFromUtf8(isolate, "flag"), flag_local);
                cv::imwrite("/home/group37/project/server/error.jpg", imageCopy);
                cv::putText(image_result, "Wrong Code!", cv::Point(80, 80),
                            cv::FONT_HERSHEY_SIMPLEX, INFO_FONT, cv::Scalar(0, 0, 255), 2);
                cv::imshow("marker", image_result);
                gpio.write(1);
                cv::waitKey(WAIT_TIME);
                gpio.write(0);
                cv::destroyAllWindows();
                break;
            }
        }
    } 
    args.GetReturnValue().Set(objOut);
}

void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "userIn", userIn);
    NODE_SET_METHOD(exports, "userFaceIn", userFaceIn);
    NODE_SET_METHOD(exports, "userFaceCheck", userFaceCheck);
    NODE_SET_METHOD(exports, "markerDetect_linux", markerDetect_linux);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
    
