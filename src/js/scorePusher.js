

var xhrRequest = function (url, type, scoreToPush, callback) 
{
  var xhr = new XMLHttpRequest();
  var tokenToPush = Pebble.getWatchToken();
  
  var json = JSON.stringify(
    {
      type: "score",
      username: tokenToPush,
      score : scoreToPush
    }
  );
  console.log("Json to validate:");
  console.log(json);
  
  xhr.addEventListener("error", function(evt)
                       {
                         console.log("something went wrong");
                         console.log(evt.payload);
                       });
  
  xhr.onload = function () 
  {
    callback(this.responseText);
  };
  xhr.open(type, url,true);
  xhr.setRequestHeader("Content-Type", "application/json");
  xhr.send(json);
};

var xhrRequestForUsername = function (url, type, callback) 
{
  var xhr = new XMLHttpRequest();
  var tokenToPush = Pebble.getWatchToken();
  var json = JSON.stringify(
    {
      type: "username",
      username: tokenToPush
    }
  );
  console.log("Json to validate:");
  console.log(json);
  
  xhr.addEventListener("error", function(evt)
                       {
                         console.log("something went wrong");
                       });
  
  xhr.onload = function () 
  {
    callback(this.responseText);
  };
  xhr.open(type, url,true);
  xhr.setRequestHeader("Content-Type", "application/json");
  xhr.send(json);
};

function sendScore(score) 
{
  var scoreToPush = score.KEY_SCORE;
  console.log("score: ");
  console.log(scoreToPush);
  
  if (typeof scoreToPush === undefined || scoreToPush === null)
  {
    console.log("undefined score!");
    return;
  }
  
  // Construct URL
  //var url = "http://54.194.206.38:80/json";
  var url = "http://54.194.206.38:80/jsonFire";
  
  // Push request to url
  xhrRequest(url, 'POST', scoreToPush,
    function(responseText) 
    {
      console.log("Response from server:");
      console.log(responseText);
    }      
  );
}
function getUserName()
{
  //var url = "http://54.194.206.38:80/json";
  var url = "http://54.194.206.38:80/jsonFire"; 
  // Push request to url
  xhrRequestForUsername(url, 'POST',
    function(responseText) 
    {
      console.log("Response from server:");
      console.log(responseText);
      var dict = {
      'KEY_FRIENDLY':responseText
      };
      
      // Send a string to Pebble
      Pebble.sendAppMessage(dict, function(e) 
      {
        console.log('Send name to c successful.');
      }, function(e) 
      {
        console.log('Send name to c failed!');
      });  
      
      
    }      
  );
}


// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");
    console.log("Getting user name");
    getUserName();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received! Score ready to submit");
    console.log(JSON.stringify(e.payload));
    sendScore(e.payload);
  }                     
);
