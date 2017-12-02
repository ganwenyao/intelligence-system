// hello.js
const addon = require('./build/Release/addon');
let arry = [0,1,2,3];
// console.log(addon);
// let ans1 = addon.userFaceIn("ganwenyao");
// console.log(ans1.faceID, ans1.flag);
// let ans2 = addon.userFaceCheck("ganwenyao", ans1.faceID);
// console.log(ans2.flag);
// let ans3 = addon.markerDetect_linux("A", 0);
// console.log(ans3.flag);
let ans4 = addon.userIn();
console.log(ans4.flag);

// hello.js
// const addon1 = require('./userFaceIn_linux/build/Release/addon');
// const addon2 = require('./userFaceCheck_linux/build/Release/addon');
// let arry = [0,1,2,3];
// // console.log(addon);
// let ans1 = addon1.userFaceIn("ganwenyao");
// console.log(ans1.faceID, ans1.flag);
// let ans2 = addon2.userFaceCheck("ganwenyao");
// console.log(ans2);