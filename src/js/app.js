var xhrRequest = function(url, type, callback){
  var xhr = new XMLHttpRequest();
  xhr.onload = function(){
    callback(this.responseText);
  };
  
  xhr.open(type, url);
  xhr.send();
};

function getMetar(){
  //pull newest metar/speci
  //take IATA as arg
  var url = 'http://avwx.rest/api/metar.php?station=CYKF&format=JSON';
  xhrRequest(url, 'GET', 
    function(responseText){
      var json = JSON.parse(responseText);
    
      var metarData = json["Raw-Report"];
      console.log(metarData);
      
      var dictionary = {
        'METAR_KEY': metarData
      };
      
      Pebble.sendAppMessage(dictionary,
        function(e){
          console.log('METAR info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending METAR info to Pebble!');
        }
      );
    }
  );
  
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    getMetar();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getMetar();
    
  }                     
);