const Sequelize = require('sequelize');
const Op = Sequelize.Op;
const config = require('./config');

console.log('init sequelize...');

var sequelize = new Sequelize(config.database, config.username, config.password, {
    host: config.host,
    dialect: 'mysql',
    pool: {
        max: 5,
        min: 0,
        idle: 30000
    }
});

var User = sequelize.define('users', {
    email: {
        type: Sequelize.STRING(100),
        primaryKey: true
    },
    passwd: Sequelize.STRING(100),
    code: Sequelize.STRING(100),
    faceID: Sequelize.BIGINT,
}, {
        timestamps: false
    });

module.exports = User;


// var now = Date.now();

// async () => {
//     var person = await User.create({
//         email: '807105461@qq.com',
//         passwd: 'asd',
//         createdAt: now,
//         updatedAt: now,
//         version: 0
//     });
//     console.log('created: ' + JSON.stringify(person));
// }

// async () => {
//     var users = await User.findAll({
//         where: {
//             email: '807105461@qq.com'
//         }
//     });
//     console.log(`find ${users.length} users:`);
//     for (let p of users) {
//         console.log(JSON.stringify(p));
//         console.log('update...');
//         p.updatedAt = Date.now();
//         p.version ++;
//         await p.save();
//         if (p.version === 3) {
//             await p.destroy();
//             console.log(`${p.name} was destroyed.`);
//         }
//     }
// }

// sign in:

// let fs = require('fs');

// let markerID;
// let useCode;
// let useEmail;

// let nodemailer = require('nodemailer');
// let transporter = nodemailer.createTransport({
//     service: 'qq',
//     port: 465, // SMTP 端口
//     secureConnection: true, // 使用 SSL
//     auth: {
//         user: 'ganwenyaozerg@foxmail.com',
//         //设置的smtp密码
//         pass: 'aqnkgtkcxtnqbdcc'
//     }
// });

// // setup email data with unicode symbols
// let mailOptions = {
//     from: '智能系统设计37组<ganwenyaozerg@foxmail.com>', // 发件地址
//     to: '807105461@qq.com', // 收件列表
//     subject: '账户登录失败提醒', // 标题
//     //text和html两者只支持一种
//     // text: '账户登录失败提醒', // 标题
//     html: `<h1>有人使用您的账户登录失败!</h1>
//            <h1>您的账户信息可能已经泄露!</h1>
//            <h1>下图为相关信息!</h1>
//            <p><img src="cid:00000001"/></p>`, // html 内容
//     attachments: 
//     [
//         {
//             filename: 'error.jpg',            // 改成你的附件名
//             path: './error.jpg',  // 改成你的附件路径
//             cid : '00000001'                 // cid可被邮件使用
//         }
//     ]
// };
// transporter.sendMail(mailOptions, function(error, info){
//     if(error){
//         return console.log(error);
//     }
//     console.log('Message sent: ' + info.response);
// });

