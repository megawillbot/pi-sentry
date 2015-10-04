var opencv = require('opencv');
var bg = new opencv.BackgroundSubtractor();
opencv.readImage("./test.jpg", function(err, im) {
  im.save("out.jpg");
});
