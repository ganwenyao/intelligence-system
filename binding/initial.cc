#include <node.h>

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

// This is the implementation of the "add" method
// Input arguments are passed using the
// const FunctionCallbackInfo<Value>& args struct
void Add(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  // Perform the operation
  // double value = args[0]->NumberValue() + args[1]->NumberValue();
  // Local<Number> num = Number::New(isolate, value);
  
//   Local<Object> obj = args[1]->ToObject();
//   Local<Array> props = obj->GetPropertyNames();
//   for(unsigned int i = 0; i < props->Length(); i++) {
//     std::cout << obj->Get(i)->NumberValue() << std::endl;
//     obj->Get(i) = Number::New(isolate, 1000);
//     std::cout << obj->Get(i)->NumberValue() << std::endl;

// }

    // Object 类型的声明
    Local<Object> obj = v8::Object::New(isolate);
    // Boolean 类型的声明
    Local<Boolean> flag = Boolean::New(isolate, true);
    obj->Set(String::NewFromUtf8(isolate, "flag"), flag);

    // Array 类型的声明
    Local<Array> arr = Array::New(isolate);
    // Array 赋值
    for (size_t i = 0; i < 10; ++i) {
      arr->Set(i, Number::New(isolate, i));
    }
    obj->Set(String::NewFromUtf8(isolate, "faceID"), arr);




        // std::string param1(*v8::String::Utf8Value(args[0]->ToString()));
        // std::cout << param1 << std::endl;

  // const char name[] = "gqwean";
  // Local<String> str = String::NewFromUtf8(isolate, name);
  // Local<String> str = args[0]->ToString();


  // Set the return value (using the passed in
  // FunctionCallbackInfo<Value>&)
  args.GetReturnValue().Set(obj);
}

void Init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "add", Add);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
