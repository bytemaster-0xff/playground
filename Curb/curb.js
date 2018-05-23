var http = require('http');
var fs = require('fs');
var path = require('path');
var qs = require('querystring');
var url = require('url');

var curb = new require('./curbauth');

var persistentFileName = "CurbBridgeData.json"
var persistentState = 
{
    curbClientId:"",
    curbClientSecret:"",
    curbRefreshToken:""
};


function savePersistentState()
{
    var state = JSON.stringify(persistentState);
    fs.writeFile(persistentFileName, state, function(err){if(err) return console.error(err);})
}

/*
var server = http.createServer(function (request, response)
{
    if(request.method == "GET")
    {
        console.log("Request Received");

        console.log("Curb onConnect");
*/
const userInfo = {};
userInfo.username = ********;
userInfo.password = ********;
userInfo.curb_client_id = 'R7LHLp5rRr6ktb9hhXfMaILsjwmIinKa'
userInfo.curb_client_secret = 'pcxoDsqCN7o_ny5KmEKJ2ci0gL5qqOSfxnzF6JIvwsfRsUVXFdD-DUc40kkhHAZR'

persistentState.curbClientId = userInfo.curb_client_id;
persistentState.curbClientSecret = userInfo.curb_client_secret;
savePersistentState();

var saveCurbToken = function(refreshToken)
{
    console.log("Saving curb token");
    persistentState.curbRefreshToken = refreshToken;
    savePersistentState();
}

curb.connect(userInfo, saveCurbToken);

 /*        response.writeHead(200, {"Content-Type": "text/plain"});
        response.write("Got it");
        response.end();
   }
});*/

//server.listen(8000);
//console.log("Server running at http://127.0.0.1:8000/");
