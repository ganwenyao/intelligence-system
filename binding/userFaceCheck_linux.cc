#include "group37.h"

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
//        std::cout << cnt << std::endl;
        if (cnt == VERTIFY_COUNTS) {
            // Boolean 类型的声明
            Local<Boolean> flag_local = Boolean::New(isolate, false);
            objOut->Set(String::NewFromUtf8(isolate, "flag"), flag_local);
            cv::putText(imageCopy, "Time Out!", cv::Point(100, 100),
                        cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 2);
            cv::imshow("faceID", imageCopy);
            cv::resizeWindow("faceID", WINDOW_WIDTH, WINDOW_HEIGHT);
            cv::waitKey(WAIT_TIME);
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
            //std::cout << sim <<endl;
            flag = sim > 0.7 ? true : false;
            if (flag) {
                if (checkTimes == CHECK_COUNTS) {
                    Local<Boolean> flag_local = Boolean::New(isolate, true);
                    objOut->Set(String::NewFromUtf8(isolate, "flag"), flag_local);
                    cv::putText(imageCopy, "Certification Successful!", cv::Point(100, 100),
                                cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 0, 255), 2);
                    cv::putText(imageCopy, name, cv::Point(face_rect.x, face_rect.y - 10),
                                cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(255, 0, 0), 2);
                    cv::imshow("faceID", imageCopy);
                    cv::resizeWindow("faceID", WINDOW_WIDTH, WINDOW_HEIGHT);
                    cv::waitKey(WAIT_TIME);
                    cv::destroyAllWindows();
                    break;
                }
                flag = false;
                ++checkTimes;
            } else {
                checkTimes = 0;
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
    args.GetReturnValue().Set(objOut);
}

void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "userFaceCheck", userFaceCheck);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
  