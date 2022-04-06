var state = "Go";
  var powerStoredState = false;
  var powerOutputState = false;
  var humidState = false;
  var rainState = false;
  var tempState = false;
  var daylightState = false;
  var rainToggleState = false;

  window.addEventListener('load', onLoad);
  function onLoad(event) {
    // Add some initializing sensor data values function


    initButton();
  }
  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
    document.getElementById('intervalSet').addEventListener('click', toggle);
    document.getElementById('refresh').addEventListener('click', toggle);
    document.getElementById('powerStoredInfo').addEventListener('click', toggle);
    document.getElementById('powerOutputInfo').addEventListener('click', toggle);
    document.getElementById('humidInfo').addEventListener('click', toggle);
    document.getElementById('rainInfo').addEventListener('click', toggle);
    document.getElementById('tempInfo').addEventListener('click', toggle);
    document.getElementById('daylightInfo').addEventListener('click', toggle);
    document.getElementById('rainToggle').addEventListener('click', toggle);
  }
  function toggle(){
    if (this.value == 'button') {
      if (state == "Go") {
        // Cleaning is now in progress, update firebase


        state = "Cleaning...";
        document.getElementById('state').innerHTML = state;

        this.style.background = '#8c8c8c';
        // Disables button so you can't press it again
        //this.disabled="disabled";

        // Loading Gif
        document.getElementById('loading').style.visibility = 'visible';
        document.getElementById('loading').style.width = '500px';
        document.getElementById('loading').style.height = '50px';
      }
      else {
        // Cleaning is over, retrieve update from firebase


        state = "Go";
        document.getElementById('state').innerHTML = state;

        this.style.background = '#0f8b8d';
        // Re enables button to press again
        this.enabled = "enabled";

        // Loading Gif
        document.getElementById('loading').style.visibility = 'hidden';
        document.getElementById('loading').style.width = '0px';
        document.getElementById('loading').style.height = '0px';
      }
    }
    else if (this.value == 'powerStored') {
      if (!powerStoredState) {
        powerStoredState = true;
          document.getElementById('powerStoredDesc').style.visibility = 'visible';
          document.getElementById('powerStoredDesc').style.fontSize = 'medium';
      }
      else {
        powerStoredState = false;
        document.getElementById('powerStoredDesc').style.visibility = 'hidden';
        document.getElementById('powerStoredDesc').style.fontSize = 'xx-small';
      }
    }
    else if (this.value == 'powerOutput') {
      if (!powerOutputState) {
        powerStoredState = true;
          document.getElementById('powerOutputDesc').style.visibility = 'visible';
          document.getElementById('powerOutputDesc').style.fontSize = 'medium';
      }
      else {
        powerOutputState = false;
        document.getElementById('powerOutputDesc').style.visibility = 'hidden';
        document.getElementById('powerOutputDesc').style.fontSize = 'xx-small';
      }
    }
    else if (this.value == 'humid') {
      if (!humidState) {
          humidState = true;
          document.getElementById('humidDesc').style.visibility = 'visible';
          document.getElementById('humidDesc').style.fontSize = 'medium';
      }
      else {
        humidState = false;
        document.getElementById('humidDesc').style.visibility = 'hidden';
        document.getElementById('humidDesc').style.fontSize = 'xx-small';
      }
    }
    else if (this.value == 'rain') {
      if (!rainState) {
          rainState = true;
          document.getElementById('rainDesc').style.visibility = 'visible';
          document.getElementById('rainDesc').style.fontSize = 'medium';
      }
      else {
        rainState = false;
        document.getElementById('rainDesc').style.visibility = 'hidden';
        document.getElementById('rainDesc').style.fontSize = 'xx-small';
      }
    }
    else if (this.value == 'temp') {
      if (!tempState) {
          tempState = true;
          document.getElementById('tempDesc').style.visibility = 'visible';
          document.getElementById('tempDesc').style.fontSize = 'medium';
      }
      else {
        tempState = false;
        document.getElementById('tempDesc').style.visibility = 'hidden';
        document.getElementById('tempDesc').style.fontSize = 'xx-small';
      }
    }
    else if (this.value == 'daylight') {
      if (!daylightState) {
          daylightState = true;
          document.getElementById('daylightDesc').style.visibility = 'visible';
          document.getElementById('daylightDesc').style.fontSize = 'medium';

      }
      else {
        daylightState = false;
        document.getElementById('daylightDesc').style.visibility = 'hidden';
        document.getElementById('daylightDesc').style.fontSize = 'xx-small';
      }
    }
    else if (this.value == 'interval') {
      // setting interval and checking for bad values
      // checking for 0 0 0
      var day = document.getElementById('Days').value;
      var hour = document.getElementById('Hours').value;
      var min = document.getElementById('Minutes').value;
      if ((day == "0" && hour == "0" && min == "0") || day == "Days" || hour == "Hours" || min == "Minutes") {
        alert("Please enter valid time interval");
      }
      else {
        // Add logic to calculate months days etc.
        document.getElementById('lastCleaning').innerHTML = "00 / 00 / 0000 at 00 : 00";
        document.getElementById('nextCleaning').innerHTML = "00 / 00 / 0000 at 00 : 00";
      }
    }
    else if (this.value == 'refresh') {
      // retrieve the respective data from firebase

      document.getElementById('daylight').innerHTML = "0";
      document.getElementById('temp').innerHTML = "0";
      document.getElementById('rain').innerHTML = "0";
      document.getElementById('humid').innerHTML = "0";
      document.getElementById('powerStored').innerHTML = "0";
      document.getElementById('powerOutput').innerHTML = "0";

      // update refresh time
      document.getElementById('refreshTime').innerHTML = "00/00/0000 at 00:00";
    }
    else if (this.value == 'rainToggle') {
      if (!rainToggleState) {
        // auto-clean toggle on, update in firebase


        rainToggleState = true;
        document.getElementById('rainToggleImg').src = 'src/toggleOn.png';
        document.getElementById('rainTog').innerHTML = 'On';
      }
      else {
        // auto-clean toggle off, update in firebase


        rainToggleState = false;
        document.getElementById('rainToggleImg').src = 'src/toggleOff.png';
        document.getElementById('rainTog').innerHTML = 'Off';
      }
    }
  }