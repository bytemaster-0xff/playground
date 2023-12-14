var curbAccessToken;
var curbRefreshToken;

var clientId;
var clientSecret;

var locations;

var nextSendTime = 0;

var request = require('request');
var io = require('socket.io-client');
var version = require('./version');

function getCurbToken(userInfo, refreshTokenCb) {
    console.log("Logging in to Curb");
    clientId = userInfo.curb_client_id;
    clientSecret = userInfo.curb_client_secret;

    console.log(version);
    console.log(userInfo);

    request.post({
        url: 'https://energycurb.auth0.com/oauth/token',
        form:
            {
                grant_type: 'password',
                audience: 'app.energycurb.com/api',
                scope: 'offline_access',
                username: userInfo.username,
                password: userInfo.password,
                client_id: userInfo.curb_client_id,
                client_secret: userInfo.curb_client_secret
            }
    },
        function (err, res, body) {
            if (res && res.statusCode == 200) {
                console.log("Response: " + body);

                curbAccessToken = JSON.parse(body).access_token;
                curbRefreshToken = JSON.parse(body).refresh_token;

                console.log("Curb Access Token: " + curbAccessToken);
                console.log("Curb Refresh Token: " + curbRefreshToken);

                refreshTokenCb(curbRefreshToken);

                setInterval(function () { refreshToken(function () { }); }, 20 * 60 * 60 * 1000);

                getCurbLocations();
            }
            else {
                console.log("Something Went Wrong while submitting form data to Curb " + res.statusCode + ": " + body);
                if (err) throw err;
            }
        })

}


function refreshToken(refreshCompleteCb) {
    console.log("Refreshing Curb auth");

    request.post({
        url: 'https://energycurb.auth0.com/oauth/token',
        form:
            {
                grant_type: 'refresh_token',
                client_id: clientId,
                client_secret: clientSecret,
                refresh_token: curbRefreshToken
            }
    },
        function (err, res, body) {
            if (res && res.statusCode == 200) {
                //console.log("Response: " + body);
                curbAccessToken = JSON.parse(body).access_token;

                //console.log("Curb Access Token: " + curbAccessToken);
                refreshCompleteCb();
            }
            else {
                console.log("Something Went Wrong while getting refresh token " + res.statusCode + ": " + body);
                if (err) throw err;
            }
        })
}

function useCurbToken(token, id, secret, st) {
    curbRefreshToken = token;
    clientId = id;
    clientSecret = secret;

    refreshToken(getCurbLocations);
    setInterval(function () { refreshToken(function () { }); }, 20 * 60 * 60 * 1000);
}

function getCurbLocations() {
    console.log("Requesting Curb location info");

    request.get('https://app.energycurb.com/api/locations',
        function (error, response, body) {
            if (response && response.statusCode == 200) {
                console.log("Curb Location Info: " + body);
                locations = JSON.parse(body);

                getHistoricalUsage();
                setInterval(function () { getHistoricalUsage(); }, 5 * 60 * 1000);
                setTimeout(function () { connectToLiveData(); }, 10 * 1000);
            }
            else {
                console.log("Something went wrong getting location info");
                console.log(response.statusCode);
                console.log(error);
            }
        })
        .auth(null, null, true, curbAccessToken);
}

function postPowerConsumption(msg) {
    msg.msgid = 'power';
    msg.version = version.version.version;
    msg.timeStamp = new Date().toISOString();
    request.post({
        url: "http://lagovista.softwarelogistics.iothost.net:9000",
        json: msg
    });

    console.log(msg);
}

function connectToLiveData() {
    var socket = io('https://app.energycurb.com/api/circuit-data',
        {
            reconnect: true,
            transports: ['websocket']
        });

    socket.on('connect', function () {
        console.log("Connected to socket.io, authenticating");
        socket.emit('authenticate', { token: curbAccessToken }, function (data) { console.log("Auth Ack: " + data); });
    });
    socket.on('authorized', function () {
        console.log("Authorized for socket.io, suscribing to live data");
        socket.emit('subscribe', locations[0].id);
    });
    socket.on('connect_timeout', function (data) {
        json = JSON.stringify(data);
        console.log("Error Message: " + json);
    });
    socket.on('connect_error', function (data) {
        json = JSON.stringify(data);
        console.log("Error Message: " + json);
    });
    socket.on('data', function (data) {
        if(nextSendTime < Date.now()) {

        let mainWatts = 0;
        let acWatts = 0;

        for (let idx = 0; idx < data.circuits.length; ++idx) {
            var postData = {};

            if (data.circuits[idx].id === 'fb5b1626-a901-4f72-9218-6eb70ee8bd13') {
                postData = { deviceid: 'labac', watts: data.circuits[idx].w };
            }

            if (data.circuits[idx].id === 'd9e21f81-b446-4c5f-9757-e2d54c7daf28') {
                postData = { deviceid: 'poolPump', watts: data.circuits[idx].w };
            }

            if (data.circuits[idx].id === '21d49d68-4f3a-49e1-bc45-d939db6917b2') {
                postData = { deviceid: 'poolHeater', watts: data.circuits[idx].w };
            }

            if (data.circuits[idx].id === '92885469-e589-4fa4-b916-eaf5e9e4758e') {
                postData = { deviceid: 'lab', watts: data.circuits[idx].w };
            }

            if (data.circuits[idx].id === '5f25ad49-1f8c-4a3c-985b-f40f0d271531') {
                postData = { deviceid: 'kdwoffice', watts: data.circuits[idx].w };
            }

            if (data.circuits[idx].id === 'f42bc047-c32c-4d1e-8474-cd5c1c3013e3') {
                postData = { deviceid: 'wirecloset', watts: data.circuits[idx].w };
            }

            if (data.circuits[idx].id === '2029bd4a-b725-4efc-9226-c4dde4918881' ||
                data.circuits[idx].id === 'bf954fb8-d843-4e7b-aad8-1b621690717b') {
                acWatts += data.circuits[idx].w;
            }

            if (data.circuits[idx].main === true) {
                mainWatts += data.circuits[idx].w;
            }

            if (postData.deviceid) {
                postPowerConsumption(postData);
            }
        }

        var acPost = { deviceid: 'centeralac', watts: acWatts };
        postPowerConsumption(acPost);
        var mainPost = { deviceid: 'mainpower', watts: mainWatts };
        postPowerConsumption(mainPost);
        console.log("--------");

        nextSendTime = Date.now() + 5000;
        }
    });
    //socket.on('disconnect', connectToLiveData);
}

function getHistoricalUsage() {
    var url = "https://app.energycurb.com/api/historical/" + locations[0].id + "/1h/5m"

    //console.log("Getting historical data: " + url);

    request.get(url,
        function (error, response, body) {
            if (response && response.statusCode == 200) {
                console.log("Got historical data: " + body);
            }
            else {
                console.log("Something went wrong getting historical data");
                if (response) { console.log(response.statusCode); }
                if (error) { console.log(error); }
            }
        })
        .auth(null, null, true, curbAccessToken);
}

exports.connect = getCurbToken;
exports.reconnect = useCurbToken;
