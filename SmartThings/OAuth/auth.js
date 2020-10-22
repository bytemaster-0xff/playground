/// <reference path="typings/node/node.d.ts"/>
/*
 OAUTH Client Example
 needs 
 npm install express
 npm install request
 npm install JSON
*/


// expected commandline node.js script CLIENT_ID CLIENT_SECRET
if (process.argv.length != 4) {
  console.log("usage: " + process.argv[0] + " " + process.argv[1] + " CLIENT_ID CLIENT_SECRET");
  process.exit();
}

var CLIENT_ID = process.argv[2];
var CLIENT_SECRET = process.argv[3];

var request = require('request');
var express = require('express'),
  app = express();

var endpoints_uri = 'https://graph.api.smartthings.com';

const credentials = {
  client: {
    id: CLIENT_ID,
    secret: CLIENT_SECRET
  },
  auth: {
    tokenHost: endpoints_uri,
    tokenPath: "/oauth/token",
	authorizePath: "/oauth/authorize",
  }
};

var oauth2 = require('simple-oauth2').create(credentials);
 
// Authorization uri definition 
var authorization_uri = oauth2.authorizationCode.authorizeURL({
  redirect_uri: 'http://localhost:3000/callback',
  scope: 'app',
  state: '3(#0/!~'
});
 
// Initial page redirecting to Github 
app.get('/auth', function (req, res) {
  res.redirect(authorization_uri);
});
 
// Callback service parsing the authorization token and asking for the access token 
app.get('/callback', async function (req, res) {
  var code = req.query.code;
  console.log('/callback got code' + code);

  const tokenConfig = {
	code: code,
	redirect_uri: 'http://localhost:3000/callback',
	scope: 'app', // also can be an array of multiple scopes, ex. ['<scope1>, '<scope2>', '...']
  };

  try {
	  const result = await oauth2.authorizationCode.getToken(tokenConfig)
	  const accessToken = oauth2.accessToken.create(result);
		console.log("Got Access Token");
		console.log(accessToken);
   } catch (error) {
	  console.log('Access Token Error', error.message);
	}
}, function(err) {
	console.log(err);
});

app.get('/', function (req, res) {
  res.send('<a href="/auth">Connect with SmartThings</a>');
});

app.listen(3000);

console.log('Express server started on port 3000');
