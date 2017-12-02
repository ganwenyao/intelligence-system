// sign in:
const User = require('../mysql');

const addon = require('../../binding/build/Release/addon');

let fs = require('fs');

let markerID;
let useCode;
let useEmail;

let nodemailer = require('nodemailer');
let transporter = nodemailer.createTransport({
    service: 'qq',
    port: 465, // SMTP 端口
    secureConnection: true, // 使用 SSL
    auth: {
        user: '',
        //设置的smtp密码
        pass: ''
    }
});

// setup email data with unicode symbols
let mailOptions = {
    from: '智能系统设计37组', // 发件地址
    to: '', // 收件列表
    subject: '账户登录失败提醒', // 标题
    //text和html两者只支持一种
    // text: '账户登录失败提醒', // 标题
    html: `<h1>有人使用您的账户登录失败!</h1>
           <h1>您的账户信息可能已经泄露!</h1>
           <h1>下图为相关信息!</h1>
           <p><img src="cid:00000001"/></p>`, // html 内容
    attachments: 
    [
        {
            filename: 'error.jpg',            // 改成你的附件名
            path: './error.jpg',  // 改成你的附件路径
            cid : '00000001'                 // cid可被邮件使用
        }
    ]
};

module.exports = {
    'POST /signin': async (ctx, next) => {
        let email = ctx.request.body.email || '';   
        let password = ctx.request.body.password || '';
        let users = await User.findAll({
            where: {
                email: email
            }
        });
        let faceData = fs.readFileSync('faceID.txt', 'utf-8');
        // faceLib Initial
        let faceLib = [];
        let faceTmp = [];
        faceData.trim().split('\n').forEach(element => {
            faceTmp.push(element.trim().split(' '));
        });
        faceLib = faceTmp.map(x => x.map(y => Number(y)));
        // console.log("users", users);
        if (users.length !== 0 && users[0].passwd == password) {
            useEmail = users[0].email;
            let ans2 = addon.userFaceCheck(users[0].email, faceLib[users[0].faceID]);
            if (ans2.flag) {
                markerID = Math.round(Math.random() * 250);
                useCode = users[0].code;
                console.log("markerID", markerID);
                ctx.render('marker.html', {
                    message: "人脸识别成功!(๑¯◡¯๑)",
                    title: 'Face In OK',
                    markerID: markerID
                });
            } else {
                // console.log('signin failed!');
                mailOptions.to = useEmail;
                console.log("mailOptions", useEmail);
                transporter.sendMail(mailOptions, function(error, info){
                    if(error){
                        return console.log(error);
                    }
                    console.log('Message sent: ' + info.response);
                });
                ctx.render('failed.html', {
                    failed : "登录失败!",
                    message: "人脸检测失败!(〜￣▽￣)〜",
                    title: 'Sign In Failed'
                });
            }
        } else {
            // console.log('signin failed!');
            ctx.render('failed.html', {
                failed : "登录失败!",
                message: "密码错误!(灬ºｏº灬)",
                title: 'Sign In Failed'
            });
        }
    },
    'POST /marker': async (ctx, next) => {
        // console.log("markerID/marker :", markerID);
        let ans3 = addon.markerDetect_linux(useCode, markerID);
        if (ans3.flag) {
            ctx.render('successful.html', {
                message: "账号登录成功!(๑¯◡¯๑)",
                title: 'Sign In OK',
            });
        } else {
            // console.log('signin failed!');
            mailOptions.to = useEmail;
            console.log("mailOptions", useEmail);
            transporter.sendMail(mailOptions, function(error, info){
                if(error){
                    return console.log(error);
                }
                console.log('Message sent: ' + info.response);
            });
            ctx.render('failed.html', {
                failed : "登录失败!",
                message: "字符识别失败!(－_－)",
                title: 'Sign In Failed'
            });
        }
    }
};

