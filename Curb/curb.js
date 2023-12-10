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
userInfo.username = '***';
userInfo.password = '***';
userInfo.curb_client_id = 'iKAoRkr3qyFSnJSr3bodZRZZ6Hm3GqC3'
userInfo.curb_client_secret = 'dSoqbfwujF72a1DhwmjnqP4VAiBTqFt3WLLUtnpGDmCf6_CMQms3WEy8DxCQR3KY'

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

/*
curl POST https://energycurb.auth0.com/oauth/token  -H 'Cache-Control: no-cache'  -H 'Content-Type: application/json' -d '{"grant_type": "password","audience": "app.energycurb.com/api","username": "kevinw@software-logistics.com","client_id": "iKAoRkr3qyFSnJSr3bodZRZZ6Hm3GqC3","client_secret": "dSoqbfwujF72a1DhwmjnqP4VAiBTqFt3WLLUtnpGDmCf6_CMQms3WEy8DxCQR3KY"}'

"eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCIsImtpZCI6IjJ5YVdyWmpDWC1ndEE1QWxwSWxVeCJ9.eyJodHRwczovL2VuZXJneWN1cmIuY29tL3ZlbmRvciI6ImV4YW1wbGUtdmVuZG9yIiwiaXNzIjoiaHR0cHM6Ly9lbmVyZ3ljdXJiLmF1dGgwLmNvbS8iLCJzdWIiOiJhdXRoMHw1OTAwOTQwY2RkYjZhNDBmMjc0YjY1MGUiLCJhdWQiOiJhcHAuZW5lcmd5Y3VyYi5jb20vYXBpIiwiaWF0IjoxNzAxNzI1MDEzLCJleHAiOjE3MDE4MTE0MTMsImF6cCI6ImlLQW9Sa3IzcXlGU25KU3IzYm9kWlJaWjZIbTNHcUMzIiwic2NvcGUiOiJjb3N0X2NhbGN1bGF0aW9uIiwiZ3R5IjoicGFzc3dvcmQifQ.q5vK_FueV38GNcmpYD7brgRu2N30OfA-X_SKohX8yE5LS_E6PgHLNx3Ki31yTCieMXmgwLagCoLMspMX9dBg0HB0zkBHQF3PelqG_KdyUKYraiOM2edX11SL3d22HOVe4LllF9j6-GmAnuS7oLwvldVxB5hQExQBFXfAcH2VvkRgr4YE0x89g4_tA-sK6k4ytmhwIl5udDhMavx_BKWn3oT_WZl271bAQPA2XH8dLMalM7TdCjn5UhdtS4trOZB0BcsnGSQt7z9HsUcQGpNXc2EKkyQ9wSXLKrwVMxwjdp_jaHXstSZ4DUrnouyPNXLN_wg_whkVNgeBOh9zQnTLtw"
*/

