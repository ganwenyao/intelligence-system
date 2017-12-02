#include "group37.h"

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
            cv::putText(imageCopy, "Time Out!", cv::Point(100, 100),
                        cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 2);
            cv::imshow("faceID", imageCopy);
            cv::resizeWindow("faceID", WINDOW_WIDTH, WINDOW_HEIGHT);
            cv::waitKey(WAIT_TIME);
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
                for (size_t i = 0; i < 5; i++) {
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
                cv::putText(imageCopy, "Registration Successful!", cv::Point(100, 90),
                            cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 0, 255), 2);
                cv::putText(imageCopy, name, cv::Point(face_rect.x, face_rect.y - 20),
                            cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(255, 0, 0), 2);
                cv::imshow("faceID", imageCopy);
                cv::resizeWindow("faceID", WINDOW_WIDTH, WINDOW_HEIGHT);
                cv::waitKey(WAIT_TIME);
                cv::destroyAllWindows();
                break;
            }
            cv::putText(imageCopy, std::to_string(findTimes), cv::Point(face_rect.x + face_rect.width / 2 - 20, face_rect.y - 10),
                        cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255, 255, 255), 2);
            // Extract face identity feature
            float gallery_fea[2048];
            face_recognizer.ExtractFeatureWithCrop(gallery_img_data_color, gallery_points, gallery_fea);
            for (int i =0; i < 2048; ++i) {
                faceID[i] += gallery_fea[i];
            }
            cv::imshow("faceID", imageCopy);
            cv::resizeWindow("faceID", WINDOW_WIDTH, WINDOW_HEIGHT);
            key = (char)cv::waitKey(DETECT_TIME);
        }
        else {
            cv::imshow("faceID", imageCopy);
            cv::resizeWindow("faceID", WINDOW_WIDTH, WINDOW_HEIGHT);
            key = (char)cv::waitKey(FRAME_TIME);
        }
    }
    args.GetReturnValue().Set(obj);
}

void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "userFaceIn", userFaceIn);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
    
