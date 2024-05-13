/*
  GPS Module Interface with NodeMCU HTML WebServer Code 
  http:://www.electronicwings.com
*/

const char html_page[] PROGMEM = R"RawString(
<!DOCTYPE html>
<html>
  <style>
    body {font-family: sans-serif;}
    h1 {text-align: center; font-size: 30px;}
    p {text-align: center; color: #4CAF50; font-size: 40px;}
  </style>

<body>
  <h1>GPS Module with NodeMCU</h1><br>
  <p>Latitude in Decimal Degrees : <span id="latVal">0</span></p>
  <p>Longitude in Decimal Degrees : <span id="lngVal">0</span></p>

<script>
  setInterval(function() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        const text = this.responseText;
        const myArr = JSON.parse(text);
       document.getElementById("latVal").innerHTML = myArr[0];
       document.getElementById("lngVal").innerHTML = myArr[1];
      }
    };
    xhttp.open("GET", "readgps", true);
    xhttp.send();
  },50);
</script>
</body>
</html>
)RawString";
