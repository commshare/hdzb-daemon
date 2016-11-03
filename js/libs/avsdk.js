'use strict'
const spawn = require('child_process').spawn;
const Packet = require('./packet.js');
const path = require('path');

var avsdk = {};


// @param param{sdk_app_id, account_type: 107}
avsdk.Init = function(param) {
  let _param = {
    sdk_app_id: 1104620500,
    account_type: '107',
    app_id_at3rd: '1104620500',
    identifier: 'xsy357864',
    user_sig: '123'
  }

  if (param)
    avsdk._param = param
  else
    avsdk._param = _param;

  // avsdk._child = spawn('../../bin/release/QAVSDKDemo.exe', [], {
  // "D:\\02 Git\\hdzb\\hdzb-server\\release\\hdzb-server.exe"
  avsdk._child = spawn(path.join(__dirname, '../../release/hdzb-server'), [], {

    stdio: [null, null, null,
      'pipe', 'pipe', // signal
      'pipe', // local camera
      'pipe', 'pipe', 'pipe', 'pipe' // remote streams
    ]
  });

  avsdk.send({
    type: 'Init',
    param: avsdk._param
  });
}

avsdk.Login = function() {
  avsdk.send({
    type: 'Login'
  })
};

avsdk.Logout = function() {
  avsdk.send({
    type: 'Logout'
  })
}

avsdk.EnterRoom = function(roomId) {
  avsdk.send({
    type: 'EnterRoom',
    param: roomId
  });
}

avsdk.ExitRoom = function() {
  avsdk.send({
    type: 'ExitRoom'
  });
}

avsdk.OpenCamera = function() {
  avsdk.send({
    type: 'OpenCamera'
  });
}
avsdk.CloseCamera = function() {
  avsdk.send({
    type: 'CloseCamera'
  });
}

avsdk.OpenMic = function() {
  avsdk.send({
    type: 'OpenMic'
  });
}
avsdk.CloseMic = function() {
  avsdk.send({
    type: 'CloseMic'
  });
}

// avsdk.AddVolume
// avsdk.SubVolume
// avsdk.GetVolume


avsdk.OpenSpeaker = function() {
  avsdk.send({
    type: 'OpenSpeaker'
  });
}

avsdk.CloseSpeaker = function() {
  avsdk.send({
    type: 'CloseSpeaker'
  });
}

// 获取当前房间人员列表
// avsdk.GetEndpointList = function() {
//   avsdk.send({
//     type: 'GetEndpointList'
//   });
// }

// @param identifiers  邀请用户列表(房间内)
avsdk.RequestViewList = function(identifiers) {
  avsdk.send({
    type: 'RequestView',
    param: identifiers
  });
}


avsdk.send = function(obj) {
  try {
    let str = JSON.stringify(obj);
    avsdk._child.stdio[3].write(str);
  } catch (e) {
    console.log(e);
  }
}


avsdk.OnMessage = function(func) {
  avsdk._child.stdio[4].on('data', function(data) {
    func(data);
  });
}

// 接收事件

// 本地视频
// var canvas = document.getElementById("canvas");
avsdk.StartRenderCameraView = function(canvas) {
  // avsdk._canvas = canvas;
  pipe_to_canvas(avsdk._child.stdio[5], canvas);
}

// 远端视频
// @param id  房间成员对应的 id 号
// @param canvas  html canvas 元素。e.g. var canvas = document.getElementById("canvas");
avsdk.StartRenderRemoteView = function(id, canvas) {
  // avsdk._canvas = canvas;
  pipe_to_canvas(avsdk._child.stdio[id], canvas);
}



function pipe_to_canvas(pipe, canvas) {
  var renderContext;
  if (canvas)
    renderContext = require("webgl-video-renderer").setupCanvas(canvas);
  var p = new Packet;

  // var io = pipe; //avsdk._child.stdio[5];
  try {
    pipe.setEncoding(null);
    delete pipe._readableState.decoder;
  } catch (e) {
    console.log(e);
  }

  pipe.on('data', (data) => {
    var d = data;
    while (d) {
      var offset = p.parse(data);
      if (offset === d.length) {
        d = null;
      } else {
        d = d.slice(offset, d.length);
      }

      if (p.isFull()) {
        var header = p.getHeader();
        var body = p.getBody();
        var w = header.width;
        var h = header.height;
        var uOffset = w * h;
        var vOffset = uOffset + w * h / 4;
        if (renderContext)
          renderContext.render(body, w, h, uOffset, vOffset);
        // p.printHeader();
        p.reset();
      }
    }
  });
}

module.exports = avsdk;
