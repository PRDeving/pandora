// const fs = require('fs');
var net = require('net');

var TCPServer = net.createServer(function(socket) {

  socket.write('node tcp socket reporting\0');

  socket.write('hello\0');

  setTimeout(function() {
    socket.write('bitch\0');
  },400);

  setTimeout(function() {
    socket.write('patata\0');
  },1300);

  setTimeout(function() {
    socket.write('hello\0');
  },2000);

  setTimeout(function() {
    socket.write('checking\0');
  },2300);

  setTimeout(function() {
    socket.write('stop\0');
  },3000);
    // socket.pipe(socket);
    TCPServer.close();
});
TCPServer.listen(1337, '127.0.0.1');
