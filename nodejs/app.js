var http = require('http');
var ip = require('ip');
var path = require('path');

var session = require('express-session')(
  { secret: 'jbnu-keyboard-cat', resave: false, saveUninitialized: false }
);

var express = require('express');
var exphbs  = require('express-handlebars');
var hbs = exphbs.create({
  defaultLayout: 'main'
});
var app = express();
var server = http.createServer(app);

var bodyParser = require('body-parser');
var cookieParser = require('cookie-parser');
var multer = require('multer'); // v1.0.5
var upload = multer(); // for parsing multipart/form-data
var flash = require('connect-flash');

app.use(bodyParser.json()); // for parsing application/json
app.use(bodyParser.urlencoded({ extended: true })); // for parsing application/x-www-form-urlencoded
app.use(cookieParser());
app.use(session);
app.use(flash());
app.engine('handlebars', hbs.engine);
app.set('view engine', 'handlebars');

app.use('/', require('./routes'));
app.use('/images', require('./routes/images'));
app.use(express.static(path.resolve(__dirname, 'client')));

app.use(require('./routes/error'));

// Listen Server
server.listen(process.env.PORT || 80, process.env.IP || ip.address(), function(){
  var addr = server.address();
  console.log("Chat server listening at", addr.address + ":" + addr.port);
});
