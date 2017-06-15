var express = require('express');
var router = express.Router();

var path = require('path');
const fs = require('fs');
var htmlspecialchars = require('htmlspecialchars');

// middleware that is specific to this router
router.use(function (req, res, next) {
  next();
});

router.get('/', function(req, res) {
  res.render('pages/index');
});

router.get('/forbidden', function(req, res) {
  res.render('pages/error', { status: 403, message: 'Forbidden' });
});

exports = module.exports = router;