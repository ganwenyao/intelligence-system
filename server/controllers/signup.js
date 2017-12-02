// sign up:

const User = require('../mysql');

const addon = require('../../binding/build/Release/addon');

let fs = require('fs');
let faceData = fs.readFileSync('faceID.txt', 'utf-8');

// faceLib Initial
let faceLib = [];
let faceTmp = [];
faceData.trim().split('\n').forEach(element => {
    faceTmp.push(element.trim().split(' '));
});
if (!faceTmp[0][0]) {
    faceLib = [];
} else {
    faceLib = faceTmp.map(x => x.map(y => Number(y)));
}
// fs.writeFileSync('faceID.txt', "1 2 3" + '\n' , {flag : 'a'});
// fs.writeFileSync('faceID.txt', "3 4 5" + '\n', {flag : 'a'});
// console.log("faceLib: ", faceLib.length);
module.exports = {
    'POST /signup': async (ctx, next) => {
        let email = ctx.request.body.email || '';   
        let password = ctx.request.body.password || '';
        let text = ctx.request.body.text || '';
        let users = await User.findAll({
            where: {
                email: email
            }
        });

        // console.log("users: ", users);
        // console.log(users.length);
        if (users.length === 0) {
            let ans1 = addon.userFaceIn(email);
            if (ans1.flag) {
                let person = await User.create({
                    email: email,
                    passwd: password,
                    code: text,
                    faceID: faceLib.length
                });
                // console.log('created: ' + JSON.stringify(person));
                // faceLib.push(ans1.faceID);
                fs.writeFileSync('faceID.txt', ans1.faceID.join(' ') + '\n', {flag : 'a'});
                // console.log("faceLib: ", faceLib);
                ctx.render('successful.html', {
                    message: "账号注册成功!୧(๑•̀⌄•́๑)૭✧",
                    title: 'Sign Up OK',
                });
            } else {
                // console.log('signup failed!');
                ctx.render('failed.html', {
                    failed : "注册失败!",
                    message: "脸部信息录入失败!(╯°Д°）╯",
                    title: 'Sign Up Failed'
                });
            }
        } else {
            // console.log('signup failed!');
            ctx.render('failed.html', {
                failed : "注册失败!",
                message: "该账号已有人使用!T△T",
                title: 'Sign Up Failed'
            });
        }
    }
};
