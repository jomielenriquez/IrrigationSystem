const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP SMS AND WEATHER</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP SMS and Weather</h2>
  <div id="div_reading1"></div>
  </br>
  <div id="div_reading2"></div>
  </br>
  <div id="div_Ave"></div>
  </br>
  <div id="div_IsWater"></div>
  </br>
  <div id="div_Day"></div>
  </br>
  <div id="div_Date"></div>
  </br>
  <div id="div_Weather"></div>
  </br>
  <div id="div_SdStatus"></div>
  </br>
  <div id="div_Water"></div>
  </br>
  <button onclick="downloadSMS()">Download</button>
<script>

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var y = this.responseText;
      const Data = y.split(",");
      document.getElementById("div_reading1").innerHTML=Data[0];
      document.getElementById("div_reading2").innerHTML=Data[1];
      document.getElementById("div_Ave").innerHTML=Data[2];
      document.getElementById("div_IsWater").innerHTML=Data[3];
      document.getElementById("div_Day").innerHTML=Data[7];
      document.getElementById("div_Date").innerHTML=Data[8];
      document.getElementById("div_Weather").innerHTML=Data[10];
      document.getElementById("div_SdStatus").innerHTML=Data[9];
      const Water = Data[6].split(" ");
      console.log(Water);
      var int_water = 30-Water[2];
      if(int_water <=15){
        var liter = (79.796453401 * (int_water));
        if(liter>0){
          if(liter>1000){
            liter=liter/1000;
            document.getElementById("div_Water").innerHTML="water volume is now " + liter.toFixed(2) + " L";
          }
          else{
            document.getElementById("div_Water").innerHTML="water volume is now " + liter.toFixed(2) + " ml";
          }
          
        }        
      }
      else{
        document.getElementById("div_Water").innerHTML="";
      }
    }
  };
  xhttp.open("GET", "/Reading", true);
  xhttp.send();
}, 1000 );

function downloadSMS(){
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    console.log("Downloading");
  };
  xhttp.open("GET", "/downloadSMS", true);
  xhttp.send();
}

</script>
</body>
</html>
)rawliteral";
