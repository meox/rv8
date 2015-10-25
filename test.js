"use strict";

function getRandomInt(min, max) {
    return Math.floor(Math.random() * (max - min)) + min;
}


function getRandomString(n){
    var s = "";
    for (let i = 0; i < n; i++) {
        s += getRandomInt(0, 10);
    }
    return s;
}

var actual_time = function() {
    var filler = function (n) {
        if (n < 10)
            return "0" + n;
        else
            return "" + n;
    };

    var d = new Date();

    return "" + d.getFullYear()
              + filler(d.getMonth()+1)
              + filler(d.getDate())
              + "_"
              + filler(d.getHours())
              + filler(d.getMinutes());
}


println("Hello World!", ((a,b) => a + b)(1, 41), [1, 2, "yuppi!"], actual_time());

var socket = new zsocket("*", 9090);
socket.bind();

for (var i = 0; i < 10; i++)
{
    var msg = socket.recv();
    println("Received: " + msg);
    socket.send("R: " + msg);
}


function exit_callback(msg)
{
    println(msg);
}


exit(0);
