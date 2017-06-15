var express = require('express');
var router = express.Router();

var path = require('path');
const fs = require('fs');
var htmlspecialchars = require('htmlspecialchars');

var mysql = require('mysql');
var connection = mysql.createConnection({
  host: 'localhost',
  port: 3306,
  user: 'root',
  password: 'dbswnsgk',
  database: 'imagehouse'
});

connection.connect(function(err) {
  if (err) {
    console.error('mysql connection error');
    console.error(err);
    throw err;
  }
});

// Where fileName is name of the file and response is Node.js Reponse. 
function responseFile(fileName, response) {
  var projectRoot = path.join(__dirname, '../../server');
  var imageDir = path.join(projectRoot, 'images');
  var filePath = path.join(imageDir, fileName);

  // Check if file specified by the filePath exists 
  fs.exists(filePath, function(exists){
    if (exists) {     
      // Content-type is very interesting part that guarantee that
      // Web browser will handle response in an appropriate manner.
      response.writeHead(200, {
        "Content-Type": "image/png"
    	});
      fs.createReadStream(filePath).pipe(response);
    } else {
      response.redirect('/not-found');
    }
  });
}

// middleware that is specific to this router
router.use(function (req, res, next) {
  next();
});

// get list of images
router.get('/', function(req, res) {
  var query = connection.query(
    'SELECT id, name, extension, upload_at FROM images',
    function(err, rows){
      res.json(rows);
    }
  );
});

router.get('/i/:imageName', function(req, res) {
  var query = connection.query(
  	'SELECT * FROM images WHERE name = ? LIMIT 1', req.params.imageName,
  	function(err, rows){
  		if( rows.length > 0 ){
  			responseFile(rows[0].realpath, res);
  		} else {
  			res.send('not-found');
  		}
	  }
	);
});

router.get('/forbidden', function(req, res) {
  res.render('pages/error', { status: 403, message: 'Forbidden' });
});

exports = module.exports = router;