var socket = new zsocket("127.0.0.1", 9090);
socket.connect();

for (var i = 0; i < 10; i++)
{
    socket.send("test " + i);
    var reply = socket.recv();
    println(reply);
}
