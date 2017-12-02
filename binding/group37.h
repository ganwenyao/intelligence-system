#include <node.h>

#include <unistd.h>
#include <signal.h>

#include "mraa.hpp"
#include "math.h"

#include "seetaFace/face_identification.h"
#include "seetaFace/recognizer.h"
#include "seetaFace/face_detection.h"
#include "seetaFace/face_alignment.h"
#include "seetaFace/math_functions.h"

#include "opencv2/opencv.hpp"
#include "opencv2/text.hpp"
#include "opencv2/aruco.hpp"

using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Array;
using v8::Boolean;


int running = 0;

void
sig_handler(int signo)
{
    if (signo == SIGINT) {
        printf("closing nicely\n");
        running = -1;
    }
}

// Share Const
const int ROWS = 480;
const int COLS = 640;
const int WAIT_TIME = 2000;
const int FRAME_TIME = 30;
const int DETECT_TIME = 300;
const int WINDOW_WIDTH = 2000;
const int WINDOW_HEIGHT = 1000;
const float NAME_FONT = 1;
const float INFO_FONT = 1;
const float TIME_FONT = 2;
const float MARKER_FONT = 1;


// userFaceIn
const int TRAIN_COUNTS = 300;
const int FIND_COUNTS = 5;

// userFaceCheck
const int VERTIFY_COUNTS = 300;
const int CHECK_COUNTS = 3;
const float CONFIDENCE = 0.7;

// markerDetect_linux
const int MARKER_COUNTS = 30000;
const float MAX_DIST = 2.5;

